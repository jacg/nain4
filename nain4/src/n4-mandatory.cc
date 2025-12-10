#include <n4-mandatory.hh>

#include <G4ClassificationOfNewTrack.hh>
#include <G4Run.hh>

#include <memory>

namespace nain4 {

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wshadow"

// ----- actions --------------------------------------------------------------------
void actions::Build() const {
  SetUserAction(generator_);
  if (  run_) { SetUserAction(  run_); }
  if (event_) { SetUserAction(event_); }
  if ( step_) { SetUserAction( step_); }
  if (track_) { SetUserAction(track_); }
  if (stack_) { SetUserAction(stack_); }
}
// ----- primary generator -----------------------------------------------------------
void generator::geantino_along_x(G4Event* event) {
  // TODO this doesn't really belong in n4 itself
  // auto geantino  = nain4::find_particle("geantino");
  // auto p         = 1 * CLHEP::MeV;
  // auto vertex    = new G4PrimaryVertex();
  // vertex->SetPrimary(new G4PrimaryParticle(geantino, p, 0, 0));
  // event->AddPrimaryVertex(vertex);
}

action_builder::action_builder(G4VUserPrimaryGeneratorAction* generator)
    : actions_            {std::make_unique<actions>(generator)},
      run_begin_          {},
      run_end_            {},
      event_begin_        {},
      event_end_          {},
      stacking_classify_  {},
      stacking_next_stage_{},
      stacking_next_event_{},
      tracking_pre_       {},
      tracking_post_      {},
      stepping_           {}
{}

#define CALL_ACTIONS(ACT, VAR, TYPE, METHOD)                                   \
  if (!VAR.empty()) {                                                          \
    auto doit = [it=std::move(VAR)](TYPE arg) {                                \
      for (auto& f : it) f(arg);                                               \
      };                                                                       \
    ACT -> METHOD(doit);                                                       \
  }

actions* action_builder::done() {
  if (!run_begin_.empty() ||
      !run_end_  .empty()  ) {
    auto run = new n4::run_action{};
    CALL_ACTIONS(run, run_begin_, const G4Run*, begin)
    CALL_ACTIONS(run, run_end_  , const G4Run*,   end)
    actions_ -> set(run);
  }

  if (!event_begin_.empty() ||
      !event_end_  .empty()  ) {
    auto event = new n4::event_action{};
    CALL_ACTIONS(event, event_begin_, const G4Event*, begin)
    CALL_ACTIONS(event, event_end_  , const G4Event*,   end)
    actions_ -> set(event);
  }

  if (!stacking_classify_  .empty() ||
      !stacking_next_event_.empty() ||
      !stacking_next_stage_.empty()  ) {
    auto stacking = new n4::stacking_action{};
    CALL_ACTIONS(stacking, stacking_next_stage_, G4StackManager* const, next_stage)

    // special cases
    if (!stacking_classify_.empty()) {
      auto doit = [it=std::move(stacking_classify_)](G4Track const *arg) {
        for (auto &f : it) {
          // By default, G4 tracks everything eagerly, so we assume the same.
          auto c = f(arg);
          if (c != G4ClassificationOfNewTrack::fUrgent) return c;
        }
        return G4ClassificationOfNewTrack::fUrgent;
      };
      stacking -> classify(doit);
    }

    if (!stacking_next_event_.empty()) {
      auto doit = [it=std::move(stacking_next_event_)]() {
        for (auto& f : it) f();
      };
      stacking -> next_event(doit);
    }

    actions_ -> set(stacking);
  }

  if (!tracking_pre_ .empty() ||
      !tracking_post_.empty()  ) {
    auto tracking = new n4::tracking_action{};
    CALL_ACTIONS(tracking, tracking_pre_ , const G4Track*,  pre)
    CALL_ACTIONS(tracking, tracking_post_, const G4Track*, post)
    actions_ -> set(tracking);
  }

  if (!stepping_.empty()) {
    auto stepping = new n4::stepping_action{};
    CALL_ACTIONS(stepping, stepping_, const G4Step*, set)
    actions_ -> set(stepping);
  }
  return actions_.release();
}

#undef CALL_ACTIONS


#pragma GCC diagnostic pop

} // namespace nain4
