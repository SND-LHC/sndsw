 #include <TClonesArray.h>           // or TClonesArray
 #include <TGenericClassInfo.h>      // for TGenericClassInfo
 #include <TMath.h>                  // for Sqrt
 #include <TRandom.h>                // for TRandom, gRandom
 #include <TFile.h>
 #include <TROOT.h>
 #include <iostream>                 // for operator<<, basic_ostream, endl
 #include <algorithm>                // std::sort
 #include <vector>                   // std::vector
 #include "FairMCEventHeader.h"      // for FairMCEventHeader
 #include "FairLink.h"               // for FairLink
 #include "FairRunSim.h"             // for FairRunSim
 #include "FairRunAna.h"             // for FairRunAna
 #include "FairRootManager.h"        // for FairRootManager
 #include "DigiTaskSND.h"   	     // for Digitization
 #include "ScifiPoint.h"	     // for SciFi Point
 #include "Scifi.h"	             // for SciFi detector
 #include "MuFilterPoint.h"	     // for Muon Filter Point
 #include "sndScifiHit.h"	     // for SciFi Hit
 #include "MuFilterHit.h"	     // for Muon Filter Hit
 #include "AdvTargetHit.h"	     // for AdvTarget Hit
 #include "Hit2MCPoints.h"           // for linking hits to true MC points

DigiTaskSND::DigiTaskSND()
    : FairTask("DigTaskSND")
    , fScifiPointArray(nullptr)
    , fMuFilterPointArray(nullptr)
    , fEventHeader(nullptr)
    , fScifiDigiHitArray(nullptr)
    , fMuFilterDigiHitArray(nullptr)
    , fScifiHit2MCPointsArray(nullptr)
    , fMuFilterHit2MCPointsArray(nullptr)
{}

DigiTaskSND::~DigiTaskSND() {}

InitStatus DigiTaskSND::Init()
{
    FairRootManager* ioman = FairRootManager::Instance();
    if (!ioman) {
        cout << "-E- DigiTaskSND::Init: "   /// todo replace with logger!
                  << "RootManager not instantiated!" << endl;
        return kFATAL;
    }

    // Get the SciFi detector and sipm to fibre mapping
    scifi = dynamic_cast<Scifi*> (gROOT->GetListOfGlobals()->FindObject("Scifi") );
    scifi->SiPMmapping();
    fibresSiPM = scifi->GetSiPMmap();
    siPMFibres = scifi->GetFibresMap();

    // Get event header
    // Try classic FairRoot approach first
    fMCEventHeader = static_cast<FairMCEventHeader*> (ioman->GetObject("MCEventHeader."));	
    // .. with a safety net for trailing dots mischief
    if ( fMCEventHeader == nullptr ) {
       fMCEventHeader = static_cast<FairMCEventHeader*>(gROOT->FindObjectAny("MCEventHeader."));
    }
    // Get input MC points
    fScifiPointArray = static_cast<TClonesArray*>(ioman->GetObject("ScifiPoint"));
    fvetoPointArray = static_cast<TClonesArray*>(ioman->GetObject("vetoPoint"));
    fEmulsionPointArray = static_cast<TClonesArray*>(ioman->GetObject("EmulsionDetPoint"));
    fMuFilterPointArray = static_cast<TClonesArray*>(ioman->GetObject("MuFilterPoint"));
    if (!fScifiPointArray and !fMuFilterPointArray) {
        cout << "-W- DigiTaskSND::Init: "
                  << "No Scifi and no MuFilter MC Point array!" << endl;
        return kERROR;
    }
    // copy branches from input file:
    fMCTrackArray = static_cast<TClonesArray*>(ioman->GetObject("MCTrack"));
    ioman->Register("MCTrack", "ShipMCTrack", fMCTrackArray, kTRUE);
    ioman->Register("vetoPoint", "vetoPoints", fvetoPointArray, kTRUE);
    ioman->Register("EmulsionDetPoint", "EmulsionDetPoints", fvetoPointArray, kTRUE);
    ioman->Register("ScifiPoint", "ScifiPoints", fScifiPointArray, kTRUE);
    ioman->Register("MuFilterPoint", "MuFilterPoints", fMuFilterPointArray, kTRUE);
 
    // Event header
    fEventHeader = new SNDLHCEventHeader();
    ioman->Register("EventHeader.", "sndEventHeader", fEventHeader, kTRUE);

    // Create and register output array - for SciFi and MuFilter
    fScifiDigiHitArray = new TClonesArray("sndScifiHit");
    ioman->Register("Digi_ScifiHits", "DigiScifiHit_det", fScifiDigiHitArray, kTRUE);
    // Branche containing links to MC truth info
    fScifiHit2MCPointsArray = new TClonesArray("Hit2MCPoints");
    ioman->Register("Digi_ScifiHits2MCPoints", "DigiScifiHits2MCPoints_det", fScifiHit2MCPointsArray, kTRUE);
    fScifiHit2MCPointsArray->BypassStreamer(kTRUE);
    fMuFilterDigiHitArray = new TClonesArray("MuFilterHit");
    ioman->Register("Digi_MuFilterHits", "DigiMuFilterHit_det", fMuFilterDigiHitArray, kTRUE);
    // Branche containing links to MC truth info
    fMuFilterHit2MCPointsArray = new TClonesArray("Hit2MCPoints");
    ioman->Register("Digi_MuFilterHits2MCPoints", "DigiMuFilterHits2MCPoints_det", fMuFilterHit2MCPointsArray, kTRUE);
    fMuFilterHit2MCPointsArray->BypassStreamer(kTRUE);
    fAdvTargetHits = new std::vector<AdvTargetHit*>;
    ioman->RegisterAny("Digi_advTargetHits", fAdvTargetHits, kTRUE);
    
    return kSUCCESS;
}

