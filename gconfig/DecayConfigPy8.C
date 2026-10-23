#include "G4ParticleTable.hh"

void DecayConfigPythia8(){
  // Decay heavy hadrons and taus with Pythia8

  // Particles to skip. These are not in the default Geant4 table.
  std::vector<int> to_skip = {413, 415, 423, 425, 433, 435, 440, 445, 513, 515,
  523, 525, 533, 535, 543, 545, 551, 555, 4114, 4214, 4224, 4324, 4314, 4334,
  4414, 4424, 4434, 4312, 4322, 4412, 4422, 4432, 4444, 5114, 5214, 5224, 5312,
  5322, 5314, 5324, 5334, 5412, 5422, 5414, 5424, 5432, 5434, 5444, 5514, 5524,
  5534, 5544, 5142, 5242, 5342, 5442, 5512, 5522, 5532, 5542, 5554};
  
  // Get particle database
  TDatabasePDG *db= TDatabasePDG::Instance();

  // Create a new external decayer instance
  TPythia8Decayer* decayer = new TPythia8Decayer();
  decayer->Init(); // Currently does nothing, but leave it in case it gets implemented.
  
  TVirtualMC::GetMC()->SetExternalDecayer(decayer);

  // Need the Geant4 Particle table to use Geant4 naming convention
  G4ParticleTable* particleTable = G4ParticleTable::GetParticleTable();
  
  // Set list of particles to decay with external decayer via geant4 command
  std::string g4cmd = "/mcPhysics/setExtDecayerSelection";

  std::cout << "DecayConfigPy8 macro: setting heavy flavour decays via Pythia8" << std::endl;
  // First loop through all particles in database and add heavy flavour
  for (TObject * obj : *(db->ParticleList())) {
    TParticlePDG * p = dynamic_cast<TParticlePDG*>(obj);

    if (!p) {
      std::cout << "DecayConfigPy8 WARNING: got object other than TParticlePDG from particle list" << std::endl;
      continue;
    }
    
    bool isCharm = (strcmp("CharmedMeson", p->ParticleClass()) == 0) or
      (strcmp("CharmedBaryon", p->ParticleClass()) == 0) or
      p->Charm(); // p->Charm() doesn't work. Leave here for future compat.
    bool isBeauty = (strcmp("B-Meson", p->ParticleClass()) == 0) or
      (strcmp("B-Baryon", p->ParticleClass()) == 0) or
      p->Beauty(); // p->Beauty() doesn't work. Leave here for future compat.

    if (isCharm or isBeauty){
      g4cmd += " ";

      // Skip particles Geant4 doesn't know about
      if (std::find(to_skip.begin(), to_skip.end(), abs(p->PdgCode())) != to_skip.end()){
	continue;
      }
      g4cmd += particleTable->FindParticle(p->PdgCode())->GetParticleName();
    }
  }
  
  std::cout << "DecayConfigPy8 macro: setting other decays (short-lived and tau) via Pythia8" << std::endl;
  std::vector<int> pdgs_to_decay_with_pythia8 = {
    // Short-lived resonances, for rare decays.
    221, // eta
    //    223, // omega CRASHES
    //    113, // rho CRASHES
    331, // eta_prime
    //    333, // phi CRASHES
    // Tau
    15, // tau
    -15 // tau bar
  };

  for (int pdg : pdgs_to_decay_with_pythia8){
    g4cmd += " ";
    g4cmd += db->GetParticle(pdg)->GetName();
  }
  dynamic_cast<TGeant4*>(TVirtualMC::GetMC())->ProcessGeantCommand(g4cmd.c_str());
}

void DecayConfig() { DecayConfigPythia8(); }
