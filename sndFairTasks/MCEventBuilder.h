#ifndef MCEVENTBUILDER_H
#define MCEVENTBUILDER_H
#include <TString.h> 
#include "FairTask.h"
#include <string>
#include <vector>
#include <TObjString.h>
#include "FairMCPoint.h"
#include <Rtypes.h>
#include <TClonesArray.h>
#include "FairMCEventHeader.h"
#include "MuFilter.h"
#include "Scifi.h"
#include "MuFilterPoint.h"
#include "SNDLHCEventHeader.h"

class TFile;
class TTree;
class MuFilterPoint;
class ScifiPoint;
class ShipMCTrack;

class MCEventBuilder : public FairTask {
public:
  MCEventBuilder();
  ~MCEventBuilder();

  virtual InitStatus Init();
  virtual void Exec(Option_t* opt);
  virtual void FinishTask();
  
  //void SetInputFile(const std::string& path)  { fInputFile = path; }
  //void SetOutputFile(const std::string& path) { fOutputFile = path; }

private:
  //Function I need later for ordering the mc points
  std::vector<int> OrderedIds(const std::vector<double>& times, double firstTime) const;
  
  //Function that does all the processing
  void ProcessEvent();

  // Fast filter functions 
  bool FastNoiseFilterMu_Hits(TClonesArray* muArray);
  bool FastNoiseFilterMu_Boards(TClonesArray* muArray);

  bool FastNoiseFilterScifi_Hits(TClonesArray* scifiArray);
  bool FastNoiseFilterScifi_Boards(
    TClonesArray* scifiArray,
    const std::map<Int_t, std::map<Int_t, std::array<float, 2>>>& siPMFibres);


  //Advance Noise Filter
  bool AdvancedNoiseFilterScifi(
    TClonesArray* scifiArray,
    const std::map<Int_t, std::map<Int_t, std::array<float, 2>>>& siPMFibres);
    
  bool AdvancedNoiseFilterMu(TClonesArray* muArray);

  //Input
  TString       fOutputFileName;
  FairMCEventHeader* fInHeader;
  TClonesArray*      fInMuArray;
  TClonesArray*      fInSciArray;
  TClonesArray*      fInMCTrackArray;


  //Output
  TFile*      fOutFile;
  TTree*      fOutTree;
  FairMCEventHeader* fOutHeader;
  TClonesArray*      fOutMuArray;
  TClonesArray*      fOutSciArray;
  TClonesArray*      fOutMCTrackArray;

  //Global Variables
  float timeWindow;

  MuFilter* MuFilterDet;
  float DsPropSpeed;
  float VandUpPropSpeed;

  Scifi* ScifiDet;
  float ScifisignalSpeed;
  std::map<Int_t, std::map<Int_t, std::array<float, 2>>> siPMFibres;


  ClassDef(MCEventBuilder, 1)
};

#endif // MCEVENTBUILDER_H
