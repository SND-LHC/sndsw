//tool to load hit from emulsion to FEDRA (13 aprile 2018) //updated to work with the new TTreeReader structure (6 March 2019)
//to use it, go in a directory and create the folders b000001/p001 to b000001/p029
//then launch it from the directory mother of b000001

void fromsndsw2FEDRA(TString simfilename, TString geofilename);
void smearing (Float_t &TX, Float_t &TY, const float angres);
bool efficiency(const float emuefficiency);
bool efficiency(const float tantheta, TH1D * emuefficiency);

void set_default(TEnv &cenv){ //setting default parameters, if not presents from file
 cenv.SetValue("FairShip2Fedra.nbricks",20);//5 walls in 2x2 configuration
 cenv.SetValue("FairShip2Fedra.nplates",60);
 //cenv.SetValue("FairShip2Fedra.nevents",10000); // number of events to be passed to FEDRA
 cenv.SetValue("FairShip2Fedra.useefficiencymap",0);
 cenv.SetValue("FairShip2Fedra.emuefficiency",0.85); //only if useefficiency map is set to false
 cenv.SetValue("FairShip2Fedra.dosmearing",1);
 cenv.SetValue("FairShip2Fedra.maxtheta",1.); //angular max of scanning
 cenv.SetValue("FairShip2Fedra.minmomentum",0.1); //do not pass particles beyond this value, track ID would be -2
 cenv.SetValue("FairShip2Fedra.ngrains",70); // to set weight
 cenv.SetValue("FairShip2Fedra.angres",0.003);//used for smearing, if dosmearing = true
 cenv.SetValue("FairShip2Fedra.useLocalBrickCoordinates",1); //local brick coordinates

}

