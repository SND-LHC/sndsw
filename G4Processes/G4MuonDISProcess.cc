#include "G4MuonDISProcess.hh"
#include "G4SystemOfUnits.hh"
#include "G4PhysicalConstants.hh"
#include "G4ParticleTable.hh"
#include "G4MuonMinus.hh"
#include "G4MuonPlus.hh"
#include "G4StepPoint.hh"
#include "G4TrackStatus.hh"
#include "G4DynamicParticle.hh"
#include "G4ParticleChange.hh"
#include "MuDISGenerator.h"
#include "G4RandomTools.hh"
#include "G4EventManager.hh"
#include "G4ThreeVector.hh"
#include "G4RunManager.hh"
#include "G4TransportationManager.hh"
#include "G4Navigator.hh"
#include "G4ProcessType.hh"
#include "G4PhysicsFreeVector.hh"
#include "globals.hh"

#include "TGeoManager.h"
#include "TGeoNode.h"
#include "TPythia6.h"
#include "TRandom.h"
#include "TMath.h"
#include "TROOT.h"

#include <iostream>
#include <ctime>
#include <cmath>
#include <map>
#include <tuple>
#include <vector>
#include <string>

using std::vector;
using std::string;

G4MuonDISProcess::G4MuonDISProcess(const G4String &name) : G4VDiscreteProcess(name)
{
   if (verboseLevel > 0) {
      std::cout << "G4MuonDISProcess created: " << name << " " << fCrossSection << std::endl;
   }
}

void G4MuonDISProcess::ResetParameters(){
   was_simulated = 0;
   bparam = 0.;
   std::memset(mparam, 0, sizeof(mparam));
   max_density = 0.;
   prev_max_density = 0.;
   density_along_path = 0.;
   std::memset(prev_start, 0, sizeof(prev_start));
}

void G4MuonDISProcess::SetRange(vector<float> *x_range, vector<float> *y_range, vector<float> *z_range)
{
   // The base length unit in sndsw is cm, but mm in Pythia 6 and GEANT4
   fXStart = x_range->at(0) * cm;
   fXEnd = x_range->at(1) * cm;
   fYStart = y_range->at(0) * cm;
   fYEnd = y_range->at(1) * cm;
   fZStart = z_range->at(0) * cm;
   fZEnd = z_range->at(1) * cm;
   LOG(INFO) << "G4MuonDISProcess: Setting xyz ranges[cm] for muon DIS\nx: "
                << fXStart/cm << ", " << fXEnd/cm << "\ny: " << fYStart/cm << ", " << fYEnd/cm << "\nz: "
                << fZStart/cm << ", " << fZEnd/cm;
}

G4bool G4MuonDISProcess::IsApplicable(const G4ParticleDefinition &particle)
{
   return (&particle == G4MuonMinus::Definition() || &particle == G4MuonPlus::Definition());
}

G4VParticleChange *G4MuonDISProcess::PostStepDoIt(const G4Track &track, const G4Step &step)
{
   aParticleChange.Initialize(track);
   G4ThreeVector pos = track.GetPosition();
   if (track.GetParentID() == 0 && was_simulated == 0 && (fZEnd - pos.z()) > 1e-3) {
      if ((fZStart - pos.z()) > 1e-3) {
         // do nothing
      } else {
         string geoVolumePath =gGeoManager->GetPath();
         LOG(DEBUG) << "Proposing DIS in volume path: " << geoVolumePath;
         // Check if the track position is in the specified x-y range, z is surely in the z_range,
         // and in the desired geometry volume (e.g. the target), if such is set
         if (pos.x() >= fXStart && pos.x() <= fXEnd && pos.y() >= fYStart && pos.y() <= fYEnd &&\
             geoVolumePath.find(fVolumeName) != string::npos) {
            LOG(DEBUG) << "DIS vertex in the x-y-z range, proceeding to DIS generation at "<< pos/cm
                       << "\nin pre-set geometry volume "<< fVolumeName;
            // Simulate the DIS interaction - call Pythia6
            SimulateDIS(track);
         }
         // To save on computing, drop the whole event shall the selected DIS interaction point be
         // outside the desired x-y range!
         else {
            LOG(DEBUG) << "DIS vertex out of x-y-z range and/or volume, aborting the event at " << pos/cm;
            ResetParameters();
            G4RunManager::GetRunManager()->AbortEvent();
         }
      }
   }
   return &aParticleChange;
}

G4double G4MuonDISProcess::GetMeanFreePath(const G4Track &track, G4double previousStepSize, G4ForceCondition *condition)
{
   return DBL_MAX;
}

