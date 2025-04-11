#include "DriftTubeHit.h"
#include "DriftTube.h"
#include "TROOT.h"
#include "FairRunSim.h"
#include "TGeoNavigator.h"
#include "TGeoManager.h"
#include "TGeoBBox.h"
#include <TRandom.h>
#include <iomanip>

// -----   Default constructor   -------------------------------------------
DriftTubeHit::DriftTubeHit() : SndlhcHit()
{
   flag = true;
}
// -----   Standard constructor   ------------------------------------------
DriftTubeHit::DriftTubeHit(Int_t detID) : SndlhcHit(detID)
{
   flag = true;
}
// -----   constructor from point class  ------------------------------------------
DriftTubeHit::DriftTubeHit(int detID, std::vector<DriftTubePoint *> V, std::vector<Float_t> W)
{
   DriftTube *DriftTubeDet = dynamic_cast<DriftTube *>(gROOT->GetListOfGlobals()->FindObject("DriftTube"));
   // Float_t timeResol = DriftTubeDet->GetConfParF("DriftTube/timeResol"); // example

   nSides = 1;
   for (unsigned int j = 0; j < 16; ++j) {
      signals[j] = -1;
      times[j] = -1;
   }

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
   std::cout << "-I- DriftTubeHit: DriftTube hit " << " in station " << GetStation();
   if (isVertical()) {
      std::cout << " vertical plane ";
   } else {
      std::cout << " horizontal plane ";
   }
   std::cout << "layer nr " << GetLayer() << " cell nr " << GetCell() << std::endl;
}
// -------------------------------------------------------------------------

ClassImp(DriftTubeHit)
