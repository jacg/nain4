#include "generator.hh"

#include <n4-all.hh>

#include <G4Event.hh>
#include <G4PrimaryParticle.hh>
#include <G4SystemOfUnits.hh>
#include <Randomize.hh>

void generate_back_to_back_511_keV_gammas(G4Event* event) {
    static auto gamma = n4::find_particle("gamma");

    auto source_radius =  1.5*mm;
    auto  scint_size   =  3  *mm;
    auto   ring_radius = 12.5*mm;
    auto position = G4RandomDirection() * source_radius * G4UniformRand();
    position.setX(0);

    auto direction = G4ThreeVector{ n4::random::uniform_width( scint_size + position.getX())
                                  , n4::random::uniform_width( scint_size + position.getY())
                                  ,                           ring_radius + position.getZ()
                                  }.unit(); // random unit vector which hits scintillator
    auto p = 0.511*MeV * direction;

    auto time   = 0;
    auto vertex = new G4PrimaryVertex(position, time);
    vertex -> SetPrimary(new G4PrimaryParticle(gamma,  p.x(),  p.y(),  p.z()));
    vertex -> SetPrimary(new G4PrimaryParticle(gamma, -p.x(), -p.y(), -p.z()));
    event  -> AddPrimaryVertex(vertex);
}