G4double G4MuonDISProcess::PostStepGetPhysicalInteractionLength(const G4Track &track, G4double previousStepSize,
                                                                G4ForceCondition *condition)
{
   if ((previousStepSize < 0.0) || (theNumberOfInteractionLengthLeft <= 0.0)) {
      // beggining of tracking (or just after DoIt of this process)
      ResetNumberOfInteractionLengthLeft();
   } else if (previousStepSize > 0.0) {
      // subtract NumberOfInteractionLengthLeft
      SubtractNumberOfInteractionLengthLeft(previousStepSize);
   } else {
      // zero step
   }

   G4ThreeVector pos = track.GetPosition();
   G4ThreeVector mom = track.GetMomentum();
   g4Navigator = G4TransportationManager::GetTransportationManager()->GetNavigatorForTracking();
   G4ThreeVector extrap_pos;
   if (track.GetParentID() == 0){
     LOG(DEBUG) << "primary muons's position[cm]: " << pos/cm << " and momentum[GeV] " << mom.mag()/GeV;
   }
   // don't bother simulating DIS if the energy of the primary is below 5 GeV
   if (track.GetParentID() != 0 ||
      (track.GetParentID() == 0 && (pos.z() > fZEnd || track.GetTotalEnergy() < 5. * GeV))) {
      *condition = NotForced;
      currentInteractionLength = DBL_MAX; // make sure this process won't be picked
      // reset parameters once primary particle passes z_end and/or we're done simulating DIS for the event
      if (max_density != 0 || density_along_path != 0 || was_simulated == 1) {
         ResetParameters();
      }
   } else if ((fZStart - pos.z()) > 2e-3) {
      // Make sure the muon eventually lands at fZStart, use tiny tolerances.
      currentInteractionLength =  fZStart - pos.z() - 1e-3;
      LOG(DEBUG) << "pre-range currentInteractionLength[cm]: " << currentInteractionLength/cm;
      theNumberOfInteractionLengthLeft = 0; // to force this process
      *condition = NotForced;               // does nothing at this point
   } else {                                 // primary muon that didn't undergo DIS yet
      *condition = NotForced;

      // In regions having multiple tiny volumes(e.g. ECC, SciFi), particle propagation is stopped
      // numerous times at boundaries. This leads to sampling z for DIS too frequently, and due to
      // competition with 'border crossing' leads to preference for small steps for the DIS.
      // As a result, all DIS vertices are placed close to the set z_start. To avoid that, if the
      // previous step was limited by boundary crossing (fTransportation), subtract its length
      // from the proposed currentInteractionLength for DIS, and continue.
      if (currentInteractionLength >= previousStepSize &&\
          track.GetStep()->GetPreStepPoint()->GetProcessDefinedStep()->GetProcessType()==fTransportation ) {
	      currentInteractionLength -=previousStepSize;
	      return currentInteractionLength;
       }

      // Calculate start/end positions along this muon track, and amount of material in between
      // Use the track's position as start since position of DIS cannot be before current pos
      // Convert to cm as needed in the MuDISGenerator and sndsw!
      Double_t start[3] = {pos.x() / 10., pos.y() / 10., pos.z() / 10.};
      extrap_pos = linear_track_extrapolation(pos, mom, fZEnd);
      extrap_pos /= 10.;
      Double_t end[3] = {extrap_pos.x(), extrap_pos.y(), extrap_pos.z()};

      Double_t current_dir[3] = {track.GetMomentumDirection().x(), track.GetMomentumDirection().y(),
                                 track.GetMomentumDirection().z()};

      // Use the MeanMaterialBudget code from the MuDISGenerator class
      // Keep track of mean densities in the (likely) multiple steps
      // from z_start to z_end. Update the maxmimum density encountered so far.
      if (bparam == 0) {
         memcpy(prev_start, start, sizeof(prev_start));
      }
      if (bparam != 0) {
         // Subtract density of previous range i.e. prev_pos to z_end
         density_along_path -= (mparam[0] * mparam[4]);
         // Recalculate the density of material traversed in previous step
         bparam = disGen->MeanMaterialBudget(prev_start, start, mparam);
         // Update with the actual mean density between prev_pos to current pos
         density_along_path += (mparam[0] * mparam[4]);
         // Keep track of the max density encountered so far: from z_start to current pos
         prev_max_density = std::max(prev_max_density, mparam[7]);
         memcpy(prev_start, start, sizeof(prev_start)); // update to current posistion
         // Reset navigator
         gGeoManager->CdTop(); // go to top volume
         gGeoManager->InitTrack(start, current_dir);
         gGeoManager->FindNode();
         g4Navigator->LocateGlobalPointAndSetup(pos, nullptr, false);
      }

      // Calculate the density of material predicted to be traversed
      bparam = disGen->MeanMaterialBudget(start, end, mparam);

      // Reset navigator
      gGeoManager->CdTop(); // go to top volume
      gGeoManager->InitTrack(start, current_dir);
      gGeoManager->FindNode();
      g4Navigator->LocateGlobalPointAndSetup(pos, nullptr, false);

      // revert back to mm unit
      for (int i = 0; i < 3; i++) {
         start[i] *= 10.;
         end[i] *= 10.;
      }

      // Set the max density to the larger value between max density encountered from z_start to current pos
      // and max density along the predicted path from current pos to z_end
      max_density = std::max(prev_max_density, mparam[7]);
      // Density along trajectory is the weight, g/cm^2
      density_along_path += (mparam[0] * mparam[4]);
      fOutTracksWeight = density_along_path;

      LOG(DEBUG) << "in-range: density_along_path[g/cm2] " << density_along_path
                << " max_density[g/cm3] " << max_density;

      G4double prob2int{}, zmu;
      G4int count{};

      // Loop over trajectory between start and end to pick an interaction point
      while (prob2int < gRandom->Uniform(0., 1.)) {
         zmu = gRandom->Uniform(start[2], end[2]);
         extrap_pos = linear_track_extrapolation(pos, mom, zmu);
         // get local material at this point
         TGeoNode *node = gGeoManager->FindNode(extrap_pos.x() / 10., extrap_pos.y() / 10., zmu / 10.);
         TGeoMaterial *material = 0;
         if (node && !gGeoManager->IsOutside()){
            material = node->GetVolume()->GetMaterial();
         }
         LOG(DEBUG) << "#trials= " << count << ", material name is " << material->GetName()
                    << ", density is " << material->GetDensity();
         // density relative to Prob largest density along this trajectory, i.e. use rho(Pt)
         prob2int = material->GetDensity() / max_density;
         if (prob2int > 1.) {
            LOG(WARNING) << "MuDISGenerator: prob2int > Maximum density???? " << prob2int
                         << " maxrho along path in [z_start, z_end]: " << max_density
                         << " material: " << material->GetName()
                         << " full path: " << gGeoManager->GetPath()
                         << "\ncurrent pos " << pos
                         << "\nstart pos " << start[0] << " " << start[1] << " " << start[2]
                         << "\nend pos " << end[0] << " " << end[1] << " " << end[2];
         }
         // Reset navigator again
         gGeoManager->CdTop(); // go to top volume
         gGeoManager->InitTrack(start, current_dir);
         gGeoManager->FindNode();
         g4Navigator->LocateGlobalPointAndSetup(pos, nullptr, false);

         count += 1;
      }

      LOG(DEBUG) << "prob2int " << prob2int << ", " << count;
      LOG(DEBUG) << "put position[cm] " << extrap_pos/cm;

      // brilliant solution! Set the next to be the distance btw current pos and the chosen z for the DIS
      currentInteractionLength = (extrap_pos - pos).mag();
      LOG(DEBUG) << "in-range currentInteractionLength[cm]: " << currentInteractionLength/cm;
      theNumberOfInteractionLengthLeft = 0; // to force the interaction
   }

#ifdef G4VERBOSE
   if (verboseLevel > 1) {
      G4cout << "G4VDiscreteProcess::PostStepGetPhysicalInteractionLength ";
      G4cout << "[ " << GetProcessName() << "]" << G4endl;
      track.GetDynamicParticle()->DumpInfo();
      G4cout << " in Material  " << track.GetMaterial()->GetName() << G4endl;
      G4cout << "InteractionLength= " << currentInteractionLength / cm << "[cm] " << G4endl;
   }
#endif
   return currentInteractionLength;
}

