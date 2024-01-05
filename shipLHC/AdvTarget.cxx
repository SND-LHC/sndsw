//
//  AdvTarget.cxx
//
//  O. Lantwin and D. Centanni
//  Dec 2023
//

#include "AdvTarget.h"

#include "AdvTargetPoint.h"
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
#include "TClonesArray.h"
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
#include "TString.h"   // for TString
#include "TVector3.h"
#include "TVirtualMC.h"

#include <ROOT/TSeq.hxx>
#include <iosfwd>     // for ostream
#include <iostream>   // for operator<<, basic_ostream,etc
#include <stddef.h>   // for NULL

using ROOT::TSeq;
using namespace ShipUnit;

AdvTarget::AdvTarget()
    : FairDetector("AdvTarget", "", kTRUE)
    , fTrackID(-1)
    , fVolumeID(-1)
    , fPos()
    , fMom()
    , fTime(-1.)
    , fLength(-1.)
    , fELoss(-1)
    , fAdvTargetPointCollection(new TClonesArray("AdvTargetPoint"))
{
}

AdvTarget::AdvTarget(const char *name, Bool_t Active, const char *Title)
    : FairDetector(name, true, kAdvSNDTarget)
    , fTrackID(-1)
    , fVolumeID(-1)
    , fPos()
    , fMom()
    , fTime(-1.)
    , fLength(-1.)
    , fELoss(-1)
    , fAdvTargetPointCollection(new TClonesArray("AdvTargetPoint"))
{
}

AdvTarget::~AdvTarget()
{
   if (fAdvTargetPointCollection) {
      fAdvTargetPointCollection->Delete();
      delete fAdvTargetPointCollection;
   }
}

