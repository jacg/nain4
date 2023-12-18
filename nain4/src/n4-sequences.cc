#include <n4-sequences.hh>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wshadow"

namespace nain4 {

std::vector<G4double> linspace(G4double start, G4double stop, size_t n_entries) {
  if (n_entries == 0) { return {     }; }
  if (n_entries == 1) { return {start}; }
  auto step = (stop - start) / (n_entries - 1);
  std::vector<G4double> output(n_entries);

  std::generate(begin(output), end(output), [start, step, i=0] () mutable { return start + i++ * step; } );

  return output;
}

std::vector<G4double> scale_by(G4double factor, std::initializer_list<G4double> const& data) {
  std::vector<G4double> out;
  out.reserve(data.size());
  std::transform(cbegin(data), cend(data), back_inserter(out), [factor](auto d){ return d*factor; });
  return out;
}

std::vector<G4double> const_over(G4double factor, std::initializer_list<G4double> const& data) {
  std::vector<G4double> out;
  out.reserve(data.size());
  std::transform(cbegin(data), cend(data), back_inserter(out), [factor](auto d){ return factor/d; });
  return out;
}


std::function<std::optional<double>(double)> interpolator(const std::vector<double> x, const std::vector<double> y) {
  return [x=std::move(x), y=std::move(y)] (double xk) -> std::optional<double> {
    for (const auto& [i, x1] : enumerate(x) ) {
      if (xk < x1) {
        if (i==0) { return {}; }

        auto x0 = x[i-1];
        auto y0 = y[i-1];
        auto y1 = y[i  ];
        return y0 + (xk - x0) * (y1 - y0)/(x1 - x0);
      }
    }
    return {};
  };
}

} // namespace nain4

#pragma GCC diagnostic pop
