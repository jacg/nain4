#include "testing.hh"

#include <G4Box.hh>
#include <G4UnitsTable.hh>

#include <n4-place.hh>
#include <n4-volume.hh>

#include <catch2/generators/catch_generators.hpp>

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

// TEST_CASE("nain overlap", "[nain][overlap]") {
//
//   auto material = n4::material("G4_AIR");
//   auto box   = n4::box("box").cube(1*m);
//   auto outer = n4::box("outer").cube(10*m).volume(material);
//   auto one   = box.place(material).name("one").at( 0.4*m,0,0);
//   auto two   = box.place(material).name("two").at(-0.4*m,0,0);
//
//   SECTION("checks off by default") {
//     one.in(outer).now();
//     two.in(outer).now();
//   }
//
//   SECTION("checks off explicitly globally") {
//     n4::place::check_overlaps_switch_off();
//     one.in(outer).now();
//     two.in(outer).now();
//   }
//
//   SECTION("checks on explicitly globally") {
//     n4::place::check_overlaps_switch_on();
//     one.in(outer).now();
//     two.in(outer).now();
//   }
//
//   SECTION("checks on explicitly locally") {
//     one.in(outer).now();
//     two.in(outer).check_overlaps().now();
//   }
//
//   SECTION("checks on locally override off globally") {
//     n4::place::check_overlaps_switch_off();
//     one.in(outer).now();
//     two.in(outer).check_overlaps().now();
//   }
