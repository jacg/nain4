#pragma once

#include <n4-all.hh>

#include <FTFP_BERT.hh>         // our choice of physics list
#include <G4Box.hh>             // for creating shapes in the geometry
#include <G4Event.hh>           // needed to inject primary particles into an event
#include <G4GenericMessenger.hh>
#include <G4PVPlacement.hh>
#include <G4PrimaryParticle.hh>
#include <G4RandomDirection.hh> // for launching particles in random directions
#include <G4Sphere.hh>          // for creating shapes in the geometry
#include <G4String.hh>
#include <G4SystemOfUnits.hh>   // physical units such as `m` for metre
#include <G4ThreeVector.hh>

#include <memory>


class my {
public:
  G4double       straw_radius{0.1 * m};
  G4double      bubble_radius{0.2 * m};
  G4double      socket_rot   {-90 * deg};
  G4String      particle_name{"geantino"};
  G4double      particle_energy{511 * keV};
  G4ThreeVector particle_dir {};
  G4int         phys_verbosity{0};
  my();


private:
  std::unique_ptr<G4GenericMessenger> messenger;

};

std::function<void (G4Event*)> my_generator(const my& my);

n4::actions* create_actions(my& my, unsigned& n_event);

G4PVPlacement* my_geometry(const my& my);
