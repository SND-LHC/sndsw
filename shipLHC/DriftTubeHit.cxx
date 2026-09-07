#include "DriftTubeHit.h"
#include "DriftTube.h"
#include "TROOT.h"
#include "FairRunSim.h"
#include "TGeoNavigator.h"
#include "TGeoManager.h"
#include "TGeoBBox.h"
#include "DriftTubeConstants.h"
#include <TRandom.h>
#include <iomanip>

// -----   Default constructor   -------------------------------------------
DriftTubeHit::DriftTubeHit() 
   : TObject(),
    fDetectorID(-1)
{
}
// -----   Standard constructor   ------------------------------------------
DriftTubeHit::DriftTubeHit(Int_t detID) 
   : TObject(),
   fDetectorID(detID)
{
   flag = true;
}
DriftTubeHit::DriftTubeHit(Int_t detID, const Double_t& m_timestamp) : TObject(), fDetectorID(detID), timestamp(m_timestamp)
{
   flag = true;
}

// -----   constructor from point class  ------------------------------------------
DriftTubeHit::DriftTubeHit(int detID, std::vector<DriftTubePoint *> V, std::vector<Float_t> W)
{
   DriftTube *DriftTubeDet = dynamic_cast<DriftTube *>(gROOT->GetListOfGlobals()->FindObject("DriftTube"));
   Float_t timeResol = DriftTubeDet->GetConfParF("DriftTube/timeResol"); // example

   for (auto p = std::begin(V); p != std::end(V); ++p) {

      Double_t signal = (*p)->GetEnergyLoss();
      // Find the distance from MCPoint to the center of cell (the anode)
      TVector3 vLeft, vRight;
      TVector3 impact((*p)->GetX(), (*p)->GetY(), (*p)->GetZ());
      DriftTubeDet->GetPosition(detID, vLeft, vRight);
      Double_t distance = (vLeft - impact).Perp(); // transverse component

      // for the timing - what to do?
      Double_t ptime = (*p)->GetTime();
   }
   // what needs to be set: distance? time?

   LOG(DEBUG) << "signal created";
}

// -----   Destructor   ----------------------------------------------------
DriftTubeHit::~DriftTubeHit() {}
// -------------------------------------------------------------------------

// -----   Public method Print   -------------------------------------------
void DriftTubeHit::Print()
{
   std::cout << "-I- DriftTubeHit: DriftTube hit " << " in station " << GetPlane();
   if (isVertical()) {
      std::cout << " vertical plane ";
   } else {
      std::cout << " horizontal plane ";
   }
   std::cout << "layer nr " << GetLayer() << " cell nr " << GetCell() << std::endl;
}
// -------------------------------------------------------------------------

TVector3 DriftTubeHit::GetPosition() { 
   TVector3 position {};
   int plane = GetPlane();
   int layer = GetLayer();
   std::string layer_str = std::to_string(layer);
   std::string node = Form("/Detector_0/volDriftTubePlane_%d/volLayer_%d/volCell_%d/volAnode_2", plane, layer, GetDetectorID());
   auto navigator = gGeoManager->GetCurrentNavigator();

   DriftTube *DriftTubeDet = dynamic_cast<DriftTube *>(gROOT->GetListOfGlobals()->FindObject("DriftTube"));
   const auto WCELL = static_cast<double>(DriftTubeDet->GetConfParF("DriftTube/cellWidth")); 

   //extract alignment parameter from the geometry
   Float_t dx =0., dy = 0.;
   TString orientation;
   if (plane==0)
   {
     orientation = "Y";
     dy = DriftTubeDet->GetConfParF("DriftTube/YdyL"+layer_str);
   }
   else
   {
     orientation = "X";
     dx = DriftTubeDet->GetConfParF("DriftTube/XdxL"+layer_str);
   }
   Float_t RotX = DriftTubeDet->GetConfParF("DriftTube/"+orientation+"RotX");
   Float_t RotY = DriftTubeDet->GetConfParF("DriftTube/"+orientation+"RotY");
   Float_t RotZ = DriftTubeDet->GetConfParF("DriftTube/"+orientation+"RotZL"+layer_str);

   if (gGeoManager->cd(node.c_str())) {
      TGeoShape *shape = gGeoManager->GetCurrentNode()->GetVolume()->GetShape();
      if (shape->InheritsFrom("TGeoBBox")) {
         TGeoBBox *box = dynamic_cast<TGeoBBox *>(shape);
         const Double_t *origin = box->GetOrigin();  

         navigator->cd(node.c_str());
         Double_t localPosition[3] {origin[0] + std::min(static_cast<double>((timestamp - drifttube::tped) * drifttube::vdrift), WCELL * 0.5) * laterality, origin[1], origin[2]};
         Double_t loc[3] = {0,0,0};
         localPosition[0] -= (dx+dy); // dx or dy is zero for the respective non-measured coordinate
         loc[0] = localPosition[0];
         loc[1] = localPosition[1] + RotY*localPosition[0];
         loc[2] = localPosition[2] - (RotZ + RotX)*localPosition[0];

         Double_t globalPosition[3] {};
         navigator->LocalToMaster(loc, globalPosition);

        position.SetXYZ(globalPosition[0], globalPosition[1], globalPosition[2]);
      }
   }

   return position;
}

ClassImp(DriftTubeHit)
