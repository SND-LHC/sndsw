#include "MCEventBuilder.h"
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

MCEventBuilder::MCEventBuilder(const std::string& outputFileName)
  : FairTask("MCEventBuilder"),
    fOutputFileName(outputFileName),
    fOutFile(nullptr),
    fOutTree(nullptr),
    fInMuArray(nullptr),
    fInSciArray(nullptr),
    fInMCTrackArray(nullptr),
    fInHeader(nullptr),
    fOutMuArray(nullptr),
    fOutSciArray(nullptr),
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
  fInMuArray  = static_cast<TClonesArray*>(ioman->GetObject("MuFilterPoint"));
  fInSciArray = static_cast<TClonesArray*>(ioman->GetObject("ScifiPoint"));
  fInMCTrackArray  = static_cast<TClonesArray*>(ioman->GetObject("MCTrack"));

  if (!fInMuArray && !fInSciArray) {
    LOG(ERROR) << "No Scifi and no MuFilter MC points array!";
    return kERROR;
  }

  // —---------- OUTPUT FILE & TREE —----------

  fOutFile = TFile::Open(fOutputFileName.c_str(), "RECREATE");
  fOutTree = new TTree("cbmsim", "RebuiltEvents");

  fOutHeader   = new FairMCEventHeader();
  fOutMuArray  = new TClonesArray("MuFilterPoint");
  fOutSciArray = new TClonesArray("ScifiPoint");
  fOutMCTrackArray  = new TClonesArray("ShipMCTrack");
  
  fOutTree->Branch("MuFilterPoint",  &fOutMuArray,  32000, 1);
  fOutTree->Branch("ScifiPoint",     &fOutSciArray, 32000, 1);
  fOutTree->Branch("MCTrack",        &fOutMCTrackArray,  32000, 1);
  fOutTree->Branch("MCEventHeader.", &fOutHeader,   32000, 1);


  // --------------Global Variables--------------------
  float t_electronics = 0.0; //If known
  timeWindow = 25.0 + t_electronics; //Change the size of the chunk windows
  
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

  for (int i = 0; i < fInMuArray->GetEntriesFast(); ++i) {
    auto* p = static_cast<MuFilterPoint*>(fInMuArray->At(i));
    muFilterPoints.push_back(static_cast<MuFilterPoint*>(p->Clone()));
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

    double tLeft  = p->GetTime() + (vLeft - impact).Mag()  / propspeed;
    double tRight = p->GetTime() + (vRight - impact).Mag() / propspeed;
    double arrivalTime = std::min(tLeft, tRight);
    muArrivalTimes.push_back(arrivalTime);
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

  for (int i = 0; i < fInSciArray->GetEntriesFast(); ++i) {
    auto* p = static_cast<ScifiPoint*>(fInSciArray->At(i));
    scifiPoints.push_back(static_cast<ScifiPoint*>(p->Clone()));
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
  for (auto&& obj : *fInMCTrackArray) {
    auto* t = static_cast<ShipMCTrack*>(obj);
    mcTrackClones.push_back(static_cast<ShipMCTrack*>(t->Clone()));
  }

  //---------Finding the earliest time between Scifi and MuFilter-----------------
  double tMu  = sortedMuArrivalTimes.empty()  ? -1 : sortedMuArrivalTimes.front();
  double tScifi = sortedScifiArrivalTimes.empty() ? -1 : sortedScifiArrivalTimes.front();
  bool has   = (tMu >= 0 || tScifi >= 0);
  double firstT = has ? (tMu < 0 ? tScifi : (tScifi < 0 ? tMu : std::min(tMu, tScifi))): 0;

  if (!has) {
    fOutMuArray->Clear();
    fOutSciArray->Clear();
    fOutMCTrackArray->Clear();
    fOutTree->Fill();
    return;
  }

  //------------------Preparations before chunking the data---------------------
  auto idsMu  = OrderedIds(sortedMuArrivalTimes,  firstT);
  auto idsScifi = OrderedIds(sortedScifiArrivalTimes, firstT);

  std::vector<int> used;  
  int im = 0, is = 0, sliceMu = 0, sliceScifi = 0;

  while (im < (int)sortedMuArrivalTimes.size() || is < (int)sortedScifiArrivalTimes.size()) {
    fOutMuArray->Clear("C");
    fOutSciArray->Clear("C");
    fOutMCTrackArray->Clear("C");

    std::vector<MuFilterPoint*> muSlicePoints;
    std::vector<ScifiPoint*> scifiSlicePoints;
    fOutMCTrackArray->Delete();  
    fOutMCTrackArray->ExpandCreate(mcTrackClones.size());
    //Adding the mother track:
    ShipMCTrack* origTrack = mcTrackClones[0];
    Int_t trackPdgCode = origTrack->GetPdgCode();
    Int_t motherID     = origTrack->GetMotherId();
    Double_t px = origTrack->GetPx();
    Double_t py = origTrack->GetPy();
    Double_t pz = origTrack->GetPz();
    Double_t E  = origTrack->GetEnergy();
    Double_t x  = origTrack->GetStartX();
    Double_t y  = origTrack->GetStartY();
    Double_t z  = origTrack->GetStartZ();
    Double_t t  = origTrack->GetStartT();
    Int_t nPoints = 0;
    Double_t w    = origTrack->GetWeight();

    ShipMCTrack* newTrack = new ((*fOutMCTrackArray)[0])
      ShipMCTrack(trackPdgCode, motherID, px, py, pz, E, x, y, z, t, nPoints, w);
    newTrack->SetProcID(origTrack->GetProcID());

    //Muon Filter Points chunking
    while (im < (int)sortedMuArrivalTimes.size() && idsMu[im] == sliceMu) {
      MuFilterPoint* origMu = sortedMuPoints[im];
      muSlicePoints.push_back(origMu);
      Int_t trackID   = origMu->GetTrackID();
      Int_t detID     = origMu->GetDetectorID();
      TVector3 pos;    origMu->Position(pos);
      TVector3 mom;    origMu->Momentum(mom);
      Double_t time   = origMu->GetTime();
      Double_t length = origMu->GetLength();
      Double_t eLoss  = origMu->GetEnergyLoss();
      Int_t pdgCode   = origMu->PdgCode();
      new ((*fOutMuArray)[fOutMuArray->GetEntriesFast()])
        MuFilterPoint(trackID, detID, pos, mom, time, length, eLoss, pdgCode);

      int tid = sortedMuTrackIDs[im++];
      if (tid != -2) {
        ShipMCTrack* origTrack = mcTrackClones[tid];
        Int_t trackPdgCode = origTrack->GetPdgCode();
        Int_t motherID     = origTrack->GetMotherId();
        Double_t px = origTrack->GetPx();
        Double_t py = origTrack->GetPy();
        Double_t pz = origTrack->GetPz();
        Double_t E  = origTrack->GetEnergy();
        Double_t x  = origTrack->GetStartX();
        Double_t y  = origTrack->GetStartY();
        Double_t z  = origTrack->GetStartZ();
        Double_t t  = origTrack->GetStartT();
        Int_t nPoints = 0;
        Double_t w    = origTrack->GetWeight();

        ShipMCTrack* newTrack = new ((*fOutMCTrackArray)[tid])
          ShipMCTrack(trackPdgCode, motherID, px, py, pz, E, x, y, z, t, nPoints, w);
        newTrack->SetProcID(origTrack->GetProcID());  
      }
    }
    ++sliceMu;

    //Scifi Points chunking
    while (is < (int)sortedScifiArrivalTimes.size() && idsScifi[is] == sliceScifi) {
      ScifiPoint* origSci = sortedScifiPoints[is];
      scifiSlicePoints.push_back(origSci);

      Int_t trackID   = origSci->GetTrackID();
      Int_t detID     = origSci->GetDetectorID();
      TVector3 pos;    origSci->Position(pos);
      TVector3 mom;    origSci->Momentum(mom);
      Double_t time   = origSci->GetTime();
      Double_t length = origSci->GetLength();
      Double_t eLoss  = origSci->GetEnergyLoss();
      Int_t pdgCode   = origSci->PdgCode();

      new ((*fOutSciArray)[fOutSciArray->GetEntriesFast()])
        ScifiPoint(trackID, detID, pos, mom, time, length, eLoss, pdgCode);

      int tid = sortedScifiTrackIDs[is++];
      if (tid != -2) {
        ShipMCTrack* origTrack = mcTrackClones[tid];
        Int_t trackPdgCode = origTrack->GetPdgCode();
        Int_t motherID     = origTrack->GetMotherId();
        Double_t px = origTrack->GetPx();
        Double_t py = origTrack->GetPy();
        Double_t pz = origTrack->GetPz();
        Double_t E  = origTrack->GetEnergy();
        Double_t x  = origTrack->GetStartX();
        Double_t y  = origTrack->GetStartY();
        Double_t z  = origTrack->GetStartZ();
        Double_t t  = origTrack->GetStartT();
        Int_t nPoints = 0;
        Double_t w    = origTrack->GetWeight();

        ShipMCTrack* newTrack = new ((*fOutMCTrackArray)[tid])
          ShipMCTrack(trackPdgCode, motherID, px, py, pz, E, x, y, z, t, nPoints, w);
        newTrack->SetProcID(origTrack->GetProcID());
      
      }
    }
    ++sliceScifi; 

    //Noise Filters
    if (!(FastNoiseFilterScifi_Hits(fOutSciArray) || FastNoiseFilterMu_Hits(fOutMuArray))) {
      fOutMuArray->Clear("C");
      fOutSciArray->Clear("C");
      fOutMCTrackArray->Clear("C");
      continue;  
    }
    else if(!(FastNoiseFilterScifi_Boards(fOutSciArray, siPMFibres) || FastNoiseFilterMu_Boards(fOutMuArray))){
      fOutMuArray->Clear("C");
      fOutSciArray->Clear("C");
      fOutMCTrackArray->Clear("C");
      continue;
    }

    if (!(AdvancedNoiseFilterScifi(fOutSciArray, siPMFibres) || AdvancedNoiseFilterMu(fOutMuArray))){
      fOutMuArray->Clear("C");
      fOutSciArray->Clear("C");
      fOutMCTrackArray->Clear("C");
      continue;
    }
    fOutTree->Fill();

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
          if (veto.size() >= 5) {
            return true;
          }
        }
        else if (system == 3) {
          ds.insert(detID);
          if (ds.size() >= 2) {
            return true;
          }
        }
    }
    return false;
}

