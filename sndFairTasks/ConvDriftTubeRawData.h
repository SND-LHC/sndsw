#ifndef SNDFAIRTASKS_CONVDRIFTTUBERAWDATA_H_
#define SNDFAIRTASKS_CONVDRIFTTUBERAWDATA_H_

#include <Rtypes.h>     // for THashConsistencyHolder, ClassDef
#include <RtypesCore.h> // for Double_t, Int_t, Option_t
#include <TClonesArray.h>
#include <TChain.h>
#include "FairTask.h"     // for FairTask, InitStatus
#include "DriftTube.h"    // for DriftTube detector
#include "DriftTubeHit.h" // for DriftTube hits
class TBuffer;
class TClass;
class TClonesArray;
class TMemberInspector;

class ConvDriftTubeRawData : public FairTask {
   struct HitPoint;
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

   /** Set DriftTubeHit detID as in the SND geometry **/
   int SetDetID(const int& chamber, const int& layer, const int& wire);

   /** Find neighbouring cells to build clusters **/
   std::vector<HitPoint> GetNeighbours(const int& L, const int& C);

   /** Find hit clusters **/
   std::vector<std::vector<int>> FindClusters(const TClonesArray * hits);

   /** Find lateralities with slopes method**/
   void FindLateralitySlope(const TClonesArray * hits, const std::vector<std::vector<int>>& clusters);


private:
   /** Processing of raw data **/
   void Process();

   DriftTube *DriftTubeDet;


   // Input
   TTree *fSNDTree;
   TChain *fMiniDTChain;
   int frunNumber, eventNumber; 
   long MiniDTeventNumber;
   int fnStart = 0;
   int fnEvents;
   double runStartUTC;
   // Output
   TFile *fOut;
   TClonesArray *fDigiDriftTube;

   ConvDriftTubeRawData(const ConvDriftTubeRawData &);
   ConvDriftTubeRawData &operator=(const ConvDriftTubeRawData &);

   ClassDef(ConvDriftTubeRawData, 1);
};

#endif // SNDFAIRTASKS_CONVDRIFTTUBERAWDATA_H_