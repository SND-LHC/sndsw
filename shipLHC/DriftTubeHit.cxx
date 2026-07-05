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
   std::string node = Form("/Detector_0/volDriftTubePlane_%d/volLayer_%d/volCell_%d/volAnode_2", GetPlane(), GetLayer(), GetDetectorID());
   auto navigator = gGeoManager->GetCurrentNavigator();

   DriftTube *DriftTubeDet = dynamic_cast<DriftTube *>(gROOT->GetListOfGlobals()->FindObject("DriftTube"));
   const auto WCELL = static_cast<double>(DriftTubeDet->GetConfParF("DriftTube/cellWidth")); 

   if (gGeoManager->cd(node.c_str())) {
      TGeoShape *shape = gGeoManager->GetCurrentNode()->GetVolume()->GetShape();
      if (shape->InheritsFrom("TGeoBBox")) {
         TGeoBBox *box = dynamic_cast<TGeoBBox *>(shape);
         const Double_t *origin = box->GetOrigin();  

         navigator->cd(node.c_str());
         Double_t localPosition[3] {origin[0] + std::min(static_cast<double>((timestamp - drifttube::tped) * drifttube::vdrift), WCELL * 0.5) * laterality, origin[1], origin[2]};
         Double_t globalPosition[3] {};
         navigator->LocalToMaster(localPosition, globalPosition);

         position.SetXYZ(globalPosition[0], globalPosition[1], globalPosition[2]);
      }
   }

   return position;
}

ClassImp(DriftTubeHit)
