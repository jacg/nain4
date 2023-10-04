#pragma once

namespace nain4 {
namespace physical_const {
  // Planck's constant * speed of light
  // Useful to convert wavelength to energy
  // E = hc / wavelength
  extern const double hc;
}
}

namespace n4{ using namespace nain4;                 }
namespace c4{ using namespace nain4::physical_const; }
