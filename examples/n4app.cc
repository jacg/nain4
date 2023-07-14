// ANCHOR: full_file
// ANCHOR: includes
#include "nain4.hh"
#include "g4-mandatory.hh"

#include <G4RunManager.hh>
#include <G4RunManagerFactory.hh>
#include <G4SystemOfUnits.hh>
#include <G4VPhysicalVolume.hh>
#include <G4Event.hh>
#include <G4Box.hh>
#include <FTFP_BERT.hh>

#include <stdlib.h>
// ANCHOR_END: includes

// ANCHOR: print_usage
void print_usage() {
  std::cout << "Usage:" << std::endl
            << "./n4app <number of events>" << std::endl;
}
// ANCHOR_END: print_usage


// ANCHOR: my_geometry
G4VPhysicalVolume* my_geometry() {
  auto world_halfsize = 1 * m;

  auto world = n4::volume<G4Box>( "world"
                                , n4::material("G4_AIR")
                                , world_halfsize, world_halfsize, world_halfsize);
  return n4::place(world).now();
}
// ANCHOR_END: my_geometry


// ANCHOR: my_generator
void my_generator(G4Event* event) {
  auto geantino = n4::find_particle("geantino");
  auto vertex   = new G4PrimaryVertex();
  vertex->SetPrimary(new G4PrimaryParticle(geantino, 1, 0, 0));
  event->AddPrimaryVertex(vertex);
}
// ANCHOR_END: my_generator


// ANCHOR: my_physics_list
G4VUserPhysicsList* my_physics_list() {
  return new FTFP_BERT{0};
}
// ANCHOR_END: my_physics_list

// ANCHOR: pick_cli_arguments
int main(int argc, char* argv[]) {
  if (argc != 2) {
    std::cerr << "Wrong number of parameters: " << argc << std::endl;
    print_usage();
    std::exit(EXIT_FAILURE);
  }

  auto n_events = atoi(argv[1]);
  // ANCHOR_END: pick_cli_arguments

  // ANCHOR: create_run_manager
  auto run_manager = std::unique_ptr<G4RunManager>
  {G4RunManagerFactory::CreateRunManager(G4RunManagerType::Default)};
  // ANCHOR_END: create_run_manager

  // ANCHOR: build_minimal_framework
  // Important! physics list has to be set before the generator!
  run_manager -> SetUserInitialization(my_physics_list());

  auto geometry  = new n4::geometry {my_geometry};
  auto generator = new n4::generator{my_generator};
  auto actions   = new n4::actions  {generator};

  run_manager -> SetUserInitialization(geometry);
  run_manager -> SetUserInitialization(actions);
  run_manager -> Initialize();
  // ANCHOR_END: build_minimal_framework

  // ANCHOR: run
  run_manager -> BeamOn(n_events);
  // ANCHOR_END: run

// ANCHOR: closing_bracket
}
// ANCHOR_END: closing_bracket
// ANCHOR_END: full_file
