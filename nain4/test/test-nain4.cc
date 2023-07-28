#include "nain4.hh"
#include "test_utils.hh"
#include "n4-volumes.hh"

// Solids
#include <CLHEP/Units/SystemOfUnits.h>
#include <G4Box.hh>
#include <G4Cons.hh>
#include <G4LogicalVolume.hh>
#include <G4Orb.hh>
#include <G4PVPlacement.hh>
#include <G4Sphere.hh>
#include <G4Trd.hh>

// Managers
#include <G4NistManager.hh>

// Units
#include <G4SystemOfUnits.hh>
#include <G4UnitsTable.hh>

// Other G4
#include <G4Material.hh>
#include <G4Gamma.hh>

#include <catch2/catch_approx.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <catch2/catch_test_macros.hpp>
#include <type_traits>

using Catch::Approx;

// Many of the tests below check physical quantities. Dividing physical
// quantities by their units gives raw numbers which are easily understandable
// by a human reader, which is important test failures are reported. Sometimes
// this gives rise to the apparently superfluous division by the same unit on
// both sides of an equation, in the source code.

#include <numeric>

TEST_CASE("nain material", "[nain][material]") {

  // nain4::material finds the same materials as the verbose G4 style
  SECTION("material NIST") {
    auto material_name = GENERATE("G4_AIR", "G4_WATER", "G4_H", "G4_A-150_TISSUE");
    auto nain_material = nain4::material(material_name);
    auto nist_material = G4NistManager::Instance()->FindOrBuildMaterial(material_name);
    REQUIRE(nain_material == nist_material);
    REQUIRE(nain_material != nullptr);
  }

  // Basic material properties make sense (except for solid water at RTP!)
  SECTION("material properties") {
    SECTION("water") {
      auto water = nain4::material("G4_WATER");
      CHECK(water->GetName()                  == "G4_WATER");
      CHECK(water->GetChemicalFormula()       == "H_2O");
      CHECK(water->GetTemperature() /  kelvin == Approx(293.15));
      CHECK(water->GetPressure() / atmosphere == Approx(1));
      CHECK(water->GetDensity() /     (kg/m3) == Approx(1000));
      CHECK(water->GetState()                 == G4State::kStateSolid); // WTF!?
    }
  }

  // Making and retrieving materials with nain4
  SECTION("material creation from N atoms") {

    // The values used to construct the material
    auto name = "n4test_FR4";
    auto density = 1.85 * g/cm3;
    auto state = kStateSolid;
    auto [nH, nC, nO] = std::make_tuple(12, 18, 3);

    // Make the material using nain4::material_from_elements
    auto fr4 = nain4::material_from_elements_N(name, density, state,
                                               {{"H", nH}, {"C", nC}, {"O", nO}});
    CHECK(fr4 != nullptr);

    // Verify that the material can be retrieved with nain4::material
    auto fr4_found = nain4::material(name);
    CHECK(fr4 == fr4_found);

    // Grab elements and calculate some properties for use in tests lower down
    auto H = nain4::element("H"); auto mH = H->GetAtomicMassAmu();
    auto C = nain4::element("C"); auto mC = C->GetAtomicMassAmu();
    auto O = nain4::element("O"); auto mO = O->GetAtomicMassAmu();
    auto total_mass = nH*mH + nC*mC + nO*mO;

    // Elements used correctly?
    CHECK(fr4 -> GetElement(0) == H);
    CHECK(fr4 -> GetElement(1) == C);
    CHECK(fr4 -> GetElement(2) == O);

    // Correct number of each element?
    auto atoms = fr4 -> GetAtomsVector();
    CHECK(atoms[0] == nH);
    CHECK(atoms[1] == nC);
    CHECK(atoms[2] == nO);

    // Basic properties set corretly?
    CHECK(fr4 -> GetNumberOfElements() == 3);
    CHECK(fr4 -> GetDensity() == density);
    CHECK(fr4 -> GetState() == state);

    // Fractional composition correct?
    auto fracs = fr4 -> GetFractionVector();
    CHECK(fracs[0] == Approx(nH*mH / total_mass));
    CHECK(fracs[1] == Approx(nC*mC / total_mass));
    CHECK(fracs[2] == Approx(nO*mO / total_mass));

    // Does fractional composition sum to 1?
    CHECK(std::accumulate(fracs, fracs + fr4->GetNumberOfElements(), 0.0) == Approx(1));
  }

  // Making and retrieving materials with nain4
  SECTION("material creation from mass fractions") {

    // The values used to construct the material
    auto name = "n4test_LYSO";
    auto density = 7.1 * g/cm3;
    auto state = kStateSolid;
    auto [fLu, fY, fSi, fO] = std::make_tuple(0.714, 0.040, 0.064, 0.182);

    // Make the material using nain4::material_from_elements
    auto lyso = nain4::material_from_elements_F(name, density, state,
                                                {{"Lu", fLu}, {"Y", fY}, {"Si", fSi}, {"O", fO}});
    CHECK(lyso != nullptr);

    // Verify that the material can be retrieved with nain4::material
    auto fr4_found = nain4::material(name);
    CHECK(lyso == fr4_found);

    // Grab elements and calculate some properties for use in tests lower down
    auto Lu = nain4::element("Lu");
    auto Y  = nain4::element("Y" );
    auto Si = nain4::element("Si");
    auto O  = nain4::element("O" );
    //auto total_mass = nH*mH + nC*mC + nO*mO;

    // Elements used correctly?
    CHECK(lyso -> GetElement(0) == Lu);
    CHECK(lyso -> GetElement(1) == Y );
    CHECK(lyso -> GetElement(2) == Si);
    CHECK(lyso -> GetElement(3) == O );

    // Atom counts produce nonsense when material built with mass fractions
    auto atoms = lyso -> GetAtomsVector();
    CHECK(atoms[0] == 1);
    CHECK(atoms[1] == 0);
    CHECK(atoms[2] == 0);
    CHECK(atoms[3] == 2);

    // Basic properties set corretly?
    CHECK(lyso -> GetNumberOfElements() == 4);
    CHECK(lyso -> GetDensity() == density);
    CHECK(lyso -> GetState() == state);

    // Fractional composition correct?
    auto fracs = lyso -> GetFractionVector();
    CHECK(fracs[0] == fLu);
    CHECK(fracs[1] == fY );
    CHECK(fracs[2] == fSi);
    CHECK(fracs[3] == fO );

    // Does fractional composition sum to 1?
    CHECK(std::accumulate(fracs, fracs + lyso->GetNumberOfElements(), 0.0) == Approx(1));

  }
}

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

  CHECK(box_h -> GetCubicVolume() / m3 == box_s -> GetCubicVolume() / m3);
  CHECK(box_h -> GetSurfaceArea() / m2 == box_s -> GetSurfaceArea() / m2);

  CHECK(box_l -> TotalVolumeEntities() == 1);
  CHECK(box_l -> GetMass() / kg        == Approx(lx * ly * lz * density / kg));
  CHECK(box_l -> GetMaterial()         == water);
  CHECK(box_l -> GetName()             == "box_l");

  auto solid = box_l -> GetSolid();
  CHECK(solid -> GetCubicVolume() / m3 == Approx(     lx    * ly    * lz     / m3));
  CHECK(solid -> GetSurfaceArea() / m2 == Approx(2 * (lx*ly + ly*lz + lz*lx) / m2));
  CHECK(solid -> GetName()             == "box_l");

  CHECK(box_s -> GetCubicVolume() / m3 == solid -> GetCubicVolume() / m3);
  CHECK(box_s -> GetSurfaceArea() / m2 == solid -> GetSurfaceArea() / m2);
  CHECK(box_s -> GetName()             == "box_s");

  CHECK(box_p -> GetTranslation() . x() / m == xc / m);
  CHECK(box_p -> GetTranslation() . y() / m == yc / m);
  CHECK(box_p -> GetTranslation() . z() / m == zc / m);

  auto small_cube = n4::box("small_cube").cube     (lz).solid();
  auto   big_cube = n4::box(  "big_cube").half_cube(lz).solid();
  CHECK(big_cube -> GetCubicVolume() / m3 == 8 * small_cube -> GetCubicVolume() / m3);
  CHECK(big_cube -> GetSurfaceArea() / m2 == 4 * small_cube -> GetSurfaceArea() / m2);
  CHECK(big_cube -> GetXHalfLength() / m  == 2 * small_cube -> GetYHalfLength() / m );
  CHECK(big_cube -> GetXHalfLength() / m  == 2 * small_cube -> GetZHalfLength() / m );

  auto hmm = [lx, ly, lz] (auto box) {
    CHECK(box -> GetXHalfLength() / m  == lx / 2 / m);
    CHECK(box -> GetYHalfLength() / m  == ly / 2 / m);
    CHECK(box -> GetZHalfLength() / m  == lz / 2 / m);
  };

  hmm(n4::box("box_xyz")     .     xyz(lx  , ly  , lz  ).solid());
  hmm(n4::box("box_half_xyz").half_xyz(lx/2, ly/2, lz/2).solid());
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

  using CLHEP::pi;
  CHECK(sphere_l -> TotalVolumeEntities() == 1);
  CHECK(sphere_l -> GetMass() / kg        == Approx(4 * pi / 3 * r * r * r * density / kg));
  CHECK(sphere_l -> GetMaterial()         == water);
  CHECK(sphere_l -> GetName()             == "sphere_l");

  CHECK(sphere_s -> GetCubicVolume() / m3 == Approx(4 * pi / 3 * r * r * r / m3));
  CHECK(sphere_s -> GetSurfaceArea() / m2 == Approx(4 * pi     * r * r     / m2));

  CHECK(sphere_p -> GetTranslation() . x() / m == xc / m);
  CHECK(sphere_p -> GetTranslation() . y() / m == yc / m);
  CHECK(sphere_p -> GetTranslation() . z() / m == zc / m);

  using CLHEP::twopi;
  auto start = twopi/8; auto end = twopi/2; auto delta = twopi/4;
  auto phi_s  = n4::sphere("phi_s" ).r(1).phi_start(start) /*.end(360)*/   .solid();
  auto phi_se = n4::sphere("phi_se").r(1).phi_start(start).phi_end  (end  ).solid();
  auto phi_sd = n4::sphere("phi_sd").r(1).phi_start(start).phi_delta(delta).solid();
  auto phi_es = n4::sphere("phi_es").r(1).phi_end  (end  ).phi_start(start).solid();
  auto phi_ds = n4::sphere("phi_ds").r(1).phi_delta(delta).phi_start(start).solid();
  auto phi_e  = n4::sphere("phi_e" ).r(1).phi_end  (end  ) /* .start(0) */ .solid();
  auto phi_d  = n4::sphere("phi_d" ).r(1).phi_delta(delta) /* .start(0) */ .solid();

  auto down = [] (auto g4vsolid) { return dynamic_cast<G4Sphere*>(g4vsolid); };

  auto check_phi = [&down] (auto solid, auto start, auto delta) {
      CHECK( down(solid) -> GetStartPhiAngle() == start);
      CHECK( down(solid) -> GetDeltaPhiAngle() == delta);
  };
  check_phi(phi_s , start, twopi - start );
  check_phi(phi_se, start,   end - start );
  check_phi(phi_sd, start, delta         );
  check_phi(phi_es, start,   end - start );
  check_phi(phi_ds, start, delta         );
  check_phi(phi_e ,     0,   end         );
  check_phi(phi_d ,     0, delta         );

  using CLHEP::pi;
  start = pi/8; end = pi/2; delta = pi/4;
  auto theta_s  = n4::sphere("theta_s" ).r(1).theta_start(start) /*.end(180)*/     .solid();
  auto theta_se = n4::sphere("theta_se").r(1).theta_start(start).theta_end  (end  ).solid();
  auto theta_sd = n4::sphere("theta_sd").r(1).theta_start(start).theta_delta(delta).solid();
  auto theta_es = n4::sphere("theta_es").r(1).theta_end  (end  ).theta_start(start).solid();
  auto theta_ds = n4::sphere("theta_ds").r(1).theta_delta(delta).theta_start(start).solid();
  auto theta_e  = n4::sphere("theta_e" ).r(1).theta_end  (end  ) /* .start(0) */   .solid();
  auto theta_d  = n4::sphere("theta_d" ).r(1).theta_delta(delta) /* .start(0) */   .solid();

  auto check_theta = [&down] (auto solid, auto start, auto delta) {
      CHECK( down(solid) -> GetStartThetaAngle() == start);
      CHECK( down(solid) -> GetDeltaThetaAngle() == delta);
  };

  check_theta(theta_s , start,    pi - start );
  check_theta(theta_se, start,   end - start );
  check_theta(theta_sd, start, delta         );
  check_theta(theta_es, start,   end - start );
  check_theta(theta_ds, start, delta         );
  check_theta(theta_e ,     0,   end         );
  check_theta(theta_d ,     0, delta         );

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
      CHECK( down(solid) -> GetInnerRadius() == inner);
      CHECK( down(solid) -> GetOuterRadius() == outer);
  };

  // check_r(r_s , start,    pi - start); // Shouldn't work
  check_r(r_se,       start,   end         );
  check_r(r_es,       start,   end         );

  check_r(r_sd,       start, start + delta );
  check_r(r_ds,       start, start + delta );

  check_r(r_ed, end - delta,   end         );
  check_r(r_de, end - delta,   end         );

  check_r(r_e ,           0,   end         );

  check_r(r_d ,           0, delta         );
}

