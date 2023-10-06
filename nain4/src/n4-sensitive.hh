#pragma once

#include <G4VSensitiveDetector.hh>
#include <G4SDManager.hh>

namespace nain4 {

// TODO: sensitive_detector needs tests
class sensitive_detector : public G4VSensitiveDetector {
public:
  using process_hits_fn = std::function<bool(G4Step*)>;
  using initialize_fn   = std::function<void(G4HCofThisEvent*)>;
  using end_of_event_fn = std::function<void(G4HCofThisEvent*)>;

  sensitive_detector* initialize  (initialize_fn   f) { init = f; return this; }
  sensitive_detector* end_of_event(end_of_event_fn f) { eoev = f; return this; }

  sensitive_detector(G4String name, process_hits_fn process_hits);
  bool ProcessHits(G4Step* step, G4TouchableHistory*) override { return process_hits(step); };
  void Initialize (G4HCofThisEvent* hc)               override {        init        (hc  ); };
  void EndOfEvent (G4HCofThisEvent* hc)               override {        eoev        (hc  ); };
private:
  process_hits_fn process_hits;
  initialize_fn   init = [] (auto) {};
  end_of_event_fn eoev = [] (auto) {};
};
// --------------------------------------------------------------------------------
template<class SENSITIVE>
auto fully_activate_sensitive_detector(SENSITIVE* detector) {
  detector -> Activate(true);
  G4SDManager::GetSDMpointer() -> AddNewDetector(detector);
  return detector;
}

} // namespace nain4

namespace n4 { using namespace nain4; }
