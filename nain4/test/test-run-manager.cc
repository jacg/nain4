#include "nain4.hh"
#include "test_utils.hh"

// Solids
#include <G4Box.hh>
#include <G4Cons.hh>
#include <G4Trd.hh>

// Managers
#include <G4NistManager.hh>

// Units
#include <G4SystemOfUnits.hh>
#include <G4UnitsTable.hh>

// Other G4
#include <G4Material.hh>
#include <G4Gamma.hh>

#include <catch2/catch_approx.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <catch2/catch_test_macros.hpp>

using Catch::Approx;

// Many of the tests below check physical quantities. Dividing physical
// quantities by their units gives raw numbers which are easily understandable
// by a human reader, which is important test failures are reported. Sometimes
// this gives rise to the apparently superfluous division by the same unit on
// both sides of an equation, in the source code.

#include <numeric>

TEST_CASE("nain run_manager build_fn initialization", "[nain][run_manager]") {
  auto hush = n4::silence{std::cout};

  auto rm = n4::run_manager::create()
     .physics(default_physics_lists)
     .geometry(water_box)
     .actions(do_nothing);
}


struct dummy_geometry : G4VUserDetectorConstruction {
  dummy_geometry(double, double, double) {}
  G4PVPlacement* Construct() {
    auto box = n4::volume<G4Box>("box", n4::material("G4_AIR"), 1., 1., 1.);
    return n4::place(box).now();
  }
};

struct dummy_actions : G4VUserActionInitialization {
  dummy_actions(int) {}
  void Build() const override {SetUserAction(new n4::generator([] (auto) {}));}
};

TEST_CASE("nain run_manager construct initialization", "[nain][run_manager]") {
  auto hush = n4::silence{std::cout};

  auto rm = n4::run_manager::create()
     .physics<FTFP_BERT>(0) // verbosity 0
     .geometry<dummy_geometry>(1., 2., 3.)
     .actions<dummy_actions>(10);
}

TEST_CASE("nain run_manager basic initialization", "[nain][run_manager]") {
  auto hush = n4::silence{std::cout};

  auto rm = n4::run_manager::create()
     .physics (new FTFP_BERT{0}) // verbosity 0
     .geometry(new dummy_geometry{1., 2., 3.})
     .actions<dummy_actions>(10);
}
