#include "MCEventBuilder.h"
#include <ROOT/RRangeCast.hxx>
#include <numeric>
#include <TClonesArray.h>           
#include <TGenericClassInfo.h>      
#include <TMath.h>                  
#include <TRandom.h>               
#include <TFile.h>
#include <TROOT.h>
#include <iostream>                
#include <algorithm>              
#include <vector>      
#include <TString.h>            
#include "FairMCEventHeader.h"
#include "MuFilter.h"
#include "Scifi.h"
#include "FairLink.h"              
#include "FairRunSim.h"            
#include "FairRunAna.h"            
#include "FairRootManager.h"
#include "MuFilterPoint.h"
#include "ScifiPoint.h"
#include "ShipMCTrack.h"
#include "ShipUnit.h"

namespace {
  int n_clockcycles = 4;

  float t_electronics = 0.0; //if known
  float timeWindow = n_clockcycles * (1 /(ShipUnit::snd_freq)) + t_electronics;

  MuFilter* MuFilterDet = nullptr;
  float DsPropSpeed = 0.0;
  float VandUpPropSpeed = 0.0;

  Scifi* ScifiDet = nullptr;
  float ScifisignalSpeed = 0.0;
  std::map<Int_t, std::map<Int_t, std::array<float, 2>>> siPMFibres;

  //Scifi Fast & Advanced Noise Filter Params
  int min_scifi_boards = 3;
  int min_us_boards = 5;
  int min_ds_boards = 2;

  int min_veto_hits = 5;
  int min_scifi_hits = 10;
  int min_ds_hits =2;

  int min_veto_planes = 1;
  int min_scifi_planes = 4;
  int min_us_planes = 2;
  int min_ds_planes = 2;
}

MCEventBuilder::MCEventBuilder(const std::string& outputFileName,
                               bool saveFirst25nsOnly)
  : FairTask("MCEventBuilder"),
    fOutputFileName(outputFileName),
    fSaveFirst25nsOnly(saveFirst25nsOnly),
    fOutFile(nullptr),
    fOutTree(nullptr),
    fInMufiArray(nullptr),
    fInSciFiArray(nullptr),
    fInMCTrackArray(nullptr),
    fInHeader(nullptr),
    fOutMufiArray(nullptr),
    fOutSciFiArray(nullptr),
    fOutMCTrackArray(nullptr),
    fOutHeader(nullptr)
{}

MCEventBuilder::~MCEventBuilder() {}

InitStatus MCEventBuilder::Init() {
  LOG(INFO) << "Initializing MCEventBuilder";

  FairRootManager* ioman = FairRootManager::Instance();
  if (!ioman) {
    LOG(FATAL) << "MCEventBuilder::Init: RootManager not instantiated!";
  }

  // —---------- INPUT BRANCHES —-----------
  fInHeader  = static_cast<FairMCEventHeader*>(ioman->GetObject("MCEventHeader."));
  fInMufiArray  = static_cast<TClonesArray*>(ioman->GetObject("MuFilterPoint"));
  fInSciFiArray = static_cast<TClonesArray*>(ioman->GetObject("ScifiPoint"));
  fInMCTrackArray  = static_cast<TClonesArray*>(ioman->GetObject("MCTrack"));

  if (!fInMufiArray && !fInSciFiArray) {
    LOG(ERROR) << "No Scifi and no MuFilter MC points array!";
    return kERROR;
  }

  // —---------- OUTPUT FILE & TREE —----------
  fOutFile = TFile::Open(fOutputFileName.c_str(), "RECREATE");
  fOutTree = new TTree("cbmsim", "RebuiltEvents");

  fOutHeader   = new FairMCEventHeader();
  fOutMufiArray  = new TClonesArray("MuFilterPoint");
  fOutSciFiArray = new TClonesArray("ScifiPoint");
  fOutMCTrackArray  = new TClonesArray("ShipMCTrack");
  
  fOutTree->Branch("MuFilterPoint",  &fOutMufiArray,  32000, 1);
  fOutTree->Branch("ScifiPoint",     &fOutSciFiArray, 32000, 1);
  fOutTree->Branch("MCTrack",        &fOutMCTrackArray,  32000, 1);
  fOutTree->Branch("MCEventHeader.", &fOutHeader,   32000, 1);


  // --------------Global Variables--------------------
  //Muon Filter
  MuFilterDet = dynamic_cast<MuFilter*>(gROOT->GetListOfGlobals()->FindObject("MuFilter"));
  if (!MuFilterDet) {
    LOG(ERROR) << "MuFilter detector not found in gROOT";
    return kERROR;
  }
  DsPropSpeed = MuFilterDet->GetConfParF("MuFilter/DsPropSpeed");
  VandUpPropSpeed = MuFilterDet->GetConfParF("MuFilter/VandUpPropSpeed");

  //Scifi
  ScifiDet = dynamic_cast<Scifi*>(gROOT->GetListOfGlobals()->FindObject("Scifi"));
  if (!ScifiDet) {
    LOG(ERROR) << "Scifi detector not found in gROOT";
    return kERROR;
  }
  ScifisignalSpeed = ScifiDet->GetConfParF("Scifi/signalSpeed");
  ScifiDet->SiPMmapping();
  siPMFibres = ScifiDet->GetFibresMap();

  LOG(INFO) << "MCEventBuilder initialized successfully.";
  return kSUCCESS;
}

