//
//  DriftTube.cxx
//
//  S. Ilieva
//  April 2025
//

#include "DriftTube.h"

#include "DriftTubePoint.h"
#include "FairGeoBuilder.h"
#include "FairGeoInterface.h"
#include "FairGeoLoader.h"
#include "FairGeoMedia.h"
#include "FairGeoMedium.h"
#include "FairGeoNode.h"
#include "FairGeoTransform.h"
#include "FairGeoVolume.h"
#include "FairRootManager.h"
#include "FairVolume.h"
#include "ShipDetectorList.h"
#include "ShipStack.h"
#include "ShipUnit.h"
#include "TGeoArb8.h"
#include "TGeoBBox.h"
#include "TGeoCompositeShape.h"
#include "TGeoGlobalMagField.h"
#include "TGeoManager.h"
#include "TGeoMaterial.h"
#include "TGeoMedium.h"
#include "TGeoSphere.h"
#include "TGeoTrd1.h"
#include "TGeoTrd2.h"
#include "TGeoTube.h"
#include "TGeoUniformMagField.h"
#include "TParticle.h"
#include "TString.h" // for TString
#include "TVector3.h"
#include "TVirtualMC.h"

#include <ROOT/TSeq.hxx>
#include <iosfwd>   // for ostream
#include <iostream> // for operator<<, basic_ostream,etc
#include <stddef.h> // for NULL

using ROOT::TSeq;
using namespace ShipUnit;

DriftTube::DriftTube()
   : FairDetector("DriftTube", "", kTRUE), fTrackID(-1), fVolumeID(-1), fEntryPoint(), fMom(), fTime(-1.), fLength(-1.),
     fELoss(-1), fDriftTubePointCollection(new TClonesArray("DriftTubePoint"))
{
}

DriftTube::DriftTube(const char *name, Bool_t Active, const char *Title)
   : FairDetector(name, true, kDriftTube), fTrackID(-1), fVolumeID(-1), fEntryPoint(), fMom(), fTime(-1.), fLength(-1.),
     fELoss(-1), fDriftTubePointCollection(new TClonesArray("DriftTubePoint"))
{
}

DriftTube::~DriftTube()
{
   if (fDriftTubePointCollection) {
      fDriftTubePointCollection->Delete();
      delete fDriftTubePointCollection;
   }
}

void DriftTube::Initialize()
{
   FairDetector::Initialize();
}

// -----   Private method InitMedium
Int_t DriftTube::InitMedium(const char *name)
{
   static FairGeoLoader *geoLoad = FairGeoLoader::Instance();
   static FairGeoInterface *geoFace = geoLoad->getGeoInterface();
   static FairGeoMedia *media = geoFace->getMedia();
   static FairGeoBuilder *geoBuild = geoLoad->getGeoBuilder();

   FairGeoMedium *ShipMedium = media->getMedium(name);

   if (!ShipMedium) {
      Fatal("InitMedium", "Material %s not defined in media file.", name);
      return -1111;
   }
   TGeoMedium *medium = gGeoManager->GetMedium(name);
   if (medium != NULL)
      return ShipMedium->getMediumIndex();
   return geoBuild->createMedium(ShipMedium);
}

