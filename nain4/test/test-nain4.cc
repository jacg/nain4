#include "testing.hh"

#include <n4-all.hh>

// Solids
#include <CLHEP/Units/SystemOfUnits.h>
#include <G4Box.hh>
#include <G4Cons.hh>
#include <G4GeometryTolerance.hh>
#include <G4LogicalVolume.hh>
#include <G4Orb.hh>
#include <G4PVPlacement.hh>
#include <G4RotationMatrix.hh>
#include <G4Sphere.hh>
#include <G4ThreeVector.hh>
#include <G4Transform3D.hh>
#include <G4Trd.hh>

// Managers
#include <G4NistManager.hh>

// Units
#include <G4SystemOfUnits.hh>
#include <G4UnitsTable.hh>

// Other G4
#include <G4Material.hh>
#include <G4Gamma.hh>

#include <catch2/generators/catch_generators.hpp>

#include <cmath>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <numeric>
#include <stdexcept>
#include <type_traits>
#include <unordered_set>


// Many of the tests below check physical quantities. Dividing physical
// quantities by their units gives raw numbers which are easily understandable
// by a human reader, which is important test failures are reported. Sometimes
// this gives rise to the apparently superfluous division by the same unit on
// both sides of an equation, in the source code.

using namespace n4::test;

template<class T>
void error_if_do_not_like_type(T) {
  static_assert(
    std::negation_v<std::is_same<T, int>>,
    "\n\n\n\nWe do not like `int`s\n\n\n\n\n"
  );
  static_assert(
    std::negation_v<std::is_same<T, std::string>>,
    "\n\n\n\n\n`std::string`s NOT welcome\n\n\n\n"
  );
}


//TEST_CASE("static assert int", "[static][int]") {  error_if_do_not_like_type(2); }
//TEST_CASE("static assert string", "[static][string]") {  error_if_do_not_like_type(std::string{"bla"}); }
//TEST_CASE("static assert double", "[static][double]") {  error_if_do_not_like_type(3.2); }

void check_solid_volume_placed_equivalence(G4VSolid* solid, G4LogicalVolume* volume, G4PVPlacement* placed, double tol) {
  CHECK_THAT(volume -> GetMass()        / kg, WithinRel(placed -> GetLogicalVolume() -> GetMass()        / kg, tol));
  CHECK_THAT(solid  -> GetCubicVolume() / m3, WithinRel(volume -> GetSolid        () -> GetCubicVolume() / m3, tol));
}

auto check_properties (n4::boolean_shape& shape, G4Material* mat, G4String name, double vol, double density) {
  auto solid  = shape.solid();
  auto volume = shape.volume(mat);
  auto placed = shape.place (mat).now();

  check_solid_volume_placed_equivalence(solid, volume, placed, 1e-4);

  CHECK     (volume -> TotalVolumeEntities() == 1);
  CHECK     (volume -> GetMaterial()         == mat);
  CHECK_THAT(solid  -> GetCubicVolume() / m3, WithinRel(vol           / m3, 1e-3));
  CHECK_THAT(volume -> GetMass() / kg       , WithinRel(vol * density / kg, 1e-3));

  CHECK(solid  -> GetName() == name);
  CHECK(volume -> GetName() == name);
  CHECK(placed -> GetName() == name);
};


TEST_CASE("nain boolean single add", "[nain][geometry][boolean][add]") {
  auto l   = 1*m;
  auto sep = 3*l;
  auto air = n4::material("G4_AIR");
  auto density = air -> GetDensity();
  auto given_name = "double-cube";
  auto vol = l * l * l;

#define VARIANT(NAME, METHOD, EXTRA)                     \
  auto NAME = n4::box("cube-L").cube(l)                  \
                 .METHOD(n4::box("cube-R").cube(l)EXTRA) \
                 .at(sep, 0, 0)                          \
                 .name(given_name);

  VARIANT(shape_long_val , join,         )
  VARIANT(shape_long_ptr , join, .solid())
  VARIANT(shape_short_val, add ,         )
  VARIANT(shape_short_ptr, add , .solid())
#undef VARIANT

#define CHECKP(shape) check_properties(shape, air, given_name, 2 * vol, density);
  CHECKP(shape_long_val)
  CHECKP(shape_long_ptr)
  CHECKP(shape_short_val)
  CHECKP(shape_short_ptr)
#undef CHECKP
}

