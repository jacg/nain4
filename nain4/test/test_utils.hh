#ifndef n4_test_utils_hh
#define n4_test_utils_hh

#include "nain4.hh"

#include <G4VUserDetectorConstruction.hh>
#include <G4VUserActionInitialization.hh>
#include <FTFP_BERT.hh>
#include <G4EmStandardPhysics_option4.hh>
#include <G4OpticalPhysics.hh>

class dummy_detector : public G4VUserDetectorConstruction {
public:
  dummy_detector() : G4VUserDetectorConstruction() {}
  G4VPhysicalVolume* Construct() override { return nullptr; };
};


class dummy_action_init : public G4VUserActionInitialization {
public:
  dummy_action_init() : G4VUserActionInitialization() {}
  void BuildForMaster() const override {}
  void Build         () const override {}
};

inline auto water_box() {
  auto water = n4::material("G4_WATER");
  auto box   = n4::volume<G4Box>("box", water, 1., 1., 1.);
  return n4::place(box).now();
}

inline void do_nothing(G4Event*) {}


inline auto default_physics_lists() {
  auto verbosity = 0;
  auto physics_list = new FTFP_BERT{verbosity};
  physics_list -> ReplacePhysics(new G4EmStandardPhysics_option4());
  physics_list -> RegisterPhysics(new G4OpticalPhysics{});
  return physics_list;
}


inline auto default_run_manager(){

  // Redirect G4cout to /dev/null while Geant4 makes noise RAII would be a pain,
  // as `run_manager` (defined next) must outlive `hush`.
  auto hush = std::make_unique<n4::silence>(std::cout);

  // Construct the default run manager

  auto run_manager = n4::run_manager::create()
                        .physics(default_physics_lists)
                        .geometry(water_box)
                        .actions(do_nothing);

  // Stop redicecting G4cout to /dev/null
  hush = nullptr;
  return run_manager;
}

#endif // test_utils_hh
