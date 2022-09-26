//
//  Magnet.cxx
//
//  by A. Escalante, D. Centanni
//  Aug 2022
//

#include "Magnet.h"
#include "MagnetPoint.h"

#include "TGeoManager.h"
#include "FairRun.h"                    // for FairRun
#include "FairRuntimeDb.h"              // for FairRuntimeDb
#include <iosfwd>                    // for ostream
#include "TList.h"                      // for TListIter, TList (ptr only)
#include "TObjArray.h"                  // for TObjArray
#include "TString.h"                    // for TString

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
#include <stddef.h>                     // for NULL
#include <iostream>                     // for operator<<, basic_ostream,etc
#include <string.h>

using std::cout;
using std::endl;

using namespace ShipUnit;

Magnet::Magnet()
: FairDetector("Magnet", "",kTRUE),
  fTrackID(-1),
  fVolumeID(-1),
  fPos(),
  fMom(),
  fTime(-1.),
  fLength(-1.),
  fELoss(-1),
  fMagnetPointCollection(new TClonesArray("MagnetPoint"))
{
}

Magnet::Magnet(const char* name, Bool_t Active,const char* Title)
: FairDetector(name, true, kSNDMagnet),
  fTrackID(-1),
  fVolumeID(-1),
  fPos(),
  fMom(),
  fTime(-1.),
  fLength(-1.),
  fELoss(-1),
  fMagnetPointCollection(new TClonesArray("MagnetPoint"))
{
}

Magnet::~Magnet()
{
    if (fMagnetPointCollection) {
        fMagnetPointCollection->Delete();
        delete fMagnetPointCollection;
    }
}

void Magnet::Initialize()
{
  FairDetector::Initialize();
}


// -----   Private method InitMedium
Int_t Magnet::InitMedium(const char* name)
{
  static FairGeoLoader *geoLoad=FairGeoLoader::Instance();
  static FairGeoInterface *geoFace=geoLoad->getGeoInterface();
  static FairGeoMedia *media=geoFace->getMedia();
  static FairGeoBuilder *geoBuild=geoLoad->getGeoBuilder();
    
  FairGeoMedium *ShipMedium=media->getMedium(name);
    
  if (!ShipMedium)
    {
      Fatal("InitMedium","Material %s not defined in media file.", name);
      return -1111;
    }
  TGeoMedium* medium=gGeoManager->GetMedium(name);
  if (medium!=NULL)
    return ShipMedium->getMediumIndex();
  return geoBuild->createMedium(ShipMedium);
}

void Magnet::ConstructGeometry()
{
    // Geometry implementation from D. Centanni

    TGeoVolume *top=gGeoManager->GetTopVolume();
    TGeoVolume *detector = gGeoManager->FindVolumeFast("Detector");
    if(!detector)  LOG(ERROR) << "no Detector volume found " ;

    // Materials

    InitMedium("air");
    TGeoMedium *air =gGeoManager->GetMedium("air");  

    InitMedium("iron");
    TGeoMedium *Fe =gGeoManager->GetMedium("iron");

    InitMedium("Polystyrene");
    TGeoMedium *Polystyrene = gGeoManager->GetMedium("Polystyrene");

    // MAGNET STRUCTURE
    Double_t fInMagX = conf_floats["Magnet/InMagX"];
    Double_t fInMagY = conf_floats["Magnet/InMagY"];
    Double_t fIronYokeX = conf_floats["Magnet/IronYokeX"];
    Double_t fIronYokeY = conf_floats["Magnet/IronYokeY"];
    Double_t fCoilX = conf_floats["Magnet/CoilX"];
    Double_t fCoilY = conf_floats["Magnet/CoilY"];
    Double_t fOutMagX = conf_floats["Magnet/OutMagX"];
    Double_t fOutMagY = conf_floats["Magnet/OutMagY"];
    Double_t fMagZ = conf_floats["Magnet/MagZ"];

    // MAGNETIC FIELD
    Double_t fField = conf_floats["Magnet/Field"];

    // TRACKING STATIONS STRUCTURE
    Double_t fTrackerZ = conf_floats["Magnet/TrackerZ"];
    Double_t fTSpacingZ = conf_floats["Magnet/TSpacingZ"];
    Double_t fLevArm = conf_floats["Magnet/LevArm"];

	// Shifts
  	Double_t fShiftX     = conf_floats["Magnet/ShiftX"];
  	Double_t fShiftY     = conf_floats["Magnet/ShiftY"];
  	Double_t fShiftZ     = conf_floats["Magnet/ShiftZ"];

    // Magnetic Field
    LOG(DEBUG)<< "Magnetic field set is " << fField/10 << " Tesla";
    TGeoUniformMagField *magField = new TGeoUniformMagField(-fField,0, 0);
    TGeoGlobalMagField::Instance()->SetField(magField);

    TGeoVolumeAssembly *MagnetVol = new TGeoVolumeAssembly("Magnet");


    // Shapes creation
    TGeoBBox *CoilContainer = new TGeoBBox("CoilContainer", fOutMagX/2., fOutMagY/2., fMagZ/2.);
    TGeoBBox *MagRegion = new TGeoBBox("MagRegion", fInMagX/2., fInMagY/2., fMagZ/2.+0.5);
    TGeoBBox *Coil = new TGeoBBox("Coil", fCoilX/2., fCoilY/2., fMagZ/2.+0.5);

    // Translations
    TGeoTranslation *CoilUpPos = new TGeoTranslation("CoilUpPos", 0, (fInMagY+fCoilY)/2.-0.001, 0);
    TGeoTranslation *CoilDownPos = new TGeoTranslation("CoilDownPos", 0, -(fInMagY+fCoilY)/2.+0.001, 0);
    CoilUpPos->RegisterYourself();
    CoilDownPos->RegisterYourself();

    // Yoke shape
    TGeoCompositeShape *FeYoke = new TGeoCompositeShape("FeYoke", "CoilContainer-MagRegion-(Coil:CoilUpPos)-(Coil:CoilDownPos)");

    // Volumes
    TGeoVolume *volFeYoke = new TGeoVolume("volFeYoke", FeYoke, Fe);
    volFeYoke->SetLineColor(kGray);
    TGeoVolume *volCoil = new TGeoVolume("volCoil", Coil, Cu);
    volCoil->SetLineColor(kOrange+1);
    TGeoVolume *volMagRegion = new TGeoVolume("volMagRegion", MagRegion, air);
    volMagRegion->SetField(magField);

    // Positioning
    MagnetVol->AddNode(volFeYoke, 0);
    MagnetVol->AddNode(volCoil, 0, new TGeoTranslation(0, (fInMagY+fCoilY)/2., 0));
    MagnetVol->AddNode(volCoil, 1, new TGeoTranslation(0, -(fInMagY+fCoilY)/2., 0));
    MagnetVol->AddNode(volMagRegion, 0, 0);


    TGeoBBox *TrackPlane = new TGeoBBox("TrackPlane", fInMagX/2., fInMagY/2., fTrackerZ/2.);

    // TO BE PROPERLY ADDED
    TGeoVolume *volTrackPlane = new TGeoVolume("volTrackPlane", TrackPlane, Polystyrene); // add medium
    volTrackPlane->SetLineColorAlpha(kBlue, 0.4);
    AddSensitiveVolume(volTrackPlane);

    MagnetVol->AddNode(volTrackPlane, 1, new TGeoTranslation(0, 0, -fMagZ/2.-fTSpacingZ-fTrackerZ/2.));
    MagnetVol->AddNode(volTrackPlane, 2, new TGeoTranslation(0, 0, +fMagZ/2.+fTSpacingZ+fTrackerZ/2.));
    MagnetVol->AddNode(volTrackPlane, 0, new TGeoTranslation(0, 0, -fMagZ/2.-fTSpacingZ-fTrackerZ-fLevArm-fTrackerZ/2.));
    MagnetVol->AddNode(volTrackPlane, 3, new TGeoTranslation(0, 0, +fMagZ/2.+fTSpacingZ+fTrackerZ+fLevArm+fTrackerZ/2.));

    detector->AddNode(MagnetVol, 0, new TGeoTranslation(fShiftX,fShiftY,fShiftZ)); // see Alberto implementation
}

