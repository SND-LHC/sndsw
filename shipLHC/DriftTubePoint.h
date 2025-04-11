#ifndef SHIPLHC_DRIFTTUBEPOINT_H_
#define SHIPLHC_DRIFTTUBEPOINT_H_

#include "FairMCPoint.h"

#include "TObject.h"
#include "TVector3.h"

class DriftTubePoint : public FairMCPoint {

public:
   /** Default constructor **/
   DriftTubePoint();

   /** Constructor with arguments
    *@param trackID  Index of MCTrack
    *@param detID    Detector ID
    *@param pos      Ccoordinates at entrance to active volume [cm]
    *@param mom      Momentum of track at entrance [GeV]
    *@param tof      Time since event start [ns]
    *@param length   Track length since creation [cm]
    *@param eLoss    Energy deposit [GeV]
    **/
   DriftTubePoint(Int_t trackID, Int_t detID, TVector3 pos, TVector3 mom, Double_t tof, Double_t length, Double_t eLoss,
                  Int_t pdgcode);

   /** Destructor **/
   virtual ~DriftTubePoint();

   /** Output to screen **/
   virtual void Print(const Option_t *opt) const;
   Int_t PdgCode() const { return fPdgCode; }

   /* STLCC
    First digit S: 		system # within the muon filter part of the detector
    Second digit T: 		type of the plane: 0-horizontal plane, 1-vertical plane
    Third digit L: 		determines the layer number (in Z direction)
    Last two digits CC: 	        cell number
   */

   Int_t GetSystem() { return floor(fDetectorID / 10000); }
   Int_t GetStation() { return 1; } // do we need such a method? FIXME
   Int_t GetPlane() { return int(fDetectorID / 1000) % 10; }
   Int_t GetLayer() { return int(fDetectorID % 1000) / 100; }
   Int_t GetCell() { return int(fDetectorID % 100); }
   bool isVertical() { return GetPlane() == 1; }

private:
   /** Copy constructor **/
   Int_t fPdgCode;
   DriftTubePoint(const DriftTubePoint &point);
   DriftTubePoint operator=(const DriftTubePoint &point);

   ClassDef(DriftTubePoint, 1)
};

#endif // SHIPLHC_DRIFTTUBEPOINT_H_
