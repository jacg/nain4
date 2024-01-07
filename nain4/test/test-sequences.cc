#include "testing.hh"

#include <n4-sequences.hh>

using Catch::Matchers::WithinULP;

TEST_CASE("nain scale_by", "[nain][scale_by]") {
  check_all_within1ULP(n4::scale_by(eV, {1, 2.3, 4.5}), {1*eV, 2.3*eV, 4.5*eV});
  check_all_within1ULP(n4::scale_by(cm, {6, 7       }), {6*cm, 7*cm          });
}

TEST_CASE("nain const_over", "[nain][const_over]") {
  check_all_within1ULP(n4::const_over(2*3*4, {2., 3., 4.}), {3*4, 2*4, 2*3});
  check_all_within1ULP(n4::const_over(   1., {10, 100   }), {0.1, 0.01    });
}

TEST_CASE("enumerate", "[utils][enumerate]") {
  // NB, const is ignored and by-reference is implicit!
  SECTION("primitives") {
    std::vector<unsigned> stuff {5, 4, 3, 2, 1};
    for (const auto [n, el] : n4::enumerate(stuff)) {
      CHECK(el == 5 - n);
      el++;
    }
    CHECK(stuff[0] == 6);
  }
  SECTION("objects") {
    struct Foo {
      Foo(unsigned n): n{n} {}
      unsigned n;
    };
    std::vector<Foo> stuff {2, 8};
    for (auto [n, el] : n4::enumerate(stuff)) {
      if (n == 0) { CHECK(el.n == 2); }
      else        { CHECK(el.n == 8); }
      el.n += 1000;
    }
    CHECK(stuff[0].n == 1002);
    CHECK(stuff[1].n == 1008);
  }
  SECTION("literal braces primitives") {
    for (const auto [n, el] : n4::enumerate({7,6,5,4})) {
      CHECK(static_cast<unsigned>(el) == 7 - n);
    }
  }
  SECTION("literal braces objects") {
    struct Foo {
      Foo(unsigned n): n{n} {}
      unsigned n;
    };
    for (auto [n, el] : n4::enumerate({Foo{4}, Foo{2}})) {
      if (n == 0) { CHECK(el.n == 4); }
      else        { CHECK(el.n == 2); }
    }
  }
}


TEST_CASE("nain linspace", "[nain][linspace]") {
  auto start     = 0.;
  auto stop      = 5.;
  auto n_entries = 11 ;

  auto values = n4::linspace(start, stop, n_entries);
  check_all_within1ULP(values, {0, 0.5, 1, 1.5, 2, 2.5, 3, 3.5, 4, 4.5, 5});

  auto one_item = n4::linspace(0.3, 0.4, 1);
  REQUIRE   (one_item.size() == 1);
  CHECK_THAT(one_item.front(), Within1ULP(0.3));

  auto zero_items = n4::linspace(0.3, 0.4, 0);
  CHECK(zero_items.size() == 0);
}

int ff(int x) {
  return x + 27;
}

TEST_CASE("nain map", "[nain][map]") {
  int a=9, b=5, c=1, d=7;
  std::vector<int> input_x = {a, b, c, d};
  auto f = [] (auto x) { return x + 27; };
  auto mapped_lambda    = n4::map<int>( f, input_x);
  auto mapped_fn        = n4::map<int>(ff, input_x);

  CHECK( mapped_lambda == std::vector<int>{ f(a),  f(b),  f(c),  f(d)} );
  CHECK( mapped_fn     == std::vector<int>{ff(a), ff(b), ff(c), ff(d)} );

}

