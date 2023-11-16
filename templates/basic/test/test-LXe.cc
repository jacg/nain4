#include "LXe.hh"

#include <n4-all.hh>

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
  auto air = n4::material("G4_AIR");
  auto LXe = LXe_with_properties();
  REQUIRE_THAT( LXe -> GetDensity() / (g / cm3)
              , WithinRel(2.98, 1e-6)
              );

  auto xe_sphere = [&air, &LXe] (auto radius) {
    return [&air, &LXe, radius] () {
      auto lab = n4::box   ("LAB"   ).half_cube(1.1*radius).place(air)        .now();
                 n4::sphere("Sphere").r        (    radius).place(LXe).in(lab).now();
      return lab;
    };
  };

  // --- Generator -----
  auto shoot_gamma = [](G4Event* event) {
     auto vertex = new G4PrimaryVertex{};
     auto p      = 511*keV * G4RandomDirection();
     vertex -> SetPrimary(new G4PrimaryParticle(n4::find_particle("gamma"), p.x(), p.y(), p.z()));
     event -> AddPrimaryVertex(vertex);
  };

  // --- Count unscathed gammas (in stepping action) -----
  size_t unscathed = 0;
  auto count_unscathed = [&unscathed](auto step) {
    auto energy = step -> GetTrack() -> GetTotalEnergy();
    if (WithinRel(energy, 1e-6).match(511*keV)) { // ignore post-Compton gammas
      auto name = step -> GetPreStepPoint() -> GetTouchable() -> GetVolume() -> GetName();
      if (name == "LAB") { unscathed++; }
    }
  };

  // --- Eliminate secondaries (in stacking action)  -----
  auto kill_secondaries = [](auto track) {
    auto kill = track -> GetParentID() > 0;
    return kill > 0 ? G4ClassificationOfNewTrack::fKill : G4ClassificationOfNewTrack::fUrgent;
  };

  auto our_optical_physics = [&] {
    auto phys_list = new FTFP_BERT{0};
    phys_list -> ReplacePhysics(new G4EmStandardPhysics_option4{0});
    phys_list -> ReplacePhysics(new G4OpticalPhysics{0});
    return phys_list;
  };

  auto create_actions = [&] {
    return (new n4::actions{shoot_gamma})
         -> set((new n4::stacking_action) -> classify(kill_secondaries))
         -> set (new n4::stepping_action{count_unscathed});
  };

  // ----- Initialize and run Geant4 -------------------------------------------
  {
    n4::silence _{G4cout};

    auto rm = n4::run_manager::create()
      .fake_ui ()
      .physics (our_optical_physics)
      .geometry(xe_sphere(1))
      .actions(create_actions)
      .run()
      ;

    auto events = 10000;
    auto radii  = n4::scale_by(cm, {1, 2, 3, 4, 5, 6, 7, 8});

    // --- Infer attenuation length by gathering statistics for given radius -------------
    auto check_att_length = [&unscathed, rm, &xe_sphere, events] (auto radius) {
      unscathed = 0;

      rm -> replace_geometry(xe_sphere(radius)).run(events);

      auto ratio = unscathed / (1.0 * events);
      auto expected_attenuation_length = 3.74 * cm;
      auto attenuation_length = - radius / log(ratio);
      REQUIRE_THAT( attenuation_length
                  , WithinRel(expected_attenuation_length, 0.05));
   };

    // --- Check attenuation length across range of radii --------------------------------
    for (auto radius : radii) {
      check_att_length(radius);
    }
  }
}
