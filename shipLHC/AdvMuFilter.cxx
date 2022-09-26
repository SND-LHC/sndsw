//
//  AdvMuFilter.cxx
//
//  by D. Centanni
//  Sept 2022
//

#include "AdvMuFilter.h"
#include "AdvMuFilterPoint.h"

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

AdvMuFilter::AdvMuFilter()
: FairDetector("AdvMuFilter", "",kTRUE),
  fTrackID(-1),
  fVolumeID(-1),
  fPos(),
  fMom(),
  fTime(-1.),
  fLength(-1.),
  fELoss(-1),
  fAdvMuFilterPointCollection(new TClonesArray("AdvMuFilterPoint"))
{
}

AdvMuFilter::AdvMuFilter(const char* name, Bool_t Active,const char* Title)
: FairDetector(name, true, kAdvSNDMuFilter),
  fTrackID(-1),
  fVolumeID(-1),
  fPos(),
  fMom(),
  fTime(-1.),
  fLength(-1.),
  fELoss(-1),
  fAdvMuFilterPointCollection(new TClonesArray("AdvMuFilterPoint"))
{
}

AdvMuFilter::~AdvMuFilter()
{
    if (fAdvMuFilterPointCollection) {
        fAdvMuFilterPointCollection->Delete();
        delete fAdvMuFilterPointCollection;
    }
}

void AdvMuFilter::Initialize()
{
  FairDetector::Initialize();
}


// -----   Private method InitMedium
Int_t AdvMuFilter::InitMedium(const char* name)
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

void AdvMuFilter::ConstructGeometry()
{
    // Geometry implementation from D. Centanni
    TGeoVolume *top=gGeoManager->GetTopVolume();
    TGeoVolume *detector = gGeoManager->FindVolumeFast("Detector");
    if(!detector)  LOG(ERROR) << "no Detector volume found " ;

    // Materials

    InitMedium("iron");
	TGeoMedium *Fe = gGeoManager->GetMedium("iron");

    InitMedium("polyvinyltoluene");
	TGeoMedium *Scint = gGeoManager->GetMedium("polyvinyltoluene");

    Double_t fWallX = conf_floats["AdvMuFilter/WallX"];
    Double_t fWallY = conf_floats["AdvMuFilter/WallY"];
    Double_t fWallZ = conf_floats["AdvMuFilter/WallZ"];
    Double_t fPlaneX = conf_floats["AdvMuFilter/PlaneX"];
    Double_t fPlaneY = conf_floats["AdvMuFilter/PlaneY"];
    Double_t fPlaneZ = conf_floats["AdvMuFilter/PlaneZ"];
    Int_t fnPlanes = conf_ints["AdvMuFilter/nPlanes"];

    TGeoBBox *MFWall = new TGeoBBox("MFWall", fWallX/2., fWallY/2., fWallZ/2.);
    TGeoBBox *MFPlane = new TGeoBBox("MFPlane", fPlaneX/2., fPlaneY/2., fPlaneZ/2.);

    TGeoVolume *volMFWall = new TGeoVolume("volMFWall", MFWall, Fe);
    TGeoVolume *volMFPlane = new TGeoVolume("volMFPlane", MFPlane, Scint);

    volMFWall->SetLineColor(kGreen-4);
    volMFPlane->SetLineColor(kGray);
    AddSensitiveVolume(volMFPlane);

    //Definition of the target box containing tungsten walls + target trackers (TT) 
    TGeoVolumeAssembly *volAdvMuFilter  = new TGeoVolumeAssembly("volAdvMuFilter");

    detector->AddNode(volAdvMuFilter,1,new TGeoTranslation(-2.4244059999999976,0,0));

    // Positioning calculations, to be deleted once the whole AdvSND apparatus
    // will be positioned correctly
    TVector3 EmWall0_survey(5.35+42.2/2., 17.2+42.2/2., 288.92+10/2.+100); // cm
    Double_t TargetDiff = 100. - 63.941980;
    Double_t fTargetWallX = 100;
    // adding walls & trackers
    LOG(INFO) << " nPlanes: " << fnPlanes;

    for(int i=0; i<fnPlanes; i++)
    {
        volAdvMuFilter->AddNode(volMFWall, i, new TGeoTranslation(-EmWall0_survey.X()+(fTargetWallX-42.2)/2., EmWall0_survey.Y(), -TargetDiff+EmWall0_survey.Z()+i*(fWallZ+fPlaneZ)+fWallZ/2.));
        if(i!=(fnPlanes-1)) volAdvMuFilter->AddNode(volMFPlane, i, new TGeoTranslation(-EmWall0_survey.X()+(fTargetWallX-42.2)/2., EmWall0_survey.Y(), -TargetDiff+EmWall0_survey.Z()+i*(fWallZ+fPlaneZ)+fWallZ+fPlaneZ/2.));
    }
    LOG(INFO) <<"  MuFilter X: "<< -EmWall0_survey.X()+(fTargetWallX-42.2)/2.<<"  Y: "<< EmWall0_survey.Y()<< "  Z: "<< -TargetDiff+EmWall0_survey.Z()+fWallZ/2.;
}

Bool_t  AdvMuFilter::ProcessHits(FairVolume* vol)
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

  // Create AdvMuFilterPoint at exit of active volume
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
    LOG(DEBUG)<< "AdvMuFilterPoint DetID " << detID << " Hit volume name " << name;
    fVolumeID = detID;
    Double_t xmean = (fPos.X()+Pos.X())/2. ;
	Double_t ymean = (fPos.Y()+Pos.Y())/2. ;
	Double_t zmean = (fPos.Z()+Pos.Z())/2. ;
    AddHit(fTrackID, detID,  TVector3(xmean, ymean,  zmean),
			TVector3(fMom.Px(), fMom.Py(), fMom.Pz()), fTime, fLength,
			fELoss, pdgCode);

    // Increment number of det points in TParticle
		ShipStack* stack = (ShipStack*) gMC->GetStack();
		stack->AddPoint(kAdvSNDMuFilter);
    }
        return kTRUE;
}

void AdvMuFilter::EndOfEvent()
{
    fAdvMuFilterPointCollection->Clear();
}

void AdvMuFilter::Register()
{
    /** This will create a branch in the output tree called
     AdvMuFilterPoint, setting the last parameter to kFALSE means:
     this collection will not be written to the file, it will exist
     only during the simulation.
     */

    FairRootManager::Instance()->Register("AdvMuFilterPoint", "AdvMuFilter",
                                            fAdvMuFilterPointCollection, kTRUE);
}
TClonesArray* AdvMuFilter::GetCollection(Int_t iColl) const
{
    if (iColl == 0) { return fAdvMuFilterPointCollection; }
    else { return NULL; }
}

void AdvMuFilter::Reset()
{
    fAdvMuFilterPointCollection->Clear();
}

AdvMuFilterPoint* AdvMuFilter::AddHit(Int_t trackID, Int_t detID,
                           TVector3 pos, TVector3 mom,
                           Double_t time, Double_t length,
                           Double_t eLoss, Int_t pdgCode)
{
    TClonesArray& clref = *fAdvMuFilterPointCollection;
    Int_t size = clref.GetEntriesFast();
    return new(clref[size]) AdvMuFilterPoint(trackID, detID, pos, mom,
                                       time, length, eLoss, pdgCode);
}


ClassImp(AdvMuFilter)