#include <n4-mandatory.hh>

#include <G4Run.hh>

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
// --------------------------------------------------------------------------------
// definition of sensitive_detector
sensitive_detector::sensitive_detector(G4String name, process_hits_fn process_hits)
: G4VSensitiveDetector{name}
, process_hits{process_hits} {
  fully_activate_sensitive_detector(this);
}

#pragma GCC diagnostic pop

} // namespace nain4
