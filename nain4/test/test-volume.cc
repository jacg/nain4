#include "testing.hh"

#include <n4-volume.hh>

TEST_CASE("nain volume", "[nain][volume]") {
  // nain4::volume produces objects with sensible sizes, masses, etc.
  auto water = nain4::material("G4_WATER"); auto density = water->GetDensity();
  auto lx = 1 * m;
  auto ly = 2 * m;
  auto lz = 3 * m;
  auto box = n4::volume<G4Box>("test_box", water, lx, ly, lz);
  CHECK     (box->TotalVolumeEntities() == 1);
  CHECK     (box->GetMaterial()         == water);
  CHECK     (box->GetName()             == "test_box");
  CHECK_THAT(box->GetMass() / kg        , Within1ULP(8 * lx * ly * lz * density / kg));

  auto solid = box->GetSolid();
  CHECK     (solid->GetName()             == "test_box");
  CHECK_THAT(solid->GetCubicVolume() / m3 , Within1ULP(8 *  lx    * ly    * lz     / m3));
  CHECK_THAT(solid->GetSurfaceArea() / m2 , Within1ULP(8 * (lx*ly + ly*lz + lz*lx) / m2));
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
