#pragma once

#include <G4ThreeVector.hh>
#include <Randomize.hh>
#include <G4Types.hh>

#include <algorithm>

namespace nain4 {

template <typename T,
          typename TIter = decltype(std::begin(std::declval<T>())),
          typename       = decltype(std::  end(std::declval<T>()))>
constexpr auto enumerate(T&& iterable) {
  struct iterator {
    size_t i;
    TIter iter;
    bool operator != (const iterator& other) const { return this->iter != other.iter; }
    void operator ++ ()       {               ++i; ++iter; }
    auto operator *  () const { return std::tie(i,  *iter); }
  };
  struct iterable_wrapper {
    T iterable;
    auto begin() { return iterator{0, std::begin(iterable)}; }
    auto end  () { return iterator{0, std::  end(iterable)}; }
  };
  return iterable_wrapper{ std::forward<T>(iterable) };
}

template <typename T>
constexpr auto enumerate(const std::initializer_list<T>& data) {
  struct iterator {
    size_t i;
    decltype(std::begin(std::vector<T>())) iter;
    bool operator != (const iterator& other) const { return this->iter != other.iter; }
    void operator ++ ()       {               ++i; ++iter; }
    auto operator *  () const { return std::tie(i,  *iter); }
  };
  struct iterable_wrapper {
    std::vector<T> iterable;
    auto begin() { return iterator{0, std::begin(iterable)}; }
    auto end  () { return iterator{0, std::  end(iterable)}; }
  };
  return iterable_wrapper{ data };
}

std::vector<G4double> linspace(G4double start, G4double stop, size_t n_entries);

template<class O, class I, class F> std::vector<O> map(F f, I const& input) {
  std::vector<O> output(std::distance(begin(input), end(input)));
  std::transform(begin(input), end(input), begin(output), f);
  return output;
}

// --------------------------------------------------------------------------------
// Utility for creating a vector of physical quantity data, without having to
// repeat the physical unit in each element.

// TODO const version?
std::vector<G4double> scale_by  (G4double factor, std::initializer_list<G4double> const& data);
std::vector<G4double> const_over(G4double factor, std::initializer_list<G4double> const& data);

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

namespace n4 { using namespace nain4; }
