#pragma once

#include <n4-all.hh>

#include <G4PrimaryParticle.hh>
#include <G4PVPlacement.hh>
#include <G4String.hh>
#include <G4SystemOfUnits.hh>   // physical units such as `m` for metre
#include <G4Event.hh>           // needed to inject primary particles into an event
#include <G4Box.hh>             // for creating shapes in the geometry
#include <G4Sphere.hh>          // for creating shapes in the geometry
#include <FTFP_BERT.hh>         // our choice of physics list
#include <G4RandomDirection.hh> // for launching particles in random directions
#include <G4ThreeVector.hh>


struct my {
  G4double       straw_radius{0.1 * m};
  G4double      bubble_radius{0.2 * m};
  G4double      socket_rot   {-90 * deg};
  G4String      particle_name{"geantino"};
  G4double      particle_energy{511 * keV};
  G4ThreeVector particle_dir {};
};

auto my_generator(const my& my);

n4::actions* create_actions(my& my, unsigned& n_event);

G4PVPlacement* my_geometry(const my& my);