TEST_CASE("nain boolean double add", "[nain][geometry][boolean][add]") {
  auto l   = 1*m;
  auto sep = 3*l;
  auto air = n4::material("G4_AIR");
  auto density = air -> GetDensity();
  auto given_name = "triple-cube";
  auto vol = l * l * l;

#define VARIANT(NAME, METHOD, EXTRA)                     \
  auto NAME = n4::box("cube").cube(l)                    \
                 .METHOD(n4::box("cube-R").cube(l)EXTRA) \
                 .at( sep,   0, 0)                       \
                 .METHOD(n4::box("cube-L").cube(l)EXTRA) \
                 .at(-sep,   0, 0)                       \
                 .name(given_name);

  VARIANT(shape_long_val , join,         )
  VARIANT(shape_long_ptr , join, .solid())
  VARIANT(shape_short_val, add ,         )
  VARIANT(shape_short_ptr, add , .solid())
#undef VARIANT

#define CHECKP(shape) check_properties(shape, air, given_name, 3 * vol, density);
  CHECKP(shape_long_val)
  CHECKP(shape_long_ptr)
  CHECKP(shape_short_val)
  CHECKP(shape_short_ptr)
#undef CHECKP
}


TEST_CASE("nain boolean add overlap", "[nain][geometry][boolean][add]") {
  auto l   = 1*m;
  auto sep = l/2;
  auto air = n4::material("G4_AIR");
  auto density = air -> GetDensity();
  auto given_name = "two-boxes-ovelap";
  auto vol = l * l * l;

#define VARIANT(NAME, METHOD, EXTRA)                        \
  auto NAME = n4::box("cube-L").cube(l)                     \
                 .METHOD(n4::box("cube-R").cube(l)EXTRA)    \
                 .at(sep, sep, sep) /* Overlapping 1/8th */ \
                 .name(given_name);

  VARIANT(shape_long_val , join,         )
  VARIANT(shape_long_ptr , join, .solid())
  VARIANT(shape_short_val, add ,         )
  VARIANT(shape_short_ptr, add , .solid())
#undef VARIANT

#define CHECKP(shape) check_properties(shape, air, given_name, vol * 15 / 8, density);
  CHECKP(shape_long_val)
  CHECKP(shape_long_ptr)
  CHECKP(shape_short_val)
  CHECKP(shape_short_ptr)
#undef CHECKP
}


TEST_CASE("nain boolean single subtract", "[nain][geometry][boolean][subtract]") {
  auto l   = 1*m;
  auto sep = 0.5*l;
  auto air = n4::material("G4_AIR");
  auto density = air -> GetDensity();
  auto given_name = "double-cube";
  auto vol = l * l * l;

#define VARIANT(NAME, METHOD, EXTRA)                        \
  auto NAME = n4::box("cube-L").cube(l)                     \
                 .METHOD(n4::box("cube-R").cube(l)EXTRA)    \
                 .at(sep, 0, 0) /* Overlapping 1/2 vol */   \
                 .name(given_name);

  VARIANT(shape_long_val , subtract,         )
  VARIANT(shape_long_ptr , subtract, .solid())
  VARIANT(shape_short_val, sub     ,         )
  VARIANT(shape_short_ptr, sub     , .solid())
#undef VARIANT

#define CHECKP(shape) check_properties(shape, air, given_name, vol / 2, density);
  CHECKP(shape_long_val)
  CHECKP(shape_long_ptr)
  CHECKP(shape_short_val)
  CHECKP(shape_short_ptr)
#undef CHECKP
}