void MCEventBuilder::Exec(Option_t*) {
  ProcessEvent();
}

std::vector<int> MCEventBuilder::OrderedIds(const std::vector<double>& times,
                                            double firstTime) const {
  size_t n = times.size();
  std::vector<long long> bins(n);
  for (size_t i = 0; i < n; ++i) {
        bins[i] = static_cast<long long>((times[i] - firstTime) / timeWindow);
    }

  std::vector<int> ids(n, 0);
  for (size_t i = 1; i < n; ++i) {
    if ((bins[i] - bins[i - 1]) < 1) {
        ids[i] = ids[i - 1];
    } else {
        ids[i] = ids[i - 1] + 1;
    }
  }
  return ids;
}

void MCEventBuilder::ProcessEvent() {
  *fOutHeader = *fInHeader;

  //---------------------------Muon filter-------------------------------------
  std::vector<MuFilterPoint*> muFilterPoints;
  std::vector<double> muArrivalTimes;
  std::vector<int> muTrackIDs;

  for (auto* p : ROOT::RRangeCast<MuFilterPoint*, false, decltype(*fInMufiArray)>(*fInMufiArray)) { 
    muFilterPoints.push_back(p); 
    muTrackIDs.push_back(p->GetTrackID());

    int detID = p->GetDetectorID();

    float propspeed;
    if (floor(detID / 10000) == 3)
      propspeed = DsPropSpeed;
    else
      propspeed = VandUpPropSpeed;

    TVector3 vLeft,vRight;
    TVector3 impact(p->GetX(), p->GetY(), p->GetZ());
    MuFilterDet->GetPosition(detID, vLeft, vRight);
    TVector3 vTop = vLeft; //Used for vertical bars

    //Vertical bars with only 1 readout at the top
    if ( (floor(detID/10000)==3&&detID%1000>59) || 
          (floor(detID/10000)==1&&int(detID/1000)%10==2) )  {
      double arrivalTime = p->GetTime() + (vTop - impact).Mag() / propspeed;
      muArrivalTimes.push_back(arrivalTime);
    }
    //Horizontal
    else{
      double tLeft  = p->GetTime() + (vLeft - impact).Mag()  / propspeed;
      double tRight = p->GetTime() + (vRight - impact).Mag() / propspeed;
      double arrivalTime = std::min(tLeft, tRight);
      muArrivalTimes.push_back(arrivalTime);
    }
  }

  std::vector<size_t> idxM(muArrivalTimes.size());
  std::iota(idxM.begin(), idxM.end(), 0);
  std::sort(idxM.begin(), idxM.end(), [&](size_t a, size_t b) {
    return muArrivalTimes[a] < muArrivalTimes[b];
  });

  std::vector<MuFilterPoint*> sortedMuPoints;
  std::vector<double> sortedMuArrivalTimes;
  std::vector<int> sortedMuTrackIDs;

  sortedMuPoints.reserve(muFilterPoints.size());
  sortedMuArrivalTimes.reserve(muArrivalTimes.size());
  sortedMuTrackIDs.reserve(muTrackIDs.size());

  for (auto i : idxM) {
    sortedMuPoints.push_back(muFilterPoints[i]);
    sortedMuArrivalTimes.push_back(muArrivalTimes[i]);
    sortedMuTrackIDs.push_back(muTrackIDs[i]);
  }
  
  //---------------------------Scifi-------------------------------------
  std::vector<ScifiPoint*> scifiPoints;
  std::vector<double> scifiArrivalTimes;
  std::vector<int> scifiTrackIDs;

  float signalSpeed = ScifisignalSpeed;

  for (auto* p : ROOT::RRangeCast<ScifiPoint*, false, decltype(*fInSciFiArray)>(*fInSciFiArray)) { 
    scifiPoints.push_back(p); 
    scifiTrackIDs.push_back(p->GetTrackID());

    TVector3 impact(p->GetX(), p->GetY(), p->GetZ());
    int point_detID = p->GetDetectorID();
    int localFiberID = (point_detID)%100000;
    int a_sipmChan = static_cast<int>(siPMFibres[localFiberID].begin()->first);
    int detID_geo = int(point_detID/100000)*100000+a_sipmChan;

    TVector3 a, b;
    ScifiDet->GetSiPMPosition(detID_geo, a, b);
    bool verticalHit = int(detID_geo / 100000) % 10 == 1;
    double distance;
    if (verticalHit) {
      distance = (b - impact).Mag();
    } else {
      distance = (impact - a).Mag();
    }
    double arrivalTime = p->GetTime() + distance / signalSpeed;
    scifiArrivalTimes.push_back(arrivalTime);
  }

  std::vector<size_t> idxS(scifiArrivalTimes.size());
  std::iota(idxS.begin(), idxS.end(), 0);
  std::sort(idxS.begin(), idxS.end(), [&](size_t a, size_t b) {
    return scifiArrivalTimes[a] < scifiArrivalTimes[b];
  });

  std::vector<ScifiPoint*> sortedScifiPoints;
  std::vector<double> sortedScifiArrivalTimes;
  std::vector<int> sortedScifiTrackIDs;

  sortedScifiPoints.reserve(scifiPoints.size());
  sortedScifiArrivalTimes.reserve(scifiArrivalTimes.size());
  sortedScifiTrackIDs.reserve(scifiTrackIDs.size());

  for (auto i : idxS) {
    sortedScifiPoints.push_back(scifiPoints[i]);
    sortedScifiArrivalTimes.push_back(scifiArrivalTimes[i]);
    sortedScifiTrackIDs.push_back(scifiTrackIDs[i]);
  }

  //----------------------------------Tracks-------------------------------------
  std::vector<ShipMCTrack*> mcTrackClones;
  for (auto* t : ROOT::RRangeCast<ShipMCTrack*, false, decltype(*fInMCTrackArray)>(*fInMCTrackArray)) { 
    mcTrackClones.push_back(t); 
  }

  //---------Finding the earliest time between Scifi and MuFilter-----------------
  double tMufi  = sortedMuArrivalTimes.empty()  ? -1 : sortedMuArrivalTimes.front();
  double tScifi = sortedScifiArrivalTimes.empty() ? -1 : sortedScifiArrivalTimes.front();
  bool hasMCPoints   = (tMufi >= 0 || tScifi >= 0);
  double firstT = hasMCPoints ? (tMufi < 0 ? tScifi : (tScifi < 0 ? tMufi : std::min(tMufi, tScifi))): 0;

  if (!hasMCPoints) {
    fOutMufiArray->Clear();
    fOutSciFiArray->Clear();
    fOutMCTrackArray->Clear();
    fOutTree->Fill();
    return;
  }

  //------------------Preparations before chunking the data---------------------
  auto idsMufi  = OrderedIds(sortedMuArrivalTimes,  firstT);
  auto idsScifi = OrderedIds(sortedScifiArrivalTimes, firstT);

  bool FirstEvent = true;

  std::vector<int> used;  
  int i_mufi = 0, i_scifi = 0, sliceMufi = 0, sliceScifi = 0;

  while (i_mufi < (int)sortedMuArrivalTimes.size() || i_scifi < (int)sortedScifiArrivalTimes.size()) {
    fOutMufiArray->Clear("C");
    fOutSciFiArray->Clear("C");
    fOutMCTrackArray->Clear("C");

    std::vector<MuFilterPoint*> muSlicePoints;
    std::vector<ScifiPoint*> scifiSlicePoints;
    fOutMCTrackArray->Delete();  
    fOutMCTrackArray->ExpandCreate(mcTrackClones.size());

    //Adding the mother track:
    ShipMCTrack* newTrack = new ((*fOutMCTrackArray)[0]) ShipMCTrack(*mcTrackClones[0]);

    //MUON FILTER POINTS CHUNKING
    while (i_mufi < (int)sortedMuArrivalTimes.size() && idsMufi[i_mufi] == sliceMufi) {
      MuFilterPoint* origMu = sortedMuPoints[i_mufi];
      muSlicePoints.push_back(origMu);
      Int_t trackID   = origMu->GetTrackID();
      Int_t detID     = origMu->GetDetectorID();
      TVector3 pos;    origMu->Position(pos);
      TVector3 mom;    origMu->Momentum(mom);
      Double_t time   = origMu->GetTime();
      Double_t length = origMu->GetLength();
      Double_t eLoss  = origMu->GetEnergyLoss();
      Int_t pdgCode   = origMu->PdgCode();
      
      new ((*fOutMufiArray)[fOutMufiArray->GetEntriesFast()])
        MuFilterPoint(trackID, detID, pos, mom, time, length, eLoss, pdgCode);

      int tid = sortedMuTrackIDs[i_mufi++];
      //MC tracks having ID=-2 are below the pre-set energy threshold (default is 100MeV) and cannot be linked to their corresponding MC point. Such tracks are not saved to the chunked events.
      if (tid != -2) {
        ShipMCTrack* newTrack = new ((*fOutMCTrackArray)[tid]) ShipMCTrack(*mcTrackClones[tid]);  
      }
    }
    ++sliceMufi;

    //SCIFI POINTS CHUNKING
    while (i_scifi < (int)sortedScifiArrivalTimes.size() && idsScifi[i_scifi] == sliceScifi) {
      ScifiPoint* origSci = sortedScifiPoints[i_scifi];
      scifiSlicePoints.push_back(origSci);
      Int_t trackID   = origSci->GetTrackID();
      Int_t detID     = origSci->GetDetectorID();
      TVector3 pos;    origSci->Position(pos);
      TVector3 mom;    origSci->Momentum(mom);
      Double_t time   = origSci->GetTime();
      Double_t length = origSci->GetLength();
      Double_t eLoss  = origSci->GetEnergyLoss();
      Int_t pdgCode   = origSci->PdgCode();

      new ((*fOutSciFiArray)[fOutSciFiArray->GetEntriesFast()])
        ScifiPoint(trackID, detID, pos, mom, time, length, eLoss, pdgCode);

      int tid = sortedScifiTrackIDs[i_scifi++];
      //MC tracks having ID=-2 are below the pre-set energy threshold (default is 100MeV) and cannot be linked to their corresponding MC point. Such tracks are not saved to the chunked events.
      if (tid != -2) {
        ShipMCTrack* newTrack = new ((*fOutMCTrackArray)[tid]) ShipMCTrack(*mcTrackClones[tid]);
      }
    }
    ++sliceScifi; 

    //Noise Filters
    if (!(FastNoiseFilterScifi_Hits(fOutSciFiArray, siPMFibres) || FastNoiseFilterMu_Hits(fOutMufiArray))) {
      fOutMufiArray->Clear("C");
      fOutSciFiArray->Clear("C");
      fOutMCTrackArray->Clear("C");
      FirstEvent = false;
      continue;  
    }
    else if(!(FastNoiseFilterScifi_Boards(fOutSciFiArray, siPMFibres) || FastNoiseFilterMu_Boards(fOutMufiArray))){
      fOutMufiArray->Clear("C");
      fOutSciFiArray->Clear("C");
      fOutMCTrackArray->Clear("C");
      FirstEvent = false;
      continue;
    }

    if (!(AdvancedNoiseFilterScifi(fOutSciFiArray, siPMFibres) || AdvancedNoiseFilterMu(fOutMufiArray))){
      fOutMufiArray->Clear("C");
      fOutSciFiArray->Clear("C");
      fOutMCTrackArray->Clear("C");
      FirstEvent = false;
      continue;
    }

    //--------Save only first 25ns chunks mode-------
    if (fSaveFirst25nsOnly) {
      if (FirstEvent) {
        fOutTree->Fill();
        FirstEvent = false;
      }
    }
    //-----Save all chunks mode-------
    else {
      fOutTree->Fill();
    }
  }
}

