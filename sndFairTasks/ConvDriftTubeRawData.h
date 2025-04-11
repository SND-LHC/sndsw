#ifndef SNDFAIRTASKS_CONVDRIFTTUBERAWDATA_H_
#define SNDFAIRTASKS_CONVDRIFTTUBERAWDATA_H_

#include <Rtypes.h>     // for THashConsistencyHolder, ClassDef
#include <RtypesCore.h> // for Double_t, Int_t, Option_t
#include <TClonesArray.h>
#include "FairTask.h"     // for FairTask, InitStatus
#include "DriftTube.h"    // for DriftTube detector
#include "DriftTubeHit.h" // for DriftTube hits
class TBuffer;
class TClass;
class TClonesArray;
class TMemberInspector;

class ConvDriftTubeRawData : public FairTask {
public:
   /** Default constructor **/
   ConvDriftTubeRawData();

   /** Destructor **/
   ~ConvDriftTubeRawData();

   /** Virtual method Init **/
   virtual InitStatus Init();

   /** Virtual method Exec **/
   virtual void Exec(Option_t *opt);

   /** Update input raw-data file and first-to-process event **/
   void UpdateInput(int n);

private:
   /** Processing of raw data **/
   void Process();

   /** Data structures to be used in the class **/
   std::map<int, DriftTubeHit *> digiDTStore{};

   DriftTube *DriftTubeDet;

   // Input
   TTree *fEventTree;
   int frunNumber, eventNumber;
   int fnStart, fnEvents;
   double runStartUTC;
   // Output
   TFile *fOut;
   TClonesArray *fDigiDriftTube;

   ConvDriftTubeRawData(const ConvDriftTubeRawData &);
   ConvDriftTubeRawData &operator=(const ConvDriftTubeRawData &);

   ClassDef(ConvDriftTubeRawData, 1);
};

#endif // SNDFAIRTASKS_CONVDRIFTTUBERAWDATA_H_
