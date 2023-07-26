// ANCHOR: full_file
// ANCHOR: includes
#include "nain4.hh"
#include "g4-mandatory.hh"
#include "n4_ui.hh"
#include "n4-volumes.hh"

#include <G4PrimaryParticle.hh>
#include <G4SystemOfUnits.hh>   // physical units such as `m` for metre
#include <G4Event.hh>           // needed to inject primary particles into an event
#include <G4Box.hh>             // for creating shapes in the geometry
#include <G4Sphere.hh>          // for creating shapes in the geometry
#include <FTFP_BERT.hh>         // our choice of physics list
#include <G4RandomDirection.hh> // for launching particles in random directions


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

  n4::sensitive_detector::process_hits_fn process_hits = [] (G4Step* step) {
    auto pt = step -> GetPreStepPoint();
    auto pos = pt -> GetPosition();
    auto sd = pt -> GetSensitiveDetector();
    auto sd_name = sd -> GetName();
    auto sd_path = sd -> GetPathName();
    auto sd_full_path = sd -> GetFullPathName();
    auto volume_name = pt -> GetTouchable() -> GetVolume() -> GetName();
    std::cout << sd_path << ' ' << sd_full_path << ' ' << sd_name << " " << volume_name << " " << pos << std::endl;
    return true;
  };

  auto sd = new n4::sensitive_detector{"/foo/bar/baz", process_hits, [](auto) {}};

  auto water  = n4::material("G4_WATER");
  auto air    = n4::material("G4_AIR");
  auto world  = n4::box("world").cube(2*m).x(3*m).volume(water);

  auto bubble = n4::sphere("bubble").r(0.2*m)         .place(air).in(world).at(1.3*m, 0.8*m, 0.3*m).now();
  auto straw  = n4::tubs  ("straw" ).r(0.1*m).z(1.9*m).place(air).in(world).at(0.2*m, 0    , 0    ).now();
  bubble -> GetLogicalVolume() -> SetSensitiveDetector(sd);
  straw  -> GetLogicalVolume() -> SetSensitiveDetector(sd);
  return n4::place(world).now();
}
// ANCHOR_END: my_geometry

// ANCHOR: my_generator
void my_generator(G4Event* event) {
  auto geantino = n4::find_particle("geantino");
  auto vertex   = new G4PrimaryVertex();
  auto r = G4RandomDirection();
  vertex -> SetPrimary(new G4PrimaryParticle(geantino, r.x(), r.y(), r.z()));
  event  -> AddPrimaryVertex(vertex);
}
// ANCHOR_END: my_generator

// ANCHOR: pick_cli_arguments
int main(int argc, char* argv[]) {
  // ANCHOR_END: pick_cli_arguments

  // ANCHOR: create_run_manager
  auto run_manager = n4::run_manager::create()
  // ANCHOR_END: create_run_manager

  // ANCHOR: build_minimal_framework
  // Important! physics list has to be set before the generator!
  .physics<FTFP_BERT>(0) // version 0
  .geometry(my_geometry)
  .actions(my_generator);
  // ANCHOR_END: build_minimal_framework

  // ANCHOR: run
  n4::ui(argc, argv);
  // ANCHOR_END: run
// ANCHOR: closing_bracket
}
// ANCHOR_END: closing_bracket
// ANCHOR_END: full_file
