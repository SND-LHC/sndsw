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
	
	InitMedium("CoilCopper");
	TGeoMedium *Cu = gGeoManager->GetMedium("CoilCopper");
	
	InitMedium("Polystyrene");
    TGeoMedium *Polystyrene = gGeoManager->GetMedium("Polystyrene");

	/*
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
	*/
	
	Double_t fMuonSysPlaneX = conf_floats["AdvMuFilter/MuonSysPlaneX"];
    Double_t fMuonSysPlaneY = conf_floats["AdvMuFilter/MuonSysPlaneY"]; 
    Double_t fCutOffset     = conf_floats["AdvMuFilter/CutOffset"];  
    Double_t fFeX           = conf_floats["AdvMuFilter/FeX"];
    Double_t fFeY           = conf_floats["AdvMuFilter/FeY"];
    Double_t fFeZ           = conf_floats["AdvMuFilter/FeZ"];  
    Double_t fFeGap         = conf_floats["AdvMuFilter/FeGap"];  
    Int_t    fNplanes       = conf_ints["AdvMuFilter/Nplanes"];  
    Double_t fCoilX         = conf_floats["AdvMuFilter/CoilX"];
    Double_t fCoilY         = conf_floats["AdvMuFilter/CoilY"];  
    Double_t fCoilZ         = (fNplanes)*(fFeZ+fFeGap)-fFeGap;
    Double_t fFeYokeX       = (fFeX-fMuonSysPlaneX)/2.;
    Double_t fFeYokeY       = (fFeY-fMuonSysPlaneY-fCoilY)/2.;
    Double_t fFeCutX        = fFeYokeX - fCutOffset;
    Double_t fFeCutY        = fFeYokeY - fCutOffset;
    
    TGeoVolumeAssembly *volAdvMuFilter  = new TGeoVolumeAssembly("volAdvMuFilter");
    
    // Moving the detector forward in the Z direction adding:
    //	-	354.362 which is the last target detector plane midpoint position
    // 	-	3 is the target detector plane half Z-dimension
    //	-	1 is the clearance between volAdvTarget and volAdvMuilter
    
    Double_t fTargetWallX = 50.; //cm
    TVector3 EmWall0_survey(5.35+42.2/2.-(fTargetWallX-42.2)/2., 17.2+42.2/2., 288.92+10/2.+100); // cm

    detector->AddNode(volAdvMuFilter,0,new TGeoTranslation(-2.4244059999999976-EmWall0_survey.X(), 38.3, 354.862+3+1+fFeZ/2.-41.895793+1.)); // hardcoded, try to find and elegant solution
    
    TGeoBBox *FeWall = new TGeoBBox("FeWall", fFeX/2., fFeY/2., fFeZ/2.);
    TGeoBBox *MuonSysPlane = new TGeoBBox("MuonSysPlane", fMuonSysPlaneX/2., fMuonSysPlaneY/2., fFeZ/2.+0.001);
    TGeoBBox *CoilSpace = new TGeoBBox("CoilSpace", fCoilX/2., fCoilY/2.+0.005, fFeZ/2.+0.05);
    TGeoBBox *Coil = new TGeoBBox("Coil", fCoilX/2., fCoilY/2., fCoilZ/2.);
    TGeoBBox *MuonSysDet = new TGeoBBox("MuonSysDet", fMuonSysPlaneX/2., fMuonSysPlaneY/2., fFeGap/2.);

    Double_t cutvers[8][2];
    cutvers[0][0] = 0;
    cutvers[0][1] = 0;
    cutvers[1][0] = 0;
    cutvers[1][1] = -fFeCutY;
    cutvers[2][0] = 0;
    cutvers[2][1] = -fFeCutY;
    cutvers[3][0] = +fFeCutX;
    cutvers[3][1] = 0;

    cutvers[4][0] = 0;
    cutvers[4][1] = 0;
    cutvers[5][0] = 0;
    cutvers[5][1] = -fFeCutY;
    cutvers[6][0] = 0;
    cutvers[6][1] = -fFeCutY;
    cutvers[7][0] = +fFeCutX;
    cutvers[7][1] = 0;
    TGeoArb8 *FeCut = new TGeoArb8("FeCut", fFeZ/2.+0.001, (Double_t *)cutvers);

    TGeoTranslation *CutUpRight = new TGeoTranslation("CutUpRight", -fFeX/2.-0.001, fFeY/2.+0.001, 0);
    CutUpRight->RegisterYourself();
    TGeoCombiTrans *CutDownRight = new TGeoCombiTrans( TGeoTranslation(-fFeX/2.-0.001, -fFeY/2.-0.001, 0), TGeoRotation("rot", 0, 0, 90));
    CutDownRight->SetName("CutDownRight");
    CutDownRight->RegisterYourself();
    TGeoCombiTrans *CutDownLeft = new TGeoCombiTrans(TGeoTranslation(+fFeX/2.+0.001, -fFeY/2.-0.001, 0), TGeoRotation("rot1", 0, 0, 180));
    CutDownLeft->SetName("CutDownLeft");
    CutDownLeft->RegisterYourself();
    TGeoCombiTrans *CutUpLeft = new TGeoCombiTrans(TGeoTranslation(+fFeX/2.+0.001, +fFeY/2.+0.001, 0), TGeoRotation("rot2", 0, 0, -90));
    CutUpLeft->SetName("CutUpLeft");
    CutUpLeft->RegisterYourself();


    TGeoTranslation *CoilUp = new TGeoTranslation("CoilUp", 0, fMuonSysPlaneY/2.+fCoilY/2., 0);
    TGeoTranslation *CoilDown = new TGeoTranslation("CoilDown", 0, -fMuonSysPlaneY/2.-fCoilY/2., 0);
    CoilUp->RegisterYourself();
    CoilDown->RegisterYourself();

    TGeoCompositeShape *MuonSysFe = new TGeoCompositeShape("MuonSysFe", "FeWall-MuonSysPlane-(CoilSpace:CoilUp)-(CoilSpace:CoilDown)-(FeCut:CutUpRight)-(FeCut:CutDownRight)-(FeCut:CutDownLeft)-(FeCut:CutUpLeft)");
    TGeoVolume *volFeWall = new TGeoVolume("volFeWall", MuonSysFe, Fe);
    TGeoVolume *volMagFe = new TGeoVolume("volMagFe", MuonSysPlane, Fe);
    volFeWall->SetLineColor(kGreen-4);
    volMagFe->SetLineColor(kGreen);
    
    //Double_t fField = 1.5; // Tesla
    //fField = fField/10; // kGauss (complying with GEANT3)
    Double_t fField = conf_floats["AdvMuFilter/Field"];
    TGeoUniformMagField *magField = new TGeoUniformMagField(-fField,0, 0);
    TGeoGlobalMagField::Instance()->SetField(magField);
    volMagFe->SetField(magField);

    TGeoVolume *volCoil = new TGeoVolume("volCoil", Coil, Cu);
    volCoil->SetLineColor(kOrange+1);

    TGeoVolume *volMuonSysDet = new TGeoVolume("volMuonSysDet", MuonSysDet, Scint);
    volMuonSysDet->SetLineColor(kGray-2);
    AddSensitiveVolume(volMuonSysDet);

    for(int i = 0; i<fNplanes; i++)
    {
        volAdvMuFilter->AddNode(volFeWall, i, new TGeoTranslation(0, 0, i*(fFeZ+fFeGap)));
        volAdvMuFilter->AddNode(volMagFe, i, new TGeoTranslation(0, 0, i*(fFeZ+fFeGap)));
        if (i == fNplanes-1) continue;
        volAdvMuFilter->AddNode(volMuonSysDet, i, new TGeoTranslation(0, 0, (fFeZ+fFeGap)/2.+i*(fFeZ+fFeGap)));
    }
    volAdvMuFilter->AddNode(volCoil, 0, new TGeoTranslation(0, fMuonSysPlaneY/2.+fCoilY/2., fCoilZ/2.-fFeZ/2.));
    volAdvMuFilter->AddNode(volCoil, 1, new TGeoTranslation(0, -fMuonSysPlaneY/2.-fCoilY/2., fCoilZ/2.-fFeZ/2.));
    
    // Now adding the second part of the spectrometer, TO DO: think about how to implement it in the Magnet.cxx class with all of the shapes.
    // for now just add a 1000 offset to identify Magnet hits from MuFilter ones
    Int_t    fDetIDOffset = 1000;
    Double_t fNplanes2 = 20;
    Double_t fMagnetsGap = 90+2.5; // cm
    Double_t FirstMagZ = fNplanes*(fFeZ+fFeGap);
    Double_t fShortCoilZ = fNplanes2*fFeZ;
    Double_t fSpacing = 8.75; // cm

    TGeoBBox *ShortCoil = new TGeoBBox("ShortCoil", fCoilX/2., fCoilY/2., fShortCoilZ/2.);
    TGeoVolume *volShortCoil = new TGeoVolume("volShortCoil", ShortCoil, Cu);
    volShortCoil->SetLineColor(kOrange+1);


    for(int i = 0; i< 20; i++)
    {
        volAdvMuFilter->AddNode(volFeWall, i+fNplanes, new TGeoTranslation(0, 0, FirstMagZ+fMagnetsGap+i*fFeZ-fFeGap+fSpacing));
        volAdvMuFilter->AddNode(volMagFe, i+fNplanes, new TGeoTranslation(0, 0, FirstMagZ+fMagnetsGap+i*fFeZ-fFeGap+fSpacing));
    }
    volAdvMuFilter->AddNode(volShortCoil, 0, new TGeoTranslation(0, fMuonSysPlaneY/2.+fCoilY/2., fShortCoilZ/2.-fFeZ/2.+fMagnetsGap+FirstMagZ-fFeGap+fSpacing));
    volAdvMuFilter->AddNode(volShortCoil, 1, new TGeoTranslation(0, -fMuonSysPlaneY/2.-fCoilY/2., fShortCoilZ/2.-fFeZ/2.+fMagnetsGap+FirstMagZ-fFeGap+fSpacing));


    // Trackers part
    Double_t fMagTrackerZ = 2.5; // cm Alu tubes diameter
    TGeoBBox *MagTracker = new TGeoBBox("MagTracker", fMuonSysPlaneX/2., fMuonSysPlaneY/2., fMagTrackerZ/2.);
    TGeoVolume *volMagTracker = new TGeoVolume("volMagTracker", MagTracker, Polystyrene);
    volMagTracker->SetLineColor(kGray);
	AddSensitiveVolume(volMagTracker);
	
    volAdvMuFilter->AddNode(volMagTracker, 0+fDetIDOffset, new TGeoTranslation(0, 0, FirstMagZ-fFeZ/2.-fFeGap+fMagTrackerZ/2.));
    volAdvMuFilter->AddNode(volMagTracker, 1+fDetIDOffset, new TGeoTranslation(0, 0, FirstMagZ-fFeZ/2.-fFeGap+fMagnetsGap-fMagTrackerZ/2.));
    volAdvMuFilter->AddNode(volMagTracker, 2+fDetIDOffset, new TGeoTranslation(0, 0, FirstMagZ-fFeZ/2.-fFeGap+fMagnetsGap+fShortCoilZ+fMagTrackerZ/2.+2*fSpacing));
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
