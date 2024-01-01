#include "testing.hh"

#include <G4Gamma.hh>

#include <n4-all.hh>

using namespace n4::test;

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
