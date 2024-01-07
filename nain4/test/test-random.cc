#include "testing.hh"

#include <n4-random.hh>
#include <G4Types.hh>

#include <tuple>

TEST_CASE("biased choice", "[random][biased][choice]") {
  // TODO Ideally this should be done with a proptesting generator/shrinker
  std::vector<G4double> weights{9.1, 1.2, 3.4, 100, 0.3, 12.4, 6.7};
  auto pick = n4::random::biased_choice{weights};
  std::vector<size_t> hits(weights.size(), 0);
  for (size_t i=0; i<1000000; ++i) {
    hits[pick()] += 1;
  }

  // Verify that ratio of weights matches ratio of generated choices
  auto check = [&](auto l, auto r) {
    CHECK_THAT( static_cast<G4double>(   hits[l]) /    hits[r]
              ,             WithinRel(weights[l]  / weights[r], 0.01));
  };

  for (size_t i=1; i<weights.size(); ++i) { check(0, i); }
}



// Ratio of volumes of two spherical shells of equal thickness, the inner-radius
// of the larger being equal to the outer radius of the smaller
//
// Shpere volume         proportional to  n^3           in units of shell thickness
// Smaller sphere volume proportional to        (n-1)^3
// Shell volume          proportional to  n^3 - (n-1)^3
// Ratio of touching shells = (n^3 - (n-1)^3) / ((n-1)^3 - (n-2)^3)
// A bit of algebra simplifies this to the following
double shell_ratio(double n) {
  return
    (3 * n * (n-1) + 1) /
    (3 * n * (n-3) + 7);
}


TEST_CASE("random point in sphere", "[random][sphere]") {
  G4double const r_max = 3.456;
  size_t   const N_bins = 10;
  size_t   const N_per_bin = 1e5;
  std::vector<double> r_hits(N_bins, 0); // Concentric shells
  std::vector<size_t> x_hits(N_bins, 0); // Equal-angle wedge-bins around x-axis
  std::vector<size_t> y_hits(N_bins, 0); //                               y
  std::vector<size_t> z_hits(N_bins, 0); //                               z

  // ----- Helpers to identify in which bin to place the points ---------
  auto radius_bin = [=](auto r)         { return floor(N_bins *         r   / r_max       ); };
  auto  angle_bin = [=](auto x, auto y) { return floor(N_bins * (atan2(x,y) / twopi + 0.5)); };

  // ----- Collect samples ----------------------------------------------
  size_t const N_SAMPLES = N_per_bin * N_bins;
  for (size_t i=0; i<N_SAMPLES; ++i) {
    auto pt = n4::random::random_in_sphere(r_max);
    auto [x, y, z] = std::make_tuple(pt.x(), pt.y(), pt.z());
    auto r = pt.mag();
    r_hits[radius_bin( r  )]++;
    x_hits[ angle_bin(y, z)]++;
    y_hits[ angle_bin(z, x)]++;
    z_hits[ angle_bin(x, y)]++;
  }

  // ----- Check distribution in concentric shells ----------------------
  for (size_t n=1; n<N_bins; ++n) {
    CHECK_THAT(r_hits[n] / r_hits[n-1], WithinRel(shell_ratio(n+1), 0.02));
  }

  // ----- Check angular distribution around each axis ------------------
  auto check_around_axis = [=](auto const& bin) {
    for (size_t n = 0; n < N_bins; ++n) {
      CHECK_THAT(bin[n], WithinRel(N_per_bin, 0.01));
    }
  };

  check_around_axis(x_hits);
  check_around_axis(y_hits);
  check_around_axis(z_hits);
}



