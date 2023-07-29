#include "nain4.hh"
#include "test_utils.hh"
#include "n4-volumes.hh"

// Solids
#include <CLHEP/Units/SystemOfUnits.h>
#include <G4Box.hh>
#include <G4Cons.hh>
#include <G4GeometryTolerance.hh>
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

#include <cmath>
#include <type_traits>
#include <cmath>

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

TEST_CASE("nain shape name", "[nain][shape][name]") {
  auto name1  = "fulanito";
  auto name2  = "menganito";
  auto box    = n4::box{name1}.cube(1*m); // proto-solid
  auto solid1 = box            .solid();  // real  solid
  auto solid2 = box.name(name2).solid();  // real  solid

  CHECK( solid1 -> GetName() == name1 );
  CHECK( solid2 -> GetName() == name2 );

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
  auto tubs_p = n4::tubs("tubs_p").r(r).z(z).place (water).at(xc, yc, zc).now();

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
  //  Meaningless case: auto r_s  = n4::tubs("r_s" ).r_inner(start) /*.end(180)*/ .solid();
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

  using CLHEP::pi;
  // V = (1/3) * π * h * (r² + r * R + R²)
  auto volume = pi * 1/3 * z * (r1*r1 + r1*r2 + r2*r2);

  // s = √((R - r)² + h²).  Lateral = π × (R + r) × s
  auto dr = (r2 - r1);
  auto slant_height = std::sqrt(dr*dr + z*z);
  auto area         = pi * (r1+r2) * slant_height
                    + pi * (r1*r1 + r2*r2);

  CHECK(cons_l -> TotalVolumeEntities() == 1);
  CHECK(cons_l -> GetMass() / kg        == Approx(volume * density / kg));
  CHECK(cons_l -> GetMaterial()         == water);
  CHECK(cons_l -> GetName()             == "cons_l");

  CHECK(cons_s -> GetCubicVolume() / m3 == Approx(volume / m3));
  CHECK(cons_s -> GetSurfaceArea() / m2 == Approx(area   / m2));

  CHECK(cons_p -> GetTranslation() . x() / m == xc / m);
  CHECK(cons_p -> GetTranslation() . y() / m == yc / m);
  CHECK(cons_p -> GetTranslation() . z() / m == zc / m);

  using CLHEP::twopi;
  auto start = twopi/8; auto end = twopi/2; auto delta = twopi/4;
  auto phi_s  = n4::cons("phi_s" ).r1(1).r2(1).z(1).phi_start(start) /*.end(360)*/   .solid();
  auto phi_se = n4::cons("phi_se").r1(1).r2(1).z(1).phi_start(start).phi_end  (end  ).solid();
  auto phi_sd = n4::cons("phi_sd").r1(1).r2(1).z(1).phi_start(start).phi_delta(delta).solid();
  auto phi_es = n4::cons("phi_es").r1(1).r2(1).z(1).phi_end  (end  ).phi_start(start).solid();
  auto phi_ds = n4::cons("phi_ds").r1(1).r2(1).z(1).phi_delta(delta).phi_start(start).solid();
  auto phi_e  = n4::cons("phi_e" ).r1(1).r2(1).z(1).phi_end  (end  ) /* .start(0) */ .solid();
  auto phi_d  = n4::cons("phi_d" ).r1(1).r2(1).z(1).phi_delta(delta) /* .start(0) */ .solid();

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

  auto z_full = n4::cons("z_full").r1(1).r2(1).     z(z  ).solid();
  auto z_half = n4::cons("z_half").r1(1).r2(1).half_z(z/2).solid();

  CHECK(z_full -> GetZHalfLength() == z_half -> GetZHalfLength());
  CHECK(z_full -> GetZHalfLength() == z/2);

  start = m/8; end = m/2; delta = m/4;
  // r1 = 0 would be meaningless

  auto cons1 = [] (auto name) { return n4::cons(name).r1_inner(0.1).r1(1); };
  auto cons2 = [] (auto name) { return n4::cons(name).r2_inner(0.1).r2(1); };

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

  auto check_r2 = [] (auto solid, auto inner, auto outer) {
      CHECK( solid -> GetInnerRadiusPlusZ() == inner);
      CHECK( solid -> GetOuterRadiusPlusZ() == outer);
  };
  auto check_r1 = [] (auto solid, auto inner, auto outer) {
      CHECK( solid -> GetInnerRadiusMinusZ() == inner);
      CHECK( solid -> GetOuterRadiusMinusZ() == outer);
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

  CHECK(trd_xy      -> GetXHalfLength1()   ==   trd_xy      -> GetYHalfLength1());
  CHECK(trd_xy      -> GetXHalfLength2()   ==   trd_xy      -> GetYHalfLength2());
  CHECK(trd_xy      -> GetXHalfLength1()   !=   trd_xy      -> GetZHalfLength ());
  CHECK(trd_xy      -> GetXHalfLength2()   !=   trd_xy      -> GetZHalfLength ());
  CHECK(trd_half_xy -> GetXHalfLength1()   ==   trd_half_xy -> GetYHalfLength1());
  CHECK(trd_half_xy -> GetXHalfLength2()   ==   trd_half_xy -> GetYHalfLength2());
  CHECK(trd_half_xy -> GetXHalfLength1()   !=   trd_half_xy -> GetZHalfLength ());

  auto dlx     = lx2 - lx1;
  auto dly     = ly2 - ly1;
  auto slx     = lx2 + lx1;
  auto sly     = ly2 + ly1;

  auto volume  = ( slx * sly + dlx * dly / 3 ) * lz / 4;
  auto surface = lx1 * ly1 + lx2 * ly2
               + sly * std::sqrt(lz*lz + dlx * dlx / 4)
               + slx * std::sqrt(lz*lz + dly * dly / 4);

  CHECK(trd_h -> GetCubicVolume() / m3 == trd_s -> GetCubicVolume() / m3);
  CHECK(trd_h -> GetSurfaceArea() / m2 == trd_s -> GetSurfaceArea() / m2);

  CHECK(trd_l -> TotalVolumeEntities() == 1);
  CHECK(trd_l -> GetMass() / kg        == Approx(volume * density / kg));
  CHECK(trd_l -> GetMaterial()         == water);
  CHECK(trd_l -> GetName()             == "trd_l");

  auto solid = trd_l -> GetSolid();
  CHECK(solid -> GetCubicVolume() / m3 == Approx(volume  / m3));
  CHECK(solid -> GetSurfaceArea() / m2 == Approx(surface / m2));
  CHECK(solid -> GetName()             == "trd_l");

  CHECK(trd_s -> GetCubicVolume() / m3 == solid -> GetCubicVolume() / m3);
  CHECK(trd_s -> GetSurfaceArea() / m2 == solid -> GetSurfaceArea() / m2);
  CHECK(trd_s -> GetName()             == "trd_s");

  CHECK(trd_p -> GetTranslation() . x() / m == xc / m);
  CHECK(trd_p -> GetTranslation() . y() / m == yc / m);
  CHECK(trd_p -> GetTranslation() . z() / m == zc / m);

  auto check_dimensions = [&] (auto trd) {
    CHECK(trd -> GetXHalfLength1() / m  == lxy1 / 2 / m);
    CHECK(trd -> GetYHalfLength1() / m  == lxy1 / 2 / m);
    CHECK(trd -> GetXHalfLength2() / m  == lxy2 / 2 / m);
    CHECK(trd -> GetYHalfLength2() / m  == lxy2 / 2 / m);
    CHECK(trd -> GetZHalfLength () / m  == lz   / 2 / m);
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

  SECTION("rotations") {
    auto water = nain4::material("G4_WATER");
    auto box   = nain4::volume<G4Box>("box", water, 0.3*m, 0.2*m, 0.1*m);

    auto sign      = GENERATE(-1, 1);
    auto angle_deg = GENERATE(0, 12.345, 30, 45, 60, 90, 180);
    auto angle     = sign * angle_deg * deg;

    auto   xrot = nain4::place(box).rotate_x(angle)                                .now();
    auto   yrot = nain4::place(box).rotate_y(angle)                                .now();
    auto   zrot = nain4::place(box).rotate_z(angle)                                .now();
    auto  xyrot = nain4::place(box).rotate_x(angle).rotate_y(angle)                .now();
    auto  xzrot = nain4::place(box).rotate_x(angle).rotate_z(angle)                .now();
    auto  yzrot = nain4::place(box).rotate_y(angle).rotate_z(angle)                .now();
    auto xyzrot = nain4::place(box).rotate_x(angle).rotate_y(angle).rotate_z(angle).now();

    auto rotmat  = [&] (auto x, auto y, auto z){
       auto rm = G4RotationMatrix();
       rm.rotateX(x);
       rm.rotateY(y);
       rm.rotateZ(z);
       return rm;
    };

    CHECK(*  xrot -> GetObjectRotation() == rotmat(angle,     0,     0));
    CHECK(*  yrot -> GetObjectRotation() == rotmat(    0, angle,     0));
    CHECK(*  zrot -> GetObjectRotation() == rotmat(    0,     0, angle));
    CHECK(* xyrot -> GetObjectRotation() == rotmat(angle, angle,     0));
    CHECK(* xzrot -> GetObjectRotation() == rotmat(angle,     0, angle));
    CHECK(* yzrot -> GetObjectRotation() == rotmat(    0, angle, angle));
    CHECK(*xyzrot -> GetObjectRotation() == rotmat(angle, angle, angle));
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

TEST_CASE("nain boolean rotation", "[nain][geometry][boolean][rotation]") {
  auto l1  = 3*m;
  auto l2  = 1*m;

  auto box_along_x = n4::box("along-x").xyz(l2, l1, l1);
  auto box_along_y = n4::box("along-y").xyz(l1, l2, l1);
  auto box_along_z = n4::box("along-z").xyz(l1, l1, l2);

  auto without_rot_xy = box_along_x.subtract(box_along_y).                 name("without_rot_xy").solid();
  auto without_rot_zx = box_along_z.subtract(box_along_x).                 name("without_rot_zx").solid();
  auto without_rot_yz = box_along_y.subtract(box_along_z).                 name("without_rot_yz").solid();
  auto with_z_rot     = box_along_x.subtract(box_along_y).rotate_z(90*deg).name("with_z_rot"    ).solid();
  auto with_y_rot     = box_along_z.subtract(box_along_x).rotate_y(90*deg).name("with_y_rot"    ).solid();
  auto with_x_rot     = box_along_y.subtract(box_along_z).rotate_x(90*deg).name("with_x_rot"    ).solid();

  // When rotated, the volumes overlap perfectly, resulting in a null volume
  // Cannot use GetCubicVolume because gives nonsense
  auto n   = 100000;
  auto eps = 1e-3;
  CHECK( without_rot_xy -> EstimateCubicVolume(n, eps) >  0 );
  CHECK( without_rot_zx -> EstimateCubicVolume(n, eps) >  0 );
  CHECK( without_rot_yz -> EstimateCubicVolume(n, eps) >  0 );
  CHECK( with_x_rot     -> EstimateCubicVolume(n, eps) == 0 );
  CHECK( with_y_rot     -> EstimateCubicVolume(n, eps) == 0 );
  CHECK( with_z_rot     -> EstimateCubicVolume(n, eps) == 0 );
}
