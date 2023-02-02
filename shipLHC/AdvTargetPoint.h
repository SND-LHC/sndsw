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
    

    Int_t PdgCode() const {return fPdgCode;}
    int GetStation(){return floor(fDetectorID/10000);}
    int GetPlane(){return floor(fDetectorID/1000)%10;} // 0 is X-plane, 1 is Y-pane
    int GetRow(){return floor(fDetectorID/100)%10;}
    int GetColumn(){return floor(fDetectorID/10)%10;}
    int GetSensor(){return floor(fDetectorID%10);}

  private:


    Int_t fPdgCode;
    
    /** Copy constructor **/
    AdvTargetPoint(const AdvTargetPoint& point);
    AdvTargetPoint operator=(const AdvTargetPoint& point);
    ClassDef(AdvTargetPoint, 1)

};

#endif
