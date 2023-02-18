#ifndef ADVTARGETHIT_H
#define ADVTARGETHIT_H 1

#include "SndlhcHit.h"
#include "AdvTargetPoint.h"
#include "TObject.h"
#include "TVector3.h"
#include <map>

class AdvTargetHit : public SndlhcHit
{
  public:

    /** Default constructor **/
    AdvTargetHit();
    AdvTargetHit(Int_t detID);

    // Constructor from vector of AdvTargetPoint
    AdvTargetHit(Int_t detID,std::vector<AdvTargetPoint*>);

    // Constructor from AdvTargetPoint
    AdvTargetHit(Int_t detID, AdvTargetPoint* point, Int_t index);

 /** Destructor **/
    virtual ~AdvTargetHit();

    /** Output to screen **/
    void Print() const;
    Float_t GetEnergy();
    bool isValid() const {return flag;}
    Double_t GetX() {return fX;}
    Double_t GetY() {return fY;}
    Double_t GetZ() {return fZ;}
        Int_t GetMCPoint() {return fMCPoint;}
  private:
    /** Copy constructor **/
    AdvTargetHit(const AdvTargetHit& hit);
    AdvTargetHit operator=(const AdvTargetHit& hit);

    Float_t flag;   ///< flag
    Double_t fX;   ///< fX
    Double_t fY;   ///< fY
    Double_t fZ;   ///< fZ
    Int_t fMCPoint;  ///< fMCPoint

    ClassDef(AdvTargetHit,1);
    

};

#endif