//-----------------------Mu Fast Noise Filtering-------------------------------
bool MCEventBuilder::FastNoiseFilterMu_Hits(TClonesArray* muArray) {
    std::set<int> veto, ds;
    for (int i = 0; i < muArray->GetEntriesFast(); ++i) {
        auto* p = static_cast<MuFilterPoint*>(muArray->At(i));
        int detID = p->GetDetectorID();
        int system = detID / 10000;
        if (system == 1) {
          veto.insert(detID);
          if (veto.size() >= min_veto_hits) {
            return true;
          }
        }
        else if (system == 3) {
          ds.insert(detID);
          if (ds.size() >= min_ds_hits) {
            return true;
          }
        }
    }
    return false;
}

bool MCEventBuilder::FastNoiseFilterMu_Boards(TClonesArray* muArray) {
    //The digits used here to assign IDs to systems are set to match the DAQ board IDs from the 2022 board_mapping.json file
    std::set<int> us, ds;
    for (int i = 0; i < muArray->GetEntriesFast(); ++i) {
        auto* p = static_cast<MuFilterPoint*>(muArray->At(i));
        int detID = p->GetDetectorID();
        int system = detID / 10000;
        int TypeofPlane = (detID / 1000) % 10;
        if (system == 2) {
            if (TypeofPlane == 1 || TypeofPlane == 2) {
                us.insert(7);
            } else if (TypeofPlane == 3 || TypeofPlane == 4) {
                us.insert(60);
            } else if (TypeofPlane == 5) {
                us.insert(52);
            }
            if (us.size() >= min_us_boards) {
                return true;
            }
        } else if (system == 3) {
            if (TypeofPlane == 1) {
                ds.insert(41);
            } else if (TypeofPlane == 2) {
                ds.insert(35);
            } else if (TypeofPlane == 3 || TypeofPlane == 4) {
                ds.insert(55);
            }
            if (ds.size() >= min_ds_boards) {
                return true;
            }
        }
    }
    return false;
}

