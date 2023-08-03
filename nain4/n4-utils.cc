#include "n4-utils.hh"

#include <G4ThreeVector.hh>
#include <numeric>
#include <stack>

namespace nain4 {


std::vector<G4double> linspace(G4double start, G4double stop, size_t n_entries) {
  auto step = (stop - start) / (n_entries - 1);
  std::vector<G4double> output(n_entries);

  std::generate(begin(output), end(output), [start, step, i=0] () mutable { return start + i++ * step; } );

  return output;
}

namespace random {

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
