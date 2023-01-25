//
//  AdvTarget.cxx
//
//  by D. Centanni
//  Sept 2022
//

#include "AdvTarget.h"
#include "AdvTargetPoint.h"

#include "TGeoManager.h"
#include "FairRun.h"       // for FairRun
#include "FairRuntimeDb.h" // for FairRuntimeDb
#include <iosfwd>          // for ostream
#include "TList.h"         // for TListIter, TList (ptr only)
#include "TObjArray.h"     // for TObjArray
#include "TString.h"       // for TString

#include "TGeoBBox.h"
#include "TGeoTrd1.h"
#include "TGeoSphere.h"
#include "TGeoCompositeShape.h"
#include "TGeoTube.h"
#include "TGeoMaterial.h"
#include "TGeoMedium.h"
#include "TGeoTrd1.h"
#include "TGeoArb8.h"
#include "TParticle.h"

#include "TClonesArray.h"
#include "TVirtualMC.h"

#include "FairVolume.h"
#include "FairGeoVolume.h"
#include "FairGeoNode.h"
#include "FairRootManager.h"
#include "FairGeoLoader.h"
#include "FairGeoInterface.h"
#include "FairGeoTransform.h"
#include "FairGeoMedia.h"
#include "FairGeoMedium.h"
#include "FairGeoBuilder.h"
#include "FairRun.h"
#include "FairRuntimeDb.h"

#include "ShipDetectorList.h"
#include "ShipUnit.h"
#include "ShipStack.h"

#include "TGeoTrd2.h"
#include "TGeoCompositeShape.h"

#include "TGeoUniformMagField.h"
#include "TGeoGlobalMagField.h"
#include "TVector3.h"
#include <ROOT/TSeq.hxx>
#include <stddef.h> // for NULL
#include <iostream> // for operator<<, basic_ostream,etc
#include <string.h>

using ROOT::TSeq;
using namespace ShipUnit;

AdvTarget::AdvTarget()
   : FairDetector("AdvTarget", "", kTRUE), fTrackID(-1), fVolumeID(-1), fPos(), fMom(), fTime(-1.), fLength(-1.),
     fELoss(-1), fAdvTargetPointCollection(new TClonesArray("AdvTargetPoint"))
{
}

AdvTarget::AdvTarget(const char *name, Bool_t Active, const char *Title)
   : FairDetector(name, true, kAdvSNDTarget), fTrackID(-1), fVolumeID(-1), fPos(), fMom(), fTime(-1.), fLength(-1.),
     fELoss(-1), fAdvTargetPointCollection(new TClonesArray("AdvTargetPoint"))
{
}

AdvTarget::~AdvTarget()
{
   if (fAdvTargetPointCollection) {
      fAdvTargetPointCollection->Delete();
      delete fAdvTargetPointCollection;
   }
}

void AdvTarget::Initialize()
{
   FairDetector::Initialize();
}

// -----   Private method InitMedium
Int_t AdvTarget::InitMedium(const char *name)
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

