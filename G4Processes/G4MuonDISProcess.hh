#pragma once

#ifndef G4MuonDISProcess_h
#define G4MuonDISProcess_h

#include "G4VDiscreteProcess.hh"
#include "G4Track.hh"
#include "G4Step.hh"
#include "G4ParticleDefinition.hh"
#include "G4VParticleChange.hh"
#include "G4ThreeVector.hh"
#include "globals.hh"
#include <tuple>
#include <vector>
#include <map>

class MuDISGenerator;
class G4Navigator;
class G4PhysicsFreeVector;

class G4MuonDISProcess : public G4VDiscreteProcess {
public:
   /** Constructors **/
   G4MuonDISProcess(const G4String &name = "muonDIS");
   virtual ~G4MuonDISProcess() {}

   /** Called to check if this process should apply to a given particle **/
   virtual G4bool IsApplicable(const G4ParticleDefinition &particle) override;

   virtual G4double PostStepGetPhysicalInteractionLength(const G4Track &track, G4double previousStepSize,
                                                         G4ForceCondition *condition) override;

   /**  Called to calculate the mean free path **/
   virtual G4double
   GetMeanFreePath(const G4Track &track, G4double previousStepSize, G4ForceCondition *condition) override;

   /**  Actual interaction **/
   virtual G4VParticleChange *PostStepDoIt(const G4Track &track, const G4Step &step) override;

   /**  Set the nucleon type **/
   void SetNucleonType(char *nucleon) { fNucleon = nucleon; }
   /** Set the x-y-z range of the DIS interaction **/
   void SetRange(std::vector<float> *x_range, std::vector<float> *y_range, std::vector<float> *z_range);
   /** Set the name of the geometry volume selected for placement of the DIS interaction **/
   void SetVolume(char* volume) { fVolumeName = volume; }
   /**  Set the DIS cross section lookup tables for the set nucleon type  **/
   void SetCrossSectionTables(std::shared_ptr<std::map<int, G4PhysicsFreeVector*>> xsecTables) { fXsecTables = xsecTables; }

private:
   /**  Pythia6 call to generate the DIS process **/
   void SimulateDIS(const G4Track &track);
   
   /** Reset private class member variables **/
   void ResetParameters();

   /** Rotation of Pythia6 outgoing particles to match physics CS **/
   std::tuple<double, double, double>
   rotate(double ctheta, double stheta, double cphi, double sphi, double px, double py, double pz);

   /*** 3D linear track extrapolation **/
   G4ThreeVector linear_track_extrapolation(G4ThreeVector pos, G4ThreeVector mom, G4double z_ref);

   MuDISGenerator *disGen; /// class instance used to calculate material budget

   G4Navigator *g4Navigator; /// Geant4 geo navigator

   G4double fCrossSection;    /// DIS cross section from the external generator
   G4double fOutTracksWeight; /// Weight of outgoing particles
   char *fNucleon;            //! nucleon type
   std::shared_ptr<std::map<int, G4PhysicsFreeVector*>> fXsecTables; /// DIS cross section lookup tables
   char *fVolumeName;         //! geometry volume name to place the DIS
   G4double fXStart{};        /// start of x range to place the DIS
   G4double fXEnd{};          /// end of x range to place the DIS
   G4double fYStart{};        /// start of y range to place the DIS
   G4double fYEnd{};          /// end of y range to place the DIS
   G4double fZStart{};        /// start of z range to place the DIS
   G4double fZEnd{};          /// end of z range to place the DIS
   // The following are items that update on a per-event basis and not for each step!
   G4bool was_simulated = 0;    /// true if the DIS was already simulated, false otherwise
   double bparam{};             /// mean density in a range
   double mparam[8]{};          //[8] density parameters along track
   double max_density{};        /// maximum density in the whole range
   double prev_max_density{};   /// maximum density in the path traversed so far
   double density_along_path{}; /// mean density in a range
   double prev_start[3]{};      //[3] track position at previous step in the [z_start, z_end] range
};

#endif