TEST_CASE("nain boolean double sub", "[nain][geometry][boolean][sub]") {
  auto l   = 1*m;
  auto sep = 0.75*l;
  auto air = n4::material("G4_AIR");
  auto density = air -> GetDensity();
  auto given_name = "double-cube";
  auto vol = l * l * l;

#define VARIANT(NAME, METHOD, EXTRA)                     \
  auto NAME = n4::box("cube").cube(l)                    \
                 .METHOD(n4::box("cube-R").cube(l)EXTRA) \
                 .at( sep, 0, 0) /* Overlapping 1/4th */ \
                 .METHOD(n4::box("cube-L").cube(l)EXTRA) \
                 .at(-sep, 0, 0) /* Overlapping 1/4th */ \
                 .name(given_name);

  VARIANT(shape_long_val , subtract,         )
  VARIANT(shape_long_ptr , subtract, .solid())
  VARIANT(shape_short_val, sub     ,         )
  VARIANT(shape_short_ptr, sub     , .solid())
#undef VARIANT

#define CHECKP(shape) check_properties(shape, air, given_name, vol / 2, density);
  CHECKP(shape_long_val)
  CHECKP(shape_long_ptr)
  CHECKP(shape_short_val)
  CHECKP(shape_short_ptr)
#undef CHECKP
}


TEST_CASE("nain boolean single intersect", "[nain][geometry][boolean][intersect]") {
  auto l   = 1*m;
  auto sep = 0.75*l;
  auto air = n4::material("G4_AIR");
  auto density = air -> GetDensity();
  auto given_name = "double-cube";
  auto vol = l * l * l;

#define VARIANT(NAME, METHOD, EXTRA)                     \
  auto NAME = n4::box("cube-L").cube(l)                  \
                 .METHOD(n4::box("cube-R").cube(l)EXTRA) \
                 .at(sep, 0, 0) /* Overlapping 1/4th */  \
                 .name(given_name);

  VARIANT(shape_long_val , intersect,         )
  VARIANT(shape_long_ptr , intersect, .solid())
  VARIANT(shape_short_val, inter    ,         )
  VARIANT(shape_short_ptr, inter    , .solid())
#undef VARIANT

#define CHECKP(shape) check_properties(shape, air, given_name, vol / 4, density);
  CHECKP(shape_long_val)
  CHECKP(shape_long_ptr)
  CHECKP(shape_short_val)
  CHECKP(shape_short_ptr)
#undef CHECKP
}


TEST_CASE("nain boolean double intersect", "[nain][geometry][boolean][intersect]") {
  auto l   = 1*m;
  auto sep = 0.75*l;
  auto air = n4::material("G4_AIR");
  auto density = air -> GetDensity();
  auto given_name = "triple-cube";
  auto vol = l * l * l;

#define VARIANT(NAME, METHOD, EXTRA)                      \
  auto NAME = n4::box("cube-L").cube(l)                   \
                 .METHOD(n4::box("cube-R").cube(l)EXTRA)  \
                 .at(sep,   0, 0) /* Overlapping 1/4th */ \
                 .METHOD(n4::box("cube-T").cube(l)EXTRA)  \
                 .at(sep, sep, 0) /* Overlapping 1/4th */ \
                 .name(given_name);

  VARIANT(shape_long_val , intersect,         )
  VARIANT(shape_long_ptr , intersect, .solid())
  VARIANT(shape_short_val, inter    ,         )
  VARIANT(shape_short_ptr, inter    , .solid())
#undef VARIANT

#define CHECKP(shape) check_properties(shape, air, given_name, vol / 16, density);
  CHECKP(shape_long_val)
  CHECKP(shape_long_ptr)
  CHECKP(shape_short_val)
  CHECKP(shape_short_ptr)
#undef CHECKP
}


TEST_CASE("nain boolean at", "[nain][geometry][boolean][at]") {
  auto lx = 1*m;
  auto ly = 2*m;
  auto lz = 3*m;

  auto box = n4::box("box").xyz(lx, ly, lz);

  auto at_full = box.intersect(box).at  (lx,      ly,      lz).solid();
  auto at_x    = box.intersect(box).at_x(lx)                  .solid();
  auto at_y    = box.intersect(box)         .at_y(ly)         .solid();
  auto at_z    = box.intersect(box)                  .at_z(lz).solid();
  auto at_xy   = box.intersect(box).at_x(lx).at_y(ly)         .solid();
  auto at_xz   = box.intersect(box).at_x(lx)         .at_z(lz).solid();
  auto at_yz   = box.intersect(box)         .at_y(ly).at_z(lz).solid();
  auto at_xyz  = box.intersect(box).at_x(lx).at_y(ly).at_z(lz).solid();

  // When displaced, the volumes do not overlap at all, resulting in a null volume
  // Cannot use GetCubicVolume because gives nonsense
  auto n   = 10000;
  auto eps = 1e-3;
  auto check_zero_volume = [=] (auto solid) { CHECK_THAT(solid -> EstimateCubicVolume(n, eps) / m3, WithinAbs(0, eps)); };

  check_zero_volume(at_full);
  check_zero_volume(at_x   );
  check_zero_volume(at_y   );
  check_zero_volume(at_z   );
  check_zero_volume(at_xy  );
  check_zero_volume(at_xz  );
  check_zero_volume(at_yz  );
  check_zero_volume(at_xyz );
}

