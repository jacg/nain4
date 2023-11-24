#pragma once

#include <G4Material.hh>
#include <G4ParticleDefinition.hh>
#include <G4VUserPhysicsList.hh>

struct test_config {
  G4VUserPhysicsList*   physics;
  G4Material*           material;
  G4ParticleDefinition* particle;
  double                particle_energy;
};


std::vector<std::pair<double, double>> measure_abslength(test_config const&);
