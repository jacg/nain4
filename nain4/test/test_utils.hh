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


inline n4::run_manager default_run_manager(){

  // Redirect G4cout to /dev/null while Geant4 makes noise RAII would be a pain,
  // as `run_manager` (defined next) must outlive `hush`.
  auto hush = std::make_unique<n4::silence>(std::cout);

  // Construct the default run manager
  auto run_manager = n4::run_manager();
  // Set mandatory initialization classes

  // run_manager takes ownership of detector_construction
  run_manager.geometry(new dummy_detector{});

  { // Physics list
    auto verbosity = 0;
    auto physics_list = new FTFP_BERT{verbosity};
    physics_list -> ReplacePhysics(new G4EmStandardPhysics_option4());
    physics_list -> RegisterPhysics(new G4OpticalPhysics{});
    run_manager.physics(physics_list);
  } // run_manager owns physics_list

  // User action initialization
  run_manager.actions(new dummy_action_init{});

  // Stop redicecting G4cout to /dev/null
  hush = nullptr;
  return run_manager;
}