void DriftTube::ConstructGeometry()
{
   // Geometry implementation from D. Centanni
   // TGeoVolume *top = gGeoManager->GetTopVolume();//FIXME is it needed
   TGeoVolume *detector = gGeoManager->FindVolumeFast("Detector");
   if (!detector)
      LOG(ERROR) << "no Detector volume found ";

   // Materials

   InitMedium("aluminium");
   TGeoMedium *aluminium = gGeoManager->GetMedium("aluminium");
   InitMedium("steel");
   TGeoMedium *steel = gGeoManager->GetMedium("steel");
   /*InitMedium("gold");
   TGeoMedium *gold = gGeoManager->GetMedium("gold");
   InitMedium("mylar");
   TGeoMedium *mylar = gGeoManager->GetMedium("mylar");*/
   InitMedium("DTGasMixture");
   TGeoMedium *DTGasMixture = gGeoManager->GetMedium("DTGasMixture");

   /*Double_t fX = conf_floats["DriftTube/fX"];
   Double_t fY = conf_floats["DriftTube/fY"];
   Double_t fZ = conf_floats["DriftTube/fZ"];*/
   Double_t fCellWidth = conf_floats["DriftTube/cellWidth"];   // drift cell dims
   Double_t fCellHeight = conf_floats["DriftTube/cellHeight"]; // drift cell dims
   Double_t fCellLength = conf_floats["DriftTube/cellLength"]; // drift cell dims
   Double_t fIBeamThickness = conf_floats["DriftTube/IBeamThickness"];
   Double_t fIBeamWingThickness = conf_floats["DriftTube/IBeamWingThickness"];
   Double_t fIBeamWingWidth = conf_floats["DriftTube/IBeamWingWidth"];
   // Plates separating the active layers
   Double_t fPlateThickness = conf_floats["DriftTube/plateThickness"];
   Double_t fPlateWidth = conf_floats["DriftTube/plateWidth"];
   Double_t fPlateLength = conf_floats["DriftTube/plateLength"];
   // Cover plates
   Double_t fcoverPlateThickness = conf_floats["DriftTube/coverPlateThickness"];
   Double_t fcoverPlateWidth = conf_floats["DriftTube/coverPlateWidth"];
   Double_t fcoverPlateLength = conf_floats["DriftTube/coverPlateLength"];
   Double_t fAnodeRad = conf_floats["DriftTube/anodeRad"];
   // Frame
   Double_t fFrameThickness = conf_floats["DriftTube/frameThickness"];
   Double_t fFrameWidth = conf_floats["DriftTube/frameWidth"];
   Double_t fFrameLength = conf_floats["DriftTube/frameLength"];
   Double_t fFrameHoleThickness = conf_floats["DriftTube/frameHoleThickness"];
   Double_t fFrameHoleWidth = conf_floats["DriftTube/frameHoleWidth"];
   Double_t fFrameHoleLength = conf_floats["DriftTube/frameHoleLength"];
   Double_t fFrameTopThickness =  conf_floats["DriftTube/frameTopThickness"];
   // Side bars
   Double_t fSideBarThickness = conf_floats["DriftTube/sideBarThickness"];
   Double_t fSideBarWidth = conf_floats["DriftTube/sideBarWidth"];
   Double_t fSideBarLength = conf_floats["DriftTube/sideBarLength"];   
   Int_t nPlanes = conf_ints["DriftTube/nPlanes"]; // Number of DT planes
   Int_t nLayers = conf_ints["DriftTube/nLayers"]; // Number of layers per plane
   Int_t nCells = conf_ints["DriftTube/nCells"];   // Number of cells per layer

   // position of XX( e.g. left bottom) edges in survey coordinate system converted to physicist friendly coordinate
   // system
   std::map<int, TVector3> edge_DriftTube;
   edge_DriftTube[1] =
      TVector3(-conf_floats["DriftTube/DT1Dx"], conf_floats["DriftTube/DT1Dz"], conf_floats["DriftTube/DT1Dy"]);
   edge_DriftTube[2] =
      TVector3(-conf_floats["DriftTube/DT2Dx"], conf_floats["DriftTube/DT2Dz"], conf_floats["DriftTube/DT2Dy"]);
   // local position of bottom XX(e.g. horizontal) cell to survey edge
   std::map<int, TVector3> LocCellDT;
   LocCellDT[1] =
      TVector3(-conf_floats["DriftTube/DT1LocX"], conf_floats["DriftTube/DT1LocZ"], conf_floats["DriftTube/DT1LocY"]);
   LocCellDT[2] =
      TVector3(-conf_floats["DriftTube/DT2LocX"], conf_floats["DriftTube/DT2LocZ"], conf_floats["DriftTube/DT2LocY"]);
   // system alignment parameters
   // Double_t fDriftTubeShiftX   = conf_floats["DriftTube/ShiftX"];

   TVector3 displacement;

   // DriftTube layout
   // Define I-beam
   TGeoBBox *beam = new TGeoBBox("beam", fIBeamWingWidth / 2, fCellHeight / 2, fCellLength / 2);
   // Define the half-tube segment for subtraction
   TGeoTubeSeg *tube =
      new TGeoTubeSeg("tube", 0., fCellHeight / 2 - fIBeamWingThickness + 1e-4, fCellLength / 2 + 1e-4, 270., 90.);
   TGeoRotation *rot1 = new TGeoRotation("rot1", 0., 0., 180.);
   TGeoCombiTrans *transRbeam = new TGeoCombiTrans(fIBeamWingWidth / 2 + 1e-4, 0., 0., rot1);
   transRbeam->SetName("transRbeam");
   transRbeam->RegisterYourself();
   TGeoCompositeShape *IbeamShape = new TGeoCompositeShape("IbeamShape", "beam-(tube:transRbeam)");
   TGeoVolume *volIbeam = new TGeoVolume("volIbeam", IbeamShape, aluminium);
   volIbeam->SetLineColor(kGray + 3);
   // Define the sensitive volume
   TGeoBBox *cellEnvelope =
      new TGeoBBox("cellEnvelope", fCellWidth / 2 - 1e-5, fCellHeight / 2 - 1e-5, fCellLength / 2 - 1e-5);
   TGeoCombiTrans *transR = new TGeoCombiTrans((fCellWidth - fIBeamWingWidth) / 2, 0., 0., rot1);
   transR->SetName("transR");
   transR->RegisterYourself();
   TGeoCombiTrans *transL = new TGeoCombiTrans(TGeoTranslation(-(fCellWidth - fIBeamWingWidth) / 2, 0., 0.),
                                               TGeoRotation("rot0", 0., 0., 0.));
   transL->SetName("transL");
   transL->RegisterYourself();
   // Define the anode wire
   TGeoTube *anode = new TGeoTube("anode", 0., fAnodeRad, fCellLength / 2);
   TGeoTranslation *t0 = new TGeoTranslation("t0", 0, 0, 0);
   t0->RegisterYourself();
   // Subract volumes to create the drift tube cell
   TGeoCompositeShape *cellShape =
      new TGeoCompositeShape("cellShape", "cellEnvelope-anode:t0-IbeamShape:transR-IbeamShape:transL");
   TGeoVolume *volGasCell = new TGeoVolume("volGasCell", cellShape, DTGasMixture);
   // Make the cell sensitive
   AddSensitiveVolume(volGasCell);
   volGasCell->SetLineColor(kBlue - 2);
   volGasCell->SetTransparency(50);

   // anode as volume
   TGeoVolume *volAnode = new TGeoVolume("volAnode", anode, steel);
   volAnode->SetLineColor(kViolet);
   // The drift cell = gas + anode
   TGeoVolume *volCell = new TGeoVolumeAssembly("volCell");
   volCell->AddNode(volGasCell, 1);
   volCell->AddNode(volAnode, 2);

   TGeoBBox *IbeamBox = dynamic_cast<TGeoBBox *>(volIbeam->GetShape());
   TGeoBBox *cellBox = dynamic_cast<TGeoBBox *>(volGasCell->GetShape());

   // Define the plates used for covers, support and seperators between layers
   // Endcap frame
   TGeoBBox *halfFrameOuterBox = new TGeoBBox("halfFrameOuterBox", fFrameWidth / 2, fFrameLength / 2, fFrameThickness / 2);
   // Subtract a box to make the frame hollow
   TGeoBBox *halfFrameHole =
      new TGeoBBox("halfFrameHole", fFrameHoleWidth / 2, fFrameHoleLength / 2, fFrameHoleThickness / 2 );
   TGeoBBox *halfFrameTopBox = new TGeoBBox("halfFrameTopBox", fFrameWidth / 2, fFrameLength / 2, fFrameTopThickness / 2);
   TGeoTranslation *t1 = new TGeoTranslation("t1", 0, 0, fFrameThickness/2 + fFrameTopThickness / 2);
   t1->RegisterYourself();
   TGeoCompositeShape *frameShape = new TGeoCompositeShape("frameShape", "halfFrameOuterBox-halfFrameHole:t0+halfFrameTopBox:t1");
   TGeoVolume *volFrame = new TGeoVolume("volFrame", frameShape, aluminium);
   volFrame->SetLineColor(kGray + 4);
   // Side bars
   TGeoBBox *sideBar = new TGeoBBox("sideBar", fSideBarWidth /2 , fSideBarLength /2, fSideBarThickness /2);
   TGeoVolume *volSideBar = new TGeoVolume("volSideBar", sideBar, aluminium);
   volSideBar->SetLineColor(kGray + 5);
   
   // The seperators between layers
   TGeoBBox *plate = new TGeoBBox("plate", fPlateWidth / 2, fPlateThickness / 2, fPlateLength / 2);
   TGeoVolume *volPlate = new TGeoVolume("volPlate", plate, aluminium);
   volPlate->SetLineColor(kGray);
   TGeoBBox *coverPlate =
      new TGeoBBox("coverPlate", fcoverPlateWidth / 2, fcoverPlateThickness / 2, fcoverPlateLength / 2);
   TGeoVolume *volCoverPlate = new TGeoVolume("volCoverPlate", coverPlate, aluminium);
   volCoverPlate->SetLineColor(kGray + 2);


   double DTlayerBox_x{};
   // Arrange the DT planes, layers and cells together
   for (auto &&plane : TSeq(nPlanes)) {
      TGeoVolumeAssembly *volDTplane = new TGeoVolumeAssembly("volDriftTubePlane");
      volDTplane->AddNode(volCoverPlate, 1, new TGeoTranslation(fcoverPlateWidth / 2, fcoverPlateThickness / 2, 0.));
      for (auto &&layer : TSeq(nLayers)) {
         TGeoVolumeAssembly *volDTlayer = new TGeoVolumeAssembly("volLayer");
         for (auto &&cell : TSeq(nCells)) {
            volDTlayer->AddNode(
               volIbeam, 0,
               new TGeoTranslation(IbeamBox->GetDX() + cell * (2 * cellBox->GetDX()), fCellHeight / 2, 0.));
            volDTlayer->AddNode(
               volCell, int(4e4 + plane * 1e3 + layer * 1e2 + cell),
               new TGeoTranslation(cellBox->GetDX() + cell * (cellBox->GetDX() * 2), fCellHeight / 2, 0.));
            volDTlayer->AddNode(
               volIbeam, 0,
               new TGeoCombiTrans(-IbeamBox->GetDX() + cellBox->GetDX() * 2 + cell * (cellBox->GetDX() * 2),
                                  fCellHeight / 2, 0., rot1));
         }
         volDTplane->AddNode(
            volDTlayer, layer,
            new TGeoTranslation( -fcoverPlateWidth/ 2 + nCells*(cellBox->GetDX()+IbeamBox->GetDX())  + (layer+1) % 2 * cellBox->GetDX(),
                                fcoverPlateThickness + layer * (fPlateThickness + 2 * IbeamBox->GetDY()), 0));
         // Add the side bars: one per side of a layer
         for (auto &&side : TSeq(2)) {
            volDTplane->AddNode(
               volSideBar, 0,
               new TGeoCombiTrans(TGeoTranslation( side* fcoverPlateWidth,
                                   fSideBarLength/2 + fcoverPlateThickness + layer * (fPlateThickness + 2 * IbeamBox->GetDY()),
                                   0), 
                                   TGeoRotation("rot_all", 90, 90., 90.)));
         }
         if (layer != nLayers - 1) {
            volDTplane->AddNode(volPlate, 0,
                                new TGeoTranslation(  fcoverPlateWidth/ 2,
                                                    fPlateThickness / 2 + fcoverPlateThickness + 2 * IbeamBox->GetDY() +
                                                       layer * (fPlateThickness + 2 * IbeamBox->GetDY()),
                                                    0));
         }
      }
      volDTplane->AddNode(volCoverPlate, 1,
                          new TGeoTranslation( fcoverPlateWidth / 2,
                                              fcoverPlateThickness / 2 + fcoverPlateThickness + 2 * IbeamBox->GetDY() +
                                                 (nLayers - 1) * (fPlateThickness + 2 * IbeamBox->GetDY()),
                                              0));
      // Add the endcap frame
      volDTplane->AddNode(volFrame, 2,
                          new TGeoTranslation(  fcoverPlateWidth / 2,
				               ( 3 * fPlateThickness + 2 * fcoverPlateThickness + nLayers * 2*IbeamBox->GetDY()) / 2,
                                                 fcoverPlateLength / 2 + fFrameThickness/2 ));
      volDTplane->AddNode(volFrame, 2,
                          new TGeoTranslation(  fcoverPlateWidth / 2,
                                               ( 3 * fPlateThickness + 2 * fcoverPlateThickness + nLayers * 2*IbeamBox->GetDY()) / 2,
                                                - fcoverPlateLength / 2 - fFrameThickness/2 - fFrameTopThickness ));
      displacement = edge_DriftTube[plane + 1] + LocCellDT[plane+1];
      detector->AddNode(volDTplane, plane,
                        new TGeoCombiTrans(TGeoTranslation(displacement.X(), displacement.Y(), displacement.Z()),
                                           TGeoRotation("rot3", -(plane + 1) * 90., 90., 0)));
      // volDriftTube->AddNode(volDTplane, plane, new TGeoCombiTrans( TGeoTranslation(plane*(nCells*cellBox->GetDX()),
      // -plane*fCellLength/2, plane*(-fcoverPlateThickness/2+nLayers*fCellHeight+(nLayers-1)*fPlateThickness)),
      // TGeoRotation("rot3", -(plane+1)*90., 90.,0) ));
   }
}

