#pragma once

#include <G4Material.hh>
#include <G4ParticleDefinition.hh>
#include <G4VUserPhysicsList.hh>

struct abslength_config {
  G4VUserPhysicsList* physics;
  G4Material*         material;
  std::string         particle_name;
  double              particle_energy;
  std::vector<double> distances;
  unsigned            n_events;
};


std::vector<double> measure_abslength(abslength_config const&);


// ----- Calculate 511 keV gamma interaction process fractions for given material ------------------------------
struct interaction_process_fractions { double photoelectric, compton, rayleigh; };
interaction_process_fractions calculate_interaction_process_fractions(G4Material*, G4VUserPhysicsList*);
