#include <n4-all.hh>
#include <n4-will-become-external-lib.hh>

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

using Catch::Matchers::WithinRel;

TEST_CASE("interaction length") {
  interaction_length_config config {
    .physics         = n4::test::default_physics_lists(),
    .material        = n4::material("G4_Pb"),
    .particle_name   = "gamma",
    .particle_energy = 1 * MeV,
    .distances       = n4::scale_by(1*mm, {1, 2, 3, 4, 5, 6}),
    .n_events        = 100'000
  };

  for (auto l: measure_interaction_length(config)) {
    CHECK_THAT(l, WithinRel(12.5*mm, 0.02));
  }
}
