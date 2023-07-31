#include "nain4.hh"
#include "test_utils.hh"
#include "n4-volumes.hh"

// Solids
#include <FTFP_BERT.hh>
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


TEST_CASE("nain run_manager get", "[nain][run_manager]") {
  auto  rm_value     = default_run_manager();
  auto& rm_reference = n4::run_manager::get();

  CHECK(&rm_value == &rm_reference);
}

TEST_CASE("nain run_manager no_world_volume", "[nain][run_manager]") {
  auto my_geometry = [] {
    auto air = n4::material("G4_AIR");
    auto box_world    = n4::box{"world"   }.cube(1).volume(air);
    auto box_daughter = n4::box{"daughter"}.cube(1).volume(air);

    // We place the daughter
    // But we forget to place the mother
    // And we should get an error
    return n4::place(box_daughter).in(box_world).now();
    //     n4::place(box_mother  ).             .now();
  };

  auto hush = n4::silence{std::cout};
  n4::run_manager::create()
     .physics<FTFP_BERT>(0)
     .geometry(my_geometry)
     .actions(do_nothing);
}

TEST_CASE("nain run_manager too_many_world_volumes", "[nain][run_manager]") {
  auto my_geometry = [] {
    auto air = n4::material("G4_AIR");
    auto box_world_1 = n4::box{"world-1"}.cube(1).volume(air);
    auto box_world_2 = n4::box{"world-2"}.cube(1).volume(air);

    // No `.in` call defaults to world volume
           n4::place(box_world_1).now();
    return n4::place(box_world_2).now();
  };

  auto hush = n4::silence{std::cout};
  n4::run_manager::create()
     .physics<FTFP_BERT>(0)
     .geometry(my_geometry)
     .actions(do_nothing);

}


TEST_CASE("nain run_manager exactly_one_world_volumes", "[nain][run_manager]") {
  auto my_geometry = [] {
    auto air = n4::material("G4_AIR");
    auto box_daughter = n4::box{"daughter"}.cube(1).volume(air);
    auto box_world    = n4::box{"world"   }.cube(1).volume(air);

    // No `.in` call defaults to world volume
           n4::place(box_daughter).in(box_world).now();
    return n4::place(box_world   ).now();
  };

  auto hush = n4::silence{std::cout};
  n4::run_manager::create()
     .physics<FTFP_BERT>(0)
     .geometry(my_geometry)
     .actions(do_nothing);

}
