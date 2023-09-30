//----------------------------------------------------------------------------
//
// Usage: root -l vertexing.C\(nbrick\)
// with nbrick number of the brick which we are analyzing (11,12,13,14,21,22...51,52,53,54)
//----------------------------------------------------------------------------

R__EXTERN Int_t gEDBDEBUGLEVEL;

EdbDataProc  *dproc=0;
EdbPVRec     *gAli=0;
EdbVertexRec *gEVR=0;
EdbDisplay   *ds=0;

int BRICKID = 0;
TH1D *hip = new TH1D("hip","Impact parameters",500,0,5000);

void trvol( const char *def, const char *rcut = "nseg>1" );
void init( const char *def, int iopt,  const char *rcut="1" );
void set_segments_dz(float dz=300.);
void do_propagation();
void do_vertex();
void td();
void sd();
void checkpatterns();
void vd( int trmin, float amin);

#include <vector>
namespace VERTEX_PAR
{
  float DZmax = 3000.;
  float ProbMinV   = 0.0001;  // minimum acceptable probability for chi2-distance between tracks
  float ImpMax     = 15.;    // maximal acceptable impact parameter [microns] (for preliminary check)
  bool  UseMom     = false;  // use or not track momentum for vertex calculations
  bool  UseSegPar  = true;  // use only the nearest measured segments for vertex fit (as Neuchatel)
  int   QualityMode= 0;      // vertex quality estimation method (0:=Prob/(sigVX^2+sigVY^2); 1:= inverse average track-vertex distance)
}

//---------------------------------------------------------------------
void vertexing(int brID = 0, char* dset = 0)
{

  gEDBDEBUGLEVEL=0;
  //trvol(dset);
  //trvol(dset,"nseg>1 && TMath::Abs(s.eY-50000)<5000 && TMath::Abs(s.eX-50000)<5000");   
   BRICKID = brID;
// SELECTION WHICH TRACKS TO USE FOR VERTEXING
   trvol(dset,"nseg>1");
 // trvol(dset,"nseg>1 && s.eX > 62500 && s.eY > 50000"); //fourth quarter
   // reconstruct vertexes starting from linked_tracks.root
}

//---------------------------------------------------------------------
void trvol( const char *def, const char *rcut )
{
  // this function read volume tracks and do the vertex reconstruction
  // from linked_tracks.root

  init(def, 100 ,rcut);                      // read tracks (option 100)
  gAli->FillCell(30,30,0.009,0.009);
  do_vertex();
  //vd(VERTEX_CUTS::nmintracks,VERTEX_CUTS::angleaperture);   // draw reconstructed vertex 
  //td(); //to draw tracks
  //dproc->MakeTracksTree(gAli,"linked_tracks_p.root");
}

//---------------------------------------------------------------------
void init( const char *def, int iopt,  const char *rcut)
{
  if(!def)  dproc = new EdbDataProc();
  else      dproc = new EdbDataProc(def);

  dproc->InitVolume(iopt, rcut);  
  gAli = dproc->PVR();
  if (BRICKID > 0) checkpatterns();
  set_segments_dz(300.);
}

//---------------------------------------------------------------------
void set_segments_dz(float dz)
{
  int np = gAli->Npatterns();
  for(int i=0; i<np; i++) {
    EdbPattern *p = gAli->GetPattern(i);
    int ns = p->N();
    for(int j=0; j<ns; j++) p->GetSegment(j)->SetDZ(dz);
  }
}

void checkpatterns(){
    //check for patterns, if there is one missing add it
    int np = gAli->Npatterns();
    
    TFile *setfile = TFile::Open(Form("b%06d.0.0.0.set.root",BRICKID));
    EdbScanSet *set = (EdbScanSet*) setfile->Get("set");
    
    for(int i=0; i<np; i++) {
        EdbPattern *p = gAli->GetPattern(i);
        if(!p) {
            cout<<"missing pattern "<<i<<" now adding it "<<endl;
            float zmissingPID = set->eB.GetPlate(i)->Z(); //note, set->eB->GetPlate(i) uses the PID, set->GetPlate(i) uses the number of plate, be careful!
            EdbPattern *pat = new EdbPattern( 0., 0.,zmissingPID);
            pat->SetID(i);
            pat->SetScanID(0);
            gAli->AddPatternAt(pat,i);
        }//end if
    } //end for loop
}

//---------------------------------------------------------------------
void do_vertex()
{
  using namespace VERTEX_PAR;

  //gAli->FitTracks(4.,0.139 );
  gEVR = new EdbVertexRec();
  gEVR->eEdbTracks = gAli->eTracks;
  gEVR->eVTX       = gAli->eVTX;
  gEVR->SetPVRec(gAli);

  gEVR->eDZmax=DZmax;
  gEVR->eProbMin=ProbMinV;
  gEVR->eImpMax=ImpMax;
  gEVR->eUseMom=UseMom;
  gEVR->eUseSegPar=UseSegPar;
  gEVR->eQualityMode=QualityMode;

  printf("%d tracks vor vertexing\n",  gEVR->eEdbTracks->GetEntries() );
  int nvtx = gEVR->FindVertex();
  printf("%d 2-track vertexes was found\n",nvtx);

  if(nvtx == 0) return;
  int nadd =  gEVR->ProbVertexN();

  int nl = gEVR->LinkedVertexes(); //should avoid same track associated to multiple vertices
  printf("%d vertices are linked\n",nl);

  EdbDataProc::MakeVertexTree(*(gEVR->eVTX),"vertextree.root");
}

