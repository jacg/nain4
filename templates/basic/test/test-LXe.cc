#include "LXe.hh"

#include <n4-all.hh>
#include <n4-will-become-external-lib.hh>

#include <G4OpticalPhysics.hh>
#include <G4PVPlacement.hh>
#include <G4PrimaryParticle.hh>
#include <G4PrimaryVertex.hh>
#include <G4RandomDirection.hh>
#include <G4SystemOfUnits.hh>

#include <G4Box.hh>
#include <G4Orb.hh>

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

using namespace Catch::Matchers;


TEST_CASE("dummy test1", "[dummy]") {
  CHECK(6*7 == 42);
}

TEST_CASE("dummy test2", "[dummy]") {
  CHECK(6*111 == 666);
}

TEST_CASE("liquid xenon properties", "[.xfail][xenon][properties]") {
  // --- Geometry -----
  auto LXe = LXe_with_properties();
  REQUIRE_THAT( LXe -> GetDensity() / (g / cm3)
              , WithinRel(2.98, 1e-6)
              );

  auto our_optical_physics = [&] {
    auto phys_list = new FTFP_BERT{0};
    phys_list -> ReplacePhysics(new G4EmStandardPhysics_option4{0});
    phys_list -> ReplacePhysics(new G4OpticalPhysics{0});
    return phys_list;
  };


  auto abs_lengths = measure_abslength(abslength_config{
      .physics         = our_optical_physics()
    , .material        = LXe
    , .particle_name   = "gamma"
    , .particle_energy = 511 * keV}
    , .distances       = n4::scale_by(cm , {1, 2, 3, 4, 5, 6, 7, 8}
  );

  auto expected_abs_length = 3.74 * cm;
  for (auto abs_length : abs_lengths) {
    CHECK_THAT(abs_length, WithinRel(expected_abs_length, 0.05));
  }
}
