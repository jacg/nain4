#include <n4-all.hh>
#include <catch2/catch_test_macros.hpp>

TEST_CASE("nain find sensitive", "[nain][find][sensitive]") {
  auto dummy = [] (const G4Step*) {return true;};
  auto material = n4::material("G4_AIR");

  auto sd1 = new n4::sensitive_detector{"my-sd", dummy};

  n4::box("box").cube(1).sensitive(sd1).place(material).now();

  auto found = n4::find_sensitive<n4::sensitive_detector>("my-sd");
  REQUIRE(found.has_value());
  CHECK  (found.value() == sd1);

  auto should_not_exist = n4::find_sensitive<n4::sensitive_detector>("MISSING-NAME-92zidf");
  CHECK(! should_not_exist.has_value());
}
