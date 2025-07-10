#ifndef MUONDISPROCESSINJECTOR_H
#define MUONDISPROCESSINJECTOR_H

#include "FairTask.h"
#include "G4ParticleDefinition.hh"
#include "G4MuonDISProcess.hh"
#include <vector>
#include <map>

class G4PhysicsFreeVector;

class MuonDISProcessInjector : public FairTask {
public:
   MuonDISProcessInjector(char *nucleon, std::vector<float> x_range, std::vector<float> y_range,
                          std::vector<float> z_range, char *volume, char *xsec_filename);
   virtual ~MuonDISProcessInjector() {}

   virtual InitStatus Init();

   char *fNucleon;             //! nucleon type
   char *fVolumeName;          //! geometry volume name to place the DIS
   std::vector<float> fXRange; /// x range to place the DIS
   std::vector<float> fYRange; /// y range to place the DIS
   std::vector<float> fZRange; /// z range to place the DIS
   std::shared_ptr<std::map<int, G4PhysicsFreeVector*>> fXsecTables; /// DIS cross section lookup tables
   const G4ParticleDefinition *muPlus;
   const G4ParticleDefinition *muMinus;
   G4MuonDISProcess *DISProcess;

   ClassDef(MuonDISProcessInjector, 1);
};

#endif
