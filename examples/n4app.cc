// ANCHOR: full_file
// ANCHOR: includes
#include "nain4.hh"
#include "g4-mandatory.hh"

#include <G4SystemOfUnits.hh> // physical units such as `m` for metre
#include <G4Event.hh>         // needed to inject primary particles into an event
#include <G4Box.hh>           // for creating shapes in the geometry
#include <FTFP_BERT.hh>       // our choice of physics list

#include <cstdlib>
// ANCHOR_END: includes

// ANCHOR: print_usage
void verify_number_of_args(int argc){
  if (argc != 2) {
    std::cerr << "Wrong number of arguments: " << argc
              << "\nUsage:\n./n4app <number of events>" << std::endl;
    std::exit(EXIT_FAILURE);
  }
}
// ANCHOR_END: print_usage

// ANCHOR: my_geometry
auto my_geometry() {
  auto hl = 1 * m; // HALF-length of world volume
  auto air = n4::material("G4_AIR");
  auto world = n4::volume<G4Box>("world", air, hl, hl, hl);
  return n4::place(world).now();
}
// ANCHOR_END: my_geometry

// ANCHOR: my_generator
void my_generator(G4Event* event) {
  auto geantino = n4::find_particle("geantino");
  auto vertex   = new G4PrimaryVertex();
  vertex -> SetPrimary(new G4PrimaryParticle(geantino, 1, 0, 0));
  event  -> AddPrimaryVertex(vertex);
}
// ANCHOR_END: my_generator

// ANCHOR: pick_cli_arguments
int main(int argc, char* argv[]) {
  verify_number_of_args(argc);
  auto n_events = std::stoi(argv[1]);
  // ANCHOR_END: pick_cli_arguments

  // ANCHOR: create_run_manager
  auto run_manager = n4::run_manager();
  // ANCHOR_END: create_run_manager

  // ANCHOR: build_minimal_framework
  // Important! physics list has to be set before the generator!
  run_manager -> SetUserInitialization(new FTFP_BERT{0}); // version 0
  run_manager -> SetUserInitialization(new n4::geometry{my_geometry});
  run_manager -> SetUserInitialization(new n4::actions {new n4::generator{my_generator}});
  run_manager -> Initialize();
  // ANCHOR_END: build_minimal_framework

  // ANCHOR: run
  run_manager -> BeamOn(n_events);
  // ANCHOR_END: run
// ANCHOR: closing_bracket
}
// ANCHOR_END: closing_bracket
// ANCHOR_END: full_file
