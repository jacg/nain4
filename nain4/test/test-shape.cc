#include "testing.hh"

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
