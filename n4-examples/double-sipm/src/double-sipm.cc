#include "shared.hh"

#include "generator.hh"
#include "geometry.hh"

#include <g4-mandatory.hh>
#include <n4_run_manager.hh>
#include <n4_ui.hh>
#include <nain4.hh>

#include <CLHEP/Vector/ThreeVector.h>
#include <FTFP_BERT.hh>
#include <G4EmStandardPhysics_option4.hh>
#include <G4LogicalVolume.hh>
#include <G4MaterialPropertiesTable.hh>
#include <G4OpticalPhysics.hh>
#include <G4RandomDirection.hh>
#include <G4RotationMatrix.hh>
#include <G4Run.hh>
#include <G4RunManagerFactory.hh>
#include <G4Step.hh>
#include <G4SubtractionSolid.hh>
#include <G4SystemOfUnits.hh>
#include <G4ThreeVector.hh>
#include <G4Tubs.hh>

#include <G4Types.hh>
#include <G4UIExecutive.hh>
#include <G4UImanager.hh>
#include <G4VisExecutive.hh>
#include <G4VisManager.hh>

#include <filesystem>
#include <iostream>
#include <memory>


static bool DEBUG = false;

auto physics_list() {
  auto physics_list =             new FTFP_BERT{0};
  physics_list ->  ReplacePhysics(new G4EmStandardPhysics_option4{0});
  physics_list -> RegisterPhysics(new G4OpticalPhysics{0});
  return physics_list;
}

// Add energies deposited in this step to running totals of deposited energies in whole event
void add_step_edep(std::vector<G4double>& total_edep, G4Step const* step) {
  auto step_solid_name = step -> GetPreStepPoint() -> GetTouchable() -> GetVolume() -> GetName();
  auto particle_pos    = step -> GetTrack() -> GetPosition();
  auto e_dep           = step -> GetTotalEnergyDeposit();

  if (DEBUG && e_dep > 0) {
    auto interaction     = step -> GetPostStepPoint() -> GetProcessDefinedStep() -> GetProcessName();
    auto particle_name   = step -> GetTrack() -> GetDefinition() -> GetParticleName();

    G4cout << "Particle: "  << particle_name                       << ", "
           << "Track ID: "  << step -> GetTrack() -> GetTrackID()  << ", "
           << "Parent ID: " << step -> GetTrack() -> GetParentID() << ", "
           << "E deposit: " << e_dep / keV                         << " keV, "
           << "Process: "   << interaction
           << G4endl;
  }

  if (step_solid_name == "Scintillator") {
    auto index = particle_pos.z() > 0 ? 0 : 1;
    total_edep[index] += e_dep;
  }
}

void gamma_interaction_z_pos(std::vector<std::vector<G4double>>& gamma_zs, G4Step const* step) {
  auto step_solid_name = step ->  GetPreStepPoint() -> GetTouchable() -> GetVolume() -> GetName();
  auto interaction     = step -> GetPostStepPoint() -> GetProcessDefinedStep() -> GetProcessName();
  auto particle_name   = step -> GetTrack() -> GetDefinition() -> GetParticleName();

  if ( (particle_name   == "gamma"       ) &&
       (step_solid_name == "Scintillator") &&
       ((interaction    == "compt"       ) || (interaction == "phot")) ) {
    auto z     = step -> GetTrack() -> GetPosition().z();
    auto index = z > 0 ? 0 : 1;
    gamma_zs[index].push_back(z);
  }
}

void open_files(output& output, G4String seed) {
  std::filesystem::create_directory("output");
  output.gamma_z_data_files[0].open("output/z_pos_0_seed_" + seed + ".csv");
  output.gamma_z_data_files[1].open("output/z_pos_1_seed_" + seed + ".csv");
  output.   time_data_files[0].open("output/times_0_seed_" + seed + ".csv");
  output.   time_data_files[1].open("output/times_1_seed_" + seed + ".csv");
  output.   edep_data_files[0].open("output/edeps_0_seed_" + seed + ".csv");
  output.   edep_data_files[1].open("output/edeps_1_seed_" + seed + ".csv");
}

void close_files(output& output) {
  output.gamma_z_data_files[0].close();
  output.gamma_z_data_files[1].close();
  output.   time_data_files[0].close();
  output.   time_data_files[1].close();
  output.   edep_data_files[0].close();
  output.   edep_data_files[1].close();
};

void reset_photon_count(data& data) {
  data.      total_edep[0] = 0;
  data.      total_edep[1] = 0;
  data.        gamma_zs[0].clear();
  data.        gamma_zs[1].clear();
  data.times_of_arrival[0].clear();
  data.times_of_arrival[1].clear();
}

void write_photon_count(data& data, output& output) {
  if (data.event_number % 100 == 0) {
    G4cout << "Number of double events: " << data.double_hits << "/" << data.event_number << " events" << G4endl;
  }

  // Writing the photon count
  if ( data.times_of_arrival[0].size() > 0 &&
       data.times_of_arrival[1].size() > 0 ) { data.double_hits += 1; }

  for (int side=0; side < 2; ++side) {
    auto& file = output.time_data_files[side];
    for (auto t: data.times_of_arrival[side]) { file << t << ","; }
    file << std::endl;
  }

  // Writing the gamma interaction z position
  for (int side=0; side < 2; ++side) {
    auto& file = output.gamma_z_data_files[side];
    if (data.gamma_zs[side].size() == 0) { file << 0 << ","; }
    for (auto z: data.gamma_zs[side])    { file << z << ","; }
    file << std::endl;
  }

  // Writing the deposited energy
  if (DEBUG) {
    G4cout << "Energies: " << data.total_edep      [0]        << ", " << data.total_edep      [1]        << G4endl;
    G4cout << "Counts  : " << data.times_of_arrival[0].size() << ", " << data.times_of_arrival[1].size() << G4endl;
  }
  output.edep_data_files[0] << data.total_edep[0] << ",";
  output.edep_data_files[1] << data.total_edep[1] << ",";
}

// At every step: increment running total of deposited energy during the event
void accumulate_energy(data& data, const G4Step* step) {
  add_step_edep          (data.total_edep, step);
  gamma_interaction_z_pos(data.gamma_zs  , step);
}

auto actions(data& data, output& output, G4String seed) {
  // Each event produces a pair of back-to-back 511 keV gammas
  auto two_gammas = [](auto event){ generate_back_to_back_511_keV_gammas(event, {}, 0); };

  return (new n4::actions{two_gammas})
    -> set((new n4::run_action{})
           -> begin([&, seed] (auto) { open_files(output, seed);})
           -> end  ([&      ] (auto) {close_files(output      );}) )
    -> set((new n4::event_action{})
           -> begin([&] (auto) { reset_photon_count(data        ); })
           -> end  ([&] (auto) { write_photon_count(data, output); }))
    -> set(new n4::stepping_action{[&] (auto step) { accumulate_energy(data, step); } });
}


int main(int argc, char *argv[]) {
  config config;
  data     data;
  output output;

  // Fixing the seed because we want reproducibility
  G4long seed = 123456789;
  G4Random::setTheSeed(seed);

  n4::run_manager::create()
    .ui("double-sipm", argc, argv)
    .macro_path("macs")
    .apply_cli_early() // CLI --early executed at this point
    .physics (physics_list)
    .geometry([&]{ return make_geometry(data, config); })
    .actions ([&]{ return actions(data, output, std::to_string(seed)); })
    .apply_cli_late() // CLI --late executed at this point
    .run();
}