TEST_CASE("nain boolean rotation", "[nain][geometry][boolean][rotation]") {
  auto l1  = 3*m;
  auto l2  = 1*m;

  auto box_along_x = n4::box("along-x").xyz(l2, l1, l1);
  auto box_along_y = n4::box("along-y").xyz(l1, l2, l1);
  auto box_along_z = n4::box("along-z").xyz(l1, l1, l2);

  auto without_rot_xy   = box_along_x.subtract(box_along_y).                 name("without_rot_xy").solid();
  auto without_rot_zx   = box_along_z.subtract(box_along_x).                 name("without_rot_zx").solid();
  auto without_rot_yz   = box_along_y.subtract(box_along_z).                 name("without_rot_yz").solid();
  auto    with_rotate_z = box_along_x.subtract(box_along_y).rotate_z(90*deg).name("with_rotate_z" ).solid();
  auto    with_rotate_y = box_along_z.subtract(box_along_x).rotate_y(90*deg).name("with_rotate_y" ).solid();
  auto    with_rotate_x = box_along_y.subtract(box_along_z).rotate_x(90*deg).name("with_rotate_x" ).solid();
  auto    with_rot_z    = box_along_x.subtract(box_along_y).rot_z   (90*deg).name("with_rot_z"    ).solid();
  auto    with_rot_y    = box_along_z.subtract(box_along_x).rot_y   (90*deg).name("with_rot_y"    ).solid();
  auto    with_rot_x    = box_along_y.subtract(box_along_z).rot_x   (90*deg).name("with_rot_x"    ).solid();

  auto rotation         = G4RotationMatrix{}; rotation.rotateX(90*deg);
  auto with_rotate      = box_along_y.subtract(box_along_z).rotate(rotation).name("with_rotate"   ).solid();
  auto with_rot         = box_along_y.subtract(box_along_z).rot   (rotation).name("with_rot"      ).solid();

  // When rotated, the volumes overlap perfectly, resulting in a null volume
  // Cannot use GetCubicVolume because gives nonsense
  auto n   = 100000;
  auto eps = 1e-3;
  auto nonzero_volume = [=] (auto solid) { CHECK_THAT(solid -> EstimateCubicVolume(n, eps), ! WithinAbs(0, eps)); };
  auto    zero_volume = [=] (auto solid) { CHECK_THAT(solid -> EstimateCubicVolume(n, eps),   WithinAbs(0, eps)); };

  nonzero_volume(without_rot_xy);
  nonzero_volume(without_rot_zx);
  nonzero_volume(without_rot_yz);
     zero_volume(with_rotate_x );
     zero_volume(with_rotate_y );
     zero_volume(with_rotate_z );
     zero_volume(with_rotate   );
     zero_volume(with_rot_x    );
     zero_volume(with_rot_y    );
     zero_volume(with_rot_z    );
     zero_volume(with_rot      );
}

