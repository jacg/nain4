#include <n4-sequences.hh>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wshadow"

namespace nain4 {

std::vector<G4double> linspace(G4double start, G4double stop, size_t n_entries) {
  auto step = (stop - start) / (n_entries - 1);
  std::vector<G4double> output(n_entries);

  std::generate(begin(output), end(output), [start, step, i=0] () mutable { return start + i++ * step; } );

  return output;
}

std::vector<G4double> scale_by(G4double factor, std::initializer_list<G4double> const& data) {
  std::vector<G4double> out;
  out.reserve(data.size());
  std::transform(begin(data), end(data), back_inserter(out), [factor](auto d){ return d*factor; });
  return out;
}

std::vector<G4double> const_over(G4double factor, std::initializer_list<G4double> const& data) {
  std::vector<G4double> out;
  out.reserve(data.size());
  std::transform(begin(data), end(data), back_inserter(out), [factor](auto d){ return factor/d; });
  return out;
}

} // namespace nain4

#pragma GCC diagnostic pop