TEST_CASE("nain sphere orb", "[nain][sphere][ord]") {
  using CLHEP::twopi;
  auto is_sphere = [](auto thing) { CHECK(dynamic_cast<G4Sphere*>(thing)); };
  auto is_orb    = [](auto thing) { CHECK(dynamic_cast<G4Orb   *>(thing)); };

  is_sphere(n4::sphere("r_inner"            ).r_inner(1).r      (2).solid());
  is_orb   (n4::sphere("r_inner_full"       ).r_inner(0).r      (2).solid());
  is_sphere(n4::sphere("r_delta"            ).r_delta(1).r      (2).solid());
  is_orb   (n4::sphere("r_delta_full"       ).r_delta(2).r      (2).solid());
  is_sphere(n4::sphere("r_delta_innner"     ).r_delta(2).r_inner(1).solid());
  is_orb   (n4::sphere("r_delta_innner_full").r_delta(2).r_inner(0).solid());

  auto full = twopi; auto half = twopi/2; auto more = full + half;
  is_sphere(n4::sphere("phi_start"           ).r(1).phi_start(half)                .solid());
  is_orb   (n4::sphere("phi_start_zero"      ).r(1).phi_start( 0.0)                .solid());
  is_sphere(n4::sphere("phi_start_end"       ).r(1).phi_start(half).phi_end  (full).solid());
  is_orb   (n4::sphere("phi_start_end_full"  ).r(1).phi_start(half).phi_end  (more).solid());
  is_sphere(n4::sphere("phi_delta"           ).r(1).phi_delta(half)                .solid());
  is_orb   (n4::sphere("phi_delta_full"      ).r(1).phi_delta(full)                .solid());
  is_sphere(n4::sphere("phi_delta_start"     ).r(1).phi_delta(half).phi_start(half).solid());
  is_orb   (n4::sphere("phi_delta_start_full").r(1).phi_delta(full).phi_start(half).solid());
  is_sphere(n4::sphere("phi_end"             ).r(1).phi_end  (half)                .solid());
  is_orb   (n4::sphere("phi_end_full"        ).r(1).phi_end  (full)                .solid());

  // TODO implement these (and others) if we ever allow providing angle_delta with angle_end
  // is_sphere(n4::sphere("phi_delta_end"       ).r(1).phi_delta(half).phi_end(full).solid());
  // is_orb   (n4::sphere("phi_delta_end"       ).r(1).phi_delta(full).phi_end(half).solid());

  full = twopi/2; half = twopi/4; more = full + half;
  is_sphere(n4::sphere("theta_start"           ).r(1).theta_start(half)                  .solid());
  is_orb   (n4::sphere("theta_start_zero"      ).r(1).theta_start( 0.0)                  .solid());
  is_sphere(n4::sphere("theta_start_end"       ).r(1).theta_start(half).theta_end  (full).solid());
  is_orb   (n4::sphere("theta_start_end_full"  ).r(1).theta_start(half).theta_end  (more).solid());
  is_sphere(n4::sphere("theta_delta"           ).r(1).theta_delta(half)                  .solid());
  is_orb   (n4::sphere("theta_delta_full"      ).r(1).theta_delta(full)                  .solid());
  is_sphere(n4::sphere("theta_delta_start"     ).r(1).theta_delta(half).theta_start(half).solid());
  is_orb   (n4::sphere("theta_delta_start_full").r(1).theta_delta(full).theta_start(half).solid());
  is_sphere(n4::sphere("theta_end"             ).r(1).theta_end  (half)                  .solid());
  is_orb   (n4::sphere("theta_end_full"        ).r(1).theta_end  (full)                  .solid());
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
  auto tubs_p = n4::tubs("tubs_p").r(r).z(z).place  (water).at(xc, yc, zc).now();

  using CLHEP::pi;
  CHECK(tubs_l -> TotalVolumeEntities() == 1);
  CHECK(tubs_l -> GetMass() / kg        == Approx(pi * r * r * z * density / kg));
  CHECK(tubs_l -> GetMaterial()         == water);
  CHECK(tubs_l -> GetName()             == "tubs_l");

  CHECK(tubs_s -> GetCubicVolume() / m3 == Approx(     pi * r * r * z               / m3));
  CHECK(tubs_s -> GetSurfaceArea() / m2 == Approx((2 * pi * r * z + 2 * pi * r * r) / m2));

  CHECK(tubs_p -> GetTranslation() . x() / m == xc / m);
  CHECK(tubs_p -> GetTranslation() . y() / m == yc / m);
  CHECK(tubs_p -> GetTranslation() . z() / m == zc / m);

  using CLHEP::twopi;
  auto start = twopi/8; auto end = twopi/2; auto delta = twopi/4;
  auto phi_s  = n4::tubs("phi_s" ).r(1).z(1).phi_start(start) /*.end(360)*/   .solid();
  auto phi_se = n4::tubs("phi_se").r(1).z(1).phi_start(start).phi_end  (end  ).solid();
  auto phi_sd = n4::tubs("phi_sd").r(1).z(1).phi_start(start).phi_delta(delta).solid();
  auto phi_es = n4::tubs("phi_es").r(1).z(1).phi_end  (end  ).phi_start(start).solid();
  auto phi_ds = n4::tubs("phi_ds").r(1).z(1).phi_delta(delta).phi_start(start).solid();
  auto phi_e  = n4::tubs("phi_e" ).r(1).z(1).phi_end  (end  ) /* .start(0) */ .solid();
  auto phi_d  = n4::tubs("phi_d" ).r(1).z(1).phi_delta(delta) /* .start(0) */ .solid();

  auto check_phi = [] (auto solid, auto start, auto delta) {
      CHECK( solid -> GetStartPhiAngle() == start);
      CHECK( solid -> GetDeltaPhiAngle() == delta);
  };
  check_phi(phi_s , start, twopi - start );
  check_phi(phi_se, start,   end - start );
  check_phi(phi_sd, start, delta         );
  check_phi(phi_es, start,   end - start );
  check_phi(phi_ds, start, delta         );
  check_phi(phi_e ,     0,   end         );
  check_phi(phi_d ,     0, delta         );

  auto z_full = n4::tubs("z_full").r(1).     z(z  ).solid();
  auto z_half = n4::tubs("z_half").r(1).half_z(z/2).solid();

  CHECK(z_full -> GetZHalfLength() == z_half -> GetZHalfLength());
  CHECK(z_full -> GetZHalfLength() == z/2);

  start = m/8; end = m/2; delta = m/4;
  //  auto r_s  = n4::tubs("r_s" ).r_inner(start) /*.end(180)*/     .solid(); // 1/8 - 8/8    7/8
  auto r_se = n4::tubs("r_se").r_inner(start).r      (end  ).solid();
  auto r_sd = n4::tubs("r_sd").r_inner(start).r_delta(delta).solid();
  auto r_ed = n4::tubs("r_ed").r      (end  ).r_delta(delta).solid();
  auto r_es = n4::tubs("r_es").r      (end  ).r_inner(start).solid();
  auto r_ds = n4::tubs("r_ds").r_delta(delta).r_inner(start).solid();
  auto r_de = n4::tubs("r_de").r_delta(delta).r      (end  ).solid();
  auto r_e  = n4::tubs("r_e" ).r      (end  )/*.r_inner(0)*/.solid();
  auto r_d  = n4::tubs("r_d" ).r_delta(delta)/*.r_inner(0)*/.solid();

  auto check_r = [] (auto solid, auto inner, auto outer) {
      CHECK( solid -> GetInnerRadius() == inner);
      CHECK( solid -> GetOuterRadius() == outer);
  };

  // check_r(r_s , start,    pi - start); // Shouldn't work
  check_r(r_se,       start,   end         );
  check_r(r_es,       start,   end         );

  check_r(r_sd,       start, start + delta );
  check_r(r_ds,       start, start + delta );

  check_r(r_ed, end - delta,   end         );
  check_r(r_de, end - delta,   end         );

  check_r(r_e ,           0,   end         );

  check_r(r_d ,           0, delta         );
}

