#include <n4-sensitive.hh>

namespace nain4 {

sensitive_detector::sensitive_detector(G4String name, process_hits_fn process_hits)
: G4VSensitiveDetector{name}
, process_hits{process_hits} {
  fully_activate_sensitive_detector(this);
}

} // namespace nain4
