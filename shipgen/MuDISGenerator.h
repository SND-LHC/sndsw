#ifndef PNDMuGENERATOR_H
#define PNDMuGENERATOR_H 1

#include "TROOT.h"
#include "FairGenerator.h"
#include "TTree.h"                      // for TTree
#include "TF1.h"                        // for TF1
#include "TClonesArray.h"         
#include "TVector3.h"                        
#include "FairLogger.h"                 // for FairLogger, MESSAGE_ORIGIN
#include "vector"

class FairPrimaryGenerator;

class MuDISGenerator : public FairGenerator
{
 public:
  
  /** default constructor **/
  MuDISGenerator();
  
  /** destructor **/
  virtual ~MuDISGenerator();
  
  /** public method ReadEvent **/
  Bool_t ReadEvent(FairPrimaryGenerator*);  
  virtual Bool_t Init(const char*, int); //!
  virtual Bool_t Init(const char*); //!
  Int_t GetNevents();

  /** set the 3D position of the muonDIS interaction  **/
  void SetPositions(Double_t zS=-3400., Double_t zE=2650., Double_t xS=-999., Double_t xE=999., Double_t yS=-999., Double_t yE=999.){
    startZ = zS;
    endZ   = zE; 
    startX = xS;
    endX   = xE;
    startY = yS;
    endY   = yE;
  }

 private:
  Double_t MeanMaterialBudget(const Double_t *start, const Double_t *end, Double_t *mparam);

  
 protected:
  Double_t startZ, endZ, startX, endX, startY, endY;
  TClonesArray* iMuon ;
  TClonesArray* dPart ; 
  FairLogger*  fLogger; //!   don't make it persistent, magic ROOT command
  TFile* fInputFile;
  TTree* fTree;
  int fNevents;
  int fn;
  bool fFirst;
  ClassDef(MuDISGenerator,2);
};
#endif /* !PNDMuGENERATOR_H */
