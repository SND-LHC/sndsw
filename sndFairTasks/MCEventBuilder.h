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

class TFile;
class TTree;
class MuFilterPoint;
class ScifiPoint;
class ShipMCTrack;
class FairMCEventHeader;

class MCEventBuilder : public FairTask {
public:
  MCEventBuilder(const std::string& outputFileName, bool saveOnlyFirst25);
  ~MCEventBuilder();

  virtual InitStatus Init();
  virtual void Exec(Option_t* opt);
  virtual void FinishTask();

private:
  //Function I need later for ordering the mc points
  std::vector<int> OrderedIds(const std::vector<double>& times, double firstTime) const;
  
  //Function that does all the processing
  void ProcessEvent();

  // Fast filter functions 
  bool FastNoiseFilterMu_Hits(TClonesArray* muArray);
  bool FastNoiseFilterMu_Boards(TClonesArray* muArray);

  bool FastNoiseFilterScifi_Hits(
    TClonesArray* scifiArray,
    const std::map<Int_t, std::map<Int_t, std::array<float, 2>>>& siPMFibres);
  bool FastNoiseFilterScifi_Boards(
    TClonesArray* scifiArray,
    const std::map<Int_t, std::map<Int_t, std::array<float, 2>>>& siPMFibres);

  //Advanced Noise Filter
  bool AdvancedNoiseFilterScifi(
    TClonesArray* scifiArray,
    const std::map<Int_t, std::map<Int_t, std::array<float, 2>>>& siPMFibres);
    
  bool AdvancedNoiseFilterMu(TClonesArray* muArray);

  //Input
  bool fSaveFirst25nsOnly;
  FairMCEventHeader* fInHeader;
  TClonesArray*      fInMufiArray;
  TClonesArray*      fInSciFiArray;
  TClonesArray*      fInMCTrackArray;

  //Output
  std::string fOutputFileName;
  TFile*      fOutFile;
  TTree*      fOutTree;
  FairMCEventHeader* fOutHeader;
  TClonesArray*      fOutMufiArray;
  TClonesArray*      fOutSciFiArray;
  TClonesArray*      fOutMCTrackArray;

  
  ClassDef(MCEventBuilder, 1)
};

#endif // MCEVENTBUILDER_H
