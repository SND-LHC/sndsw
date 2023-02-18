#include "AdvTargetHit.h"
#include "AdvTarget.h"
#include "TROOT.h"
#include "FairRunSim.h"
#include "TGeoNavigator.h"
#include "TGeoManager.h"
#include "TGeoBBox.h"
#include <TRandom.h>
#include <iomanip>
#include "ShipUnit.h"

// -----   Default constructor   -------------------------------------------
AdvTargetHit::AdvTargetHit()
  : SndlhcHit()
{
 flag = true;
}
// -----   Standard constructor   ------------------------------------------
AdvTargetHit::AdvTargetHit(Int_t detID)
  : SndlhcHit(detID)
{
 flag = true;
}


// -----   constructor from AdvTargetPoint   ------------------------------------------
AdvTargetHit::AdvTargetHit(Int_t detID, std::vector<AdvTargetPoint*> V)
  : SndlhcHit()
{
     AdvTarget* AdvTargetDet = dynamic_cast<AdvTarget*> (gROOT->GetListOfGlobals()->FindObject("AdvTarget"));
     // // get parameters from the AdvTarget detector for simulating the digitized information
     // nSiPMs  = AdvTargetDet->GetnSiPMs(detID);
     // if (floor(detID/10000)==3&&detID%1000>59) nSides = AdvTargetDet->GetnSides(detID) - 1;
     // else nSides = AdvTargetDet->GetnSides(detID);

     fDetectorID = detID;

     Float_t position_resolution = 35 * ShipUnit::um;

     for (auto* point : V) {
          auto plane = point->GetPlane();
          if (plane == 0) {
               fX = point->GetX() + position_resolution * gRandom->Gaus();
               fY = -500;
          } else if (plane == 1) {
               fX = -500;
               fY = point->GetY() + position_resolution * gRandom->Gaus();
          }
          fZ = point->GetZ();
     }

     if (V.size() > 1) {
          LOG(WARN) << "Multiple hits for detector ID" << detID;
     }

     flag = true;

     // Float_t timeResol = AdvTargetDet->GetConfParF("AdvTarget/timeResol");

     // Float_t attLength=0;
     // Float_t siPMcalibration=0;
     // Float_t siPMcalibrationS=0;
     // Float_t propspeed =0;
     // if (floor(detID/10000)==3) {
     //          if (nSides==2){attLength = AdvTargetDet->GetConfParF("AdvTarget/DsAttenuationLength");}
     //          else                    {attLength = AdvTargetDet->GetConfParF("AdvTarget/DsTAttenuationLength");}
     //          siPMcalibration = AdvTargetDet->GetConfParF("AdvTarget/DsSiPMcalibration");
     //          propspeed = AdvTargetDet->GetConfParF("AdvTarget/DsPropSpeed");
     // }
     // else {
     //          attLength = AdvTargetDet->GetConfParF("AdvTarget/VandUpAttenuationLength");
     //          siPMcalibration = AdvTargetDet->GetConfParF("AdvTarget/VandUpSiPMcalibration");
     //          siPMcalibrationS = AdvTargetDet->GetConfParF("AdvTarget/VandUpSiPMcalibrationS");
     //          propspeed = AdvTargetDet->GetConfParF("AdvTarget/VandUpPropSpeed");
     // }

     // for (unsigned int j=0; j<16; ++j){
     //    signals[j] = -1;
     //    times[j]    =-1;
     // }
     // LOG(DEBUG) << "detid "<<detID<< " size "<<nSiPMs<< "  side "<<nSides;

     // fDetectorID  = detID;
     // Float_t signalLeft    = 0;
     // Float_t signalRight = 0;
     // Float_t earliestToAL = 1E20;
     // Float_t earliestToAR = 1E20;
     // for(auto p = std::begin(V); p!= std::end(V); ++p) {

     //    Double_t signal = (*p)->GetEnergyLoss();

     //  // Find distances from MCPoint centre to ends of bar
     //    TVector3 vLeft,vRight;
     //    TVector3 impact((*p)->GetX(),(*p)->GetY() ,(*p)->GetZ() );
     //    AdvTargetDet->GetPosition(fDetectorID,vLeft, vRight);
     //    Double_t distance_Left    =  (vLeft-impact).Mag();
     //    Double_t distance_Right =  (vRight-impact).Mag();
     //    signalLeft+=signal*TMath::Exp(-distance_Left/attLength);
     //    signalRight+=signal*TMath::Exp(-distance_Right/attLength);

     //  // for the timing, find earliest particle and smear with time resolution
     //    Double_t ptime    = (*p)->GetTime();
     //    Double_t t_Left    = ptime + distance_Left/propspeed;
     //    Double_t t_Right = ptime + distance_Right/propspeed;
     //    if ( t_Left <earliestToAL){earliestToAL = t_Left ;}
     //    if ( t_Right <earliestToAR){earliestToAR = t_Right ;}
     // }
     // // shortSiPM = {3,6,11,14,19,22,27,30,35,38,43,46,51,54,59,62,67,70,75,78};
     // for (unsigned int j=0; j<nSiPMs; ++j){
     //    if (j==3 or j==6){
     //       signals[j] = signalRight/float(nSiPMs) * siPMcalibrationS;   // most simplest model, divide signal individually. Small SiPMS special
     //       times[j] = gRandom->Gaus(earliestToAL, timeResol);
     //    }else{
     //       signals[j] = signalRight/float(nSiPMs) * siPMcalibration;   // most simplest model, divide signal individually.
     //       times[j] = gRandom->Gaus(earliestToAL, timeResol);
     //    }
     //    if (nSides>1){
     //        signals[j+nSiPMs] = signalLeft/float(nSiPMs) * siPMcalibration;   // most simplest model, divide signal individually.
     //        times[j+nSiPMs] = gRandom->Gaus(earliestToAR, timeResol);
     //    }
     // }
     // flag = true;
     // for (Int_t i=0;i<16;i++){fMasked[i]=kFALSE;}
     // LOG(DEBUG) << "signal created";
}

// Constructor from AdvTargetPoint
AdvTargetHit::AdvTargetHit(Int_t detID, AdvTargetPoint* point, Int_t index)
     : SndlhcHit(), fMCPoint{index} {
     fDetectorID = detID;
     Float_t position_resolution = 35 * ShipUnit::um;
     auto plane = point->GetPlane();
     if (plane == 0) {
          fX = point->GetX() + position_resolution * gRandom->Gaus();
          fY = -500;
     } else if (plane == 1) {
          fX = -500;
          fY = point->GetY() + position_resolution * gRandom->Gaus();
     }
     fZ = point->GetZ();
     flag = true;
}

// -----   Destructor   ----------------------------------------------------
AdvTargetHit::~AdvTargetHit() { }
// -------------------------------------------------------------------------

// -----   Public method Print   -------------------------------------------
void AdvTargetHit::Print() const
{
  std::cout << "-I- AdvTargetHit: AdvTarget hit " << " in detector " << fDetectorID;

  if ( floor(fDetectorID/10000)==3&&fDetectorID%1000>59) {
     std::cout << " with vertical bars"<<std::endl;
     std::cout << "top digis:";
     for (unsigned int j=0; j<nSiPMs; ++j){
         std::cout << signals[j] <<" ";
     }
  }else{
     std::cout << " with horizontal bars"<<std::endl;
     for (unsigned int s=0; s<nSides; ++s){
       if (s==0) {std::cout << "left digis:";}
       else {std::cout << "right digis:";}
       for (unsigned int j=0; j<nSiPMs; ++j){
         std::cout << signals[j] <<" ";
      }
     }
 }
std::cout << std::endl;
}