void G4MuonDISProcess::SimulateDIS(const G4Track &track)
{
   was_simulated = 1;
   G4ThreeVector pos = track.GetPosition();

   G4ThreeVector mom = track.GetMomentum();
   G4int pid = track.GetDefinition()->GetPDGEncoding();
   G4double w = track.GetWeight(); // it is the weight from the input file
   G4ParticleTable *particleTable = G4ParticleTable::GetParticleTable();

   G4cout << "[DIS] Simulating muon DIS on "<< fNucleon <<" at: "
                                            << pos/cm << "cm, E="
                                            << track.GetTotalEnergy() / GeV << " GeV\n";

   double theta = mom.theta();
   double phi = mom.phi();
   double ctheta = cos(theta);
   double stheta = sin(theta);
   double cphi = cos(phi);
   double sphi = sin(phi);

   TPythia6 *myPythia = new TPythia6();
   myPythia->SetMSEL(2);    // minimum bias
   // PARP(2) is the lowest c.m. energy for the event as a whole that the program
   // will accept to simulate, default is 10 GeV.
   // Set it to 2 GeV instead.
   myPythia->SetPARP(2, 2);

   for (int kf : {211, 321, 130, 310, 3112, 3122, 3222, 3312, 3322, 3334}) {
      int kc = myPythia->Pycomp(kf);
      myPythia->SetMDCY(kc, 1, 0); // make them stable for pythia6 part
   }

   int R = static_cast<int>(std::time(nullptr) % 900000000);
   myPythia->SetMRPY(1, R); // set the generator seed

   std::map<int, const char *> mutype = {{-13, "gamma/mu+"}, {13, "gamma/mu-"}};

   myPythia->SetMSTU(11, 11); // stop pythia printout during loop
   myPythia->Initialize("FIXT", mutype[pid], fNucleon, mom.mag()/GeV); // using ’FIXT’, so 4th argument is beam mom in GeV

   myPythia->GenerateEvent();
   myPythia->Pyedit(2); // remove decayed particles
   // Integrated cross section for subprocess ISUB, based on the statistics accumulated so far
   // is directly accessible, see below. However it suffers from large uncertainty and Pythia6
   // manual reads that a proper xsec estimate can be obtained only by generating large statistics.
   // This is why we use an external file with tabulated muon DIS xsec, where 1M events per reaction
   // were used. 
   fCrossSection = (fXsecTables->at(pid))->Value(track.GetTotalEnergy()/GeV); // in mb
   G4cout << "Total xsec for this 1 event: " << myPythia->GetPyint5()->XSEC[2][0]
          << ", total xsec using 1M events: " << fCrossSection <<" mb\n";

   // Kill the incoming muon after the interaction
   // One can only propose the particle state change as the track is const!
   aParticleChange.ProposeTrackStatus(fStopAndKill);

   // outgoing tracks
   for (int itrk = 1; itrk <= myPythia->GetN(); ++itrk) {
      int did = myPythia->GetK(itrk, 2);
      double dpx, dpy, dpz;
      std::tie(dpx, dpy, dpz) =
         rotate(ctheta, stheta, cphi, sphi, myPythia->GetP(itrk, 1)*GeV, myPythia->GetP(itrk, 2)*GeV, myPythia->GetP(itrk, 3)*GeV);
      double mass = myPythia->GetP(itrk, 5) * GeV; // mass in MeV: Pythia6 uses GeV/c2, so convert to Geant4's unit of MeV!
      double p = sqrt(dpx * dpx + dpy * dpy + dpz * dpz); // mom in MeV
      G4double kineticEnergy = sqrt(p * p + mass * mass) - mass;  // Ekin in MeV
      // Retrieve the particle definition using the PDG code
      G4ParticleDefinition *particleDef = particleTable->FindParticle(did);
      G4ThreeVector momentumDirection(dpx / p, dpy / p, dpz / p);

      G4DynamicParticle *dynamicParticle = new G4DynamicParticle(particleDef, momentumDirection, kineticEnergy);
      G4ThreeVector position(pos.x(), pos.y(), pos.z()); // use position of mother
      G4double time = track.GetGlobalTime();             // use age of mother
      G4Track *newTrack = new G4Track(dynamicParticle, time, position);

      // Add to particle change
      aParticleChange.AddSecondary(newTrack);

      newTrack->SetParentID(track.GetTrackID());
      newTrack->SetTouchableHandle(track.GetTouchableHandle()); // Important for geometry context
      // weight = in_muon_weight * density along trajectory * cross_section
      // On top, one needs to make two adjustments:
      // 1. normalize material density to nucleon mass to get the number of 'targets'.
      // We use proton_mass = 1.67x1E-24 grams.
      // 2. convert xsec from mb to cm^2: 1mb = 1E-27 cm^2
      newTrack->SetWeight(w*fOutTracksWeight/1.67E-24*fCrossSection*1E-27);
   }
   //myPythia->Pystat(1); // extensive printout of parameters: xsec, BR, etc.
   //myPythia->Pylist(1); // list table of properties for all particles
   myPythia->SetMSTU(11, 6); // file number to which all program output is directed

   delete myPythia;
}

std::tuple<double, double, double>
G4MuonDISProcess::rotate(double ctheta, double stheta, double cphi, double sphi, double px, double py, double pz)
{
   // rotate around y-axis
   double px1 = ctheta * px + stheta * pz;
   double pzr = -stheta * px + ctheta * pz;
   // rotate around z-axis
   double pxr = cphi * px1 - sphi * py;
   double pyr = sphi * px1 + cphi * py;
   return std::make_tuple(pxr, pyr, pzr);
}

G4ThreeVector G4MuonDISProcess::linear_track_extrapolation(G4ThreeVector pos, G4ThreeVector mom, G4double z_ref)
{
   double lam = (z_ref - pos.z()) / mom.z();
   return G4ThreeVector(pos.x() + lam * mom.x(), pos.y() + lam * mom.y(), z_ref);
}
