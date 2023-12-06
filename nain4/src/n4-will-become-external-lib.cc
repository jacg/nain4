
#include <algorithm>
#include <n4-will-become-external-lib.hh>
#include <n4-inspect.hh>
#include <n4-material.hh>
#include <n4-random.hh>
#include <n4-sequences.hh>
#include <n4-shape.hh>
#include <n4-stream.hh>

#include <G4PrimaryVertex.hh>
#include <G4RandomDirection.hh>
#include <G4VUserPhysicsList.hh>


// Calculate interaction_lengths for given
std::vector<double> measure_interaction_length(interaction_length_config const& config) {
  std::vector<double> observed_interaction_distances;
  observed_interaction_distances.reserve(config.n_events);

  auto isotropic = n4::random::direction{};

  auto enormous_sphere = [config] () {
    return n4::sphere("huge").r(1*km).place(config.material).now();
  };

  // Generate particles isotropically from centre of sphere
  auto shoot_particle = [config, isotropic] (G4Event* event) {
    static auto particle = n4::find_particle(config.particle_name);
    auto p               = config.particle_energy * isotropic.get();
    auto vertex          = new G4PrimaryVertex({}, 0);
    vertex -> SetPrimary(new G4PrimaryParticle(particle, p.x(), p.y(), p.z()));
    event  -> AddPrimaryVertex(vertex);
  };

  // Kill the particle as soon as it interacts and record the distance travelled
  auto record_distance_and_kill = [&observed_interaction_distances] (const G4Step* step) {
    auto post    = step -> GetPostStepPoint();
    auto process = post -> GetProcessDefinedStep() -> GetProcessName();
    if (process != "Transportation") { // Transportation step length might be limited
      observed_interaction_distances.push_back(post -> GetPosition().mag());
      step -> GetTrack() -> SetTrackStatus(fStopAndKill);
    }
  };

  auto kill_secondaries = [] (const G4Track* track) {
    return track -> GetTrackID() == 1 ? G4ClassificationOfNewTrack::fUrgent : G4ClassificationOfNewTrack::fKill;
  };

  auto actions = [&] () {
    return   (new n4::actions{shoot_particle})
      -> set((new n4::stacking_action{}) -> classify(kill_secondaries))
      -> set( new n4::stepping_action{record_distance_and_kill})
      ;
  };

  // ----- Initialize and run Geant4 -------------------------------------------
  {
    n4::silence _{G4cout};
    n4::run_manager::create()
      .fake_ui()
      .physics(config.physics)
      .geometry(enormous_sphere)
      .actions(actions())
      .run(config.n_events);
  }

  std::vector<double> measured_interaction_lengths;
  measured_interaction_lengths.reserve(config.distances.size());

  auto estimate_interaction_length = [&] (auto distance) {
    auto interacted_within_distance = std::count_if( cbegin(observed_interaction_distances)
                                                   ,   cend(observed_interaction_distances)
                                                   , [=] (auto d) {return d<distance;});
    auto ratio     = 1 - interacted_within_distance / static_cast<float>(config.n_events);
    auto interaction_length = - distance / log(ratio);
    measured_interaction_lengths.push_back(interaction_length);
  };


  std::for_each( cbegin(config.distances)
               ,   cend(config.distances)
               , estimate_interaction_length
               );

  return measured_interaction_lengths;
}

// ----- Calculate 511 keV gamma interaction process fractions for given material ------------------------------
interaction_process_fractions calculate_interaction_process_fractions(G4Material* material, G4VUserPhysicsList* physics) {
  unsigned phot{0}, compt{0}, rayl{0};

  auto kill_secondaries = [] (const G4Track* track) {
    return track -> GetParentID() > 0 ? G4ClassificationOfNewTrack::fKill : G4ClassificationOfNewTrack::fUrgent;
  };

  auto record_process_and_kill = [&phot, &compt, &rayl] (const G4Step* step) {
    auto process = step -> GetPostStepPoint() -> GetProcessDefinedStep() -> GetProcessName();
    if (process != "Transportation") {
      if (process == "compt") { compt++; }
      if (process == "phot" ) {  phot++; }
      if (process == "Rayl" ) {  rayl++; }

      step -> GetTrack() -> SetTrackStatus(fStopAndKill);
    }
  };

  auto isotropic_511keV_gammas = [] () {
    auto particle_type = n4::find_particle("gamma");
    auto energy        = 511 * keV;
    auto isotropic     = n4::random::direction{};

    return [energy, particle_type, isotropic] (G4Event* event) {
      auto r        = isotropic.get() * energy;
      auto particle = new G4PrimaryParticle{particle_type, r.x(), r.y(), r.z()};
      auto vertex   = new G4PrimaryVertex{{}, 0};
      particle -> SetPolarization(isotropic.get());
      vertex   -> SetPrimary(particle);
      event    -> AddPrimaryVertex(vertex);
    };
  };

  auto enormous_sphere = [material] () {return n4::sphere("huge").r(1*km).place(material).now();};

  // Gather together all the above actions
  auto test_action = [&] {
    return  (new n4::actions{isotropic_511keV_gammas()})
      -> set((new n4::stacking_action{}) -> classify(kill_secondaries))
      -> set(new n4::stepping_action{record_process_and_kill})
      ;
  };

  n4::run_manager::create()
    .fake_ui()
    .physics(physics)
    .geometry(enormous_sphere)
    .actions(test_action)
    .run(100000);

  auto total = static_cast<float>(phot + compt + rayl);
  return { phot/total, compt/total, rayl/total };
}
