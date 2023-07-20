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

TEST_CASE("nain run_manager exists", "[nain][run_manager]") {
  CHECK(n4::make_run_manager().is_valid());
}

TEST_CASE("nain run_manager correct initialization", "[nain][run_manager]") {
  auto initialized_rm = n4::make_run_manager()
     .physics(default_physics_lists())
     .geometry(water_box)
     .actions(do_nothing)
     .init();

  CHECK(initialized_rm.is_valid());
}

TEST_CASE("nain run_manager pointer validity", "[nain][run_manager]") {
  auto rm0 = n4::make_run_manager();
  CHECK(rm0.is_valid());

  auto rm1 = rm0.physics(default_physics_lists());
  CHECK(!rm0.is_valid());
  CHECK( rm1.is_valid());

  auto rm2 = rm1.geometry(water_box);
  CHECK(!rm0.is_valid());
  CHECK(!rm1.is_valid());
  CHECK( rm2.is_valid());

  auto rm3 = rm2.actions(do_nothing);
  CHECK(!rm0.is_valid());
  CHECK(!rm1.is_valid());
  CHECK(!rm2.is_valid());
  CHECK( rm3.is_valid());

  auto rm4 = rm3.init();
  CHECK(!rm0.is_valid());
  CHECK(!rm1.is_valid());
  CHECK(!rm2.is_valid());
  CHECK(!rm3.is_valid());
  CHECK( rm4.is_valid());
}

TEST_CASE("nain run_manager move", "[nain][run_manager]") {
  auto rm0 = n4::make_run_manager();
  auto rm1 = std::move(rm0);
  CHECK(!rm0.is_valid());
  CHECK( rm1.is_valid());
}