Bool_t DriftTube::ProcessHits(FairVolume *vol)
{
   /** This method is called from the MC stepping */
   // Set parameters at entrance of volume. Reset ELoss.
   if (gMC->IsTrackEntering()) {
      fELoss = 0.;
      fTime = gMC->TrackTime() * 1.0e09;
      fLength = gMC->TrackLength();
      gMC->TrackPosition(fEntryPoint);
      gMC->TrackMomentum(fMom);
   }
   // Sum energy loss for all steps in the active volume
   fELoss += gMC->Edep();

   // Create DriftTubePoint at exit of active volume
   if (gMC->IsTrackExiting() || gMC->IsTrackStop() || gMC->IsTrackDisappeared()) {
      if (fELoss == 0.) {
         return kFALSE;
      }

      fTrackID = gMC->GetStack()->GetCurrentTrackNumber();

      TParticle *p = gMC->GetStack()->GetCurrentTrack();
      Int_t pdgCode = p->GetPdgCode();
      TLorentzVector exit_point;
      gMC->TrackPosition(exit_point);
      TLorentzVector Mom;
      gMC->TrackMomentum(Mom);
      Int_t detID = 0;
      gMC->CurrentVolID(detID);
      fVolumeID = detID;
      Double_t xmean = (fEntryPoint.X() + exit_point.X()) / 2.;
      Double_t ymean = (fEntryPoint.Y() + exit_point.Y()) / 2.;
      Double_t zmean = (fEntryPoint.Z() + exit_point.Z()) / 2.;
      AddHit(fTrackID, fVolumeID, TVector3(xmean, ymean, zmean), TVector3(fMom.Px(), fMom.Py(), fMom.Pz()), fTime,
             fLength, fELoss, pdgCode);

      // Increment number of det points in TParticle
      ShipStack *stack = (ShipStack *)gMC->GetStack();
      stack->AddPoint(kDriftTube);
   }
   return kTRUE;
}

