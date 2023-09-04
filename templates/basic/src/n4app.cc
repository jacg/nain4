#include "nain4.hh"
#include "g4-mandatory.hh"
#include "n4_ui.hh"
#include "n4-volumes.hh"

#include <G4PrimaryParticle.hh>
#include <G4RotationMatrix.hh>
#include <G4SystemOfUnits.hh>   // physical units such as `m` for metre
#include <G4Event.hh>           // needed to inject primary particles into an event
#include <G4Box.hh>             // for creating shapes in the geometry
#include <G4Sphere.hh>          // for creating shapes in the geometry
#include <FTFP_BERT.hh>         // our choice of physics list
#include <G4RandomDirection.hh> // for launching particles in random directions


#include <cstdlib>

void verify_number_of_args(int argc){
  if (argc != 2) {
    std::cerr << "Wrong number of arguments: " << argc
              << "\nUsage:\n./n4app <number of events>" << std::endl;
    std::exit(EXIT_FAILURE);
  }
}

void my_generator(G4Event* event) {
  auto geantino = n4::find_particle("geantino");
  auto vertex   = new G4PrimaryVertex();
  auto r = G4RandomDirection();
  vertex -> SetPrimary(new G4PrimaryParticle(geantino, r.x(), r.y(), r.z()));
  event  -> AddPrimaryVertex(vertex);
}

n4::actions* create_actions(unsigned& n_event) {
  auto my_stepping_action = [&] (const G4Step* step) {
    auto pt = step -> GetPreStepPoint();
    auto volume_name = pt -> GetTouchable() -> GetVolume() -> GetName();
    if (volume_name == "straw" || volume_name == "bubble") {
      auto pos = pt -> GetPosition();
      std::cout << volume_name << " " << pos << std::endl;
    }
  };

  auto my_event_action = [&] (const G4Event*) {
     n_event++;
     std::cout << "end of event " << n_event << std::endl;
  };

  return (new n4::        actions{      my_generator})
 -> set( (new n4::   event_action{                  }) -> end(my_event_action) )
 -> set(  new n4::stepping_action{my_stepping_action} );
}


auto my_geometry() {

  auto water  = n4::material("G4_WATER");
  auto air    = n4::material("G4_AIR");
  auto steel  = n4::material("G4_STAINLESS-STEEL");
  auto world  = n4::box("world").cube(2*m).x(3*m).volume(water);

  n4::sphere("bubble").r(0.2*m)         .place(air).in(world).at(1.3*m, 0.8*m, 0.3*m).now();
  n4::tubs  ("straw" ).r(0.1*m).z(1.9*m).place(air).in(world).at(0.2*m, 0    , 0    ).now();

  G4RotationMatrix rot;  rot.rotateX(-90 * deg); // TODO replace RotationMatrix with place.rotate_x()

  n4       ::sphere("socket-cap" ).r(0.3*m).phi_delta(180*deg)
    .sub(n4::box   ("socket-hole").cube(0.4*m))
    .name("socket")
    .place(steel).in(world).rotate(rot).at(1*m, 0, 0.7*m).now();

  return n4::place(world).now();
}

int main(int argc, char* argv[]) {
  unsigned n_event = 0;

  auto run_manager = n4::run_manager::create()

    // Important! physics list has to be set before the generator!
    .physics<FTFP_BERT>(0) // version 0
    .geometry(my_geometry)
    .actions(create_actions(n_event));

  n4::ui(argc, argv);

  run_manager.initialize();
}
