#ifndef GENERATOR_HH
#define GENERATOR_HH

#include <G4Types.hh>
#include <G4ThreeVector.hh> // Can't forward-declare because it's a typedef

class G4Event;

void generate_back_to_back_511_keV_gammas(G4Event* event, G4ThreeVector position, G4double time);

#endif // GENERATOR_HH