TEST_CASE("boolean transform", "[boolean][transform]") {
  auto l           = 3*m;
  auto rotation    = G4RotationMatrix{}; rotation.rotateY(90 * deg);
  auto translation = G4ThreeVector{0., 0., l/4};
  auto transform   = G4Transform3D{rotation, translation};

  auto usolid1 = n4::box("box11").xy(1*m).z(  l)
    .sub(        n4::box("box12").yz(1*m).x(l/4))
    .transform(transform)
    .solid();

  auto usolid2 = n4::box("box21").xy(1*m).z(  l)
    .sub(        n4::box("box22").yz(1*m).x(l/4))
    .trans(transform)
    .solid();

  auto n    = 100000;
  auto eps  = 1e-3;
  auto vbox = l * 1*m * 1*m;

  // The small box is 1/4 of the big box, so the result is two
  // disconnected boxes: one with 1/2 volume and another one with 1/4
  // volume
  CHECK_THAT( usolid1 -> EstimateCubicVolume(n, eps) / m3, WithinRel(3*vbox/4 / m3, 3e-3));
  CHECK_THAT( usolid1 -> EstimateCubicVolume(n, eps) / m3, WithinRel(3*vbox/4 / m3, 3e-3));
  CHECK_THAT( usolid2 -> EstimateCubicVolume(n, eps) / m3, WithinRel(3*vbox/4 / m3, 3e-3));
  CHECK_THAT( usolid2 -> EstimateCubicVolume(n, eps) / m3, WithinRel(3*vbox/4 / m3, 3e-3));
}

// TODO can the overlap check tests be automated? G4 raises an exception when an
// overlap is detected, and we do not know how to observe that in Catch2

// }

TEST_CASE("stats sum", "[stats][sum]") {
  using n4::stats::sum;

  // Sums of empty containers
  CHECK     (sum(std::vector       <int>   {}) == 0);
  CHECK     (sum(std::unordered_set<int>   {}) == 0);
  CHECK_THAT(sum(std::vector       <float> {}), Within1ULP(0.f));
  CHECK_THAT(sum(std::unordered_set<double>{}), Within1ULP(0. ));

  // Sums of single-element containers
  CHECK     (sum(std::vector       <long> {3}) == 3);
  CHECK_THAT(sum(std::unordered_set<float>{4}), Within1ULP(4.f));

  // Sums of multiple-element containers
  CHECK_THAT(sum(std::vector       <float> {3.1, 7.2}), WithinULP (10.3f, 1)); // Avoid casting float to double
  CHECK_THAT(sum(std::vector       <double>{3.1, 7.2}), Within1ULP(10.3 ));
  CHECK(     sum(std::unordered_set<long>  {7, 2, 9} )  ==         18    );
}

TEST_CASE("stats mean", "[stats][mean]") {
  using n4::stats::mean; using std::vector; using std::unordered_set;

  // Means of empty containers
  CHECK(! mean(vector    <double>{}).has_value());
  CHECK(! mean(unordered_set<int>{}).has_value());

  // Means of single-element containers
  CHECK_THAT(mean(vector      <double>{2.3 }).value(), Within1ULP( 2.3 ));
  CHECK_THAT(mean(unordered_set<float>{9.1f}).value(), Within1ULP( 9.1f));
  CHECK_THAT(mean(vector         <int>{42}  ).value(), Within1ULP(42.  ));

  // Means of multiple-value containers
  CHECK_THAT(mean(vector       <double>{1.0, 2.0}     ).value(), Within1ULP(1.5 ));
  CHECK_THAT(mean(vector       <float> {3.1, 3.6, 5.9}).value(), Within1ULP(4.2f));
  CHECK_THAT(mean(unordered_set<double>{9.0, 2.0}     ).value(), Within1ULP(5.5f));

  // Input integers give double results
  CHECK_THAT(mean(vector<int>{1,2}).value(), Within1ULP(1.5));
}

TEST_CASE("stats std_dev population", "[stats][std_dev][population]") {
  using n4::stats::std_dev_population; using n4::stats::variance_population;
  using std::vector; using std::unordered_set;

  // Standard deviations of empty containers
  CHECK(! std_dev_population(vector       <int>   {}).has_value());
  CHECK(! std_dev_population(unordered_set<int>   {}).has_value());
  CHECK(! std_dev_population(vector       <float> {}).has_value());
  CHECK(! std_dev_population(unordered_set<double>{}).has_value());

  // Standard deviations of single-element containers
  CHECK_THAT(std_dev_population(vector      <double>{3.6}).value(), Within1ULP(0. ));
  CHECK_THAT(std_dev_population(unordered_set<float>{6.3}).value(), Within1ULP(0.f));

  // Standard deviations of multi-element containers
  auto check_std_and_var = [] (vector<double> data, double expected) {
    CHECK_THAT( std_dev_population(data).value(), Within1ULP(std::sqrt(expected)));
    CHECK_THAT(variance_population(data).value(), Within1ULP(          expected ));
  };
  check_std_and_var({5, 7}                                                ,  1);
  check_std_and_var({1, 2, 3, 4, 5}                                       ,  2);
  check_std_and_var({2, 4, 4, 6, 6, 6, 8, 8, 8, 8, 10, 10, 10, 12, 12, 14}, 10);

  // Input integers give double results
  CHECK_THAT( std_dev_population(vector<int>{1,2,3}).value(), Within1ULP(std::sqrt(2.0/3)));
  CHECK_THAT(variance_population(vector<int>{1,2,3}).value(), Within1ULP(          2.0/3 ));
}

