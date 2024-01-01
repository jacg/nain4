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

// TODO can the overlap check tests be automated? G4 raises an exception when an
// overlap is detected, and we do not know how to observe that in Catch2

// }


#pragma GCC diagnostic pop
