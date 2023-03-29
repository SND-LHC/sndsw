#ifndef ADVTARGETPOINT_H
#define ADVTARGETPOINT_H 1


#include "FairMCPoint.h"

#include "TObject.h"
#include "TVector3.h"

class AdvTargetPoint : public FairMCPoint
{

  public:

    /** Default constructor **/
    AdvTargetPoint();


    /** Constructor with arguments
     *@param trackID  Index of MCTrack
     *@param detID    Detector ID
     *@param pos      Ccoordinates at entrance to active volume [cm]
     *@param mom      Momentum of track at entrance [GeV]
     *@param tof      Time since event start [ns]
     *@param length   Track length since creation [cm]
     *@param eLoss    Energy deposit [GeV]
     **/

    
    AdvTargetPoint(Int_t trackID, Int_t detID, TVector3 pos, TVector3 mom,
		Double_t tof, Double_t length, Double_t eLoss, Int_t pdgCode);

    /** Destructor **/
    virtual ~AdvTargetPoint();

    /** Output to screen **/
    virtual void Print(const Option_t* opt) const;

   Int_t PdgCode() const { return fPdgCode; }
   int GetStation() { return floor(fDetectorID >> 15); }
   int GetPlane() { return int(fDetectorID >> 14) % 2; } // 0 is X-plane, 1 is Y-pane
   int GetRow() { return int(fDetectorID >> 12) % 4; }
   int GetColumn() { return int(fDetectorID >> 11) % 2; }
   int GetSensor() { return int(fDetectorID >> 10) % 2; }
   int GetStrip() { return int(fDetectorID % 768); }

  private:


    Int_t fPdgCode;
    
    /** Copy constructor **/
    AdvTargetPoint(const AdvTargetPoint& point);
    AdvTargetPoint operator=(const AdvTargetPoint& point);
    ClassDef(AdvTargetPoint, 1)

};

#endif
