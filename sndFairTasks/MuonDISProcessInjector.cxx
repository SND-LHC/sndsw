#include "MuonDISProcessInjector.h"
#include "G4MuonMinus.hh"
#include "G4MuonPlus.hh"
#include "G4ProcessManager.hh"
#include "G4MuonDISProcess.hh"
#include "FairLogger.h"
#include <vector>

using std::vector;

MuonDISProcessInjector::MuonDISProcessInjector(char *nucleon, vector<float> x_range, vector<float> y_range,
                                               vector<float> z_range, char *volume)
   : FairTask("MuonDISProcessInjector")
{
   fNucleon = nucleon;
   fVolumeName = volume;
   fXRange = x_range;
   fYRange = y_range;
   fZRange = z_range;
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