void AdvTarget::Initialize() { FairDetector::Initialize(); }

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
    InitMedium("silicon");
    TGeoMedium *Silicon = gGeoManager->GetMedium("silicon");

    Double_t fTargetWallX = conf_floats["AdvTarget/TargetWallX"];
    Double_t fTargetWallY = conf_floats["AdvTarget/TargetWallY"];
    Double_t fTargetWallZ = conf_floats["AdvTarget/TargetWallZ"];
    Double_t fTTX = conf_floats["AdvTarget/TTX"];
    Double_t fTTY = conf_floats["AdvTarget/TTY"];
    Double_t fTTZ = conf_floats["AdvTarget/TTZ"];
    Int_t stations = conf_ints["AdvTarget/nTT"];   // Number of TT stations

    TGeoBBox *TargetWall = new TGeoBBox("TargetWall", fTargetWallX / 2., fTargetWallY / 2., fTargetWallZ / 2.);
    TGeoVolume *volTargetWall = new TGeoVolume("volTargetWall", TargetWall, tungsten);
    volTargetWall->SetLineColor(kRed);

    // Silicon tracker module
    //
    // See https://indico.cern.ch/event/1201858/#81-detector-simulation for technical diagrams and renders
    //
    // Passive part
    double module_length = 10 * cm;
    double module_width = 10 * cm;
    TGeoBBox *Support = new TGeoBBox("Support", module_length / 2, module_width / 2, 3.0 * mm / 2);
    TGeoVolume *SupportVolume = new TGeoVolume("SupportVolume", Support, Polystyrene);
    SupportVolume->SetLineColor(kGray);
    // Active part
    double sensor_width = 93.7 * mm;
    double sensor_length = 91.5 * mm;
    double strip_width = 122 * um;
    TGeoBBox *SensorShape = new TGeoBBox("SensorShape", sensor_length / 2, sensor_width / 2, 0.5 * mm / 2);
    TGeoVolume *SensorVolume = new TGeoVolume("SensorVolume", SensorShape, Silicon);
    auto *Strips = SensorVolume->Divide("SLICEY", 2, 768, -sensor_width / 2, strip_width);
    SensorVolume->SetLineColor(kGreen);
    AddSensitiveVolume(Strips);

    double sensor_gap = 3.1 * mm;

    const int rows = 1;
    const int columns = 1;
    const int planes = 2;
    const int sensors = 1;
    const int strips = 768;
    double module_row_gap = 0.5 * mm;
    double module_column_gap = 13.9 * mm;

    // Definition of the target box containing tungsten walls + silicon tracker
    TGeoVolumeAssembly *volAdvTarget = new TGeoVolumeAssembly("volAdvTarget");

    // Positioning calculations, TODO delete once the whole AdvSND apparatus is positioned correctly
    TVector3 EmWall0_survey(5.35 * cm + 42.2 / 2. * cm,
                            17.2 * cm + 42.2 / 2. * cm,
                            288.92 * cm + 10 / 2. * cm);   // Survey position of the centre of the first emulsion wall
    Double_t TargetDiff = 100. * cm - 63.941980 * cm;

    double offset_x = +35 * cm;
    double offset_y = -15 * cm;

    double line_of_sight_offset = -2.4244059999999976 * cm;
    detector->AddNode(volAdvTarget,
                      1,
                      new TGeoTranslation(line_of_sight_offset - EmWall0_survey.X() + (fTargetWallX - 42.2 * cm) / 2.,
                                          EmWall0_survey.Y(),
                                          -TargetDiff + EmWall0_survey.Z()));

    // For correct detector IDs, the geometry has to be built back to front, from the top-level
    for (auto &&station : TSeq(stations)) {
        TGeoVolumeAssembly *TrackingStation = new TGeoVolumeAssembly("TrackingStation");
        // Each tracking station consists of X and Y planes
        for (auto &&plane : TSeq(planes)) {
            TGeoVolumeAssembly *TrackerPlane = new TGeoVolumeAssembly("TrackerPlane");
            int i = 0;
            // Each plane consists of 4 modules
            for (auto &&row : TSeq(rows)) {
                for (auto &&column : TSeq(columns)) {
                    // Each module in turn consists of two sensors on a support
                    TGeoVolumeAssembly *SensorModule = new TGeoVolumeAssembly("SensorModule");
                    SensorModule->AddNode(SupportVolume, 1);
                    for (auto &&sensor : TSeq(sensors)) {
                        int sensor_id = (station << 5) + (plane << 4) + (row << 2) + (column << 1) + sensor;
                        SensorModule->AddNode(
                            SensorVolume, sensor_id, new TGeoTranslation(0, 0, +3 * mm / 2 + 0.5 * mm / 2));
                    }
                    TrackerPlane->AddNode(SensorModule, ++i);
                }
            }
            if (plane == 0) {
                // X-plane
                TrackingStation->AddNode(TrackerPlane, plane);
            } else if (plane == 1) {
                // Y-plane
                TrackingStation->AddNode(
                    TrackerPlane,
                    plane,
                    new TGeoCombiTrans(TGeoTranslation(0, 0, +3.5 * mm + 0.5 * mm), TGeoRotation("y_rot", 0, 0, 90)));
            }
        }

        volAdvTarget->AddNode(
            volTargetWall,
            station,
            new TGeoTranslation(offset_x, offset_y, -fTargetWallZ / 2 + station * fTargetWallZ + station * 7.5 * mm));
        volAdvTarget->AddNode(
            TrackingStation,
            station,
            new TGeoTranslation(
                offset_x, offset_y, (station + 0.5) * fTargetWallZ + (station - 0.5) * 7.5 * mm + 1.5 * mm));
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
        Int_t strip_id = 0;
        Int_t sensor_id = 0;
        gMC->CurrentVolID(strip_id);
        gMC->CurrentVolOffID(1, sensor_id);
        // Check which volume is actually hit and what detID is given
        fVolumeID = (sensor_id << 10) + strip_id;
        Double_t xmean = (fPos.X() + Pos.X()) / 2.;
        Double_t ymean = (fPos.Y() + Pos.Y()) / 2.;
        Double_t zmean = (fPos.Z() + Pos.Z()) / 2.;
        AddHit(fTrackID,
               fVolumeID,
               TVector3(xmean, ymean, zmean),
               TVector3(fMom.Px(), fMom.Py(), fMom.Pz()),
               fTime,
               fLength,
               fELoss,
               pdgCode);

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

AdvTargetPoint *AdvTarget::AddHit(Int_t trackID,
                                  Int_t detID,
                                  TVector3 pos,
                                  TVector3 mom,
                                  Double_t time,
                                  Double_t length,
                                  Double_t eLoss,
                                  Int_t pdgCode)
{
   TClonesArray &clref = *fAdvTargetPointCollection;
   Int_t size = clref.GetEntriesFast();
   return new (clref[size]) AdvTargetPoint(trackID, detID, pos, mom, time, length, eLoss, pdgCode);
}
