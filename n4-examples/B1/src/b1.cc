#include "n4_run_manager.hh"
#include "nain4.hh"
#include "g4-mandatory.hh"
#include "n4_ui.hh"
#include "n4-volumes.hh"

#include <G4Exception.hh>
#include <G4ExceptionSeverity.hh>
#include <G4GenericMessenger.hh>

#include <G4PrimaryParticle.hh>
#include <G4String.hh>
#include <G4SystemOfUnits.hh>
#include <G4Event.hh>
#include <G4Box.hh>
#include <G4Sphere.hh>
#include <G4SteppingVerbose.hh>
#include <QBBC.hh>
#include <G4RandomDirection.hh>


#include <G4ThreeVector.hh>
#include <G4UnitsTable.hh>
#include <cstdlib>


auto geometry() {
  // Envelope parameters
  G4double env_size_xy = 20*cm;
  G4double env_size_z  = 30*cm;

  // World
  auto world = n4::box("World")
    .xy(1.2 * env_size_xy)
    .z (1.2 * env_size_z)
    .place(n4::material("G4_AIR"))
    .now();

  // Envelope
  auto envelope = n4::box("Envelope")
    .xy(env_size_xy)
    .z (env_size_z )
    .place(n4::material("G4_WATER"))
    .in(world)
    .now();

  // Shape 1
  n4::cons("Shape1")
    .r1_inner(0*cm)
    .r1      (2*cm)
    .r2_inner(0*cm)
    .r2      (4*cm)
    .z       (6*cm)
    .place(n4::material("G4_A-150_TISSUE"))
    .in(envelope)
    .at(0, 2*cm, -7*cm)
    .now();

  // Shape 2
  n4::trd("Shape2")
    .x1(12*cm)
    .y1(10*cm)
    .x2(12*cm)
    .y2(16*cm)
    .z ( 6*cm)
    .place(n4::material("G4_BONE_COMPACT_ICRU"))
    .in(envelope)
    .at(0, -1*cm, 7*cm)
    .now();

  //always return the physical World
  return world;

}


auto generator() {
  auto particle     = G4ParticleTable::GetParticleTable() -> FindParticle("gamma");
  auto particle_gun = std::make_unique<G4ParticleGun>(particle, 1); // 1 particle
  particle_gun -> SetParticleMomentumDirection({0.,0.,1.});
  particle_gun -> SetParticleEnergy(6*MeV);

  return [particle_gun=std::move(particle_gun)] (auto event) {
    auto envelope = dynamic_cast<G4Box*>(n4::find_logical("Envelope") -> GetSolid());
    auto env_size_xy = envelope -> GetXHalfLength() * 2;
    auto env_size_z  = envelope -> GetZHalfLength() * 2;

    auto size = 0.8;
    auto x0   = size * env_size_xy * (G4UniformRand() - 0.5);
    auto y0   = size * env_size_xy * (G4UniformRand() - 0.5);
    auto z0   = -0.5 * env_size_z;

    particle_gun -> SetParticlePosition({x0, y0, z0});
    particle_gun -> GeneratePrimaryVertex(event);
  };
}

auto run_action(G4double& e_sum, G4double& e_sum2) {
  return (new n4::run_action())
    -> end([&] (auto run){
         G4int n_events = run -> GetNumberOfEvent();
         if (n_events == 0) return;

         // Compute dose = total energy deposit in a run and its variance
         G4double rms = e_sum2 - e_sum*e_sum/n_events;
         if (rms < 0) {
           G4Exception("run_action", "end()", FatalException, "Negative RMS");
         }
         rms = std::sqrt(rms);

         auto scoring_volume = n4::find_logical("Shape2");
         G4double mass     = scoring_volume -> GetMass();
         G4double dose     = e_sum / mass;
         G4double dose_rms =  rms / mass;

         std::cout << "\n"
                   << "The run consists of " << n_events << " gamma of 6 MeV\n"
                   << " Cumulated dose per run, in scoring volume : "
                   << G4BestUnit(dose, "Dose") << " rms = " << G4BestUnit(dose_rms, "Dose")
                   << "\n------------------------------------------------------------\n"
                   << G4endl;

       });
}

auto event_action(G4double& e_sum, G4double& e_sum2, G4double& e_evt) {
  return (new n4::event_action())
    -> begin([&] (auto) {e_evt = 0;})
    ->   end([&] (auto) {e_sum += e_evt; e_sum2 += e_evt * e_evt;});
}

auto stepping_action(G4double& e_evt) {
  return new n4::stepping_action([&] (auto step) {
    auto scoring_volume = n4::find_logical("Shape2");
    auto current_volume = step -> GetPreStepPoint() -> GetTouchableHandle() -> GetVolume() -> GetLogicalVolume();

    if (current_volume == scoring_volume) {
      e_evt += step -> GetTotalEnergyDeposit();
    }
  });
}


auto create_actions(G4double& e_sum, G4double& e_sum2, G4double& e_evt) {
  return (new n4::actions{generator()})
    -> set(     run_action(e_sum, e_sum2       ))
    -> set(   event_action(e_sum, e_sum2, e_evt))
    -> set(stepping_action(               e_evt));
}


int main(int argc, char* argv[]) {
  // WHAT DOES THIS DO???
  // I think it has to do with /tracking/verbose
  // But the default is precision=4
  G4int precision = 4;
  G4SteppingVerbose::UseBestUnit(precision);

  auto check_overlaps = false;
  G4double e_sum{0}, e_sum2{0}, e_evt{0};

  if (check_overlaps) { n4::place::check_overlaps_switch_on(); }

  n4::run_manager::create()
    .ui("example-b1", argc, argv)
    .macro_path("macs")
    .physics<QBBC>(0) // verbosity 0
    .geometry(geometry)
    .actions(create_actions(e_sum, e_sum2, e_evt))
    .run();
}
