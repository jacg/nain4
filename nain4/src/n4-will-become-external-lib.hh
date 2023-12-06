#pragma once

#include <G4Material.hh>
#include <G4ParticleDefinition.hh>
#include <G4VUserPhysicsList.hh>

// Configuration of abslength estimation: use with `measure_abslength`
struct interaction_length_config {
  G4VUserPhysicsList* physics;
  G4Material*         material;
  std::string         particle_name;
  double              particle_energy;
  // fulanito
  std::vector<double> distances;
  // bla bla bla
  unsigned            n_events;
};

// Comment in header
std::vector<double> measure_abslength(interaction_length_config const&);


// ----- Calculate 511 keV gamma interaction process fractions for given material ------------------------------
struct interaction_process_fractions { double photoelectric, compton, rayleigh; };
interaction_process_fractions calculate_interaction_process_fractions(G4Material*, G4VUserPhysicsList*);