bool MCEventBuilder::FastNoiseFilterMu_Boards(TClonesArray* muArray) {
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
            if (us.size() >= 5) {
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
            if (ds.size() >= 2) {
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
            if (veto_planes.size() >= 1) {
                return true;
            }
        } else if (system == 2) {
            us_planes.insert(detID / 1000);
            if (us_planes.size() >= 2) {
                return true;
            }
        } else if (system == 3) {
            ds_planes.insert(detID / 1000);
            if (ds_planes.size() >= 2) {
                return true;
            }
        }
    }
    return false;
}


//-----------------------Scifi Fast Noise Filtering-------------------------------
bool MCEventBuilder::FastNoiseFilterScifi_Hits(TClonesArray* scifiArray) {
    std::set<int> detIDs;
    for (int i = 0; i < scifiArray->GetEntriesFast(); ++i) {
        auto* p = static_cast<ScifiPoint*>(scifiArray->At(i));
        detIDs.insert(p->GetDetectorID());
        if (detIDs.size() >= 10)
            return true;
    }
    return false;
}

bool MCEventBuilder::FastNoiseFilterScifi_Boards(
    TClonesArray* scifiArray,
    const std::map<Int_t, std::map<Int_t, std::array<float, 2>>>& siPMFibresMap)
  {
    std::set<int> dividedDetIds;

    for (int i = 0; i < scifiArray->GetEntriesFast(); ++i) {
        auto* p = static_cast<ScifiPoint*>(scifiArray->At(i));
        int point_detID = p->GetDetectorID();
        int locFibreID = point_detID % 100000;
        
        for (const auto& sipmChan : siPMFibresMap.at(locFibreID)) {
            int channel = sipmChan.first;
            int detID_geo = (point_detID / 100000) * 100000 + channel;
            dividedDetIds.insert(detID_geo / 10000);

            if (dividedDetIds.size() >= 3)
                return true;
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

            if (UniquePlanes.size() >= 4) {
              return true;
            }
        }
    }
    return false;
  }  

void MCEventBuilder::FinishTask() {
  fOutSciArray->Delete();
  fOutMuArray->Delete();
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