TEST_CASE("random direction octants", "[random][direction]") {
  // costheta < 0 -> z < 0
  // costheta > 0 -> z > 0
  // 0pi/2 < phi < 1pi/2 -> x > 0, y > 0
  // 1pi/2 < phi < 2pi/2 -> x < 0, y > 0
  // 2pi/2 < phi < 3pi/2 -> x < 0, y < 0
  // 3pi/2 < phi < 4pi/2 -> x > 0, y < 0

  // Define generators for each octant
  auto ppp = n4::random::direction{}.min_cos_theta(0)                  .max_phi(  halfpi);
  auto ppn = n4::random::direction{}.max_cos_theta(0)                  .max_phi(  halfpi);
  auto pnp = n4::random::direction{}.min_cos_theta(0).min_phi(3*halfpi);
  auto pnn = n4::random::direction{}.max_cos_theta(0).min_phi(3*halfpi);
  auto npp = n4::random::direction{}.min_cos_theta(0).min_phi(  halfpi).max_phi(2*halfpi);
  auto npn = n4::random::direction{}.max_cos_theta(0).min_phi(  halfpi).max_phi(2*halfpi);
  auto nnp = n4::random::direction{}.min_cos_theta(0).min_phi(2*halfpi).max_phi(3*halfpi);
  auto nnn = n4::random::direction{}.max_cos_theta(0).min_phi(2*halfpi).max_phi(3*halfpi);

  size_t N = 1000;

  // Check that generated values are in the correct octant
#define OCTANT(CMP_X,CMP_Y,CMP_Z) { \
  CHECK(it.x_min CMP_X 0);          \
  CHECK(it.y_min CMP_Y 0);          \
  CHECK(it.z_min CMP_Z 0);          \
}

  // Check that generated values cover the whole octant
#define SPREAD {        \
  auto s = it.spread(); \
  CHECK(s.x() > 0.99);  \
  CHECK(s.y() > 0.99);  \
  CHECK(s.z() > 0.99);  \
}

#define VERIFY(NAME,CMP_X,CMP_Y,CMP_Z) {            \
  threevec_stats it{N, [&] { return NAME.get(); }}; \
  std::cerr << #NAME << " " << it;                  \
  OCTANT(CMP_X,CMP_Y,CMP_Z);                        \
  SPREAD                                            \
}

  VERIFY(ppp, >, >, >)
  VERIFY(ppn, >, >, <)
  VERIFY(pnp, >, <, >)
  VERIFY(pnn, >, <, <)
  VERIFY(npp, <, >, >)
  VERIFY(npn, <, >, <)
  VERIFY(nnp, <, <, >)
  VERIFY(nnn, <, <, <)

#undef VERIFY
#undef SPREAD
#undef OCTANT
}

TEST_CASE("random direction theta", "[random][direction]") {
  auto a = 0.3*pi, b = 0.4*pi, c = 0.7*pi, d = 0.8*pi;
  auto mintheta    = n4::random::direction{}.min_theta(a); auto implicit_theta_max = pi;
  auto maxtheta    = n4::random::direction{}.max_theta(b); auto implicit_theta_min = 0;
  auto minmaxtheta = n4::random::direction{}.min_theta(c).max_theta(d);

  size_t N = 1000;
  threevec_stats min   {N, [&] { return mintheta   .get(); }};
  threevec_stats max   {N, [&] { return maxtheta   .get(); }};
  threevec_stats minmax{N, [&] { return minmaxtheta.get(); }};

  auto check = [&] (auto label, const threevec_stats& stats, auto theta_min, auto theta_max) {
    std::cerr << label << std::endl << stats;
    CHECK_THAT(stats.z_min, WithinRel(std::cos(theta_max), 0.02));
    CHECK_THAT(stats.z_max, WithinRel(std::cos(theta_min), 0.02));
  };

  check("min"   , min   , a                 , implicit_theta_max);
  check("max"   , max   , implicit_theta_min, b                 );
  check("minmax", minmax, c                 , d                 );
}

TEST_CASE("random direction bidirectional", "[random][direction]") {
  auto theta = pi/6;
  auto sin_th = std::sin(theta);
  auto cos_th = std::cos(theta);
  auto gen = n4::random::direction{}.max_theta(theta).bidirectional();

  threevec_stats stats{1000, [&] { return gen.get(); }};
  std::cerr << stats;

  // Check that there is no directional bias
  CHECK_THAT(stats.mean().x(), WithinAbs(0, 0.02));
  CHECK_THAT(stats.mean().y(), WithinAbs(0, 0.02));
  CHECK_THAT(stats.mean().z(), WithinAbs(0, 0.02));
  // Check that user-imposed limits are respected
  CHECK_THAT(stats.rho_max,     WithinRel( sin_th, 0.03));
  CHECK_THAT(stats.  x_min,     WithinRel(-sin_th, 0.03));
  CHECK_THAT(stats.  x_max,     WithinRel( sin_th, 0.03));
  CHECK_THAT(stats.  y_min,     WithinRel(-sin_th, 0.03));
  CHECK_THAT(stats.  y_max,     WithinRel( sin_th, 0.03));
  CHECK_THAT(stats.  z_min_abs, WithinRel( cos_th, 0.03));

}