//---------------------------------------------------------------------
void td()
{
  // draw all tracks

  TObjArray *trarr=gAli->eTracks;
  gStyle->SetPalette(1);
  const char *dsname="display-t";
  ds = EdbDisplay::EdbDisplayExist(dsname);
  if(!ds)  ds=new EdbDisplay(dsname,-50000.,50000.,-50000.,50000.,-4000.,80000.);
  ds->SetVerRec(gEVR);
  ds->SetDrawTracks(4);
  ds->SetArrTr( trarr );
  printf("%d tracks to display\n", trarr->GetEntries() );
  ds->Draw();
}

//---------------------------------------------------------------------
void sd()
{
  // draw all tracks and segments (basetracks)

  TObjArray *trarr = gAli->eTracks;
  TObjArray *sarr  = new TObjArray();

  EdbSegP *s=0;
  for(int i=0; i<gAli->Npatterns(); i++) {
    EdbPattern *pat = gAli->GetPattern(i);
    for(int j=0; j<pat->N(); j++) {
      s = pat->GetSegment(j);
      if(s->Track()<0)                  // exclude segments already attached to tracks
	sarr->Add(s);
    }
  }

  printf("%d tracks to display\n",   trarr->GetEntries() );
  printf("%d segments to display\n", sarr->GetEntries()  );

  gStyle->SetPalette(1);
  const char *dsname="display-s";
  ds = EdbDisplay::EdbDisplayExist(dsname);
  if(!ds)  ds=new EdbDisplay(dsname,-50000.,50000.,-50000.,50000.,-4000.,80000.);
  ds->SetVerRec(gEVR);
  ds->SetDrawTracks(4);
  ds->SetArrSegP( sarr );
  ds->SetArrTr( trarr );
  ds->Draw();
}

//---------------------------------------------------------------------
void vd( int trmin, float amin)
{
  // draw vertexes with multiplicity>=trmin, and aperture >= amin

  TObjArray *varr = new TObjArray();
  TObjArray *tarr = new TObjArray();

  EdbVertex *v=0;
  EdbTrackP *t=0;

  int nv = gEVR->Nvtx();
  printf("nv=%d\n",nv);
  if(nv<1) return;

  for(int i=0; i<nv; i++) {
    v = (EdbVertex *)(gEVR->eVTX->At(i));
    if(v->Flag()<0)         continue;
    if( v->N()<trmin )                       continue;
    if( v->N()<3 && v->MaxAperture()<amin )  continue;

    varr->Add(v);
    for(int j=0; j<v->N(); j++) tarr->Add( v->GetTrack(j) );
  }

  gStyle->SetPalette(1);

  const char *dsname="display-v";
  ds = EdbDisplay::EdbDisplayExist(dsname);
  if(!ds)  ds=new EdbDisplay(dsname,-100000.,100000.,-100000.,100000.,-40000., 0.);
  ds->SetVerRec(gEVR);
  ds->SetArrTr( tarr );
  printf("%d tracks to display\n", tarr->GetEntries() );
  ds->SetArrV( varr );
  printf("%d vertex to display\n", varr->GetEntries() );
  ds->SetDrawTracks(4);
  ds->SetDrawVertex(1);
  ds->Draw();
}

// Experimental function to study segment distance to vertex
void add_proton(EdbVertex* vertex, EdbVertexRec* gEVR){
cout<<"start analyzing a vertex"<<endl;
   float track_ipmin = 10000.;
   int ntr  = gEVR->eEdbTracks->GetEntries(); //number of tracks 
//   EdbVTA *vta = 0;
   EdbSegP *potsegment;
   
   bool associated_track; //track near the vertex

   for(int itr=0; itr<ntr; itr++)   {
     associated_track = false;
     EdbTrackP *tr = (EdbTrackP*)(gEVR->eEdbTracks->At(itr));
    
     if (tr->N() > 26){

	float iptrack2vertex = vertex->DistTrack(tr,0.);
	if (iptrack2vertex < track_ipmin) track_ipmin = iptrack2vertex;

	if (iptrack2vertex < 200.) associated_track = true; //this is not the right track

	if (associated_track) cout<<"Let's see each segment for this track, track distance = "<<iptrack2vertex<<endl;

	for (int iseg = 0; iseg < tr->N(); iseg++){ //loop on segments

	   potsegment = (EdbSegP*)tr->GetSegment(iseg);
	   float ipseg2vertex = vertex->DistSeg(potsegment,0.);         
	   if (associated_track) cout<<ipseg2vertex<<" "<<potsegment->Z()<<endl;         	           
	}	
     }   
 if (associated_track) cout<<endl;
   } 
 hip->Fill(track_ipmin);
}
