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
    AdvTargetHit(Int_t detID, const std::vector<const AdvTargetPoint*>&);

 /** Destructor **/
    virtual ~AdvTargetHit();

    /** Output to screen **/
    void Print() const;
    Float_t GetEnergy();
    bool isValid() const {return flag;}
    Double_t GetX() {return fX;}
    Double_t GetY() {return fY;}
    Double_t GetZ() {return fZ;}
    void GetPosition(TVector3& vLeft, TVector3& vRight);
    int constexpr GetStation() { return floor(fDetectorID >> 15); }
    int constexpr GetPlane() { return int(fDetectorID >> 14) % 2; } // 0 is X-plane, 1 is Y-pane
    int constexpr GetRow() { return int(fDetectorID >> 12) % 4; }
    int constexpr GetColumn() { return int(fDetectorID >> 11) % 2; }
    int constexpr GetSensor() { return int(fDetectorID >> 10); }
    int constexpr GetStrip() { return int(fDetectorID % 1024); }
    int constexpr GetModule() { return 2 * GetRow() + 1 + GetColumn(); }
    bool constexpr isVertical(){
        return GetPlane() == 0;
    };

  private:
    /** Copy constructor **/
    AdvTargetHit(const AdvTargetHit& hit);
    AdvTargetHit operator=(const AdvTargetHit& hit);

    Float_t flag;   ///< flag
    Double_t fX;   ///< fX
    Double_t fY;   ///< fY
    Double_t fZ;   ///< fZ

    ClassDef(AdvTargetHit,2);
    
};

#endif
