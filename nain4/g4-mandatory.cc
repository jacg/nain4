#include "g4-mandatory.hh"

#include "nain4.hh"

#include <G4Run.hh>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wshadow"

namespace nain4 {

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
  auto geantino  = nain4::find_particle("geantino");
  auto p         = 1 * CLHEP::MeV;
  auto vertex    = new G4PrimaryVertex();
  vertex->SetPrimary(new G4PrimaryParticle(geantino, p, 0, 0));
  event->AddPrimaryVertex(vertex);
}
// --------------------------------------------------------------------------------
// definition of sensitive_detector
sensitive_detector::sensitive_detector(G4String name, process_hits_fn process_hits)
: G4VSensitiveDetector{name}
, process_hits{process_hits} {
  fully_activate_sensitive_detector(this);
}

} // namespace nain4

#pragma GCC diagnostic pop
