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

TEST_CASE("nain box", "[nain][box]") {
  // nain4::box is a more convenient interface for constructing G4VSolids and
  // G4LogicalVolumes based on G4Box
  auto water = nain4::material("G4_WATER"); auto density = water -> GetDensity();
  auto lx = 1 * m;
  auto ly = 2 * m;
  auto lz = 3 * m;
  auto xc = 4 * m;
  auto yc = 5 * m;
  auto zc = 6 * m;
  auto box_h = n4::box("box_h").half_x(lx/2).half_y(ly/2).half_z(lz/2).solid();
  auto box_s = n4::box("box_s")     .x(lx  )     .y(ly  )     .z(lz  ).solid();
  auto box_l = n4::box("box_l")     .x(lx  )     .y(ly  )     .z(lz  ).volume(water);
  auto box_p = n4::box("box_p")     .x(lx  )     .y(ly  )     .z(lz  ).place  (water).at(xc, yc, zc).now();

  auto lxy = 7 * m;
  auto lxz = 8 * m;
  auto lyz = 9 * m;
  auto box_xy = n4::box("box_xy").xy(lxy).z(lz).solid();
  auto box_xz = n4::box("box_xz").xz(lxz).y(ly).solid();
  auto box_yz = n4::box("box_yz").yz(lyz).x(lx).solid();

  auto box_half_xy = n4::box("box_xy").half_xy(lxy).z(lz).solid();
  auto box_half_xz = n4::box("box_xz").half_xz(lxz).y(ly).solid();
  auto box_half_yz = n4::box("box_yz").half_yz(lyz).x(lx).solid();

#define CHECK_SQUARE_PROFILE(SOLID, SQR1, SQR2, DIFF)                                                \
    CHECK_THAT(SOLID -> Get ##SQR1## HalfLength(),   Within1ULP(SOLID-> Get ##SQR2## HalfLength())); \
    CHECK_THAT(SOLID -> Get ##SQR1## HalfLength(), ! Within1ULP(SOLID-> Get ##DIFF## HalfLength()));

  CHECK_SQUARE_PROFILE(box_xy, X, Y, Z);
  CHECK_SQUARE_PROFILE(box_xz, X, Z, Y);
  CHECK_SQUARE_PROFILE(box_yz, Y, Z, X);

  CHECK_SQUARE_PROFILE(box_half_xy, X, Y, Z);
  CHECK_SQUARE_PROFILE(box_half_xz, X, Z, Y);
  CHECK_SQUARE_PROFILE(box_half_yz, Y, Z, X);

#undef CHECK_SQUARE_PROFILE

  CHECK_THAT(box_h -> GetCubicVolume() / m3, Within1ULP(box_s -> GetCubicVolume() / m3));
  CHECK_THAT(box_h -> GetSurfaceArea() / m2, Within1ULP(box_s -> GetSurfaceArea() / m2));

  CHECK     (box_l -> TotalVolumeEntities() == 1);
  CHECK     (box_l -> GetMaterial()         == water);
  CHECK     (box_l -> GetName()             == "box_l");
  CHECK_THAT(box_l -> GetMass() / kg        , Within1ULP(lx * ly * lz * density / kg));

  auto solid = box_l -> GetSolid();
  CHECK     (solid -> GetName()             == "box_l");
  CHECK_THAT(solid -> GetCubicVolume() / m3, Within1ULP(     lx    * ly    * lz     / m3));
  CHECK_THAT(solid -> GetSurfaceArea() / m2, Within1ULP(2 * (lx*ly + ly*lz + lz*lx) / m2));

  CHECK     (box_s -> GetName()             == "box_s");
  CHECK_THAT(box_s -> GetCubicVolume() / m3, Within1ULP(solid -> GetCubicVolume() / m3));
  CHECK_THAT(box_s -> GetSurfaceArea() / m2, Within1ULP(solid -> GetSurfaceArea() / m2));

  CHECK_THAT(box_p -> GetTranslation() . x() / m, Within1ULP(xc / m));
  CHECK_THAT(box_p -> GetTranslation() . y() / m, Within1ULP(yc / m));
  CHECK_THAT(box_p -> GetTranslation() . z() / m, Within1ULP(zc / m));

  auto small_cube = n4::box("small_cube").cube     (lz).solid();
  auto   big_cube = n4::box(  "big_cube").half_cube(lz).solid();
  CHECK_THAT(big_cube -> GetCubicVolume() / m3, Within1ULP(8 * small_cube -> GetCubicVolume() / m3));
  CHECK_THAT(big_cube -> GetSurfaceArea() / m2, Within1ULP(4 * small_cube -> GetSurfaceArea() / m2));
  CHECK_THAT(big_cube -> GetXHalfLength() / m , Within1ULP(2 * small_cube -> GetYHalfLength() / m ));
  CHECK_THAT(big_cube -> GetXHalfLength() / m , Within1ULP(2 * small_cube -> GetZHalfLength() / m ));

  auto check_dimensions = [lx, ly, lz] (auto box) {
    CHECK_THAT(box -> GetXHalfLength() / m, Within1ULP(lx / 2 / m));
    CHECK_THAT(box -> GetYHalfLength() / m, Within1ULP(ly / 2 / m));
    CHECK_THAT(box -> GetZHalfLength() / m, Within1ULP(lz / 2 / m));
  };

  check_dimensions(n4::box("box_xyz")     .     xyz( lx  , ly  , lz   ).solid());
  check_dimensions(n4::box("box_half_xyz").half_xyz( lx/2, ly/2, lz/2 ).solid());
  check_dimensions(n4::box("box_vec")     .     xyz({lx  , ly  , lz  }).solid());
  check_dimensions(n4::box("box_half_vec").half_xyz({lx/2, ly/2, lz/2}).solid());
}

TEST_CASE("nain sphere", "[nain][sphere]") {
  // nain4::sphere is a more convenient interface for constructing G4VSolids and
  // G4LogicalVolumes based on G4Sphere
  auto water = nain4::material("G4_WATER"); auto density = water -> GetDensity();
  auto r  = 2*m;
  auto xc = 4*m;
  auto yc = 5*m;
  auto zc = 6*m;
  auto sphere_s = n4::sphere("sphere_s").r(r).solid();
  auto sphere_l = n4::sphere("sphere_l").r(r).volume(water);
  auto sphere_p = n4::sphere("sphere_p").r(r).place  (water).at(xc, yc, zc).now();

  CHECK     (sphere_l -> TotalVolumeEntities() == 1);
  CHECK     (sphere_l -> GetMaterial()         == water);
  CHECK     (sphere_l -> GetName()             == "sphere_l");
  CHECK_THAT(sphere_l -> GetMass() / kg        , Within1ULP(4 * pi / 3 * r * r * r * density / kg));

  CHECK_THAT(sphere_s -> GetCubicVolume() / m3, Within1ULP(4 * pi / 3 * r * r * r / m3));
  CHECK_THAT(sphere_s -> GetSurfaceArea() / m2, Within1ULP(4 * pi     * r * r     / m2));

  CHECK_THAT(sphere_p -> GetTranslation() . x() / m, Within1ULP(xc / m));
  CHECK_THAT(sphere_p -> GetTranslation() . y() / m, Within1ULP(yc / m));
  CHECK_THAT(sphere_p -> GetTranslation() . z() / m, Within1ULP(zc / m));

  auto start   = twopi/8; auto end = twopi/2; auto delta = twopi/4;
  auto spherer = [&] (auto name) { return n4::sphere(name).r(1); };
  auto phi_s   = spherer("phi_s" ).phi_start(start) /*.end(360)*/   .solid();
  auto phi_se  = spherer("phi_se").phi_start(start).phi_end  (end  ).solid();
  auto phi_sd  = spherer("phi_sd").phi_start(start).phi_delta(delta).solid();
  auto phi_de  = spherer("phi_de").phi_delta(delta).phi_end  (  end).solid();
  auto phi_es  = spherer("phi_es").phi_end  (end  ).phi_start(start).solid();
  auto phi_ds  = spherer("phi_ds").phi_delta(delta).phi_start(start).solid();
  auto phi_ed  = spherer("phi_ed").phi_end  (end  ).phi_delta(delta).solid();
  auto phi_e   = spherer("phi_e" ).phi_end  (end  ) /* .start(0) */ .solid();
  auto phi_d   = spherer("phi_d" ).phi_delta(delta) /* .start(0) */ .solid();

  auto down = [] (auto g4vsolid) { return dynamic_cast<G4Sphere*>(g4vsolid); };

  auto check_phi = [&down] (auto solid, auto start, auto delta) {
    CHECK_THAT(down(solid) -> GetStartPhiAngle(), Within1ULP(start));
    CHECK_THAT(down(solid) -> GetDeltaPhiAngle(), Within1ULP(delta));
  };
  check_phi(phi_s , start      , twopi - start );
  check_phi(phi_se, start      ,   end - start );
  check_phi(phi_sd, start      , delta         );
  check_phi(phi_de, end - delta,         delta );
  check_phi(phi_es, start      ,   end - start );
  check_phi(phi_ds, start      , delta         );
  check_phi(phi_ed, end - delta, delta         );
  check_phi(phi_e ,          0.,   end         );
  check_phi(phi_d ,          0., delta         );

  start = pi/8; end = pi/2; delta = pi/4;
  auto theta_s  = spherer("theta_s" ).theta_start(start) /*.end(180)*/     .solid();
  auto theta_se = spherer("theta_se").theta_start(start).theta_end  (end  ).solid();
  auto theta_sd = spherer("theta_sd").theta_start(start).theta_delta(delta).solid();
  auto theta_de = spherer("theta_de").theta_delta(delta).theta_end  (end  ).solid();
  auto theta_es = spherer("theta_es").theta_end  (end  ).theta_start(start).solid();
  auto theta_ds = spherer("theta_ds").theta_delta(delta).theta_start(start).solid();
  auto theta_ed = spherer("theta_ed").theta_end  (end  ).theta_delta(delta).solid();
  auto theta_e  = spherer("theta_e" ).theta_end  (end  ) /* .start(0) */   .solid();
  auto theta_d  = spherer("theta_d" ).theta_delta(delta) /* .start(0) */   .solid();

  auto check_theta = [&down] (auto solid, auto start, auto delta) {
    CHECK_THAT(down(solid) -> GetStartThetaAngle(), Within1ULP(start));
    CHECK_THAT(down(solid) -> GetDeltaThetaAngle(), Within1ULP(delta));
  };

  check_theta(theta_s , start      ,    pi - start );
  check_theta(theta_se, start      ,   end - start );
  check_theta(theta_sd, start      , delta         );
  check_theta(theta_de, end - delta, delta         );
  check_theta(theta_es, start      ,   end - start );
  check_theta(theta_ds, start      , delta         );
  check_theta(theta_ed, end - delta, delta         );
  check_theta(theta_e ,          0.,   end         );
  check_theta(theta_d ,          0., delta         );

  start = m/8; end = m/2; delta = m/4;
  //  auto r_s  = n4::sphere("r_s" ).r_inner(start) /*.end(180)*/     .solid(); // 1/8 - 8/8    7/8
  auto r_se = n4::sphere("r_se").r_inner(start).r      (end  ).solid();
  auto r_sd = n4::sphere("r_sd").r_inner(start).r_delta(delta).solid();
  auto r_ed = n4::sphere("r_ed").r      (end  ).r_delta(delta).solid();
  auto r_es = n4::sphere("r_es").r      (end  ).r_inner(start).solid();
  auto r_ds = n4::sphere("r_ds").r_delta(delta).r_inner(start).solid();
  auto r_de = n4::sphere("r_de").r_delta(delta).r      (end  ).solid();
  auto r_e  = n4::sphere("r_e" ).r      (end  )/*.r_inner(0)*/.phi_delta(1).solid(); // phi_delta to avoid creating G4Orb
  auto r_d  = n4::sphere("r_d" ).r_delta(delta)/*.r_inner(0)*/.phi_delta(1).solid();

  auto check_r = [&down] (auto solid, auto inner, auto outer) {
    CHECK_THAT(down(solid) -> GetInnerRadius(), Within1ULP(inner));
    CHECK_THAT(down(solid) -> GetOuterRadius(), Within1ULP(outer));
  };

  // check_r(r_s , start,    pi - start); // Shouldn't work
  check_r(r_se,       start,   end         );
  check_r(r_es,       start,   end         );

  check_r(r_sd,       start, start + delta );
  check_r(r_ds,       start, start + delta );

  check_r(r_ed, end - delta,   end         );
  check_r(r_de, end - delta,   end         );

  check_r(r_e ,          0.,   end         );

  check_r(r_d ,          0., delta         );
}

TEST_CASE("nain sphere orb", "[nain][sphere][orb]") {
  auto is_sphere = [](auto thing) { CHECK(dynamic_cast<G4Sphere*>(thing)); };
  auto is_orb    = [](auto thing) { CHECK(dynamic_cast<G4Orb   *>(thing)); };

  is_sphere(n4::sphere("r_inner"            ).r_inner(1).r      (2).solid());
  is_orb   (n4::sphere("r_inner_full"       ).r_inner(0).r      (2).solid());
  is_sphere(n4::sphere("r_delta"            ).r_delta(1).r      (2).solid());
  is_orb   (n4::sphere("r_delta_full"       ).r_delta(2).r      (2).solid());
  is_sphere(n4::sphere("r_delta_innner"     ).r_delta(2).r_inner(1).solid());
  is_orb   (n4::sphere("r_delta_innner_full").r_delta(2).r_inner(0).solid());

  auto full    = twopi; auto half = twopi/2; auto more = full + half;
  auto spherer = [&] (auto name) { return n4::sphere(name).r(1); };

  is_sphere(spherer("phi_start"           ).phi_start(half)                .solid());
  is_orb   (spherer("phi_start_zero"      ).phi_start( 0.0)                .solid());
  is_sphere(spherer("phi_start_end"       ).phi_start(half).phi_end  (full).solid());
  is_orb   (spherer("phi_start_end_full"  ).phi_start(half).phi_end  (more).solid());
  is_sphere(spherer("phi_delta"           ).phi_delta(half)                .solid());
  is_orb   (spherer("phi_delta_full"      ).phi_delta(full)                .solid());
  is_sphere(spherer("phi_delta_start"     ).phi_delta(half).phi_start(half).solid());
  is_orb   (spherer("phi_delta_start_full").phi_delta(full).phi_start(half).solid());
  is_sphere(spherer("phi_end"             ).phi_end  (half)                .solid());
  is_orb   (spherer("phi_end_full"        ).phi_end  (full)                .solid());

  // TODO implement these (and others) if we ever allow providing angle_delta with angle_end
  // is_sphere(n4::sphere("phi_delta_end"       ).r(1).phi_delta(half).phi_end(full).solid());
  // is_orb   (n4::sphere("phi_delta_end"       ).r(1).phi_delta(full).phi_end(half).solid());

  full = twopi/2; half = twopi/4; more = full + half;
  is_sphere(spherer("theta_start"           ).theta_start(half)                  .solid());
  is_orb   (spherer("theta_start_zero"      ).theta_start( 0.0)                  .solid());
  is_sphere(spherer("theta_start_end"       ).theta_start(half).theta_end  (full).solid());
  is_orb   (spherer("theta_start_end_full"  ).theta_start(half).theta_end  (more).solid());
  is_sphere(spherer("theta_delta"           ).theta_delta(half)                  .solid());
  is_orb   (spherer("theta_delta_full"      ).theta_delta(full)                  .solid());
  is_sphere(spherer("theta_delta_start"     ).theta_delta(half).theta_start(half).solid());
  is_orb   (spherer("theta_delta_start_full").theta_delta(full).theta_start(half).solid());
  is_sphere(spherer("theta_end"             ).theta_end  (half)                  .solid());
  is_orb   (spherer("theta_end_full"        ).theta_end  (full)                  .solid());
}

TEST_CASE("nain tubs", "[nain][tubs]") {
  // nain4::tubs is a more convenient interface for constructing G4VSolids and
  // G4LogicalVolumes based on G4Tubs
  auto water = nain4::material("G4_WATER"); auto density = water -> GetDensity();
  auto r  = 2*m;
  auto z  = 1*m;
  auto xc = 4*m;
  auto yc = 5*m;
  auto zc = 6*m;
  auto tubs_s = n4::tubs("tubs_s").r(r).z(z).solid();
  auto tubs_l = n4::tubs("tubs_l").r(r).z(z).volume(water);
  auto tubs_p = n4::tubs("tubs_p").r(r).z(z).place (water).at(xc, yc, zc).now();

  CHECK     (tubs_l -> TotalVolumeEntities() == 1);
  CHECK     (tubs_l -> GetMaterial()         == water);
  CHECK     (tubs_l -> GetName()             == "tubs_l");
  CHECK_THAT(tubs_l -> GetMass() / kg        , Within1ULP(pi * r * r * z * density / kg));

  CHECK_THAT(tubs_s -> GetCubicVolume() / m3, Within1ULP(     pi * r * r * z               / m3));
  CHECK_THAT(tubs_s -> GetSurfaceArea() / m2, Within1ULP((2 * pi * r * z + 2 * pi * r * r) / m2));

  CHECK_THAT(tubs_p -> GetTranslation().x() / m, Within1ULP(xc / m));
  CHECK_THAT(tubs_p -> GetTranslation().y() / m, Within1ULP(yc / m));
  CHECK_THAT(tubs_p -> GetTranslation().z() / m, Within1ULP(zc / m));

  auto start  = twopi/8; auto end = twopi/2; auto delta = twopi/4;
  auto tubsrz = [&] (auto name) { return n4::tubs(name).r(1).z(1); };
  auto phi_s  = tubsrz("phi_s" ).phi_start(start) /*.end(360)*/   .solid();
  auto phi_se = tubsrz("phi_se").phi_start(start).phi_end  (end  ).solid();
  auto phi_sd = tubsrz("phi_sd").phi_start(start).phi_delta(delta).solid();
  auto phi_de = tubsrz("phi_de").phi_delta(delta).phi_end  (end  ).solid();
  auto phi_es = tubsrz("phi_es").phi_end  (end  ).phi_start(start).solid();
  auto phi_ds = tubsrz("phi_ds").phi_delta(delta).phi_start(start).solid();
  auto phi_ed = tubsrz("phi_ed").phi_end  (end  ).phi_delta(delta).solid();
  auto phi_e  = tubsrz("phi_e" ).phi_end  (end  ) /* .start(0) */ .solid();
  auto phi_d  = tubsrz("phi_d" ).phi_delta(delta) /* .start(0) */ .solid();

  auto check_phi = [] (auto solid, auto start, auto delta) {
    CHECK_THAT(solid -> GetStartPhiAngle(), Within1ULP(start));
    CHECK_THAT(solid -> GetDeltaPhiAngle(), Within1ULP(delta));
  };
  check_phi(phi_s , start      , twopi - start );
  check_phi(phi_se, start      ,   end - start );
  check_phi(phi_sd, start      , delta         );
  check_phi(phi_de, end - delta, delta         );
  check_phi(phi_es, start      ,   end - start );
  check_phi(phi_ds, start      , delta         );
  check_phi(phi_ed, end - delta, delta         );
  check_phi(phi_e ,          0.,   end         );
  check_phi(phi_d ,          0., delta         );

  auto z_full = n4::tubs("z_full").r(1).     z(z  ).solid();
  auto z_half = n4::tubs("z_half").r(1).half_z(z/2).solid();

  CHECK_THAT(z_full -> GetZHalfLength(), Within1ULP(z_half -> GetZHalfLength()));
  CHECK_THAT(z_full -> GetZHalfLength(), Within1ULP(z/2                       ));

  start = m/8; end = m/2; delta = m/4;
  //  Meaningless case: auto r_s  = n4::tubs("r_s" ).r_inner(start) /*.end(180)*/ .solid();
  auto tubsz = [&] (auto name) { return n4::tubs(name).z(1); };
  auto r_se  = tubsz("r_se").r_inner(start).r      (end  ).solid();
  auto r_sd  = tubsz("r_sd").r_inner(start).r_delta(delta).solid();
  auto r_ed  = tubsz("r_ed").r      (end  ).r_delta(delta).solid();
  auto r_es  = tubsz("r_es").r      (end  ).r_inner(start).solid();
  auto r_ds  = tubsz("r_ds").r_delta(delta).r_inner(start).solid();
  auto r_de  = tubsz("r_de").r_delta(delta).r      (end  ).solid();
  auto r_e   = tubsz("r_e" ).r      (end  )/*.r_inner(0)*/.solid();
  auto r_d   = tubsz("r_d" ).r_delta(delta)/*.r_inner(0)*/.solid();

  auto check_r = [] (auto solid, auto inner, auto outer) {
    CHECK_THAT(solid -> GetInnerRadius(), Within1ULP(inner));
    CHECK_THAT(solid -> GetOuterRadius(), Within1ULP(outer));
  };

  // check_r(r_s , start,    pi - start); // Shouldn't work
  check_r(r_se,       start,   end         );
  check_r(r_es,       start,   end         );

  check_r(r_sd,       start, start + delta );
  check_r(r_ds,       start, start + delta );

  check_r(r_ed, end - delta,   end         );
  check_r(r_de, end - delta,   end         );

  check_r(r_e ,          0.,   end         );

  check_r(r_d ,          0., delta         );
}

TEST_CASE("nain cons", "[nain][cons]") {
  // nain4::cons is a more convenient interface for constructing G4VSolids and
  // G4LogicalVolumes based on G4Cons
  auto water = nain4::material("G4_WATER"); auto density = water -> GetDensity();
  auto r1 = 2*m; auto r2 = 3*m;
  auto z  = 4*m;
  auto xc = 5*m; auto yc = 6*m; auto zc = 7*m;
  auto cons_s = n4::cons("cons_s").r1(r1).r2(r2).z(z).solid();
  auto cons_l = n4::cons("cons_l").r1(r1).r2(r2).z(z).volume(water);
  auto cons_p = n4::cons("cons_p").r1(r1).r2(r2).z(z).place (water).at(xc, yc, zc).now();

  // V = (1/3) * π * h * (r² + r * R + R²)
  auto volume = pi * 1/3 * z * (r1*r1 + r1*r2 + r2*r2);

  // s = √((R - r)² + h²).  Lateral = π × (R + r) × s
  auto dr = (r2 - r1);
  auto slant_height = std::sqrt(dr*dr + z*z);
  auto area         = pi * (r1+r2) * slant_height
                    + pi * (r1*r1 + r2*r2);

  CHECK     (cons_l -> TotalVolumeEntities() == 1);
  CHECK     (cons_l -> GetMaterial()         == water);
  CHECK     (cons_l -> GetName()             == "cons_l");
  CHECK_THAT(cons_l -> GetMass() / kg        , Within1ULP(volume * density / kg));

  CHECK_THAT(cons_s -> GetCubicVolume() / m3, Within1ULP(volume / m3));
  CHECK_THAT(cons_s -> GetSurfaceArea() / m2, Within1ULP(area   / m2));

  CHECK_THAT(cons_p -> GetTranslation() . x() / m, Within1ULP(xc / m));
  CHECK_THAT(cons_p -> GetTranslation() . y() / m, Within1ULP(yc / m));
  CHECK_THAT(cons_p -> GetTranslation() . z() / m, Within1ULP(zc / m));

  auto start  = twopi/8; auto end = twopi/2; auto delta = twopi/4;
  auto consrz = [&] (auto name) { return n4::cons(name).r1(1).r2(2).z(1); };
  auto phi_s  = consrz("phi_s" ).phi_start(start) /*.end(360)*/   .solid();
  auto phi_se = consrz("phi_se").phi_start(start).phi_end  (end  ).solid();
  auto phi_sd = consrz("phi_sd").phi_start(start).phi_delta(delta).solid();
  auto phi_de = consrz("phi_de").phi_delta(delta).phi_end  (end  ).solid();
  auto phi_es = consrz("phi_es").phi_end  (end  ).phi_start(start).solid();
  auto phi_ds = consrz("phi_ds").phi_delta(delta).phi_start(start).solid();
  auto phi_ed = consrz("phi_ed").phi_end  (end  ).phi_delta(delta).solid();
  auto phi_e  = consrz("phi_e" ).phi_end  (end  ) /* .start(0) */ .solid();
  auto phi_d  = consrz("phi_d" ).phi_delta(delta) /* .start(0) */ .solid();

  auto check_phi = [] (auto solid, auto start, auto delta) {
    CHECK_THAT(solid -> GetStartPhiAngle(), Within1ULP(start));
    CHECK_THAT(solid -> GetDeltaPhiAngle(), Within1ULP(delta));
  };
  check_phi(phi_s , start      , twopi - start );
  check_phi(phi_se, start      ,   end - start );
  check_phi(phi_sd, start      , delta         );
  check_phi(phi_de, end - delta, delta         );
  check_phi(phi_es, start      ,   end - start );
  check_phi(phi_ds, start      , delta         );
  check_phi(phi_ed, end - delta, delta         );
  check_phi(phi_e ,          0.,   end         );
  check_phi(phi_d ,          0., delta         );

  auto z_full = n4::cons("z_full").r1(1).r2(1).     z(z  ).solid();
  auto z_half = n4::cons("z_half").r1(1).r2(1).half_z(z/2).solid();

  CHECK_THAT(z_full -> GetZHalfLength(), Within1ULP(z_half -> GetZHalfLength()));
  CHECK_THAT(z_full -> GetZHalfLength(), Within1ULP(z/2                       ));

  start = m/8; end = m/2; delta = m/4;
  // r1 = 0 would be meaningless

  auto cons1 = [] (auto name) { return n4::cons(name).r1_inner(0.1).r1(1).z(1); };
  auto cons2 = [] (auto name) { return n4::cons(name).r2_inner(0.1).r2(1).z(1); };

  auto r2_se = cons1("r2_se").r2_inner(start).r2      (end  ).solid();
  auto r2_sd = cons1("r2_sd").r2_inner(start).r2_delta(delta).solid();
  auto r2_ed = cons1("r2_ed").r2      (end  ).r2_delta(delta).solid();
  auto r2_es = cons1("r2_es").r2      (end  ).r2_inner(start).solid();
  auto r2_ds = cons1("r2_ds").r2_delta(delta).r2_inner(start).solid();
  auto r2_de = cons1("r2_de").r2_delta(delta).r2      (end  ).solid();
  auto r2_e  = cons1("r2_e" ).r2      (end  )/*.r2_inner(0)*/.solid();
  auto r2_d  = cons1("r2_d" ).r2_delta(delta)/*.r2_inner(0)*/.solid();

  auto r1_se = cons2("r1_se").r1_inner(start).r1      (end  ).solid();
  auto r1_sd = cons2("r1_sd").r1_inner(start).r1_delta(delta).solid();
  auto r1_ed = cons2("r1_ed").r1      (end  ).r1_delta(delta).solid();
  auto r1_es = cons2("r1_es").r1      (end  ).r1_inner(start).solid();
  auto r1_ds = cons2("r1_ds").r1_delta(delta).r1_inner(start).solid();
  auto r1_de = cons2("r1_de").r1_delta(delta).r1      (end  ).solid();
  auto r1_e  = cons2("r1_e" ).r1      (end  )/*.r1_inner(0)*/.solid();
  auto r1_d  = cons2("r1_d" ).r1_delta(delta)/*.r1_inner(0)*/.solid();

  auto start_2 = 2*start, end_2 = 2*end;

  auto r12_de = n4::cons("r12_de").z(1*m).r1      (  end).r2      (  end_2).r_delta(delta).solid();
  auto r12_sd = n4::cons("r12_sd").z(1*m).r1_inner(start).r2_inner(start_2).r_delta(delta).solid();

  auto check_r2 = [] (auto solid, auto inner, auto outer) {
    CHECK_THAT(solid -> GetInnerRadiusPlusZ(), Within1ULP(inner));
    CHECK_THAT(solid -> GetOuterRadiusPlusZ(), Within1ULP(outer));
  };
  auto check_r1 = [] (auto solid, auto inner, auto outer) {
    CHECK_THAT(solid -> GetInnerRadiusMinusZ(), Within1ULP(inner));
    CHECK_THAT(solid -> GetOuterRadiusMinusZ(), Within1ULP(outer));
  };

  auto eps = n4::cons::eps;
  // check_r2(r2_s , start,    pi - start); // Shouldn't work
  check_r1(r1_se,       start,   end         );
  check_r1(r1_es,       start,   end         );
  check_r1(r1_sd,       start, start + delta );
  check_r1(r1_ds,       start, start + delta );
  check_r1(r1_ed, end - delta,   end         );
  check_r1(r1_de, end - delta,   end         );
  check_r1(r1_e ,         eps,   end         );
  check_r1(r1_d ,         eps, delta         );

  // check_r2(r2_s , start,    pi - start); // Shouldn't work
  check_r2(r2_se,       start,   end         );
  check_r2(r2_es,       start,   end         );
  check_r2(r2_sd,       start, start + delta );
  check_r2(r2_ds,       start, start + delta );
  check_r2(r2_ed, end - delta,   end         );
  check_r2(r2_de, end - delta,   end         );
  check_r2(r2_e ,         eps,   end         );
  check_r2(r2_d ,         eps, delta         );

  check_r1(r12_de, end   - delta, end            );
  check_r2(r12_de, end_2 - delta, end_2          );
  check_r1(r12_sd, start        , start   + delta);
  check_r2(r12_sd, start_2      , start_2 + delta);
}

TEST_CASE("nain trd", "[nain][trd]") {
  // nain4::trd is a more convenient interface for constructing G4VSolids and
  // G4LogicalVolumes based on G4Trd
  auto water = nain4::material("G4_WATER"); auto density = water -> GetDensity();
  auto lx1 = 1 * m; auto lx2 = 4 * m;
  auto ly1 = 2 * m; auto ly2 = 5 * m;
  auto lz  = 3 * m;
  auto xc = 7 * m;
  auto yc = 8 * m;
  auto zc = 9 * m;
  auto trd_h = n4::trd("trd_h").half_x1(lx1/2).half_y1(ly1/2).half_x2(lx2/2).half_y2(ly2/2).half_z(lz/2).solid();
  auto trd_s = n4::trd("trd_s")     .x1(lx1  )     .y1(ly1  ).     x2(lx2  )     .y2(ly2  )     .z(lz  ).solid();
  auto trd_l = n4::trd("trd_l")     .x1(lx1  )     .y1(ly1  ).     x2(lx2  )     .y2(ly2  )     .z(lz  ).volume(water);
  auto trd_p = n4::trd("trd_p")     .x1(lx1  )     .y1(ly1  ).     x2(lx2  )     .y2(ly2  )     .z(lz  ).place (water).at(xc, yc, zc).now();

  auto lxy1 = 10 * m;
  auto lxy2 = 11 * m;
  auto trd_xy      = n4::trd("trd_xy").     xy1(lxy1  ).     xy2(lxy2).z(lz).solid();
  auto trd_half_xy = n4::trd("trd_xy").half_xy1(lxy1/2).half_xy2(lxy2).z(lz).solid();

  CHECK_THAT(trd_xy      -> GetXHalfLength1(),   Within1ULP(trd_xy      -> GetYHalfLength1()));
  CHECK_THAT(trd_xy      -> GetXHalfLength2(),   Within1ULP(trd_xy      -> GetYHalfLength2()));
  CHECK_THAT(trd_xy      -> GetXHalfLength1(), ! Within1ULP(trd_xy      -> GetZHalfLength ()));
  CHECK_THAT(trd_xy      -> GetXHalfLength2(), ! Within1ULP(trd_xy      -> GetZHalfLength ()));
  CHECK_THAT(trd_half_xy -> GetXHalfLength1(),   Within1ULP(trd_half_xy -> GetYHalfLength1()));
  CHECK_THAT(trd_half_xy -> GetXHalfLength2(),   Within1ULP(trd_half_xy -> GetYHalfLength2()));
  CHECK_THAT(trd_half_xy -> GetXHalfLength1(), ! Within1ULP(trd_half_xy -> GetZHalfLength ()));

  auto dlx     = lx2 - lx1;
  auto dly     = ly2 - ly1;
  auto slx     = lx2 + lx1;
  auto sly     = ly2 + ly1;

  auto volume  = ( slx * sly + dlx * dly / 3 ) * lz / 4;
  auto surface = lx1 * ly1 + lx2 * ly2
               + sly * std::sqrt(lz*lz + dlx * dlx / 4)
               + slx * std::sqrt(lz*lz + dly * dly / 4);

  CHECK_THAT(trd_h -> GetCubicVolume() / m3, Within1ULP(trd_s -> GetCubicVolume() / m3));
  CHECK_THAT(trd_h -> GetSurfaceArea() / m2, Within1ULP(trd_s -> GetSurfaceArea() / m2));

  CHECK     (trd_l -> TotalVolumeEntities() == 1);
  CHECK     (trd_l -> GetMaterial()         == water);
  CHECK     (trd_l -> GetName()             == "trd_l");
  CHECK_THAT(trd_l -> GetMass() / kg        , Within1ULP(volume * density / kg));

  auto solid = trd_l -> GetSolid();
  CHECK     (solid -> GetName()             == "trd_l");
  CHECK_THAT(solid -> GetCubicVolume() / m3, Within1ULP(volume  / m3));
  CHECK_THAT(solid -> GetSurfaceArea() / m2, Within1ULP(surface / m2));

  CHECK     (trd_s -> GetName()             == "trd_s");
  CHECK_THAT(trd_s -> GetCubicVolume() / m3, Within1ULP(solid -> GetCubicVolume() / m3));
  CHECK_THAT(trd_s -> GetSurfaceArea() / m2, Within1ULP(solid -> GetSurfaceArea() / m2));

  CHECK_THAT(trd_p -> GetTranslation() . x() / m, Within1ULP(xc / m));
  CHECK_THAT(trd_p -> GetTranslation() . y() / m, Within1ULP(yc / m));
  CHECK_THAT(trd_p -> GetTranslation() . z() / m, Within1ULP(zc / m));

  auto check_dimensions = [&] (auto trd) {
    CHECK_THAT(trd -> GetXHalfLength1() / m , Within1ULP(lxy1 / 2 / m));
    CHECK_THAT(trd -> GetYHalfLength1() / m , Within1ULP(lxy1 / 2 / m));
    CHECK_THAT(trd -> GetXHalfLength2() / m , Within1ULP(lxy2 / 2 / m));
    CHECK_THAT(trd -> GetYHalfLength2() / m , Within1ULP(lxy2 / 2 / m));
    CHECK_THAT(trd -> GetZHalfLength () / m , Within1ULP(lz   / 2 / m));
  };

  check_dimensions(n4::trd("trd_xyz")     .     xy1(lxy1  ).     xy2(lxy2  ).z(lz).solid());
  check_dimensions(n4::trd("trd_half_xyz").half_xy1(lxy1/2).half_xy2(lxy2/2).z(lz).solid());
}



TEST_CASE("nain volume", "[nain][volume]") {
  // nain4::volume produces objects with sensible sizes, masses, etc.
  auto water = nain4::material("G4_WATER"); auto density = water->GetDensity();
  auto lx = 1 * m;
  auto ly = 2 * m;
  auto lz = 3 * m;
  auto box = nain4::volume<G4Box>("test_box", water, lx, ly, lz);
  CHECK     (box->TotalVolumeEntities() == 1);
  CHECK     (box->GetMaterial()         == water);
  CHECK     (box->GetName()             == "test_box");
  CHECK_THAT(box->GetMass() / kg        , Within1ULP(8 * lx * ly * lz * density / kg));

  auto solid = box->GetSolid();
  CHECK     (solid->GetName()             == "test_box");
  CHECK_THAT(solid->GetCubicVolume() / m3 , Within1ULP(8 *  lx    * ly    * lz     / m3));
  CHECK_THAT(solid->GetSurfaceArea() / m2 , Within1ULP(8 * (lx*ly + ly*lz + lz*lx) / m2));
}

TEST_CASE("nain place", "[nain][place]") {
  // n4::place is a good replacement for G4PVPlacement
  auto air   = n4::material("G4_AIR");
  auto water = n4::material("G4_WATER");
  auto outer = n4::volume<G4Box>("outer", air, 1*m, 2*m, 3*m);

  // Default values are sensible
  SECTION("defaults") {
    auto world = n4::place(outer).now();

    auto trans = world->GetObjectTranslation();
    CHECK(trans == G4ThreeVector{});
    CHECK(world -> GetName()          == "outer");
    CHECK(world -> GetCopyNo()        == 0);
    CHECK(world -> GetLogicalVolume() == outer);
    CHECK(world -> GetMotherLogical() == nullptr);
  }

  // Multiple optional values can be set at once.
  SECTION("multiple options") {
    G4ThreeVector translation = {1,2,3};
    auto world = n4::place(outer)
      .at(translation) // 1-arg version of at()
      .name("not outer")
      .copy_no(382)
      .now();

    CHECK(world -> GetObjectTranslation() == translation);
    CHECK(world -> GetName()   == "not outer");
    CHECK(world -> GetCopyNo() == 382);
  }

  // The at() option accepts vector components (as well as a whole vector)
  SECTION("at 3-args") {
    auto world = n4::place(outer).at(4,5,6).now(); // 3-arg version of at()
    CHECK(world->GetObjectTranslation() == G4ThreeVector{4,5,6});
  }

  SECTION("at_{x,y,z}") {
    auto place_x   = n4::place(outer).at_x( 1)                  .now(); // 1-dim version  of at()
    auto place_y   = n4::place(outer)         .at_y( 2)         .now(); // 1-dim version  of at()
    auto place_z   = n4::place(outer)                  .at_z( 3).now(); // 1-dim version  of at()
    auto place_xy  = n4::place(outer).at_x( 4).at_y( 5)         .now(); // 1-dim versions of at()
    auto place_xz  = n4::place(outer).at_x( 6)         .at_z( 7).now(); // 1-dim versions of at()
    auto place_yz  = n4::place(outer)         .at_y( 8).at_z( 9).now(); // 1-dim versions of at()
    auto place_xyz = n4::place(outer).at_x(10).at_y(11).at_z(12).now(); // 1-dim versions of at()

    CHECK(place_x   -> GetObjectTranslation() == G4ThreeVector{ 1, 0, 0});
    CHECK(place_y   -> GetObjectTranslation() == G4ThreeVector{ 0, 2, 0});
    CHECK(place_z   -> GetObjectTranslation() == G4ThreeVector{ 0, 0, 3});
    CHECK(place_xy  -> GetObjectTranslation() == G4ThreeVector{ 4, 5, 0});
    CHECK(place_xz  -> GetObjectTranslation() == G4ThreeVector{ 6, 0, 7});
    CHECK(place_yz  -> GetObjectTranslation() == G4ThreeVector{ 0, 8, 9});
    CHECK(place_xyz -> GetObjectTranslation() == G4ThreeVector{10,11,12});
  }

  // The in() option creates correct mother/daughter relationship
  SECTION("in") {
    SECTION("logical mother") {
      auto inner = n4::box("inner").xyz(0.3*m, 0.2*m, 0.1*m).volume(water);

      auto inner_placed = n4::place(inner)
        .in(outer)
        .at(0.1*m, 0.2*m, 0.3*m)
        .now();

      auto outer_placed = n4::place(outer).now();

      CHECK(inner_placed -> GetMotherLogical() == outer);
      CHECK(outer_placed -> GetLogicalVolume() == outer);
      CHECK(outer -> GetNoDaughters() == 1);
      CHECK(outer -> GetDaughter(0) -> GetLogicalVolume() == inner);

      // Quick visual check that geometry_iterator works TODO expand
      SECTION("geometry iterator") {
        std::cout << std::endl;
        for (const auto v: outer_placed) {
          std::cout << std::setw(15) << v->GetName() << ": ";
          auto l = v->GetLogicalVolume();
          std::cout
            << std::setw(12) << l->GetMaterial()->GetName()
            << std::setw(12) << G4BestUnit(l->GetMass(), "Mass")
            << std::setw(12) << G4BestUnit(l->GetSolid()->GetCubicVolume(), "Volume")
            << std::endl;
        }
        std::cout << std::endl;
      }
    }

    SECTION("physical mother") {
      // Now we place outer first and use it to place inner
      auto outer_placed = n4::place(outer).now();

      auto inner = n4::box("inner").xyz(0.3*m, 0.2*m, 0.1*m).volume(water);

      auto inner_placed = n4::place(inner)
        .in(outer_placed)
        .at(0.1*m, 0.2*m, 0.3*m)
        .now();

      CHECK(inner_placed -> GetMotherLogical() == outer);
      CHECK(outer_placed -> GetLogicalVolume() == outer);
      CHECK(outer -> GetNoDaughters() == 1);
      CHECK(outer -> GetDaughter(0) -> GetLogicalVolume() == inner);
    }

    SECTION("proto-physical mother") {
      // Now we don't place the outer volume immediately, but we store
      // it for later use

      auto outer_stored = n4::place(outer);

      auto inner = n4::box("inner").xyz(0.3*m, 0.2*m, 0.1*m).volume(water);

      auto inner_placed = n4::place(inner)
        .in(outer_stored)
        .at(0.1*m, 0.2*m, 0.3*m)
        .now();

      auto outer_placed = outer_stored.now();


      CHECK(inner_placed -> GetMotherLogical() == outer);
      CHECK(outer_stored .  get_logical()      == outer);
      CHECK(outer_placed -> GetLogicalVolume() == outer);
      CHECK(outer -> GetNoDaughters() == 1);
      CHECK(outer -> GetDaughter(0) -> GetLogicalVolume() == inner);
    }
  }



  SECTION("rotations") {
    auto box   = n4::box("box").xyz(0.3*m, 0.2*m, 0.1*m).volume(water);

    auto sign      = GENERATE(-1, 1);
    auto angle_deg = GENERATE(0, 12.345, 30, 45, 60, 90, 180);
    auto angle     = sign * angle_deg * deg;

    auto rotate_x   = n4::place(box).rotate_x(angle)                                .now();
    auto rotate_y   = n4::place(box).rotate_y(angle)                                .now();
    auto rotate_z   = n4::place(box).rotate_z(angle)                                .now();
    auto rotate_xy  = n4::place(box).rotate_x(angle).rotate_y(angle)                .now();
    auto rotate_xz  = n4::place(box).rotate_x(angle).rotate_z(angle)                .now();
    auto rotate_yz  = n4::place(box).rotate_y(angle).rotate_z(angle)                .now();
    auto rotate_xyz = n4::place(box).rotate_x(angle).rotate_y(angle).rotate_z(angle).now();
    auto rot_x      = n4::place(box).rot_x   (angle)                                .now();
    auto rot_y      = n4::place(box).rot_y   (angle)                                .now();
    auto rot_z      = n4::place(box).rot_z   (angle)                                .now();
    auto rot_xy     = n4::place(box).rot_x   (angle).rotate_y(angle)                .now();
    auto rot_xz     = n4::place(box).rot_x   (angle).rotate_z(angle)                .now();
    auto rot_yz     = n4::place(box).rot_y   (angle).rotate_z(angle)                .now();
    auto rot_xyz    = n4::place(box).rot_x   (angle).rotate_y(angle).rotate_z(angle).now();

    auto rotmat  = [&] (auto x, auto y, auto z){
       auto rm = G4RotationMatrix();
       rm.rotateX(x);
       rm.rotateY(y);
       rm.rotateZ(z);
       return rm;
    };

    auto manyrot = rotmat(angle, angle, angle);
    auto rotate  = n4::place(box).rotate(manyrot).now();
    auto rot     = n4::place(box).rot   (manyrot).now();

    CHECK(* rotate     -> GetObjectRotation() == manyrot);
    CHECK(* rot        -> GetObjectRotation() == manyrot);

    CHECK(* rotate_x   -> GetObjectRotation() == rotmat(angle,     0,     0));
    CHECK(* rotate_y   -> GetObjectRotation() == rotmat(    0, angle,     0));
    CHECK(* rotate_z   -> GetObjectRotation() == rotmat(    0,     0, angle));
    CHECK(* rotate_xy  -> GetObjectRotation() == rotmat(angle, angle,     0));
    CHECK(* rotate_xz  -> GetObjectRotation() == rotmat(angle,     0, angle));
    CHECK(* rotate_yz  -> GetObjectRotation() == rotmat(    0, angle, angle));
    CHECK(* rotate_xyz -> GetObjectRotation() == rotmat(angle, angle, angle));
    CHECK(* rot_x      -> GetObjectRotation() == rotmat(angle,     0,     0));
    CHECK(* rot_y      -> GetObjectRotation() == rotmat(    0, angle,     0));
    CHECK(* rot_z      -> GetObjectRotation() == rotmat(    0,     0, angle));
    CHECK(* rot_xy     -> GetObjectRotation() == rotmat(angle, angle,     0));
    CHECK(* rot_xz     -> GetObjectRotation() == rotmat(angle,     0, angle));
    CHECK(* rot_yz     -> GetObjectRotation() == rotmat(    0, angle, angle));
    CHECK(* rot_xyz    -> GetObjectRotation() == rotmat(angle, angle, angle));
  }

  SECTION("transforms") {
    auto box    = n4::box("box").xyz(0.3*m, 0.2*m, 0.1*m).volume(water);

    auto rotation    = G4RotationMatrix{}; rotation.rotateX(30 * deg);
    auto translation = G4ThreeVector{0., 0., 50 * cm};
    auto transform   = G4Transform3D{rotation, translation};
    auto placed      = n4::place(box).trans(transform).now();

    CHECK(*placed -> GetObjectRotation   () ==    rotation);
    CHECK( placed -> GetObjectTranslation() == translation);
  }

  SECTION("clone") {
    auto place_box = n4::box("box").cube(1*mm).place(water).in(outer).at_x(2*mm);

    auto check = [] (auto x, double expected) {
      CHECK_THAT(x -> GetTranslation().x() / mm, Within1ULP(expected));
    };

    auto a = place_box.clone().at_x(10*mm).now(); check(a, 2 + 10          );
    auto b = place_box.clone().at_x(20*mm).now(); check(b, 2 + 20          );
    auto c = place_box        .at_x(30*mm).now(); check(c, 2 + 30          );
    auto d = place_box        .at_x(40*mm).now(); check(d, 2 + 30 + 40     );
    auto e = place_box.clone().at_x(50*mm).now(); check(e, 2 + 30 + 40 + 50);

  }
}

TEST_CASE("nain find geometry", "[nain][find][geometry]") {
  default_run_manager().run();

  auto air       = nain4::material("G4_AIR");
  auto long_name = "made just for find_logical test";
  auto find_me   = nain4::volume<G4Box>(long_name, air, 1 * cm, 1 * cm, 1 * cm);
  auto found     = nain4::find_logical(long_name);
  CHECK(found == find_me);
  auto should_not_exist = nain4::find_logical("Hopefully this name hasn't been used anywhere", false);
  CHECK(should_not_exist == nullptr);

  auto placed = nain4::place(find_me).now();
  auto found_placed = nain4::find_physical(long_name);
  CHECK(found_placed == placed);
  CHECK(found_placed != nullptr);
}

TEST_CASE("nain find particle", "[nain][find][particle]") {
  default_run_manager().run();

  auto name = "gamma";
  auto pita = G4ParticleTable::GetParticleTable()->FindParticle(name);
  auto solid = G4Gamma::Definition();
  auto convenient = nain4::find_particle(name);
  CHECK(convenient == pita);
  CHECK(convenient == solid);
}

TEST_CASE("nain find solid downcast", "[nain][find][solid]") {
  auto box  = n4::box ("box" ).cube(1)  .solid();
  auto tubs = n4::tubs("tubs").r(1).z(2).solid();

  auto found_box  = n4::find_solid<G4Box >("box" );
  auto found_tubs = n4::find_solid<G4Tubs>("tubs");

  CHECK(found_box  == box );
  CHECK(found_tubs == tubs);
}


TEST_CASE("nain find solid downcast not found", "[nain][find][solid]") {
  REQUIRE_THROWS_AS(n4::find_solid<G4Box>("does_not_exist"), n4::exceptions::not_found);
}

TEST_CASE("nain find solid downcast bad cast", "[nain][find][solid]") {
  n4::box("box").cube(1).solid();
  REQUIRE_THROWS_AS(n4::find_solid<G4Tubs>("box"), n4::exceptions::bad_cast);
}


TEST_CASE("nain clear_geometry", "[nain][clear_geometry]") {
  default_run_manager().run();

  auto name = "vanish";
  auto air = nain4::material("G4_AIR");
  auto logical = nain4::volume<G4Box>(name, air, 1*cm, 1*cm, 1*cm);
  auto solid = logical -> GetSolid();
  auto physical = nain4::place(logical).now();
  auto verbose = false;
  {
    auto found_solid    = nain4::find_solid   (name, verbose);
    auto found_logical  = nain4::find_logical (name, verbose);
    auto found_physical = nain4::find_physical(name, verbose);
    CHECK(found_solid    != nullptr);
    CHECK(found_logical  != nullptr);
    CHECK(found_physical != nullptr);
    CHECK(found_solid    == solid);
    CHECK(found_logical  == logical);
    CHECK(found_physical == physical);
  }
  // Clear geometry and verify they are all gone
  nain4::clear_geometry();
  {
    auto found_solid    = nain4::find_solid   (name, verbose);
    auto found_logical  = nain4::find_logical (name, verbose);
    auto found_physical = nain4::find_physical(name, verbose);
    CHECK(found_solid    == nullptr);
    CHECK(found_logical  == nullptr);
    CHECK(found_physical == nullptr);
  }

}

TEST_CASE("nain geometry iterator", "[nain][geometry][iterator]") {
  default_run_manager().run();

  auto air = nain4::material("G4_AIR");

  auto l   = nain4::volume<G4Box>("l",   air, 100*m, 100*m, 100*m);
  auto l1  = nain4::volume<G4Box>("l1",  air,  40*m,  40*m,  40*m);
  auto l2  = nain4::volume<G4Box>("l2",  air,  10*m,  10*m,  10*m);
  auto l11 = nain4::volume<G4Box>("l11", air,  10*m,  10*m,  10*m);
  auto l21 = nain4::volume<G4Box>("l21", air,  10*m,  10*m,  10*m);
  auto l22 = nain4::volume<G4Box>("l22", air,  10*m,  10*m,  10*m);

  auto p   = nain4::place(l  )                           .now();
  auto p1  = nain4::place(l1 ).in(l) .at(-30*m, 0*m, 0*m).now();
  auto p2  = nain4::place(l2 ).in(l) .at( 30*m, 0*m, 0*m).now();
  auto p11 = nain4::place(l11).in(l1)                    .now();
  auto p21 = nain4::place(l21).in(l2).at(-20*m, 0*m, 0*m).now();
  auto p22 = nain4::place(l22).in(l2).at( 20*m, 0*m, 0*m).now();

  std::vector<G4VPhysicalVolume*> found{begin(p), end(p)};
  std::vector<G4VPhysicalVolume*> expected{p, p1, p2, p11, p21, p22};
  CHECK(found == expected);

}

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


TEST_CASE("nain envelope_of", "[nain][envelope_of]") {
  auto material_1 = n4::material("G4_Fe");
  auto material_2 = n4::material("G4_Au");
  auto a = n4::box("box")  .cube(1* m).volume(material_1);
  n4::         box("inner").cube(1*cm).place (material_2).in(a).now();
  auto c = n4::envelope_of(a);
  auto d = n4::envelope_of(a, "New-name");

  CHECK(a != c);
  CHECK(a -> GetName() + "-cloned" == c -> GetName()        );
  CHECK(a -> GetMaterial()         == c -> GetMaterial()    );
  CHECK(a -> GetSolid()            == c -> GetSolid()       );
  CHECK(a -> GetMass()             != c -> GetMass()        );
  CHECK(a -> GetNoDaughters()      != c -> GetNoDaughters() );

  CHECK(a != d);
  CHECK("New-name"                 == d -> GetName()        );
  CHECK(a -> GetMaterial()         == d -> GetMaterial()    );
  CHECK(a -> GetSolid()            == d -> GetSolid()       );
  CHECK(a -> GetMass()             != d -> GetMass()        );
  CHECK(a -> GetNoDaughters()      != d -> GetNoDaughters() );
}

// TODO can the overlap check tests be automated? G4 raises an exception when an
// overlap is detected, and we do not know how to observe that in Catch2

// TEST_CASE("nain overlap", "[nain][overlap]") {

//   auto material = n4::material("G4_AIR");
//   auto box   = n4::box("box").cube(1*m);
//   auto outer = n4::box("outer").cube(10*m).volume(material);
//   auto one   = box.place(material).name("one").at( 0.4*m,0,0);
//   auto two   = box.place(material).name("two").at(-0.4*m,0,0);

//   SECTION("checks off by default") {
//     one.in(outer).now();
//     two.in(outer).now();
//   }

//   SECTION("checks off explicitly globally") {
//     n4::place::check_overlaps_switch_off();
//     one.in(outer).now();
//     two.in(outer).now();
//   }

//   SECTION("checks on explicitly globally") {
//     n4::place::check_overlaps_switch_on();
//     one.in(outer).now();
//     two.in(outer).now();
//   }

//   SECTION("checks on explicitly locally") {
//     one.in(outer).now();
//     two.in(outer).check_overlaps().now();
//   }

//   SECTION("checks on locally override off globally") {
//     n4::place::check_overlaps_switch_off();
//     one.in(outer).now();
//     two.in(outer).check_overlaps().now();
//   }

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
