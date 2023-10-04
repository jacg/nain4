#include <nain4.hh>

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

int main(int argc, char** argv) {

  // ----- Catch2 session --------------------------------------------------
  int result = Catch::Session().run(argc, argv);

  // ----- Post-test cleanup -----------------------------------------------

  // Smart pointers should clean up all the stuff we made for G4

  // ----- Communicate test result to OS -----------------------------------
  return result;
}
