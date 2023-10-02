#include "generator.hh"

#include <g4-mandatory.hh>
#include <nain4.hh>

#include <CLHEP/Vector/ThreeVector.h>
#include <FTFP_BERT.hh>
#include <G4Event.hh>
#include <G4RandomDirection.hh>
#include <G4SystemOfUnits.hh>
#include <G4ThreeVector.hh>

void generate_back_to_back_511_keV_gammas(G4Event* event, G4ThreeVector position, G4double time) {
    auto gamma = n4::find_particle("gamma");

    G4double source_radius = 1.5*mm;
    position = G4RandomDirection() * source_radius * G4UniformRand();
    position.setX(0);

    G4double y_offset = 3*mm + position.getY();
    auto direction = G4ThreeVector{     3*mm * G4UniformRand() - 1.5*mm
                                  , y_offset * G4UniformRand() - (y_offset/2)
                                  , 12.5}.unit(); // random unit vector which hits scintillator
    auto p = 0.511*MeV * direction;

    auto vertex = new G4PrimaryVertex(position, time);
    vertex -> SetPrimary(new G4PrimaryParticle(gamma,  p.x(),  p.y(),  p.z()));
    vertex -> SetPrimary(new G4PrimaryParticle(gamma, -p.x(), -p.y(), -p.z()));
    event -> AddPrimaryVertex(vertex);
}
