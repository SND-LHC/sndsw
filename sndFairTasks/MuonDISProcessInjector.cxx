#include "MuonDISProcessInjector.h"
#include "G4MuonMinus.hh"
#include "G4MuonPlus.hh"
#include "G4ProcessManager.hh"
#include "G4MuonDISProcess.hh"
#include "FairLogger.h"
#include "TFile.h"
#include "TGraph.h"
#include <exception>
#include <vector>

using std::vector;

// Convert ROOT's TGraph to G4PhysicsFreeVector
G4PhysicsFreeVector* GraphToFreeVector(TGraph* graph) {
    auto xsec_data = new G4PhysicsFreeVector();
    for (int i = 0; i < graph->GetN(); ++i) {
        double x, y;
        graph->GetPoint(i, x, y);
        xsec_data->InsertValues(x, y);
    }
    return xsec_data;
}

MuonDISProcessInjector::MuonDISProcessInjector(char *nucleon, vector<float> x_range, vector<float> y_range,
                                               vector<float> z_range, char *volume, char *xsec_filename)
   : FairTask("MuonDISProcessInjector")
{
   fNucleon = nucleon;
   fVolumeName = volume;
   fXRange = x_range;
   fYRange = y_range;
   fZRange = z_range;

   TFile* xsec_file = new TFile(xsec_filename);
   if(!xsec_file || xsec_file->IsZombie()){
      LOG(FATAL) << "DIS cross section file not found";
      exit(0);
   }
   TGraph* crsec_muminus = static_cast<TGraph*> (xsec_file->Get(Form("g_13_%s", fNucleon)));
   TGraph* crsec_muplus = static_cast<TGraph*> (xsec_file->Get(Form("g_-13_%s", fNucleon)));
   if (!crsec_muminus || !crsec_muplus){
     LOG(FATAL) << "DIS cross section graphs per " << fNucleon <<" are missing in "<< xsec_filename;
     exit(0);
   }

   fXsecTables = std::make_shared<std::map<int, G4PhysicsFreeVector*>>();
   fXsecTables->insert({13, GraphToFreeVector(crsec_muminus)});
   fXsecTables->insert({-13, GraphToFreeVector(crsec_muplus)});

   LOG(WARNING) << "MuonDISProcessInjector: Setting xyz ranges[cm] for muon DIS\nx: "
                << fXRange[0] << ", " << fXRange[1] << "\ny: " << fYRange[0] << ", " << fYRange[1] << "\nz: "
                << fZRange[0] << ", " << fZRange[1] << "\nand volume name '" << fVolumeName
                << "' and nucleon type " << fNucleon;
}

InitStatus MuonDISProcessInjector::Init()
{
   muMinus = G4MuonMinus::Definition();
   muPlus = G4MuonPlus::Definition();

   DISProcess = new G4MuonDISProcess();

   // Set the nucleon type
   DISProcess->SetNucleonType(fNucleon);
   // Set DIS xsec lookup tables per set nucleon type
   DISProcess->SetCrossSectionTables(fXsecTables);
   // Set the z range of the DIS interaction
   DISProcess->SetRange(&fXRange, &fYRange, &fZRange);
   // Set the name of the geo volume for the DIS interaction
   DISProcess->SetVolume(fVolumeName);

   auto process_man_muminus = muMinus->GetProcessManager();
   if (process_man_muminus) {
      G4int id_muminus = process_man_muminus->AddDiscreteProcess(DISProcess);
   }
   auto process_man_muplus = muPlus->GetProcessManager();
   if (process_man_muplus) {
      G4int id_muplus = process_man_muplus->AddDiscreteProcess(DISProcess);
   }

   LOG(WARNING) << "Adding the external generation for muon DIS using Pythia6";
   return kSUCCESS;
}

ClassImp(MuonDISProcessInjector)
