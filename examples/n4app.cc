#include "nain4.hh"
#include "g4-mandatory.hh"

#include <G4RunManagerFactory.hh>
#include <G4SystemOfUnits.hh>
#include <G4VPhysicalVolume.hh>
#include <G4Event.hh>
#include <G4Box.hh>
#include <FTFP_BERT.hh>

#include <stdlib.h>


void print_usage() {
  std::cout << "Usage:" << std::endl
            << "./n4app <number of events>" << std::endl;
}


G4VPhysicalVolume* my_geometry() {
  auto world_halfsize = 1 * m;

  auto world = n4::volume<G4Box>( "world"
                                , n4::material("G4_AIR")
                                , world_halfsize, world_halfsize, world_halfsize);
  return n4::place(world).now();
}

void my_generator(G4Event* event) {
  auto geantino = n4::find_particle("geantino");
  auto vertex   = new G4PrimaryVertex();
  vertex->SetPrimary(new G4PrimaryParticle(geantino, 1, 0, 0));
  event->AddPrimaryVertex(vertex);
}

G4VUserPhysicsList* my_physics_list() {
  return new FTFP_BERT{0};
}

int main(int argc, char* argv[]) {
  if (argc != 2) {
    std::cerr << "Wrong number of parameters: " << argc << std::endl;
    print_usage();
    return 1;
  }

  auto n_events = atoi(argv[1]);


  auto* run_manager = G4RunManagerFactory::CreateRunManager(G4RunManagerType::Default);

  // Important! physics list has to be set before the generator!
  run_manager -> SetUserInitialization(my_physics_list());

  auto geometry  = new n4::geometry {my_geometry};
  auto generator = new n4::generator{my_generator};
  auto actions   = new n4::actions  {generator};

  run_manager -> SetUserInitialization(geometry);
  run_manager -> SetUserInitialization(actions);
  run_manager -> Initialize();

  run_manager -> BeamOn(n_events);

  delete run_manager;

  return 0;
}