//-----------------------Mu Advanced Noise Filtering-------------------------------
bool MCEventBuilder::AdvancedNoiseFilterMu(TClonesArray* muArray) {
    std::set<int> veto_planes, us_planes, ds_planes;
    for (int i = 0; i < muArray->GetEntriesFast(); ++i) {
        auto* p = static_cast<MuFilterPoint*>(muArray->At(i));
        int detID = p->GetDetectorID();
        int system = detID / 10000;
        if (system == 1) {
            veto_planes.insert(detID / 1000);
            if (veto_planes.size() >= min_veto_planes) {
                return true;
            }
        } else if (system == 2) {
            us_planes.insert(detID / 1000);
            if (us_planes.size() >= min_us_planes) {
                return true;
            }
        } else if (system == 3) {
            ds_planes.insert(detID / 1000);
            if (ds_planes.size() >= min_ds_planes) {
                return true;
            }
        }
    }
    return false;
}


//-----------------------Scifi Fast Noise Filtering-------------------------------
bool MCEventBuilder::FastNoiseFilterScifi_Hits(
    TClonesArray* scifiArray,
    const std::map<Int_t, std::map<Int_t, std::array<float, 2>>>& siPMFibresMap)
  {
    std::set<int> detIDs;
    for (int i = 0; i < scifiArray->GetEntriesFast(); ++i) {
        auto* p = static_cast<ScifiPoint*>(scifiArray->At(i));
        int point_detID = p->GetDetectorID();
        int locFibreID = point_detID % 100000;
        
        for (const auto& sipmChan : siPMFibresMap.at(locFibreID)) {
            int channel = sipmChan.first;
            int detID_geo = (point_detID / 100000) * 100000 + channel;
            detIDs.insert(detID_geo);

            if (detIDs.size() >= min_scifi_hits){
                return true;
              }
        }
    }
    return false;
  }