TEST_CASE("nain interpolate", "[nain][interpolate]") {
  auto f = [] (auto x) { return -x; };
  const unsigned n_points    = 11;
  const unsigned n_intervals = n_points - 1;
  const double   lower       = -3.1;
  const double   upper       = 42.6;
  const double   delta       = (upper - lower) / n_intervals;

  auto [xs, ys] = n4::interpolate(f, n_points, lower, upper);

  SECTION("number of elements") {
    CHECK(xs.size() == n_points);
    CHECK(ys.size() == n_points);
  }

  SECTION("range") {
    auto [x_min, x_max] = std::minmax_element(begin(xs), end(xs));
    auto [y_min, y_max] = std::minmax_element(begin(ys), end(ys));

    CHECK_THAT(*x_min, Within1ULP(    lower ));
    CHECK_THAT(*x_max, Within1ULP(    upper ));
    CHECK_THAT(*x_min, Within1ULP(xs.front()));
    CHECK_THAT(*x_max, Within1ULP(xs. back()));

    CHECK_THAT(*y_min, Within1ULP(   -upper ));
    CHECK_THAT(*y_max, Within1ULP(   -lower ));
    CHECK_THAT(*y_min, Within1ULP(ys. back()));
    CHECK_THAT(*y_max, Within1ULP(ys.front()));
  }

  SECTION("element order") {
    double last;
    last = xs.front(); for (auto i=1; i<xs.size(); i++) { CHECK(last < xs[i]); last = xs[i]; }
    last = ys.front(); for (auto i=1; i<ys.size(); i++) { CHECK(last > ys[i]); last = ys[i]; }
  }

  SECTION("distance between elements") {
    for (size_t i=1; i<xs.size()-1; i++) { CHECK_THAT(xs[i+1] - xs[i], WithinULP( delta, 3)); }
    for (size_t i=1; i<ys.size()-1; i++) { CHECK_THAT(ys[i+1] - ys[i], WithinULP(-delta, 3)); }
  }

}

TEST_CASE("nain interpolate values", "[nain][interpolate]") {
  auto f = [] (auto x) { return -x*x*x + 1; };
  const unsigned n_points    =  7;
  const unsigned n_intervals = n_points - 1;
  const double   lower       = -2;
  const double   upper       =  3;
  const double   range       = upper - lower;
  const double   delta       = range / n_intervals;

  auto [xs, ys]   = n4::interpolate(f, n_points, lower, upper);

  for (size_t i=0; i<ys.size(); i++) {
    auto xi = lower + i * delta;
    auto yi = f(xi);

    CHECK_THAT(xi, WithinULP(xs[i], 4));
    CHECK_THAT(yi, WithinULP(ys[i], 1));
  }
}

TEST_CASE("nain interpolator", "[nain][interpolator]") {
  std::vector<double> x{-2, -0.3, 0, 1, 3.5, 6.2, 10};
  std::vector<double> y; y.reserve(x.size());
  for (const auto& xi:x) { y.push_back(3*xi*xi); }

  auto interpolator = n4::interpolator(x, y);

  auto check = [&interpolator] (double x, double expected_y, int ulp=1) {
    auto y = interpolator(x);
    REQUIRE(y.has_value());
    CHECK_THAT(y.value(), WithinULP(expected_y, ulp));
  };

  check(-1  ,   5.1 );
  check(-0.1,   0.09);
  check( 0  ,   0   );
  check( 0.5,   1.5 );
  check( 3.1,  31.35);
  check( 3.6,  39.66);
  check( 8.8, 241.68, 2);
}

TEST_CASE("nain interpolator out of range", "[nain][interpolator]") {
  std::vector<double> x{0, 10};
  std::vector<double> y{x};
  auto interpolator = n4::interpolator(x, y);

  CHECK(! interpolator(- 1).has_value());
  CHECK(! interpolator( 11).has_value());
}

TEST_CASE("nain unpack", "[nain4][unpack]") {
  auto xi = 666., yi = 3.14, zi = 42.;
  auto v = G4ThreeVector{xi, yi, zi};
  auto [xo, yo, zo] = n4::unpack(v);
  CHECK_THAT(xo, Within1ULP(xi));
  CHECK_THAT(yo, Within1ULP(yi));
  CHECK_THAT(zo, Within1ULP(zi));
}
