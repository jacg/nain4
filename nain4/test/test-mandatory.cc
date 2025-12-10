#include <n4-defaults.hh>
#include <n4-inspect.hh>
#include <n4-mandatory.hh>

#include <catch2/catch_approx.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_contains.hpp>

using namespace n4::test;


void a_geantino(G4Event* event) {
  auto geantino  = n4::find_particle("geantino");
  auto vertex    = new G4PrimaryVertex();
  vertex->SetPrimary(new G4PrimaryParticle(geantino, 1, 0, 0));
  event->AddPrimaryVertex(vertex);
};

TEST_CASE("nain action builder all methods", "[nain][action-builder]") {
  /// Call all possible methods and count the number of calls. Each method is
  /// expected to be called exactly once. This is not true in general, but we
  /// intentionally use a geantino, which does not interact, in a single volume.
  /// This is the general workflow:
  ///  1. The run will start (run_begin)
  ///  2. It will generate one event (event_begin)
  ///  3. One track will be generated for each particle. This track is
  ///     classified (stacking_classify)
  ///  4. Tracking begins (tracking_pre)
  ///  5. The geantino will be transported. It performs exactly one step from
  ///     origin to the world boundary (stepping)
  ///     5.1 If new particles were generated, it would perform step 3 for each
  ///         of them
  ///  6. Tracking ends (tracking_post)
  ///  7. Tracks with the same priority will be tracked (none, but it would
  ///     repeat steps 4-6)
  ///  8. Preparation for tracking lower priority tracks (stacking_next_stage)
  ///  9. Tracks with lower priority will be tracked (none, but it would repeat
  ///     steps 4-8)
  /// 10. After processing all tracks, prepare for next event
  ///     (stacking_next_event)
  /// 11. Event ends (event_end), repeat steps 2-10 for each subsequent event
  /// 12. Run ends (run_end)

  auto ncalls = 0;
  auto the_actions = [&ncalls]() {
    // Dummy actions. Three variants to satisfy all signatures
    auto void_arg  = [&](auto) {ncalls++;};
    auto void_void = [&](    ) {ncalls++;};
    auto  res_arg  = [&](auto) {ncalls++; return G4ClassificationOfNewTrack::fUrgent;};

    return n4::action_builder{a_geantino}
        .run_begin           (void_arg ) // + 1
        .run_end             (void_arg ) // + 1
        .event_begin         (void_arg ) // + 1
        .event_end           (void_arg ) // + 1
        .stacking_classify   ( res_arg ) // + 1
        .stacking_next_stage (void_arg ) // + 1
        .stacking_next_event (void_void) // + 1
        .tracking_pre        (void_arg ) // + 1
        .tracking_post       (void_arg ) // + 1
        .stepping            (void_arg ) // + 1
        .done                ();         // = 10
  };

  auto shutup = n4::silence{std::cout};

  n4::run_manager::create()
    .fake_ui()
    .physics(default_physics_lists)
    .geometry(water_box)
    .actions(the_actions)
    .run(1);

  CHECK(ncalls==10);
}

TEST_CASE("nain action builder combine actions", "[nain][action-builder]") {
  /// Add multiple actions to the same method. The number of calls should add up

  auto nsum = 0;
  auto the_actions = [&nsum]() {
    auto add = [&](size_t n) { return [&nsum, n](auto) {nsum+=n;}; };
    return n4::action_builder{a_geantino}
        .event_begin(add(1))
        .event_begin(add(2))
        .event_begin(add(3))
        .done     ();
  };

  auto shutup = n4::silence{std::cout};

  n4::run_manager::create()
    .fake_ui()
    .physics(default_physics_lists)
    .geometry(water_box)
    .actions(the_actions)
    .run(7);

  CHECK(nsum==7*(1+2+3)); // 7 events times (1 + 2 + 3)
}