using namespace TMath;
TRandom3 *grandom = new TRandom3(); //creating every time a TRandom3 is a bad idea
TFile *file = NULL;
TH1D *heff = NULL ; //efficiency at different angles
EmulsionDet emureader;
//starting script
void fromsndsw2FEDRA(TString simfilename, TString geofilename){

 //now need also geofile for transfomrations
 gGeoManager->Import(geofilename);
 
 grandom->SetSeed(0);

 TEnv cenv("FairShip2Fedra");
 set_default(cenv);
 cenv.ReadFile("FairShip2Fedra.rootrc" ,kEnvLocal);

 //getting options from file
 //const Int_t nevents = cenv.GetValue("FairShip2Fedra.nevents",10000);
 const int nplates = cenv.GetValue("FairShip2Fedra.nplates",60);
 const int nbricks = cenv.GetValue("FairShip2Fedra.nbricks",20); // to set b00000%i number

 float angres = cenv.GetValue("FairShip2Fedra.angres",0.003); //Used cases: 3, 5milliradians. Constant value overwritten if useresfunction=true
 float minmomentum = cenv.GetValue("FairShip2Fedra.minmomentum",0.1);
 float maxtheta = cenv.GetValue("FairShip2Fedra.maxtheta",1.);
 const float ngrains = cenv.GetValue("FairShip2Fedra.ngrains",70.) ; //the same number for all the couples, so they have the same weigth.
 const float emuefficiency = cenv.GetValue("FairShip2Fedra.emuefficiency",0.85); // flat value
 
 const bool useefficiencymap = cenv.GetValue("FairShip2Fedra.useefficiencymap",0); //use the map instead of the constant value down
 const bool dosmearing = cenv.GetValue("FairShip2Fedra.dosmearing",1); //gaussian smearing or not
 const bool useLocalBrickCoordinates = cenv.GetValue("FairShip2Fedra.useLocalBrickCoordinates",1); //local brick coordinates

 cout<<"Starting conversion with efficiency "<<emuefficiency<<" maxtheta "<<maxtheta<<" and min momentum "<<minmomentum<<endl;
 
 if (useefficiencymap){ 
  file = TFile::Open("efficiency_alltracks.root");
  heff = (TH1D*) file->Get("heff");
 }

 map <int, int> brickindex;
 //building map 
 brickindex[11] = 0;
 brickindex[12] = 1;
 brickindex[13] = 2;
 brickindex[14] = 3;
 brickindex[21] = 4;
 brickindex[22] = 5;
 brickindex[23] = 6;
 brickindex[24] = 7;
 brickindex[31] = 8;
 brickindex[32] = 9;
 brickindex[33] = 10;
 brickindex[34] = 11;
 brickindex[41] = 12;
 brickindex[42] = 13;
 brickindex[43] = 14;
 brickindex[44] = 15;
 brickindex[51] = 16;
 brickindex[52] = 17;
 brickindex[53] = 18;
 brickindex[54] = 19;

 int ihit = 0;
 // ***********************CREATING FEDRA TREES**************************
 //gInterpreter->AddIncludePath("/afs/cern.ch/work/a/aiulian/public/fedra/include");
 EdbCouplesTree *ect[nbricks][nplates]; //2D array->which brick and which plate?
 int brickIDs[20] = {11,12,13,14,21,22,23,24,31,32,33,34,41,42,43,44,51,52,53,54};

 for (int ibrick = 0; ibrick < nbricks; ibrick++){
  for (int i = 1; i <= nplates; i++){
   ect[ibrick][i-1] = new EdbCouplesTree();
   ect[ibrick][i-1]->InitCouplesTree("couples",Form("b0000%02i/p0%02i/%i.%i.0.0.cp.root",brickIDs[ibrick],i,brickIDs[ibrick],i),"RECREATE"); //i learned padding with %0i
  }
 }

 TFile * inputfile = TFile::Open(simfilename.Data());
 if (!inputfile) return;
 
 cout<<"opening file "<<simfilename.Data()<<endl;
 //getting tree and arrays
 TTreeReader reader("cbmsim",inputfile);
 TTreeReaderArray<ShipMCTrack> tracks(reader,"MCTrack");
 TTreeReaderArray<EmulsionDetPoint> emulsionhits(reader,"EmulsionDetPoint");

 int nevents = reader.GetEntries();
 cout<<"Start processing nevents: "<<nevents<<endl;  

 double charge,mass;
 Int_t Flag = 1;
 for (int i = 0; i < nevents; i++){
  if (i%1000==0) cout<<"processing event "<<i<<" out of "<<nevents<<endl;
  reader.SetEntry(i);
  int inECCevent = i;
    
    //loop into MCPoints
    for (const EmulsionDetPoint& emupoint:emulsionhits){   
     bool savehit = true; //by default I save all hits

     float momentum = TMath::Sqrt(pow(emupoint.GetPx(),2) + pow(emupoint.GetPy(),2) + pow(emupoint.GetPz(),2));

        int detID = emupoint.GetDetectorID();
        
        int trackID = emupoint.GetTrackID();
        int motherID;
        if (trackID >= 0) motherID = tracks[trackID].GetMotherId();
        else motherID = -2; //hope I do not see them
     
        int NWall = 0, NRow = 0, NColumn = 0, NPlate = 0;

        emureader.DecodeBrickID(detID, NWall, NRow, NColumn, NPlate); //getting info about brick
        int nbrick = (detID - NPlate)*1e-3;

        int pdgcode = emupoint.PdgCode();

        if ((TDatabasePDG::Instance()->GetParticle(pdgcode))!=NULL){ 
         charge = TDatabasePDG::Instance()->GetParticle(pdgcode)->Charge();
         mass = TDatabasePDG::Instance()->GetParticle(pdgcode)->Mass();
        }
        else{ 
        charge = 0.;
        mass = 0.;
        }
        float kinenergy = TMath::Sqrt(pow(mass,2)+pow(momentum,2)) - mass;

        //first, convert positions
        float xem, yem, tx, ty;
	if (useLocalBrickCoordinates){ //each brick has its own reference system
         double globalpos[3] = {emupoint.GetX(),emupoint.GetY(),emupoint.GetZ()};
         double localpos[3] = {0,0,0};

         emureader.GetLocalPosition(detID, globalpos, localpos);

         xem = localpos[0];
         yem = localpos[1];

         //second, convert angles
         double globalang[3] = {emupoint.GetPx(),emupoint.GetPy(),emupoint.GetPz()};
         double localang[3] = {0,0,0};

         emureader.GetLocalAngles(detID, globalang, localang);
         //angles in TX, TY format
         tx = localang[0]/localang[2];
         ty = localang[1]/localang[2];
        }
	else{ //global reference system as sndsw, but in FEDRA units
     	 xem = emupoint.GetX();
     	 yem = emupoint.GetY();

     	 xem = xem* 1E+4 + 473000;
     	 yem = yem* 1E+4 - 158000;         
     
    	 tx = emupoint.GetPx()/emupoint.GetPz();
    	 ty = emupoint.GetPy()/emupoint.GetPz();  
        }
        float tantheta = pow(pow(tx,2) + pow(ty,2),0.5);

        // *************EXCLUDE HITS FROM BEING SAVED*******************
        if (tantheta > TMath::Tan(maxtheta)) savehit = false; //we scan from theta 0 to a maximum of 1 rad
        if(charge == 0.) savehit = false; //we do not track neutral particles
        if(momentum < minmomentum) savehit = false; //particles with too low kin energy 
            //saving the hits for a plate in the corresponding couples (only one layer saved, the other has ID + 10000)             
 
        if (useefficiencymap){ //efficiency map with angle
            if(!efficiency(tantheta, heff)) savehit = false; //inserting some holes due to emulsion inefficiency           
        }
        //constant value of efficiency
        else if(!efficiency(emuefficiency)) savehit = false;
        if (dosmearing){ 
            smearing(tx,ty,angres);
        }
        if (savehit){          
            int whichbrick = brickindex[nbrick]; //finding index of the array for the brick of our hit;
	        ect[whichbrick][NPlate-1]->eS->Set(ihit,xem,yem,tx,ty,1,Flag);
	        ect[whichbrick][NPlate-1]->eS->SetMC(inECCevent, trackID); //objects used to store MC true information
	        ect[whichbrick][NPlate-1]->eS->SetAid(motherID, 0); //forcing areaID member to store mother MC track information
	        ect[whichbrick][NPlate-1]->eS->SetP(momentum);
	        ect[whichbrick][NPlate-1]->eS->SetVid(pdgcode,0); //forcing viewID[0] member to store pdgcode information
	        ect[whichbrick][NPlate-1]->eS->SetW(ngrains); //need a high weight to do tracking
	        ect[whichbrick][NPlate-1]->Fill();
	        ihit++; //hit entry, increasing as the tree is filled              
        }
     }//end of loop on emulsion points
   } //end of loop on tree
 for (int ibrick = 0; ibrick < nbricks; ibrick++){
  for (int iplate = 0; iplate < nplates; iplate++){
   //ect[nbrick][iplate]->Write();  
   ect[ibrick][iplate]->Close();  
  }
 }
 cout<<"end of script, saving rootrc wih used parameters"<<endl;
 cenv.WriteFile("FairShip2Fedra.save.rootrc");
}
//Auxiliary functions

void smearing (Float_t &TX, Float_t &TY, const float angres){
  float deltaTX = grandom->Gaus(0,angres); //angular resolution, adding a gaussian offset to TX and TY
  float deltaTY = grandom->Gaus(0,angres);
  //cout<<TX<<endl;
  TX = TX + deltaTX;
  TY = TY + deltaTY;
}

bool efficiency(const float emuefficiency){ //for now, just a constant, to be replaced with an efficiency map (probably with the angle)

 float prob = grandom->Uniform(0,1);
 if (prob < emuefficiency) return true; //efficiency larger than probability, we take the event
 else return false;
}

bool efficiency(const float tantheta, TH1D * emuefficiency){ //for now, just a constant, to be replaced with an efficiency map (probably with the angle)

 float prob = grandom->Uniform(0,1);
 int ibin = emuefficiency->FindBin(tantheta); 
 const float efficiency = emuefficiency->GetBinContent(ibin);
 if (prob < efficiency) return true; //efficiency larger than probability, we take the event
 else return false;
}
