#include "nain4.hh"

#include <G4Box.hh>
#include <G4SystemOfUnits.hh>
#include <G4UIExecutive.hh>
#include <G4VUserActionInitialization.hh>
#include <G4VUserDetectorConstruction.hh>

#include <FTFP_BERT.hh>
#include <G4EmStandardPhysics_option4.hh>
#include <G4OpticalPhysics.hh>

#include <catch2/catch_session.hpp>
#include <catch2/catch_test_macros.hpp>

#include <fstream>
#include <memory>

using std::unique_ptr;

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

int main(int argc, char** argv) {

  // ----- Pre-testing setup: G4 boilerplate -------------------------------

  // Redirect G4cout to /dev/null while Geant4 makes noise RAII would be a pain,
  // as `run_manager` (defined next) must outlive `hush`.
  auto hush = std::make_unique<n4::silence>(std::cout);

  // Construct the default run manager
  auto run_manager = n4::run_manager();
  // Set mandatory initialization classes

  // run_manager takes ownership of detector_construction
  run_manager -> SetUserInitialization(new dummy_detector{});

  { // Physics list
    auto verbosity = 0;
    auto physics_list = new FTFP_BERT{verbosity};
    physics_list -> ReplacePhysics(new G4EmStandardPhysics_option4());
    physics_list -> RegisterPhysics(new G4OpticalPhysics{});
    run_manager  -> SetUserInitialization(physics_list);
  } // run_manager owns physics_list

  // User action initialization
  run_manager -> SetUserInitialization(new dummy_action_init{});

  // Stop redicecting G4cout to /dev/null
  hush = nullptr;

  // ----- Catch2 session --------------------------------------------------
  int result = Catch::Session().run(argc, argv);

  // ----- Post-test cleanup -----------------------------------------------

  // Smart pointers should clean up all the stuff we made for G4

  // ----- Communicate test result to OS -----------------------------------
  return result;
}
