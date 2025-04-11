#include <TClonesArray.h>      // or TClonesArray
#include <TGenericClassInfo.h> // for TGenericClassInfo
#include <TMath.h>             // for Sqrt
#include <TRandom.h>           // for TRandom, gRandom
#include <TFile.h>
#include <TTree.h>
#include <TBranch.h>
#include <TLeaf.h>
#include <TROOT.h>
#include <iostream>               // for operator<<, basic_ostream, endl
#include <algorithm>              // std::sort
#include <vector>                 // std::vector
#include "FairMCEventHeader.h"    // for FairMCEventHeader
#include "FairLink.h"             // for FairLink
#include "FairRunAna.h"           // for FairRunAna
#include "FairRootManager.h"      // for FairRootManager
#include "ConvDriftTubeRawData.h" // for Conversion
#include "DriftTube.h"            // for Drift Tube detector
// FIXME cleanup headers

ConvDriftTubeRawData::ConvDriftTubeRawData()
   : FairTask("ConvDriftTubeRawData"), fEventTree(nullptr), fDigiDriftTube(nullptr)
{
}

ConvDriftTubeRawData::~ConvDriftTubeRawData() {}

InitStatus ConvDriftTubeRawData::Init()
{
   FairRootManager *ioman = FairRootManager::Instance();
   if (!ioman) {
      LOG(FATAL) << "ConvDriftTubeRawData::Init: RootManager not instantiated!";
   }

   // Get the DriftTube detector from the list of globals
   DriftTubeDet = dynamic_cast<DriftTube *>(gROOT->GetListOfGlobals()->FindObject("DriftTube"));

   // Input raw data file is read from the FairRootManager
   // This allows to have it in custom format, e.g. have arbitary names of TTrees
   TFile *f0 = dynamic_cast<TFile *>(ioman->GetObject("XXX")); // FIXME name of the input TTree
   fEventTree = (TTree *)f0->Get("event");

   // Register the output
   fDigiDriftTube = new TClonesArray("DriftTubeHit");
   ioman->Register("Digi_DriftTubeHits", "DigiDriftTubeHit_det", fDigiDriftTube, kTRUE);

   // Get the FairLogger
   FairLogger *logger = FairLogger::GetLogger();

   eventNumber = fnStart;

   return kSUCCESS;
}

void ConvDriftTubeRawData::Exec(Option_t * /*opt*/)
{

   fDigiDriftTube->Clear("C"); //

   // Delete pointer map elements
   for (auto it : digiDTStore) {
      delete it.second;
   }
   digiDTStore.clear();

   // run the conversion
   Process();
}

void ConvDriftTubeRawData::Process()
{

   int indexDriftTube{}; // index of DT hits
   int detID;
   fEventTree->GetEvent(eventNumber);
   // Loop over hits per event!
   for (int n = 0; n < fEventTree->GetLeaf("nHits")->GetValue(); n++) { // FIXME - this is simply an example

      // FIXME add conversion below

      // store the hits internally per event
      if (digiDTStore.count(detID) == 0) {
         // make the hit via the hit constructor
         // e.g. digiDTStore[detID] = new DriftTubeHit(detID,xxx);
      }
      // would be nice to have a link btw raw hit and converted one - below is example
      // from the SciFi where we use the DAQ RO params
      // digiDTStore[detID]->SetDaqID(sipm_number,n, board_id, tofpet_id, tofpet_channel);
   } // end loop over hits in the event

   // write the hits to the output
   for (auto it_detID : digiDTStore) {
      (*fDigiDriftTube)[indexDriftTube] = digiDTStore[it_detID.first];
      indexDriftTube += 1;
   }
   LOG(INFO) << fnStart + 1 << " events processed out of " << fEventTree->GetEntries() << " number of events in file.";
}

void ConvDriftTubeRawData::UpdateInput(int NewStart)
{
   fEventTree->Refresh();
   eventNumber = NewStart;
}
