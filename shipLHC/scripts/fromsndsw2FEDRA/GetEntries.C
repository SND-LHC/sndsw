int GetEntries(int nbrick, int nplate){
 TFile *cpfile = TFile::Open(Form("p%03i/%i.%i.0.0.cp.root",nplate,nbrick,nplate));
 TTree *couples = (TTree*) cpfile->Get("couples");
 
 return couples->GetEntries();
}


int GetEntries(int nbrick){
 const int nplates = 60;
 for (int iplate = 1; iplate <= nplates; iplate++){
   int ncouples = GetEntries(nbrick,iplate);
   if (ncouples == 0) return 0;
  } //end of loop, all ok
 return 1;
}
