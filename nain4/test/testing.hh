#include <n4-testing.hh>

#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include <catch2/catch_test_macros.hpp>

#include <ostream>

using Catch::Matchers::WithinAbs;
using Catch::Matchers::WithinRel;
using Catch::Matchers::WithinULP;
using CLHEP::pi;
using CLHEP::halfpi;
using CLHEP::twopi;

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wshadow"


inline Catch::Matchers::WithinUlpsMatcher Within1ULP(double x) { return Catch::Matchers::WithinULP(x, 1); }

inline void check_all_within1ULP(std::vector<double> x, double ref) {
  for (auto& xi: x) {
    CHECK_THAT(xi, Within1ULP(ref));
  }
}

inline void check_all_within1ULP(std::vector<double> x, std::vector<double> ref) {
  REQUIRE(x.size() == ref.size());
  for (auto i = 0; i<x.size(); i++) {
    CHECK_THAT(x[i], Within1ULP(ref[i]));
  }
}

// Helper for writing tests about randomly-generated 3D vectors
struct threevec_stats {
  using sampler = std::function<G4ThreeVector()>;
  using V3 = G4ThreeVector;
  struct minmaxsum { V3 min; V3 max; V3 sum; V3 min_abs; double min_rho; double max_rho; };
  friend std::ostream& operator<<(std::ostream&, const threevec_stats&);

  // Generate data by providing the number of desired samples and a zero-arg function that gets one sample
  threevec_stats(size_t n, sampler sampler) : threevec_stats{take_samples(n, sampler)} { count = n; }

  G4ThreeVector mean()   const { return { x_sum / count, y_sum / count, z_sum / count}; }
  G4ThreeVector spread() const { return { x_max - x_min, y_max - y_min, z_max - z_min}; }

  const double x_min    , y_min    , z_min    ;
  const double x_max    , y_max    , z_max    ;
  const double x_sum    , y_sum    , z_sum    ;
  const double x_min_abs, y_min_abs, z_min_abs;
  const double rho_min, rho_max;

  private:
  size_t count;

  // Delegatee constructor, allows exposing public CONST min/max/sum
  threevec_stats(minmaxsum s)
    : x_min    {s.min    .x()}, y_min    {s.min    .y()}, z_min    {s.min    .z()}
    , x_max    {s.max    .x()}, y_max    {s.max    .y()}, z_max    {s.max    .z()}
    , x_sum    {s.sum    .x()}, y_sum    {s.sum    .y()}, z_sum    {s.sum    .z()}
    , x_min_abs{s.min_abs.x()}, y_min_abs{s.min_abs.y()}, z_min_abs{s.min_abs.z()}
    , rho_min{s.min_rho}, rho_max{s.max_rho}
    {}

  minmaxsum take_samples(size_t n, sampler get_one_sample) {
    double min_x    , min_y    , min_z    ;
    double max_x    , max_y    , max_z    ;
    double sum_x    , sum_y    , sum_z    ;
    double min_x_abs, min_y_abs, min_z_abs;
    double max_rho, min_rho;
    min_x     = min_y     = min_z = min_rho = +std::numeric_limits<double>::infinity();
    max_x     = max_y     = max_z = max_rho = -std::numeric_limits<double>::infinity();
    sum_x     = sum_y     = sum_z           = 0;
    min_x_abs = min_y_abs = min_z_abs       = +std::numeric_limits<double>::infinity();
    for (size_t i=0; i<n; i++) {
      auto v = get_one_sample();
      auto rho = v.rho();
      min_rho   = std::min(min_rho, rho);               max_rho   = std::max(max_rho, rho);
      min_x     = std::min(min_x, v.x());               max_x     = std::max(max_x, v.x());         sum_x += v.x();
      min_y     = std::min(min_y, v.y());               max_y     = std::max(max_y, v.y());         sum_y += v.y();
      min_z     = std::min(min_z, v.z());               max_z     = std::max(max_z, v.z());         sum_z += v.z();
      min_x_abs = std::min(min_x_abs, std::abs(v.x()));
      min_y_abs = std::min(min_y_abs, std::abs(v.y()));
      min_z_abs = std::min(min_z_abs, std::abs(v.z()));
    }
    return minmaxsum{{min_x    , min_y    , min_z     },
                     {max_x    , max_y    , max_z     },
                     {sum_x    , sum_y    , sum_z     },
                     {min_x_abs, min_y_abs, min_z_abs },
                     min_rho, max_rho};
  }
};

inline std::ostream& operator<<(std::ostream& out, const threevec_stats& s) {
  auto w = std::setw(15);
  return out
    << "samples: " << s.count << std::endl
    << "    " << w <<      "min"<< w <<      "max"<< w <<  "spread"      << w <<   "mean"     << w << "min abs"   << std::endl
    << "x  :" << w << s.  x_min << w << s.  x_max << w << s.spread().x() << w << s.mean().x() << w << s.x_min_abs << std::endl
    << "y  :" << w << s.  y_min << w << s.  y_max << w << s.spread().y() << w << s.mean().y() << w << s.y_min_abs << std::endl
    << "z  :" << w << s.  z_min << w << s.  z_max << w << s.spread().z() << w << s.mean().z() << w << s.z_min_abs << std::endl
    << "rho:" << w << s.rho_min << w << s.rho_max
    << std::endl;
}
