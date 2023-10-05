#pragma once

#include <n4-inspect.hh>
#include <n4-place.hh>
#include <n4-mandatory.hh>
#include <n4-material.hh>
#include <n4-volume.hh>


#include <G4Event.hh>
#include <G4PrimaryVertex.hh>
#include <G4PrimaryParticle.hh>
#include <FTFP_BERT.hh>
#include <G4Box.hh>


inline void shoot(G4Event* event) {
  auto particle = n4::find_particle("geantino");
  auto vertex = new G4PrimaryVertex{};
  vertex -> SetPrimary(new G4PrimaryParticle{particle, 1, 0, 0});
  event  -> AddPrimaryVertex(vertex);
}


inline auto dummy_physics_list() {
  auto verbosity = 0;
  return new FTFP_BERT{verbosity};
}

inline auto dummy_geometry() {
  auto water = n4::material("G4_WATER");
  auto box   = n4::volume<G4Box>("box", water, 1., 1., 1.);
  return n4::place(box).now();
}

inline auto dummy_actions() {
  return new n4::actions(shoot);
}
