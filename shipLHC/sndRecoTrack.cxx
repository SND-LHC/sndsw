#include "sndRecoTrack.h"
#include "Scifi.h"
#include "MuFilter.h"
#include "ShipUnit.h"
#include "TROOT.h"
#include "TMatrixD.h"
#include "TMath.h"
#include "TGraph.h"
#include "TF1.h"
#include "StateOnPlane.h"  /* genfit */
#include "FitStatus.h"
#include "RKTrackRep.h"
#include "DetPlane.h"
#include "ConstField.h"
#include "FieldManager.h"
#include "TGeoMaterialInterface.h"
#include "MaterialEffects.h"

using namespace genfit;
using namespace std;

//tests
#include <iostream>

/* Constructor from genfit::Track object */
sndRecoTrack::sndRecoTrack(Track* track)
{
   FitStatus* fitStatus = track->getFitStatus();
   chi2  = fitStatus->getChi2();
   Ndf   = fitStatus->getNdf();
   fFlag = fitStatus->isFitConverged();
      
   if (fFlag)
   {
      for ( auto i = 0; i < track->getNumPoints(); i++ )
      {
          auto state = track->getFittedState(i);
          fTrackPoints.push_back(state.getPos());
          fRawMeasDetID.push_back(track->getPointWithMeasurement(i)->getRawMeasurement()->getDetId());
          if (i ==0)
          {
             fTrackMom = state.getMom();
             start = state.getPos();
          }
          if (i == track->getNumPoints()-1)
             stop = state.getPos();
      }
   }
   // defaults
   fTrackType = 0;
   fRawMeasTimes = {};
}

pair<int, float> sndRecoTrack::TrackDirection()
{
   /* Don't use this function for now. It works ok with MC,
      not the case for data.. Work in progress. 
      Provide track direction with a probability level
      based on timing measurements of hits/clusters.
      What follows is based on a note by C.Vilela.
      Reference T0 is 1st meas in z
      1st item in the returned pair is: 
      1 = along Z axis; -1 = reverse Z direction
   */

   // Account for signal propagation in detectors
   vector<float> corr_times = getCorrTimes();

   MuFilter *MuFilterDet = dynamic_cast<MuFilter*> (gROOT->GetListOfGlobals()->FindObject("MuFilter") );
   Scifi *ScifiDet = dynamic_cast<Scifi*> (gROOT->GetListOfGlobals()->FindObject("Scifi") );
   double resol{}, resol_0{};
   int size = fTrackPoints.size();
   TMatrixD Tres1(size-1,1), Tres2(size-1,1);
   TMatrixD Sigma(size-1, size-1);
   // Extract time meas. resolution of the reference T0 measurement
   if (fRawMeasDetID[0] >= 100000) {
         resol_0 = ScifiDet->GetConfParF("Scifi/timeResol");
   }
   else resol_0 = MuFilterDet->GetConfParF("MuFilter/timeResol");
   // Set time residual vectors and covariance matrix
   for ( int i = 0; i < size-1; i++){
       Tres1[i][0] = corr_times[i+1] - corr_times[0]
                     -(fTrackPoints[i+1]-fTrackPoints[0]).Mag()/ShipUnit::c_light;
       Tres2[i][0] = corr_times[0] - corr_times[i+1]
                     -(fTrackPoints[i+1]-fTrackPoints[0]).Mag()/ShipUnit::c_light;
       //tests
       /*cout<<i<<" deltaT "<<corr_times[i+1] - corr_times[0]<<" dL "
             << (fTrackPoints[i+1]-fTrackPoints[0]).Mag()
       <<" dL/c "<<  (fTrackPoints[i+1]-fTrackPoints[0]).Mag()/ShipUnit::c_light<<endl;*/
       for ( int j = 0; j < size-1; j++){
           if (i == j) {
              if (fRawMeasDetID[i+1] >= 100000) {
                 resol = ScifiDet->GetConfParF("Scifi/timeResol");
              }
              else resol = MuFilterDet->GetConfParF("MuFilter/timeResol");
              Sigma[i][j] = pow(resol_0,2)+pow(resol,2);
           }
           else Sigma[i][j] = pow(resol_0,2);
       }
   }
   TMatrixD SigmaInv(TMatrixD::kInverted,Sigma);
   TMatrixD Tres1_T(TMatrixD::kTransposed,Tres1);
   //tests
   //Tres1_T.Print();
   TMatrixD Tres2_T(TMatrixD::kTransposed,Tres2);
   TMatrixT<double> LambdaB1 = Tres1_T*SigmaInv*Tres1;
   TMatrixT<double> LambdaB2 = Tres2_T*SigmaInv*Tres2;
   // tests
   /*cout<<LambdaB1[0][0]<<" "<<LambdaB2[0][0]
       <<" "<<LambdaB2[0][0]-LambdaB1[0][0]
       <<" "<<TMath::Prob(LambdaB1[0][0], size-1)
       <<" "<<TMath::Prob(LambdaB2[0][0], size-1)<<endl;*/
   if (LambdaB2[0][0]-LambdaB1[0][0] >= 0)
      return make_pair(1, TMath::Prob(LambdaB1[0][0], size-1));
   else return make_pair(-1, TMath::Prob(LambdaB2[0][0], size-1));
}

