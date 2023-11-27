
#include <n4-will-become-external-lib.hh>
#include <n4-inspect.hh>
#include <n4-material.hh>
#include <n4-sequences.hh>
#include <n4-shape.hh>
#include <n4-stream.hh>

#include <G4PrimaryVertex.hh>
#include <G4RandomDirection.hh>
#include <G4VUserPhysicsList.hh>


std::vector<double> measure_abslength(test_config const& config) {
  auto air    = n4::material("G4_AIR");
  auto sphere = [material=config.material, air] (auto radius) {
    return [material, air, radius] () {
      auto lab = n4::box   ("LAB"   ).half_cube(1.1*radius).place(air     )        .now();
                 n4::sphere("Sphere").r        (    radius).place(material).in(lab).now();
      return lab;
    };
  };

  // --- Generator -----
  auto shoot_gamma = [&config](G4Event* event) {
     auto vertex = new G4PrimaryVertex{};
     auto p      = config.particle_energy * G4RandomDirection();
     auto particle = n4::find_particle(config.particle_name);
     vertex -> SetPrimary(new G4PrimaryParticle(particle, p.x(), p.y(), p.z()));
     event -> AddPrimaryVertex(vertex);
  };

  // --- Count unscathed gammas (in stepping action) -----
  size_t unscathed = 0;
  auto count_unscathed = [&unscathed, initial_energy=config.particle_energy](auto step) {
    auto energy = step -> GetTrack() -> GetTotalEnergy();
    if (energy > 0.999999 * initial_energy) {
      auto name = step -> GetPreStepPoint() -> GetTouchable() -> GetVolume() -> GetName();
      if (name == "LAB") { unscathed++; }
    }
  };

  // --- Eliminate secondaries (in stacking action)  -----
  auto kill_secondaries = [](auto track) {
    auto kill = track -> GetParentID() > 0;
    return kill > 0 ? G4ClassificationOfNewTrack::fKill : G4ClassificationOfNewTrack::fUrgent;
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
      .physics (config.physics)
      .geometry(sphere(1))
      .actions(create_actions)
      .run()
      ;

    auto events = 10000;
    auto radii  = config.distances;
    std::vector<double> result;
    result.reserve(radii.size());

    // --- Infer attenuation length by gathering statistics for given radius -------------
    auto estimate_att_length = [&unscathed, rm, &sphere, events, &result] (auto radius) {
      unscathed = 0;

      rm -> replace_geometry(sphere(radius)).run(events);

      auto ratio = unscathed / (1.0 * events);
      auto attenuation_length = - radius / log(ratio);
      result.push_back(attenuation_length);
   };

    // --- Check attenuation length across range of radii --------------------------------
    for (auto radius : radii) {
      estimate_att_length(radius);
    }
    return result;
  }
}
