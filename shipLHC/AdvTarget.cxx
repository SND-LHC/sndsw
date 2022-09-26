//
//  AdvTarget.cxx
//
//  by D. Centanni
//  Sept 2022
//

#include "AdvTarget.h"
#include "AdvTargetPoint.h"

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

AdvTarget::AdvTarget()
: FairDetector("AdvTarget", "",kTRUE),
  fTrackID(-1),
  fVolumeID(-1),
  fPos(),
  fMom(),
  fTime(-1.),
  fLength(-1.),
  fELoss(-1),
  fAdvTargetPointCollection(new TClonesArray("AdvTargetPoint"))
{
}

AdvTarget::AdvTarget(const char* name, Bool_t Active,const char* Title)
: FairDetector(name, true, kAdvSNDTarget),
  fTrackID(-1),
  fVolumeID(-1),
  fPos(),
  fMom(),
  fTime(-1.),
  fLength(-1.),
  fELoss(-1),
  fAdvTargetPointCollection(new TClonesArray("AdvTargetPoint"))
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
Int_t AdvTarget::InitMedium(const char* name)
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

void AdvTarget::ConstructGeometry()
{
    // Geometry implementation from D. Centanni
    TGeoVolume *top=gGeoManager->GetTopVolume();
    TGeoVolume *detector = gGeoManager->FindVolumeFast("Detector");
    if(!detector)  LOG(ERROR) << "no Detector volume found " ;

    // Materials

    InitMedium("tungstensifon");
	  TGeoMedium *tungsten = gGeoManager->GetMedium("tungstensifon");

    InitMedium("Polystyrene");
    TGeoMedium *Polystyrene = gGeoManager->GetMedium("Polystyrene");

    Double_t fTargetWallX = conf_floats["AdvTarget/TargetWallX"];
    Double_t fTargetWallY = conf_floats["AdvTarget/TargetWallY"];
    Double_t fTargetWallZ = conf_floats["AdvTarget/TargetWallZ"];
    Double_t fTTX = conf_floats["AdvTarget/TTX"];
    Double_t fTTY = conf_floats["AdvTarget/TTY"];
    Double_t fTTZ = conf_floats["AdvTarget/TTZ"];
    Int_t fnTT = conf_ints["AdvTarget/nTT"];

    TGeoBBox *TargetWall = new TGeoBBox("TargetWall", fTargetWallX/2., fTargetWallY/2., fTargetWallZ/2.);
    TGeoBBox *TTracker = new TGeoBBox("TTracker", fTTX/2., fTTY/2., fTTZ/2.);

    TGeoVolume *volTargetWall = new TGeoVolume("volTargetWall", TargetWall, tungsten);
    TGeoVolume *volTTracker = new TGeoVolume("volTTracker", TTracker, Polystyrene);

    volTargetWall->SetLineColor(kYellow-4);
    volTTracker->SetLineColor(kGray);

    //Definition of the target box containing tungsten walls + target trackers (TT) 
    TGeoVolumeAssembly *volAdvTarget  = new TGeoVolumeAssembly("volAdvTarget");

    detector->AddNode(volAdvTarget,1,new TGeoTranslation(-2.4244059999999976,0,0));

    // Positioning calculations, to be deleted once the whole AdvSND apparatus
    // will be positioned correctly
    TVector3 EmWall0_survey(5.35+42.2/2., 17.2+42.2/2., 288.92+10/2.); // cm
    Double_t TargetDiff = 100. - 63.941980;

    AddSensitiveVolume(volTTracker);
    
    // adding walls & trackers
    LOG(INFO) << " nTT: " << fnTT;
    for(int i=0; i<fnTT; i++)
    {
        volAdvTarget->AddNode(volTargetWall, i, new TGeoTranslation(-EmWall0_survey.X()+(fTargetWallX-42.2)/2., EmWall0_survey.Y(), -TargetDiff+EmWall0_survey.Z()+i*(fTargetWallZ+fTTZ)+fTargetWallZ/2.));
        volAdvTarget->AddNode(volTTracker, i, new TGeoTranslation(-EmWall0_survey.X()+(fTargetWallX-42.2)/2., EmWall0_survey.Y(), -TargetDiff+EmWall0_survey.Z()+i*(fTargetWallZ+fTTZ)+fTargetWallZ+fTTZ/2.));
    }
    LOG(INFO) <<"  Target X: "<< -EmWall0_survey.X()+(fTargetWallX-42.2)/2.<<"  Y: "<< EmWall0_survey.Y()<< "  Z: "<< EmWall0_survey.Z();
}

Bool_t  AdvTarget::ProcessHits(FairVolume* vol)
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

  // Create AdvTargetPoint at exit of active volume
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
    LOG(DEBUG)<< "AdvTargetPoint DetID " << detID << " Hit volume name " << name;
    fVolumeID = detID;
    Double_t xmean = (fPos.X()+Pos.X())/2. ;
	Double_t ymean = (fPos.Y()+Pos.Y())/2. ;
	Double_t zmean = (fPos.Z()+Pos.Z())/2. ;
    AddHit(fTrackID, detID,  TVector3(xmean, ymean,  zmean),
			TVector3(fMom.Px(), fMom.Py(), fMom.Pz()), fTime, fLength,
			fELoss, pdgCode);

    // Increment number of det points in TParticle
		ShipStack* stack = (ShipStack*) gMC->GetStack();
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

    FairRootManager::Instance()->Register("AdvTargetPoint", "AdvTarget",
                                            fAdvTargetPointCollection, kTRUE);
}
TClonesArray* AdvTarget::GetCollection(Int_t iColl) const
{
    if (iColl == 0) { return fAdvTargetPointCollection; }
    else { return NULL; }
}

void AdvTarget::Reset()
{
    fAdvTargetPointCollection->Clear();
}

AdvTargetPoint* AdvTarget::AddHit(Int_t trackID, Int_t detID,
                           TVector3 pos, TVector3 mom,
                           Double_t time, Double_t length,
                           Double_t eLoss, Int_t pdgCode)
{
    TClonesArray& clref = *fAdvTargetPointCollection;
    Int_t size = clref.GetEntriesFast();
    return new(clref[size]) AdvTargetPoint(trackID, detID, pos, mom,
                                       time, length, eLoss, pdgCode);
}


ClassImp(AdvTarget)