pair<float, float> sndRecoTrack::Velocity()
{
   /* Extract particle velocity based on timing
      measurements as slope of linear fit of (dL,dT)
      Reference T0 is 1st measurement in z */

   // Account for signal propagation in detectors
   vector<float> corr_times = getCorrTimes();

   MuFilter *MuFilterDet = dynamic_cast<MuFilter*> (gROOT->GetListOfGlobals()->FindObject("MuFilter") );
   Scifi *ScifiDet = dynamic_cast<Scifi*> (gROOT->GetListOfGlobals()->FindObject("Scifi") );
   TGraph gr;
   double resol{}, delta_T{};
   for (int i = 1;  i < fTrackPoints.size(); i++){
     if (fRawMeasDetID[i] >= 100000) {
         resol = ScifiDet->GetConfParF("Scifi/timeResol");
     }
     else resol = MuFilterDet->GetConfParF("MuFilter/timeResol");
     delta_T = corr_times[i]-corr_times[0];
     // Use current track point measurement only if time difference is above resolution
     if (fabs(delta_T) < resol*sqrt(2)) continue;
     gr.AddPoint((fTrackPoints[i]-fTrackPoints[0]).Mag(), delta_T);
   }
   TF1 line("line", "pol1");
   // A single entry in the graph - no fit
   if (gr.GetN() < 2)
     return make_pair(9999.,999.);
   else
     gr.Fit("line", "SQ");
     // slope of fit is 1/v
     return make_pair(1./line.GetParameter(1),line.GetParError(1)/pow(line.GetParameter(1),2));
}

tuple<float, float, float> sndRecoTrack::trackDir()
{
   /* Based on the same function in SndlhcTracking.py!
      Extract direction based on timing
      measurements as slope of linear fit of (dL,dT'),
      where dT' features time of flight estimation.
      Use a nominal first position in z */

   // Account for signal propagation in detectors
   vector<float> corr_times = getCorrTimes();

   TGraph gr;
   double dist{}, delta_T{};

   double firstScifi_z = 300*ShipUnit::cm;
   TVector3 pos(start);
   TVector3 mom(fTrackMom);
   double lam = (firstScifi_z - pos.z())/mom.z();
   // nominal first position
   TVector3 pos1(pos.x()+lam*mom.x(), pos.y()+lam*mom.y(), firstScifi_z);

   for (int i = 0;  i < fTrackPoints.size(); i++){
     dist= (fTrackPoints[i]-pos1).Mag();
     gr.AddPoint(dist, corr_times[i] - dist/ShipUnit::c_light);
   }
   TF1 line("line", "pol1");
   gr.Fit("line", "SQ");
   return make_tuple(line.GetParameter(1),
                    line.GetParameter(1)/(line.GetParError(1)+1E-13), line.GetParameter(0));
}

