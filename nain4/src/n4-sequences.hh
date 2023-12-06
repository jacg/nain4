#pragma once

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wshadow"

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

// returns the tuple ([x1, x2, ... , xN], [f(x1), f(x2), ... , f(xN)])
template<class F> auto interpolate(F f, size_t N, double min, double max) {
  std::vector<double> xs(N);
  std::vector<double> ys(N);
  auto step = (max - min) / (N-1);
  size_t n = 0;
  generate ( begin(xs),  end(xs), [min, step, &n](){ return  min + (n++ * step); });
  transform(cbegin(xs), cend(xs), begin(ys), f);
  return make_tuple(xs, ys);
}

template<class T>
std::vector<T> vec_with_capacity(size_t N) {
  std::vector<T> vec;
  vec.reserve(N);
  return std::move(vec);
}

// --------------------------------------------------------------------------------
// Utility for creating a vector of physical quantity data, without having to
// repeat the physical unit in each element.

// TODO const version?
std::vector<G4double> scale_by  (G4double factor, std::initializer_list<G4double> const& data);
std::vector<G4double> const_over(G4double factor, std::initializer_list<G4double> const& data);

inline std::tuple<G4double, G4double, G4double> unpack(const G4ThreeVector& v){ return {v.x(), v.y(), v.z()};};

} // namespace nain4

namespace n4 { using namespace nain4; }

#pragma GCC diagnostic pop
