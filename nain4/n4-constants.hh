#ifndef N4_CONSTANTS_HH
#define N4_CONSTANTS_HH

#include <CLHEP/Units/PhysicalConstants.h>
#include <G4SystemOfUnits.hh>

namespace nain4 {
namespace physical_const {
  using namespace CLHEP;

  // Planck's constant * speed of light
  // Useful to convert wavelength to energy
  // E = hc / wavelength
  auto hc = h_Planck * c_light;
}
}

namespace n4{ using namespace nain4;                 }
namespace c4{ using namespace nain4::physical_const; }

#endif // N4_CONSTANTS_HH