TVector3 sndRecoTrack::extrapolateToPlaneAtZ(float z)
{   
   TVector3 NewPosition = TVector3(0., 0., z);
   /* line below assumes that plane in global coordinates
      is perpendicular to z-axis, which is not true for TI18 geometry. */
   TVector3 parallelToZ = TVector3(0., 0., 1.);
   auto plane = std::make_shared<genfit::DetPlane>(NewPosition, parallelToZ);
   RKTrackRep rep = RKTrackRep(13);
   StateOnPlane state = StateOnPlane(&rep);
   // find index(!) of track point closest in z
   float zMin = 9999;
   int index;
   for ( int i =0; i < fTrackPoints.size(); i++ ){
      if ( fabs(zMin - fTrackPoints[i].Z()) < zMin ) index = i;
   }
   TVector3 Closest_pos = fTrackPoints[index];
   rep.setPosMom(state, Closest_pos, fTrackMom);
   rep.extrapolateToPlane(state, plane);

   return state.getPos();
}

vector<float> sndRecoTrack::getCorrTimes()
{
   /* Account for signal propagation in detectors */
   vector<float> corr_times{};
   MuFilter *MuFilterDet = dynamic_cast<MuFilter*> (gROOT->GetListOfGlobals()->FindObject("MuFilter") );
   Scifi *ScifiDet = dynamic_cast<Scifi*> (gROOT->GetListOfGlobals()->FindObject("Scifi") );
   TVector3 A, B, X{};
   float scintVel, L;
   float mean{}, fastest=999.;
   for ( int i = 0;  i < fTrackPoints.size(); i++ ){
      // First get readout coordinates and parameters
      if (fRawMeasDetID[i] >= 100000){
         ScifiDet->GetSiPMPosition(fRawMeasDetID[i],A,B);
         scintVel = ScifiDet->GetConfParF("Scifi/signalSpeed");
      }
      else {
         MuFilterDet->GetPosition(fRawMeasDetID[i],A,B);
         // DS
         if (floor(fRawMeasDetID[i]/10000) == 3){
            L = MuFilterDet->GetConfParF("MuFilter/DownstreamBarX");
            scintVel = MuFilterDet->GetConfParF("MuFilter/DsPropSpeed");
         }
         // US and Veto
         else scintVel = MuFilterDet->GetConfParF("MuFilter/VandUpPropSpeed");
      }
      // calculate distance btw track point and SiPM
      // vertical detector elements
      if ( (fRawMeasDetID[i] >= 100000 && int(fRawMeasDetID[i]/100000)%10 == 1) or
           (fRawMeasDetID[i] < 100000  && floor(fRawMeasDetID[i]/10000) == 3 
                                       && fRawMeasDetID[i]%1000 > 59) ) X = B-fTrackPoints[i];
      else X = A - fTrackPoints[i];
      // Then, get calibrated hit times
      if (fRawMeasDetID[i] >= 100000) {
         corr_times.push_back(ScifiDet->GetCorrectedTime(fRawMeasDetID[i], fRawMeasTimes[i][0], 0) - X.Mag()/scintVel);
      }
      else { 
         mean = 0;
         fastest=999.;
         for (int ch = 0, N_channels = fRawMeasTimes[i].size(); ch <N_channels; ch++ ){
             // For the moment only DS is time calibrated
             if (floor(fRawMeasDetID[i]/10000) == 3){
                // vertical bars or DS clusters
                if ( N_channels==1 ){
                    corr_times.push_back(MuFilterDet->GetCorrectedTime(fRawMeasDetID[i], ch, fRawMeasTimes[i][ch], 0) - X.Mag()/scintVel);
                }
                // horizontal bar - take mean of the L/R hit times
                else{
                     mean += MuFilterDet->GetCorrectedTime(fRawMeasDetID[i], ch, fRawMeasTimes[i][ch], 0);
                     if (ch == N_channels-1) corr_times.push_back(mean/N_channels- L/scintVel/2.);
                }
             }
             // US and Veto - no time calibration yet, just use fastest hit time for now
             else{
                  fastest = min(fastest, fRawMeasTimes[i][ch]);
                  if (ch == N_channels-1) corr_times.push_back(fastest - X.Mag()/scintVel);
             }
         }
      }
      //cout<<"i "<<i<<" raw tdc "<<fRawMeasTimes[i]<<" corr t "<<corr_times.back()<<" "<< X.Mag()<<endl;
   }

   return corr_times;
}

