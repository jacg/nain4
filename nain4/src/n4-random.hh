#pragma once

#include <CLHEP/Units/SystemOfUnits.h>
#include <G4RandomDirection.hh>
#include <G4ThreeVector.hh>
#include <G4Types.hh>
#include <Randomize.hh>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wshadow"

namespace nain4 {

namespace random {

// Random result generation utilities
inline G4double uniform           ()                          { return G4Random().flat(); }
inline G4double uniform           (G4double lo, G4double  hi) { return (hi - lo) * uniform() + lo; }
inline G4double uniform_half_width(G4double dx              ) { return uniform(-dx, dx); }
inline G4double uniform_width     (G4double dx              ) { return uniform_half_width(dx/2); }

struct direction {
  G4ThreeVector get();

#define SET(NAME) direction& NAME(G4double x) {NAME##_ = x ; return *this;}
  SET(min_cos_theta)
  SET(max_cos_theta)
  SET(min_phi)
  SET(max_phi)
#undef SET

  // Might seem wrong, but it's not!
  direction& min_theta(G4double x) {max_cos_theta_ = std::cos(x); return *this;}
  direction& max_theta(G4double x) {min_cos_theta_ = std::cos(x); return *this;}

private:
  G4double min_cos_theta_ = -1;
  G4double max_cos_theta_ =  1;
  G4double min_phi_       =  0;
  G4double max_phi_       = CLHEP::twopi;
};


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