void DriftTube::GetPosition(Int_t fDetectorID, TVector3 &A, TVector3 &B)
{

   // Alignment to be added!
   int plane = int(fDetectorID / 1000) % 10;
   int layer = int(fDetectorID % 1000) / 100;
   int cell = int(fDetectorID % 100);
   double global_pos[3];
   double local_pos[3] = {0, 0, 0};
   TString path = TString::Format("/cave_1/"
                                  "Detector_0/"
                                  "volDriftTube_0/"
                                  "volDriftTubePlane_%d/"
                                  "volLayer_%d/"
                                  "volCell_%d/",
                                  plane, layer, cell);
   TGeoNavigator *nav = gGeoManager->GetCurrentNavigator();
   if (nav->CheckPath(path)) {
      nav->cd(path);
   } else {
      LOG(FATAL) << path;
   }
   // Get the corresponding node
   TGeoNode *W = nav->GetCurrentNode();
   TGeoBBox *S = dynamic_cast<TGeoBBox *>(W->GetVolume()->GetShape());
   Double_t top_pos[3] = {0, 0, -(S->GetDZ())}; // left
   Double_t bot_pos[3] = {0, 0, S->GetDZ()};    // right
   Double_t global_top_pos[3], global_bot_pos[3];
   nav->LocalToMaster(top_pos, global_top_pos);
   nav->LocalToMaster(bot_pos, global_bot_pos);
   A.SetXYZ(global_top_pos[0], global_top_pos[1], global_top_pos[2]);
   B.SetXYZ(global_bot_pos[0], global_bot_pos[1], global_bot_pos[2]);
}

void DriftTube::EndOfEvent()
{
   fDriftTubePointCollection->Clear();
}

void DriftTube::Register()
{
   /** This will create a branch in the output tree called
       DriftTubePoint, setting the last parameter to kFALSE means:
       this collection will not be written to the file, it will exist
       only during the simulation.
   */

   FairRootManager::Instance()->Register("DriftTubePoint", "DriftTube", fDriftTubePointCollection, kTRUE);
}
TClonesArray *DriftTube::GetCollection(Int_t iColl) const
{
   if (iColl == 0) {
      return fDriftTubePointCollection;
   } else {
      return NULL;
   }
}

void DriftTube::Reset()
{
   fDriftTubePointCollection->Clear();
}

DriftTubePoint *DriftTube::AddHit(Int_t trackID, Int_t detID, TVector3 entrypoint, TVector3 mom, Double_t time,
                                  Double_t length, Double_t eLoss, Int_t pdgCode)
{
   TClonesArray &clref = *fDriftTubePointCollection;
   Int_t size = clref.GetEntriesFast();
   return new (clref[size]) DriftTubePoint(trackID, detID, entrypoint, mom, time, length, eLoss, pdgCode);
}
