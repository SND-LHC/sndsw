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
#include "ROOT/RVec.hxx"

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

void AdvTargetHit::GetPosition(TVector3& vLeft, TVector3& vRight) {
    if (!gGeoManager) LOG(FATAL) << "Geofile required to get the position of AdvTargetHits.";
    TGeoNavigator* nav = gGeoManager->GetCurrentNavigator();
    auto path = TString::Format("/cave_1/"
                                "Detector_0/"
                                "volAdvTarget_1/"
                                "TrackingStation_%d/"
                                "TrackerPlane_%d/"
                                "SensorModule_%d/"
                                "SensorVolume_%d/"
                                "SLICEY_%d",
                                GetStation(),
                                GetPlane(),
                                GetModule(),
                                GetSensor(),
                                GetStrip());
    nav->cd(path);
    auto *node = nav->GetCurrentNode();
    auto* S = dynamic_cast<TGeoBBox*>(node->GetVolume()->GetShape());
    Double_t short_side = S->GetDY();
    Double_t long_side = S->GetDX();
    Double_t centre[3] = {0, 0, 0};
    Double_t Gcentre[3];
    // Use centre of strip
    Double_t top[3] = {long_side, 0, 0};
    Double_t bot[3] = {-long_side, 0, 0};
    Double_t Gtop[3], Gbot[3];
    if (short_side > 100) {
        LOG(INFO) << nav->GetPath();
        LOG(INFO) << fDetectorID;
    }
    nav->LocalToMaster(top, Gtop);
    nav->LocalToMaster(bot, Gbot);
    vLeft.SetXYZ(Gtop[0], Gtop[1], Gtop[2]);
    vRight.SetXYZ(Gbot[0], Gbot[1], Gbot[2]);
}


// -----   constructor from AdvTargetPoint   ------------------------------------------
AdvTargetHit::AdvTargetHit(Int_t detID, const std::vector<const AdvTargetPoint*> &V)
  : SndlhcHit()
{
     // AdvTarget* AdvTargetDet = dynamic_cast<AdvTarget*> (gROOT->GetListOfGlobals()->FindObject("AdvTarget"));

     // // get parameters from the AdvTarget detector for simulating the digitized information
     // nSiPMs  = AdvTargetDet->GetnSiPMs(detID);
     // if (floor(detID/10000)==3&&detID%1000>59) nSides = AdvTargetDet->GetnSides(detID) - 1;
     // else nSides = AdvTargetDet->GetnSides(detID);

     fDetectorID = detID;
     ROOT::VecOps::RVec<double> _signals;

     for (auto* point : V) {
          fX = point->GetX();
          fY = point->GetY();
          fZ = point->GetZ();
          times[0] = point->GetTime();
          _signals.push_back(point->GetEnergyLoss());
     }

     signals[0] = ROOT::VecOps::Sum(_signals);

     // if (V.size() > 1) {
     //      LOG(WARN) << "Multiple hits for detector ID" << detID;
     // }

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
