#include "testing.hh"

#include <n4-boolean-shape.hh>

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