void DigiTaskSND::Exec(Option_t* /*opt*/)
{

    fScifiDigiHitArray->Clear("C");
    fScifiHit2MCPointsArray->Clear("C");
    fMuFilterDigiHitArray->Clear("C");
    fMuFilterHit2MCPointsArray->Clear("C");
    for (auto const &x : (*fAdvTargetHits)) {
        delete x;
    }
    fAdvTargetHits->clear();

    // Set event header
    fEventHeader->SetRunId(fMCEventHeader->GetRunID());
    fEventHeader->SetEventNumber(fMCEventHeader->GetEventID());
    fEventHeader->SetBunchType(101);

    // Digitize MC points if any
    if (fMuFilterPointArray) digitizeMuFilter();
    if (fScifiPointArray)
    {
        digitizeScifi();
    }
}

void DigiTaskSND::digitizeScifi()
{
    // a map containing fibreID and vector(list) of points and weights
    map<int, pair<vector<ScifiPoint*>, vector<float>> > hitContainer{};
    Hit2MCPoints mcLinks;
    map<pair<int, int>, double> mcPoints{};
    map<int, double> norm{};
    int globsipmChan{}, detID{};
    int locFibreID{};
    
    // Fill the map
    for (int k = 0, kEnd = fScifiPointArray->GetEntries(); k < kEnd; k++)
    {
        ScifiPoint* point = static_cast<ScifiPoint*>(fScifiPointArray->At(k));
        if (!point) continue;
        // Collect all hits in same SiPM channel
        detID = point->GetDetectorID();
        locFibreID = detID%100000;
        // Check if locFibreID in a dead area
        if(siPMFibres.count(locFibreID)==0) continue;
        double dE{};
        float weight{};
        for (auto sipmChan : siPMFibres[locFibreID])
        {
            globsipmChan = int(detID/100000)*100000+sipmChan.first;
            weight = sipmChan.second[0];
            hitContainer[globsipmChan].first.push_back(point);
            hitContainer[globsipmChan].second.push_back(weight);
            dE = point->GetEnergyLoss()*weight;
            mcPoints[make_pair(globsipmChan, k)] = dE;
            norm[globsipmChan]+= dE;
        }
    }// End filling map
    int index = 0;
    // Loop over entries of the hitContainer map and collect all hits in same detector element
    for (auto it = hitContainer.begin(); it != hitContainer.end(); it++){
        new ((*fScifiDigiHitArray)[index]) sndScifiHit(it->first, hitContainer[it->first].first, hitContainer[it->first].second);
        index++;
 	for (auto mcit = mcPoints.begin(); mcit != mcPoints.end(); mcit++){
            if(it->first == mcit->first.first) mcLinks.Add(it->first, mcit->first.second, mcPoints[make_pair(it->first, mcit->first.second)]/norm[it->first]);
        }
    }
    new((*fScifiHit2MCPointsArray)[0]) Hit2MCPoints(mcLinks);
}

void DigiTaskSND::digitizeMuFilter()
{
    // a map with detID and vector(list) of points
    map<int, vector<MuFilterPoint*> > hitContainer{};
    Hit2MCPoints mcLinks;
    map<pair<int, int>, double> mcPoints{};
    map<int, double> norm{};
    int detID{};

    // Fill the map
    for (int k = 0, kEnd = fMuFilterPointArray->GetEntries(); k < kEnd; k++) {
        MuFilterPoint* point = static_cast<MuFilterPoint*>(fMuFilterPointArray->At(k));
        if (!point) continue;
        // Collect all hits in same detector element
        detID = point->GetDetectorID();
        hitContainer[detID].push_back(point);
        mcPoints[make_pair(detID, k)] = point->GetEnergyLoss();
        norm[detID]+= point->GetEnergyLoss();
    }
    int index = 0;
    // Loop over entries of the hitContainer map and collect all hits in same detector element
    for (auto it = hitContainer.begin(); it != hitContainer.end(); it++){
        /*MuFilterHit* aHit = */new ((*fMuFilterDigiHitArray)[index]) MuFilterHit(it->first, hitContainer[it->first]);
        index++;
 	for (auto mcit = mcPoints.begin(); mcit != mcPoints.end(); mcit++){
            if(it->first == mcit->first.first) mcLinks.Add(it->first, mcit->first.second, mcPoints[make_pair(it->first, mcit->first.second)]/norm[it->first]);
        }
    }
    new((*fMuFilterHit2MCPointsArray)[0]) Hit2MCPoints(mcLinks); 
}

ClassImp(DigiTaskSND);
