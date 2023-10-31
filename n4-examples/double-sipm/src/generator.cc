#include "generator.hh"

#include <n4-random.hh>
#include <n4-inspect.hh>

#include <G4Event.hh>
#include <G4PrimaryParticle.hh>
#include <G4SystemOfUnits.hh>
#include <Randomize.hh>

void generate_back_to_back_511_keV_gammas(G4Event* event) {
    static auto gamma = n4::find_particle("gamma");

    auto source_radius =  1.5*mm;
    auto  scint_size   =  3  *mm;
    auto   ring_radius = 12.5*mm;
    auto [y, z] = n4::random::random_on_disc(source_radius);

    // Not strictly correct (uniformly distributed in the plane, but not by
    // angle), OK with small angle approximation, which we exceed slightly:
    // atan(1.5 / 12.5) = 7°
    auto direction = G4ThreeVector{ n4::random::uniform_width( scint_size + 0)
                                  , n4::random::uniform_width( scint_size + y)
                                  ,                           ring_radius + z
                                  }.unit(); // random unit vector which hits scintillator
    auto p = 0.511*MeV * direction;

    auto time   = 0;
    auto vertex = new G4PrimaryVertex({0,y,z}, time);
    vertex -> SetPrimary(new G4PrimaryParticle(gamma,  p.x(),  p.y(),  p.z()));
    vertex -> SetPrimary(new G4PrimaryParticle(gamma, -p.x(), -p.y(), -p.z()));
    event  -> AddPrimaryVertex(vertex);
}