TEST_CASE("random direction rotate", "[random][direction]") {
  const size_t N = 1000;
  auto theta = pi/6;
  auto sin_th = std::sin(theta);

  auto xpos = n4::random::direction{}.max_theta(theta).rotate_y( halfpi);
  auto ypos = n4::random::direction{}.max_theta(theta).rotate_x(-halfpi);
  auto xneg = n4::random::direction{}.max_theta(theta).rotate_y(-halfpi);
  auto yneg = n4::random::direction{}.max_theta(theta).rotate_x( halfpi);

  threevec_stats xp{N, [&] { return xpos.get(); }};
  threevec_stats yp{N, [&] { return ypos.get(); }};
  threevec_stats xn{N, [&] { return xneg.get(); }};
  threevec_stats yn{N, [&] { return yneg.get(); }};

  CHECK(xp.x_min >= 0); CHECK(xp.z_max < sin_th); CHECK(xp.z_min > -sin_th);
  CHECK(yp.y_min >= 0); CHECK(yp.z_max < sin_th); CHECK(xp.z_min > -sin_th);
  CHECK(xn.x_min <= 0); CHECK(xp.z_max < sin_th); CHECK(xp.z_min > -sin_th);
  CHECK(yn.y_min <= 0); CHECK(yp.z_max < sin_th); CHECK(xp.z_min > -sin_th);
}

TEST_CASE("random direction rotate twice", "[random][direction]") {
  // z pointing to (1, 1, 0)
  const size_t N  = 100000;
  auto theta_open = pi/8;
  auto theta_rot  = pi/4;
  auto x_min =  std::sin(theta_rot) - std::cos(theta_open);
  auto x_max =  std::sin(theta_rot) + std::cos(theta_open);
  auto y_min =  std::sin(theta_rot) - std::sin(theta_open);
  auto y_max =  std::sin(theta_rot) + std::sin(theta_open);
  auto z_min =                      - std::sin(theta_open);
  auto z_max =                        std::sin(theta_open);

  auto cap = n4::random::direction{}.max_theta(theta_open).rotate_y(pi/2).rotate_z(pi/4);
  threevec_stats st{N, [&] { return cap.get(); }};

  std::cerr << "cap stats\n" << st << std::endl;
  CHECK(st.x_min >= x_min);  CHECK(st.x_max <= x_max);
  CHECK(st.y_min >= y_min);  CHECK(st.y_max <= y_max);
  CHECK(st.z_min >= z_min);  CHECK(st.z_max <= z_max);
}

TEST_CASE("random direction exclude", "[random][direction]") {
  auto theta = pi/6;
  auto sin_th = std::sin(theta);
  auto cos_th = std::cos(theta);
  size_t N = 10000;

  auto cup = n4::random::direction{}.min_theta(theta);
  auto cap = n4::random::direction{}.min_theta(theta).exclude();

  threevec_stats kup{N, [&] { return cup.get(); }};
  threevec_stats kap{N, [&] { return cap.get(); }};

  std::cerr << "cup\n" << kup;
  std::cerr << "cap\n" << kap;

  CHECK_THAT(kup.x_min, WithinRel(-1.0   , 0.05)); CHECK_THAT(kup.x_max, WithinRel(1.0         , 0.05));
  CHECK_THAT(kup.y_min, WithinRel(-1.0   , 0.05)); CHECK_THAT(kup.y_max, WithinRel(1.0         , 0.05));
  CHECK_THAT(kup.z_min, WithinRel(-1.0   , 0.05)); CHECK_THAT(kup.z_max, WithinRel(      cos_th, 0.05));

  CHECK_THAT(kap.x_min, WithinRel(-sin_th, 0.05)); CHECK_THAT(kap.x_max, WithinRel(sin_th      , 0.05));
  CHECK_THAT(kap.y_min, WithinRel(-sin_th, 0.05)); CHECK_THAT(kap.y_max, WithinRel(sin_th      , 0.05));
  CHECK_THAT(kap.z_min, WithinRel( cos_th, 0.05)); CHECK_THAT(kap.z_max, WithinRel(1.0         , 0.05));
}