bool MCEventBuilder::FastNoiseFilterScifi_Boards(
    TClonesArray* scifiArray,
    const std::map<Int_t, std::map<Int_t, std::array<float, 2>>>& siPMFibresMap)
  {
    //A DAQ board reads one Scifi mat and we use the first three digits of the detID_geo, representing the unique combination of StationPlaneMat, to count boards with fired channels.
    std::set<int> dividedDetIds;

    for (int i = 0; i < scifiArray->GetEntriesFast(); ++i) {
        auto* p = static_cast<ScifiPoint*>(scifiArray->At(i));
        int point_detID = p->GetDetectorID();
        int locFibreID = point_detID % 100000;
        
        for (const auto& sipmChan : siPMFibresMap.at(locFibreID)) {
            int channel = sipmChan.first;
            int detID_geo = (point_detID / 100000) * 100000 + channel;
            dividedDetIds.insert(detID_geo / 10000);

            if (dividedDetIds.size() >= min_scifi_boards){
                return true;
              }
        }
    }
    return false;
  }

//-----------------------Scifi Advanced Noise Filtering-------------------------------
bool MCEventBuilder::AdvancedNoiseFilterScifi(
    TClonesArray* scifiArray,
    const std::map<Int_t, std::map<Int_t, std::array<float, 2>>>& siPMFibresMap)
  {
    std::set<int> UniquePlanes;
    for (int i = 0; i < scifiArray->GetEntriesFast(); ++i) {
        auto* p = static_cast<ScifiPoint*>(scifiArray->At(i));
        int point_detID = p->GetDetectorID();
        int locFibreID = point_detID % 100000;
        
        for (const auto& sipmChan : siPMFibresMap.at(locFibreID)) {
            int channel = sipmChan.first;
            int detID_geo = (point_detID / 100000) * 100000 + channel;
            UniquePlanes.insert(detID_geo / 100000);

            if (UniquePlanes.size() >= min_scifi_planes) {
              return true;
            }
        }
    }
    return false;
  }  

void MCEventBuilder::FinishTask() {
  fOutSciFiArray->Delete();
  fOutMufiArray->Delete();
  fOutMCTrackArray->Delete();
  if (fOutTree) fOutTree->Write();
  if (fOutFile) {
    LOG(INFO) << "Writing and closing output file: " << fOutputFileName;
    fOutFile->Write();
    fOutFile->Close();
    delete fOutFile;
    fOutFile = nullptr;
  }
}