void AdvTarget::ConstructGeometry()
{
   // Geometry implementation from D. Centanni
   TGeoVolume *top = gGeoManager->GetTopVolume();
   TGeoVolume *detector = gGeoManager->FindVolumeFast("Detector");
   if (!detector)
      LOG(ERROR) << "no Detector volume found ";

   // Materials

   InitMedium("tungstensifon");
   TGeoMedium *tungsten = gGeoManager->GetMedium("tungstensifon");

   InitMedium("Polystyrene");
   TGeoMedium *Polystyrene = gGeoManager->GetMedium("Polystyrene");
   InitMedium("Silicon");
   TGeoMedium *Silicon = gGeoManager->GetMedium("Silicon");

   Double_t fTargetWallX = conf_floats["AdvTarget/TargetWallX"];
   Double_t fTargetWallY = conf_floats["AdvTarget/TargetWallY"];
   Double_t fTargetWallZ = conf_floats["AdvTarget/TargetWallZ"];
   Double_t fTTX = conf_floats["AdvTarget/TTX"];
   Double_t fTTY = conf_floats["AdvTarget/TTY"];
   Double_t fTTZ = conf_floats["AdvTarget/TTZ"];
   Int_t fnTT = conf_ints["AdvTarget/nTT"]; // Number of TT stations

   TGeoBBox *TargetWall = new TGeoBBox("TargetWall", fTargetWallX / 2., fTargetWallY / 2., fTargetWallZ / 2.);
   TGeoVolume *volTargetWall = new TGeoVolume("volTargetWall", TargetWall, tungsten);
   volTargetWall->SetLineColor(kRed);

   // Silicon tracker module
   //
   // See https://indico.cern.ch/event/1201858/#81-detector-simulation for technical diagrams and renders
   //
   // Passive part
   double module_length = 23.95 * cm;
   double module_width = 12.0 * cm;
   TGeoBBox *Support = new TGeoBBox("Support", module_length / 2, module_width / 2, 3.0 * mm / 2);
   TGeoVolume *SupportVolume = new TGeoVolume("SupportVolume", Support, Polystyrene);
   SupportVolume->SetLineColor(kGray);
   SupportVolume->SetTransparency(100);
   // Active part
   TGeoBBox *Sensor = new TGeoBBox("Sensor", 91.5 * mm / 2, 93.7 * mm / 2, 0.5 * mm / 2);
   TGeoVolume *SensorVolume = new TGeoVolume("SensorVolume", Sensor, Silicon);
   SensorVolume->SetLineColor(kGreen);
   AddSensitiveVolume(SensorVolume);
   // Combine into module
   TGeoVolumeAssembly *SensorModule = new TGeoVolumeAssembly("SensorModule");
   SensorModule->AddNode(SupportVolume, 1);
   SensorModule->AddNode(
      SensorVolume, 1,
      new TGeoTranslation(-module_length / 2 + 46.95 * mm + 91.5 * mm / 2, 0, +3 * mm / 2 + 0.5 * mm / 2));
   double sensor_gap = 3.1 * mm;
   SensorModule->AddNode(SensorVolume, 2,
                         new TGeoTranslation(-module_length / 2 + 46.95 * mm + 1.5 * 91.5 * mm + sensor_gap, 0,
                                             +3 * mm / 2 + 0.5 * mm / 2));
   // Combine modules into planes
   TGeoVolumeAssembly *TrackerPlane = new TGeoVolumeAssembly("TrackerPlane");
   const int rows = 4;
   const int columns = 2;
   double module_row_gap = 0.5 * mm;
   double module_column_gap = 13.9 * mm;
   int i = 0;
   for (auto &&row : TSeq(rows)) {
      for (auto &&column : TSeq(columns)) {
         TrackerPlane->AddNode(
            SensorModule, ++i,
            new TGeoCombiTrans(
               // Offset modules as needed by row and column
               TGeoTranslation((column % 2 ? 1 : -1) * (module_length / 2 + module_column_gap / 2),
                               (row - 1) * (-module_width - module_row_gap) - module_row_gap / 2 + module_width / 2, 0),
               // Rotate every module of the second column
               TGeoRotation(TString::Format("rot%d", i), 0, 0, column * 180)));
      }
   }
   // Combine into tracking stations of X and Y planes
   TGeoVolumeAssembly *TrackingStation = new TGeoVolumeAssembly("TrackingStation");
   // X-plane
   TrackingStation->AddNode(TrackerPlane, 0);
   // Y-plane
   TrackingStation->AddNode(
      TrackerPlane, 1,
      new TGeoCombiTrans(TGeoTranslation(0, 0, +3.5 * mm + 0.5 * mm), TGeoRotation("y_rot", 0, 0, 90)));

   // Definition of the target box containing tungsten walls + silicon tracker
   TGeoVolumeAssembly *volAdvTarget = new TGeoVolumeAssembly("volAdvTarget");


   // Positioning calculations, TODO delete once the whole AdvSND apparatus is positioned correctly
   TVector3 EmWall0_survey(5.35 * cm + 42.2 / 2. * cm, 17.2 * cm + 42.2 / 2. * cm, 288.92 * cm + 10 / 2. * cm); // Survey position of the centre of the first emulsion wall
   Double_t TargetDiff = 100. * cm - 63.941980 * cm;

   double line_of_sight_offset = -2.4244059999999976 * cm;
   detector->AddNode(volAdvTarget, 1, new TGeoTranslation(
                        line_of_sight_offset - EmWall0_survey.X() + (fTargetWallX - 42.2 * cm) / 2.,
                        EmWall0_survey.Y(),
                        -TargetDiff + EmWall0_survey.Z()
                     ));

   // adding walls & trackers
   LOG(INFO) << " nTT: " << fnTT;
   for (auto &&wall : TSeq(fnTT)) {
      volAdvTarget->AddNode(
         volTargetWall, wall,
         new TGeoTranslation(
            0, 0, -fTargetWallZ / 2 + wall * fTargetWallZ + wall * 7.5 * mm));
      volAdvTarget->AddNode(TrackingStation, wall,
                            new TGeoTranslation(0, 0,
                                                (wall + 0.5) * fTargetWallZ +
                                                (wall - 0.5) * 7.5 * mm));
   }
}