TEST_CASE("random direction exclude bidirectional", "[random][direction]") {
  auto theta = pi/6;
  auto sin_th = std::sin(theta);
  auto cos_th = std::cos(theta);

  auto no_caps = n4::random::direction{}.max_theta(pi/6).bidirectional().exclude();
  threevec_stats s{1000, [&] { return no_caps.get(); }};
  std::cerr << "no_caps\n" << s;

  // Check that user-imposed limits are respected
  CHECK_THAT(s.  z_max, WithinRel(cos_th, 0.05)); CHECK_THAT(s.z_min, WithinRel(-cos_th, 0.05));
  CHECK_THAT(s.rho_min, WithinRel(sin_th, 0.03));
  // Check that there is no directional bias
  CHECK_THAT(s.mean().x(), WithinAbs(0, 0.05));
  CHECK_THAT(s.mean().y(), WithinAbs(0, 0.05));
  CHECK_THAT(s.mean().z(), WithinAbs(0, 0.05));
}

TEST_CASE("random direction exclude rotate", "[random][direction]") {
  auto theta = pi/6;
  auto cos_th = std::cos(theta);

  auto xbeam = n4::random::direction{}.min_theta(theta).rotate_y(halfpi).exclude();
  threevec_stats s{1000, [&] { return xbeam.get(); }};
  std::cerr << "xbeam\n" << s;

  // Check that user-imposed limits are respected
  CHECK_THAT(s.x_min, WithinRel(cos_th, 0.05));
  // Check that there is no directional bias (except in x, where it is user-imposed)
  CHECK_THAT(s.mean().y(), WithinAbs(0, 0.05));
  CHECK_THAT(s.mean().z(), WithinAbs(0, 0.05));
}

TEST_CASE("random direction ranges", "[random][direction]") {
  auto costh_min = -1;
  auto costh_max =  1;
  auto th_min = 0;
  auto th_max = pi;
  auto phi_min = 0;
  auto phi_max = twopi;
  auto a = 0.25;
  auto b = 0.50;
  auto eps = 1e-5;

#define R n4::random::direction{}
  REQUIRE_THROWS_AS(R.min_cos_theta(costh_min - eps)   , std::runtime_error);
  REQUIRE_THROWS_AS(R.max_cos_theta(costh_max + eps)   , std::runtime_error);
  REQUIRE_THROWS_AS(R.min_cos_theta(b).max_cos_theta(a), std::runtime_error);
  REQUIRE_THROWS_AS(R.max_cos_theta(a).min_cos_theta(b), std::runtime_error);

  REQUIRE_THROWS_AS(R.min_theta(th_min - eps)  , std::runtime_error);
  REQUIRE_THROWS_AS(R.max_theta(th_max + eps)  , std::runtime_error);
  REQUIRE_THROWS_AS(R.min_theta(b).max_theta(a), std::runtime_error);
  REQUIRE_THROWS_AS(R.max_theta(a).min_theta(b), std::runtime_error);

  REQUIRE_THROWS_AS(R.min_phi(phi_min - eps), std::runtime_error);
  REQUIRE_THROWS_AS(R.max_phi(phi_max + eps), std::runtime_error);
  REQUIRE_THROWS_AS(R.min_phi(b).max_phi(a) , std::runtime_error);
  REQUIRE_THROWS_AS(R.max_phi(a).min_phi(b) , std::runtime_error);

  // equal upper and lower limit should work
  REQUIRE_NOTHROW(R.min_cos_theta(a).max_cos_theta(a));
  //REQUIRE_NOTHROW(R.min_theta    (a).max_theta    (a)); // floating point mess
  REQUIRE_NOTHROW(R.min_phi      (a).max_phi      (a));
#undef R
}

TEST_CASE("random piecewise_linear_distribution", "[nain][random][piecewise_linear_distribution]") {
  // simple test of a triangular distribution.
  std::vector<double> x{-10, 3, 10};
  std::vector<double> y{  0, 1,  0};

  auto N = 1'000'000;
  n4::random::piecewise_linear_distribution sampler{x, y};

  // threevec_stats would help here, but it would be a bit silly to
  // store 3 numbers to test just 1
  std::vector<double> data; data.reserve(N);
  for (auto i=0; i<N; i++) { data.push_back(sampler.sample()); }
  auto data_min  = *std::min_element(cbegin(data), cend(data));
  auto data_max  = *std::max_element(cbegin(data), cend(data));
  auto data_mean =  std::accumulate (cbegin(data), cend(data), 0.) / N;

  auto expected_mean = (x[0] + x[1] + x[2]) / 3;
  CHECK_THAT(data_min , WithinRel(     x.front(), 1e-1));
  CHECK_THAT(data_max , WithinRel(     x.back (), 1e-1));
  CHECK_THAT(data_mean, WithinRel( expected_mean, 1e-2));
}
