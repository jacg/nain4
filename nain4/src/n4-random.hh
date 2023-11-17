#pragma once

#include "G4RotationMatrix.hh"
#include <CLHEP/Geometry/Transform3D.h>
#include <CLHEP/Units/SystemOfUnits.h>
#include <CLHEP/Vector/Rotation.h>
#include <G4RandomDirection.hh>
#include <G4ThreeVector.hh>
#include <G4Types.hh>
#include <Randomize.hh>
#include <optional>
#include <stdexcept>

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
  G4ThreeVector get() const;

#define CHECK_RANGE(NAME, X, LOWER, UPPER)                \
  if ( (X < LOWER) || (X > UPPER) ) {                     \
    throw std::runtime_error("Invalid limit for " #NAME); \
  }
#define SET(NAME, LOWER, UPPER)        \
  direction& NAME(G4double x) {        \
    CHECK_RANGE(NAME, x, LOWER, UPPER) \
    NAME##_ = x ;                      \
  return *this;                        \
}

  SET(min_cos_theta,             -1, max_cos_theta_)
  SET(max_cos_theta, min_cos_theta_,              1)
  SET(min_phi      ,              0, max_phi_      )
  SET(max_phi      ,       min_phi_, CLHEP::twopi  )

  direction& rotate  (G4RotationMatrix&     m){ rotate_=true; rotation = m * rotation            ; return       *this;}
  direction& rotate_x(G4double          delta){ auto rot = G4RotationMatrix{}; rot.rotateX(delta); return rotate(rot);}
  direction& rotate_y(G4double          delta){ auto rot = G4RotationMatrix{}; rot.rotateY(delta); return rotate(rot);}
  direction& rotate_z(G4double          delta){ auto rot = G4RotationMatrix{}; rot.rotateZ(delta); return rotate(rot);}

  // Might seem wrong, but it's not!
  direction& min_theta(G4double x) {
    CHECK_RANGE(min_theta, x, 0, std::acos(min_cos_theta_))
    max_cos_theta(std::cos(x));
    return *this;
  }
  direction& max_theta(G4double x) {
    CHECK_RANGE(max_theta, x, std::acos(max_cos_theta_), CLHEP::pi)
    min_cos_theta(std::cos(x));
    return *this;
  }

  direction& bidirectional() {bidirectional_ = true; return *this; }
  direction& exclude      () {exclude_       = true; return *this; }

#undef CHECK_RANGE
#undef SET

private:
  G4RotationMatrix rotation = CLHEP::HepRotation::IDENTITY;
  G4bool           rotate_ {false};

  G4bool   bidirectional_{false};
  G4bool   exclude_      {false};
  G4double min_cos_theta_{-1};
  G4double max_cos_theta_{ 1};
  G4double min_phi_      { 0};
  G4double max_phi_      {CLHEP::twopi};

  G4ThreeVector flip         (const G4ThreeVector& in) const;
  G4ThreeVector rotate_vector(const G4ThreeVector& in) const;
  G4ThreeVector excluded     (                       ) const;
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