Bool_t  Magnet::ProcessHits(FairVolume* vol)
{
  /** This method is called from the MC stepping */
  //Set parameters at entrance of volume. Reset ELoss.
  if ( gMC->IsTrackEntering() ) {
    fELoss  = 0.;
    fTime   = gMC->TrackTime() * 1.0e09;
    fLength = gMC->TrackLength();
    gMC->TrackPosition(fPos);
    gMC->TrackMomentum(fMom);
  }
  // Sum energy loss for all steps in the active volume
  fELoss += gMC->Edep();

  // Create vetoPoint at exit of active volume
  if (  gMC->IsTrackExiting()    ||
        gMC->IsTrackStop()       ||
        gMC->IsTrackDisappeared()   ) {
        if (fELoss == 0. ) {return kFALSE; }

    fTrackID  = gMC->GetStack()->GetCurrentTrackNumber();

    TParticle* p=gMC->GetStack()->GetCurrentTrack();
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
    LOG(DEBUG)<< "MagnetPoint DetID " << detID << " Hit volume name " << name;
    fVolumeID = detID;
    Double_t xmean = (fPos.X()+Pos.X())/2. ;
	Double_t ymean = (fPos.Y()+Pos.Y())/2. ;
	Double_t zmean = (fPos.Z()+Pos.Z())/2. ;
    AddHit(fTrackID, detID,  TVector3(xmean, ymean,  zmean),
			TVector3(fMom.Px(), fMom.Py(), fMom.Pz()), fTime, fLength,
			fELoss, pdgCode);

    // Increment number of det points in TParticle
		ShipStack* stack = (ShipStack*) gMC->GetStack();
		stack->AddPoint(kSNDMagnet);
    }

    return kTRUE;
}

void Magnet::EndOfEvent()
{
    fMagnetPointCollection->Clear();
}

void Magnet::Register()
{
    /** This will create a branch in the output tree called
     MagnetPoint, setting the last parameter to kFALSE means:
     this collection will not be written to the file, it will exist
     only during the simulation.
     */

    FairRootManager::Instance()->Register("MagnetPoint", "Magnet",
                                            fMagnetPointCollection, kTRUE);
}

TClonesArray* Magnet::GetCollection(Int_t iColl) const
{
    if (iColl == 0) { return fMagnetPointCollection; }
    else { return NULL; }
}

void Magnet::Reset()
{
    fMagnetPointCollection->Clear();
}

MagnetPoint* Magnet::AddHit(Int_t trackID, Int_t detID,
                           TVector3 pos, TVector3 mom,
                           Double_t time, Double_t length,
                           Double_t eLoss, Int_t pdgCode)
{
    TClonesArray& clref = *fMagnetPointCollection;
    Int_t size = clref.GetEntriesFast();
    return new(clref[size]) MagnetPoint(trackID, detID, pos, mom,
                                       time, length, eLoss, pdgCode);
}


ClassImp(Magnet)