Bool_t AdvTarget::ProcessHits(FairVolume *vol)
{
   /** This method is called from the MC stepping */
   // Set parameters at entrance of volume. Reset ELoss.
   if (gMC->IsTrackEntering()) {
      fELoss = 0.;
      fTime = gMC->TrackTime() * 1.0e09;
      fLength = gMC->TrackLength();
      gMC->TrackPosition(fPos);
      gMC->TrackMomentum(fMom);
   }
   // Sum energy loss for all steps in the active volume
   fELoss += gMC->Edep();

   // Create AdvTargetPoint at exit of active volume
   if (gMC->IsTrackExiting() || gMC->IsTrackStop() || gMC->IsTrackDisappeared()) {
      if (fELoss == 0.) {
         return kFALSE;
      }

      fTrackID = gMC->GetStack()->GetCurrentTrackNumber();

      TParticle *p = gMC->GetStack()->GetCurrentTrack();
      Int_t pdgCode = p->GetPdgCode();
      TLorentzVector Pos;
      gMC->TrackPosition(Pos);
      TLorentzVector Mom;
      gMC->TrackMomentum(Mom);
      Int_t detID = 0;
      gMC->CurrentVolID(detID);
      const char *name;
      name = gMC->CurrentVolName();
      // Check which volume is actually hit and what detID is given
      LOG(DEBUG) << "AdvTargetPoint DetID " << detID << " Hit volume name " << name;
      fVolumeID = detID;
      Double_t xmean = (fPos.X() + Pos.X()) / 2.;
      Double_t ymean = (fPos.Y() + Pos.Y()) / 2.;
      Double_t zmean = (fPos.Z() + Pos.Z()) / 2.;
      AddHit(fTrackID, detID, TVector3(xmean, ymean, zmean), TVector3(fMom.Px(), fMom.Py(), fMom.Pz()), fTime, fLength,
             fELoss, pdgCode);

      // Increment number of det points in TParticle
      ShipStack *stack = (ShipStack *)gMC->GetStack();
      stack->AddPoint(kAdvSNDTarget);
   }
   return kTRUE;
}

void AdvTarget::EndOfEvent()
{
   fAdvTargetPointCollection->Clear();
}

void AdvTarget::Register()
{
   /** This will create a branch in the output tree called
       AdvTargetPoint, setting the last parameter to kFALSE means:
       this collection will not be written to the file, it will exist
       only during the simulation.
   */

   FairRootManager::Instance()->Register("AdvTargetPoint", "AdvTarget", fAdvTargetPointCollection, kTRUE);
}
TClonesArray *AdvTarget::GetCollection(Int_t iColl) const
{
   if (iColl == 0) {
      return fAdvTargetPointCollection;
   } else {
      return NULL;
   }
}

void AdvTarget::Reset()
{
   fAdvTargetPointCollection->Clear();
}

AdvTargetPoint *AdvTarget::AddHit(Int_t trackID, Int_t detID, TVector3 pos, TVector3 mom, Double_t time,
                                  Double_t length, Double_t eLoss, Int_t pdgCode)
{
   TClonesArray &clref = *fAdvTargetPointCollection;
   Int_t size = clref.GetEntriesFast();
   return new (clref[size]) AdvTargetPoint(trackID, detID, pos, mom, time, length, eLoss, pdgCode);
}
