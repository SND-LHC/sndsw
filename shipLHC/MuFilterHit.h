#ifndef MUFILTERHIT_H
#define MUFILTERHIT_H 1

#include "SndlhcHit.h"
#include "MuFilterPoint.h"
#include "TObject.h"
#include "TVector3.h"
#include <map>

class MuFilterHit : public SndlhcHit
{
  public:

    /** Default constructor **/
    MuFilterHit();
    explicit MuFilterHit(Int_t detID);
    /** Constructor with detector id, number of SiPMs per side, number of sides **/
    MuFilterHit(Int_t detID,Int_t nP,Int_t nS);
    MuFilterHit(const MuFilterHit& hit) = default;
    MuFilterHit& operator=(const MuFilterHit& hit) = default;

    // Constructor from MuFilterPoint
    MuFilterHit(Int_t detID,std::vector<MuFilterPoint*>);

 /** Destructor **/
    virtual ~MuFilterHit();

    /** Output to screen **/
    void Print() const;
    Float_t GetEnergy();
    Float_t SumOfSignals(char* opt,Bool_t mask=kTRUE);
    std::map<TString,Float_t> SumOfSignals(Bool_t mask=kTRUE);
    std::map<Int_t,Float_t> GetAllSignals(Bool_t mask=kTRUE,Bool_t positive=kTRUE);
    std::map<Int_t,Float_t> GetAllTimes(Bool_t mask=kTRUE);
    Float_t  GetDeltaT(Bool_t mask=kTRUE);
    Float_t  GetFastDeltaT(Bool_t mask=kTRUE);
    Float_t  GetImpactT(Bool_t mask=kTRUE);
    bool isValid() const {return flag;}
    bool isMasked(Int_t i) const {return fMasked[i];}
    void SetMasked(Int_t i) {fMasked[i]=kTRUE;}
    int GetSystem(){return floor(fDetectorID/10000);}
    int GetPlane(){return int(fDetectorID/1000)%10;}
    bool isVertical();
    bool isShort(Int_t);
  private:

    Float_t flag;   ///< flag
    Float_t fMasked[16];  /// masked signal

    ClassDef(MuFilterHit, 6);
    

};

#endif
