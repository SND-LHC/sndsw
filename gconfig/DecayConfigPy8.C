void DecayConfigPythia8(){

  // Decay heavy hadrons and taus with Pythia8
  
  TDatabasePDG *db= TDatabasePDG::Instance();

  // Create a new external decayer instance
  TPythia8Decayer* decayer = new TPythia8Decayer();
  decayer->Init(); // Currently does nothing, but leave it in case it gets implemented.
  
  TVirtualMC::GetMC()->SetExternalDecayer(decayer);

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
      g4cmd += p->GetName();
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