TEST_CASE("nain volume", "[nain][volume]") {
  // nain4::volume produces objects with sensible sizes, masses, etc.
  auto water = nain4::material("G4_WATER"); auto density = water->GetDensity();
  auto lx = 1 * m;
  auto ly = 2 * m;
  auto lz = 3 * m;
  auto box = nain4::volume<G4Box>("test_box", water, lx, ly, lz);
  CHECK(box->TotalVolumeEntities() == 1);
  CHECK(box->GetMass() / kg        == Approx(8 * lx * ly * lz * density / kg));
  CHECK(box->GetMaterial()         == water);
  CHECK(box->GetName()             == "test_box");

  auto solid = box->GetSolid();
  CHECK(solid->GetCubicVolume() / m3 == Approx(8 *  lx    * ly    * lz     / m3));
  CHECK(solid->GetSurfaceArea() / m2 == Approx(8 * (lx*ly + ly*lz + lz*lx) / m2));
  CHECK(solid->GetName()             == "test_box");
}

TEST_CASE("nain place", "[nain][place]") {
  // nain4::place is a good replacement for G4PVPlacement
  auto air = nain4::material("G4_AIR");
  auto outer = nain4::volume<G4Box>("outer", air, 1*m, 2*m, 3*m);

  // Default values are sensible
  SECTION("defaults") {
    auto world = nain4::place(outer).now();

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
    auto world = nain4::place(outer)
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
    auto world = nain4::place(outer).at(4,5,6).now(); // 3-arg version of at()
    CHECK(world->GetObjectTranslation() == G4ThreeVector{4,5,6});
  }

  // The in() option creates correct mother/daughter relationship
  SECTION("in") {
    auto water = nain4::material("G4_WATER");
    auto inner = nain4::volume<G4Box>("inner", water, 0.3*m, 0.2*m, 0.1*m);

    auto inner_placed = nain4::place(inner)
      .in(outer)
      .at(0.1*m, 0.2*m, 0.3*m)
      .now();

    auto outer_placed = nain4::place(outer).now();

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
}

TEST_CASE("nain scale_by", "[nain][scale_by]") {
  CHECK(nain4::scale_by(eV, {1, 2.3, 4.5}) == std::vector<G4double>{1*eV, 2.3*eV, 4.5*eV});
  CHECK(nain4::scale_by(cm, {6, 7})        == std::vector<G4double>{6*cm, 7*cm});
}

TEST_CASE("nain vis_attributes", "[nain][vis_attributes]") {
  // Utility for more convenient configuration of G4VisAttributes
  // TODO could do with more extensive testing
  using nain4::vis_attributes;
  auto convenient = vis_attributes{}
    .visible(true)
    .colour({1,0,0})
    .start_time(1.23)
    .end_time(4.56)
    .force_line_segments_per_circle(20)
    .force_solid(true)
    .force_wireframe(false);
  auto pita = G4VisAttributes{};
  pita.SetVisibility(true);
  pita.SetColour({1,0,0});
  pita.SetStartTime(1.23);
  pita.SetEndTime(4.56);
  pita.SetForceLineSegmentsPerCircle(20);
  pita.SetForceSolid(true);
  pita.SetForceWireframe(false);
  CHECK(convenient == pita);

  // The meaning of the different constructors

  // Default constructor sets colour: white, visibility: true
  CHECK(vis_attributes{} == vis_attributes{}.colour({1,1,1}).visible(true));
  // Can set colour via constructor
  CHECK(vis_attributes         {{0,1,0}} ==
        vis_attributes{}.colour({0,1,0}));
  // Can set visibility via constructor
  CHECK(vis_attributes          {true} ==
        vis_attributes{}.visible(true));

  CHECK(vis_attributes          {false} ==
        vis_attributes{}.visible(false));
  // Can set both visibility and colour via constructor
  CHECK(vis_attributes          {false ,       {1,1,0}} ==
        vis_attributes{}.visible(false).colour({1,1,0}));
}

TEST_CASE("nain find geometry", "[nain][find][geometry]") {
  auto run_manager = default_run_manager();

  // Utilities for retrieving from stores
  SECTION("find_logical") {
    auto air       = nain4::material("G4_AIR");
    auto long_name = "made just for find_logical test";
    auto find_me   = nain4::volume<G4Box>(long_name, air, 1 * cm, 1 * cm, 1 * cm);
    auto found     = nain4::find_logical(long_name);
    CHECK(found == find_me);
    auto should_not_exist = nain4::find_logical("Hopefully this name hasn't been used anywhere", false);
    CHECK(should_not_exist == nullptr);

    SECTION("find_physical") {
      auto placed = nain4::place(find_me).now();
      auto found_placed = nain4::find_physical(long_name);
      CHECK(found_placed == placed);
      CHECK(found_placed != nullptr);
    }
  }
}

TEST_CASE("nain find particle", "[nain][find][particle]") {
  auto run_manager = default_run_manager();

  auto name = "gamma";
  auto pita = G4ParticleTable::GetParticleTable()->FindParticle(name);
  auto solid = G4Gamma::Definition();
  auto convenient = nain4::find_particle(name);
  CHECK(convenient == pita);
  CHECK(convenient == solid);
 }


TEST_CASE("nain clear_geometry", "[nain][clear_geometry]") {
  auto run_manager = default_run_manager();

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
  auto run_manager = default_run_manager();

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

void check_solid_volume_placed_equivalence(G4VSolid* solid, G4LogicalVolume* volume, G4PVPlacement* placed, double tol=0) {
  CHECK(volume -> GetMass()        / kg ==  Approx(placed -> GetLogicalVolume() -> GetMass()        / kg).margin(tol));
  CHECK(solid  -> GetCubicVolume() / m3 ==  Approx(volume -> GetSolid        () -> GetCubicVolume() / m3).margin(tol));
}

auto check_properties (n4::boolean_shape& shape, G4Material* mat, G4String name, double vol, double density) {
  auto solid  = shape.solid();
  auto volume = shape.volume(mat);
  auto placed = shape.place (mat).now();
  check_solid_volume_placed_equivalence(solid, volume, placed, 1e-4);

  CHECK(solid  -> GetCubicVolume() / m3 == Approx(vol           / m3));
  CHECK(volume -> GetMass() / kg        == Approx(vol * density / kg));
  CHECK(volume -> TotalVolumeEntities() == 1);
  CHECK(volume -> GetMaterial()         == mat);

  CHECK(solid  -> GetName()             == name);
  CHECK(volume -> GetName()             == name);
  CHECK(placed -> GetName()             == name);
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
