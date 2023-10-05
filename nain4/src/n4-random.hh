#pragma once

#include <G4ThreeVector.hh>
#include <G4Types.hh>
#include <Randomize.hh>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wshadow"

namespace nain4 {

namespace random {

// Random result generation utilities
inline G4double uniform    ()                         { return G4Random().flat(); }
inline G4double uniform    (G4double lo, G4double hi) { return (hi - lo) * uniform() + lo; }
inline bool     biased_coin(G4double chance_of_true)  { return uniform() < chance_of_true; }
inline unsigned fair_die   (unsigned sides)           { return std::floor(uniform() * sides); }

class biased_choice {
public:

  biased_choice(std::vector<G4double> weights);
  unsigned operator()() const;

private:
  std::vector<G4double> prob;
  std::vector<unsigned> topup;
};

G4ThreeVector random_in_sphere(G4double radius);
std::tuple<G4double, G4double> random_on_disc(G4double radius);

} // namespace random
} // namespace nain4

namespace n4 { using namespace::nain4; }

#pragma GCC diagnostic pop