TEST_CASE("stats std_dev sample", "[stats][std_dev][sample]") {
  using n4::stats::std_dev_sample; using n4::stats::variance_sample;
  using std::vector; using std::unordered_set;

  // Standard deviations of empty containers
  CHECK(! std_dev_sample(vector       <int>   {}).has_value());
  CHECK(! std_dev_sample(unordered_set<int>   {}).has_value());
  CHECK(! std_dev_sample(vector       <float> {}).has_value());
  CHECK(! std_dev_sample(unordered_set<double>{}).has_value());

  // Standard deviations of single-element containers
  CHECK(! std_dev_sample(vector      <double>{4.2}).has_value());
  CHECK(! std_dev_sample(unordered_set<float>{7.9}).has_value());

  // Standard deviations of multi-element containers
  auto check_std_and_var = [] (vector<double> data, double expected) {
    CHECK_THAT( std_dev_sample(data).value(), Within1ULP(std::sqrt(expected)));
    CHECK_THAT(variance_sample(data).value(), Within1ULP(          expected ));
  };
  check_std_and_var({1, 5   },  8);
  check_std_and_var({2, 4   },  2);
  check_std_and_var({1, 2, 9}, 19);

  // Input integers give double results
  CHECK_THAT( std_dev_sample(vector<int>{1,2,3,4}).value(), Within1ULP(std::sqrt(5.0/3)));
  CHECK_THAT(variance_sample(vector<int>{1,2,3,4}).value(), Within1ULP(          5.0/3 ));
}

TEST_CASE("stats correlation", "[stats][correlation]") {

  auto corr = [] (const std::vector<double>& a, const std::vector<double>& b) {
    return n4::stats::correlation(a, b);
  };

  // Basic example of 100% correlation
  CHECK_THAT(corr({1,2},
                  {1,2}).value(), Within1ULP(1.0));

  // Basic example of 100% anti-correlation
  CHECK_THAT(corr({1,2},
                  {2,1}).value(), Within1ULP(-1.0));

  // Basic example of ZERO correlation
  CHECK_THAT(corr({-1,-1,+1,+1},
                  {-8,+8,-8,+8}).value(), Within1ULP(0.0));

  // A non-trivial example
  CHECK_THAT(corr({3,5,2,8,7},
                  {1,9,2,6,3}).value(), Within1ULP(0.4796356153459284));

  // Different lengths: no result
  CHECK(! corr({1,2,3},
               {1,2  }).has_value());

  // One sequence is constant: no result
  CHECK(! corr({1,2},
               {1,1}).has_value());

  // Too short: no result
  CHECK(! corr({1}, {1}).has_value());

  // Input integers give double results
  CHECK_THAT(n4::stats::correlation(
               std::vector<int>{1,2,3},
               std::vector<int>{1,2,2}).value(),
             Within1ULP(std::sqrt(3)/2));
}

TEST_CASE("stats min_max", "[stats][min_max]") {
  std::vector<double> empty{};
  CHECK(! n4::stats::min_max(empty).has_value());

  auto check_min_max = [] (const auto& data, const auto expected_min, const auto expected_max) {
    auto [min, max] = n4::stats::min_max(data).value();
    CHECK(min == expected_min);
    CHECK(max == expected_max);
  };

  std::vector<double> a {1.23, -9.62, 12.3, 4.56};
  check_min_max(a,             -9.62, 12.3);

  std::unordered_set<int> b {6,5,4,3};
  check_min_max(b, 3, 6);
}

#pragma GCC diagnostic pop
