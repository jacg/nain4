#include <CLHEP/Geometry/Transform3D.h>
#include <G4ThreeVector.hh>
#include <cmath>
#include <n4-random.hh>

#include <numeric>
#include <stack>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wshadow"

namespace nain4 {
namespace random {

G4ThreeVector direction::get() {
  if (exclude_) { return excluded(); }

  auto phi       = uniform(min_phi_      , max_phi_      );
  auto cos_theta = uniform(min_cos_theta_, max_cos_theta_);
  auto sin_theta = std::sqrt(1 - cos_theta*cos_theta);
  G4ThreeVector out = {sin_theta * std::cos(phi),
                       sin_theta * std::sin(phi),
                       cos_theta};

  if (bidirectional_ && uniform() < 0.5       ) { out =   flip(out); }
  if (axis_          != G4ThreeVector{0, 0, 1}) { out = rotate(out); }
  return out;
}

G4ThreeVector direction::flip(const G4ThreeVector& in) {
  return {-in.x(), -in.y(), -in.z()};
}

G4ThreeVector direction::rotate(const G4ThreeVector& in) {
  if (!rot_matrix.has_value()) {
    auto rot_axis = axis_.cross({0, 0, 1});
    auto theta    = std::acos(axis_.z());
    rot_matrix    = HepGeom::Rotate3D{-theta, rot_axis}.getRotation();
    if (exclude_) { rot_matrix = rot_matrix.value().inverse(); }
  }
  return rot_matrix.value() * in;
}

G4ThreeVector direction::excluded() {
  auto must_be_excluded = [&] (const G4ThreeVector& d) {
    auto costheta = d.cosTheta();
    auto phi      = d.   phi  (); if (phi < 0) { phi += CLHEP::twopi; }
    return (costheta >= min_cos_theta_) &&
           (costheta <= max_cos_theta_) &&
           (phi      >= min_phi_      ) &&
           (phi      <= max_phi_      );
  };
  while (true) {
    auto original_dir = G4RandomDirection();
    auto dir = axis_ != G4ThreeVector{0, 0, 1} ? rotate(original_dir) : original_dir;


    if (                  must_be_excluded(     dir )) { continue; }
    if (bidirectional_ && must_be_excluded(flip(dir))) { continue; }
    return dir;
  }
}

// Going with rejection sampling, for now
// TODO: test done, now benchmark other approaches
G4ThreeVector random_in_sphere(G4double radius) {
  G4ThreeVector point;
  do {
    point = {uniform(-1, 1), uniform(-1, 1), uniform(-1, 1)};
  } while (point.mag2() > 1);
  return point * radius;
}

// TODO: test and benchmark
std::tuple<G4double, G4double> random_on_disc(G4double radius) {
  G4double x, y;
  do {
    x = uniform(-radius, radius);
    y = uniform(-radius, radius);
  } while (x*x + y*y > radius * radius);
  return {x, y};
}

// Stack utilities
using STACK = std::stack<unsigned>;
inline bool something_in(const STACK& stack) { return ! stack.empty(); }
unsigned pop(STACK& stack) { auto element = stack.top(); stack.pop(); return element; }

// Vose's adaptation of Walker's alias method
biased_choice::biased_choice(std::vector<G4double> weights)
  : prob (weights.size(), 0)
  , topup(weights.size(), 0)
{
  const unsigned N = weights.size();
  // Normalize histogram
  auto total_weight = std::accumulate(begin(weights), end(weights), 0e0);
  for (auto& weight : weights) {
    weight *= (N / total_weight);
  }
  // Separate into above-average and below-average probabilities
  std::stack<unsigned> under;
  std::stack<unsigned>  over;
  unsigned n = 0;
  for (auto weight : weights) {
    if (weight < 1) { under.push(n++); }
    else            {  over.push(n++); }
  }
  // Top up each underfilled bin, with portion of an overfilled bin
  while (something_in(over) && something_in(under)) {
    auto underfilled = pop(under);
    auto  overfilled = pop(over);
    prob [underfilled] = weights[underfilled];
    topup[underfilled] = overfilled;
    weights[overfilled] += weights[underfilled] - 1; // Odd order: more numerically stable calculation
    if (weights[overfilled] < 1) { under.push(overfilled); } // Became underfull: move to underfulls
    else                         {  over.push(overfilled); } // Still overfull: put back with overfulls
  }
  // Only above-average remain: fill their bins to the top
  while (something_in( over)) { auto i = pop( over); prob[i] = 1; }
  while (something_in(under)) { auto i = pop(under); prob[i] = 1; } // Impossible without numerical instability
}

unsigned biased_choice::operator()() const {
  auto n = fair_die(prob.size());
  return biased_coin(prob[n]) ? n : topup[n];
}

} // namespace random
} // namespace nain4

#pragma GCC diagnostic pop
