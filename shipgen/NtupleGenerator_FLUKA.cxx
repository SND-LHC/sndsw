#include <math.h>
#include "TROOT.h"
#include "TSystem.h"
#include "TFile.h"
#include "FairPrimaryGenerator.h"
#include "NtupleGenerator_FLUKA.h"
#include "FairLogger.h"
#include "FairMCEventHeader.h"

// read events from ntuples produced by FLUKA

// -----   Default constructor   -------------------------------------------
NtupleGenerator_FLUKA::NtupleGenerator_FLUKA() {}
// -------------------------------------------------------------------------
// -----   Default constructor   -------------------------------------------
Bool_t NtupleGenerator_FLUKA::Init(const char* fileName) {
  return Init(fileName, 0);
}
// -----   Default constructor   -------------------------------------------
Bool_t NtupleGenerator_FLUKA::Init(const char* fileName, const int firstEvent) {
  TString fN = "";
   if (0 == strncmp("/eos",fileName,4) ) {
      fN = gSystem->Getenv("EOSSHIP");
   }
  fN+=fileName;
  fInputFile  = TFile::Open(fN);
  if (fInputFile->IsZombie()) {
    LOG(FATAL) << "NtupleGenerator_FLUKA: Error opening the Signal file" << fN;
    return kFALSE;
  }
  LOG(INFO) << "NtupleGenerator_FLUKA: Opening input file " << fN;

  fTree = (TTree *)fInputFile->Get("nt");

  fNevents = fTree->GetEntries();
  fn = firstEvent;
  primaries = 0;
  // Check the input TTree structure
  if (fTree->FindLeaf("primaries_") != nullptr) {     
     fTree->SetBranchAddress("primaries",&primaries);
  }
  else {  
    fTree->SetBranchAddress("run",&runN); // run number
    fTree->SetBranchAddress("event",&eventN); // event number
    fTree->SetBranchAddress("id",&id);                // particle id
    fTree->SetBranchAddress("generation",&generation);    //  origin generation number
    fTree->SetBranchAddress("t",&t);             // time of flight
    fTree->SetBranchAddress("E",&E);          // incoming muon energy
    fTree->SetBranchAddress("w",&w);                  // weight of event
    fTree->SetBranchAddress("x",&x);   // position
    fTree->SetBranchAddress("y",&y);
    fTree->SetBranchAddress("z",&z);
    fTree->SetBranchAddress("px",&px);   // momentum
    fTree->SetBranchAddress("py",&py);
    fTree->SetBranchAddress("pz",&pz);
  }
  return kTRUE;
}
// -------------------------------------------------------------------------


// -----   Destructor   ----------------------------------------------------
NtupleGenerator_FLUKA::~NtupleGenerator_FLUKA()
{
 // cout << "destroy Ntuple" << endl;
 fInputFile->Close();
 fInputFile->Delete();
 delete fInputFile;
}
// -------------------------------------------------------------------------

// -----   Passing the event   ---------------------------------------------
Bool_t NtupleGenerator_FLUKA::ReadEvent(FairPrimaryGenerator* cpg)
{
   fTree->GetEntry(fn);
   fn++;
   if (fn %10000==0)  {LOG(INFO)<< "reading event "<<fn;}
   if (fn > fNevents) {
     LOG(WARNING) << "No more input events";
     return kFALSE; }   
   /* Add all primary tracks in the event. Typically, muon bkg MC has single muon per event. 
      Several muon primaries per event exist in the multi-muon MC.
   */
   if (fTree->FindLeaf("primaries_") != nullptr) {
      int nf = primaries->GetEntries();
      for(int i=0; i<nf; i++){
          //casting
          struct PrimaryTrack* primary = dynamic_cast<struct PrimaryTrack*>(primaries->AddrAt(i));
          //convert time from ns to sec
          cpg->AddTrack(int(primary->id),primary->px,primary->py,primary->pz,
                        primary->x,primary->y,primary->z-SND_Z,-1,true,primary->E,
                        primary->t/1E9,primary->w,(TMCProcess)primary->generation);
          LOG(DEBUG) << "NtupleGenerator_FLUKA: add muon " << i << "," << int(primary->id)
                     <<","<<primary->px<<","<<primary->py<<","<<primary->pz<<","<<primary->x
                     <<","<<primary->y<<","<<primary->z-SND_Z<<","<<primary->generation
                     <<","<<primary->E<<","<<primary->t<<"ns,"<<primary->w;
      }
   }
   else {
     // convert time from ns to sec
     cpg->AddTrack(int(id[0]),px[0],py[0],pz[0],x[0],y[0],z[0]-SND_Z,-1,true,E[0],t[0]/1E9,w[0],(TMCProcess)generation[0]);
     LOG(DEBUG) << "NtupleGenerator_FLUKA: add muon " << id[0]<<","<<px[0]<<","<<py[0]<<","<<pz[0]<<","<<x[0]<<","<<y[0]<<","<<z[0]-SND_Z<<","<<generation[0]<<","<<E[0]<<","<<t[0]<<"ns,"<<w[0];
   }

   // Set the generation run and event numbers through the MC event header
   FairMCEventHeader* header = cpg->GetEvent();
   header->SetEventID(int(eventN));
   header->SetRunID(int(runN));

   return kTRUE;
}

// -------------------------------------------------------------------------
Int_t NtupleGenerator_FLUKA::GetNevents()
{
 return fNevents;
}


ClassImp(NtupleGenerator_FLUKA)
