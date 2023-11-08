#include "LXe.hh"

#include <n4-all.hh>

#include <G4SystemOfUnits.hh> // physical units such as `m` for metre



const G4double OPTPHOT_MIN_E = 1    * eV;
const G4double OPTPHOT_MAX_E = 8.21 * eV;
const G4double NO_ABSORPTION = 1e8  * m; // approx. infinity

G4double LXe_Scintillation(G4double energy) {
  using CLHEP::c_light;   using CLHEP::h_Planck;   using CLHEP::pi;
  // K. Fuji et al., "High accuracy measurement of the emission spectrum of liquid xenon
  // in the vacuum ultraviolet region",
  // Nuclear Instruments and Methods in Physics Research A 795 (2015) 293–297
  // http://ac.els-cdn.com/S016890021500724X/1-s2.0-S016890021500724X-main.pdf?_tid=83d56f0a-3aff-11e7-bf7d-00000aacb361&acdnat=1495025656_407067006589f99ae136ef18b8b35a04
  G4double lambda_peak  = 174.8 * nm;
  G4double lambda_FWHM  =  10.2 * nm;
  G4double lambda_sigma = lambda_FWHM / 2.35;

  G4double E_peak  = (h_Planck * c_light / lambda_peak);
  G4double E_sigma = (h_Planck * c_light * lambda_sigma / pow(lambda_peak, 2));

  G4double intensity = exp(-pow(E_peak / eV - energy / eV, 2) / (2 * pow(E_sigma / eV, 2)))
    / (E_sigma / eV * sqrt(pi * 2.));

  return intensity;
}

G4double LXe_refractive_index(G4double energy) {
  // Formula for the refractive index taken from
  // A. Baldini et al., "Liquid Xe scintillation calorimetry
  // and Xe optical properties", arXiv:physics/0401072v1 [physics.ins-det]

  // The Lorentz-Lorenz equation (also known as Clausius-Mossotti equation)
  // relates the refractive index of a fluid with its density:
  // (n^2 - 1) / (n^2 + 2) = - A · d_M,     (1)
  // where n is the refractive index, d_M is the molar density and
  // A is the first refractivity viral coefficient:
  // A(E) = \sum_i^3 P_i / (E^2 - E_i^2),   (2)
  // with:
  G4double P[3] = {71.23, 77.75, 1384.89}; // [eV^2 cm3 / mole]
  G4double E[3] = { 8.4 ,  8.81,   13.2 }; // [eV]

  // Note.- Equation (1) has, actually, a sign difference with respect
  // to the one appearing in the reference. Otherwise, it yields values
  // for the refractive index below 1.

  // Leave G4's system of units, to avoid loss of numerical precision
  energy /= eV;

  // Calculate the virial coefficient.
  G4double virial = 0;
  for (G4int i=0; i<3; i++)
    virial += P[i] / (energy*energy - E[i]*E[i]); // eV²*cm3/mol/eV² = cm3/mol

  G4double mol_density =  2.953 / 131.29; // (g/cm3)/g*mol = mol/cm3
  G4double alpha = virial * mol_density; // (cm3/mol)*mol/cm3 = 1

  // Isolating now the n2 from equation (1) and taking the square root
  G4double n2 = (1 - 2*alpha) / (1 + alpha);

  if (n2 < 1) {
    n2 = 1;
    //throw "up (Non-physical refractive index)";
  }
  return sqrt(n2);
}


G4MaterialPropertiesTable* LXe_optical_material_properties() {
  /// The time constants are taken from E. Hogenbirk et al 2018 JINST 13 P10031

  // Sampling from ~151 nm to 200 nm <----> from 6.20625 eV to 8.21 eV // TODO convert here
  auto [sc_energies, sc_values] = n4::interpolate(LXe_Scintillation   , 500, 6.20625*eV   , OPTPHOT_MAX_E);
  auto [ri_energies, ri_values] = n4::interpolate(LXe_refractive_index, 200, OPTPHOT_MIN_E, OPTPHOT_MAX_E);

  return n4::material_properties()
    .add("RINDEX", ri_energies, ri_values)
    .add("RESOLUTIONSCALE"   ,     1        )
    .NEW("YIELDRATIO"        ,     0.03     )
    .add("SCINTILLATIONYIELD", 58708   / MeV)
    .NEW("RAYLEIGH"          ,    36   * cm )
    .NEW("FASTTIMECONSTANT"  ,     2   * ns )
    .NEW("SLOWTIMECONSTANT"  ,    43.5 * ns )
    .NEW("ATTACHMENT"        ,  1000   * ms )
    .NEW("FASTCOMPONENT", sc_energies, sc_values)
    .NEW("SLOWCOMPONENT", sc_energies, sc_values)
    .add("ABSLENGTH", {OPTPHOT_MIN_E, OPTPHOT_MAX_E}, {NO_ABSORPTION, NO_ABSORPTION})
    .done();
}

G4Material* LXe_with_properties() {
  auto opts = n4::material_options {.state = kStateLiquid};
  auto LXe  = n4::material_from_elements_N("n4_lXe", 2.98*g/cm3, opts, {{"Xe", 1}});
  LXe -> SetMaterialPropertiesTable(LXe_optical_material_properties());
  return LXe;
}
