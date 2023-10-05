#include "materials.hh"

#include <n4-constants.hh>
#include <n4-material.hh>
#include <n4-sequences.hh>


#include <G4Material.hh>

#include <G4SystemOfUnits.hh>

const std::vector<G4double> OPTPHOT_ENERGY_RANGE{1*eV, 8.21*eV};

G4Material* csi_with_properties(const config& config) {
    auto csi = n4::material("G4_CESIUM_IODIDE");

    // csi_rindex: values taken from "Optimization of Parameters for a CsI(Tl) Scintillator Detector Using GEANT4-Based Monte Carlo..." by Mitra et al (mainly page 3)
    // csi_scint : values from Fig. 2 in "A New Scintillation Material: Pure CsI with 10ns Decay Time" by Kubota et al (these are approximate...)
    // must be in increasing ENERGY order (decreasing wavelength) for scintillation to work properly
    auto csi_energies      = n4::const_over(c4::hc/nm, {550 , 360, 300, 260}); // wavelengths in nanometers
    auto csi_energies_cold = n4::const_over(c4::hc/nm, {500 , 400, 350, 270}); // wavelengths in nanometers
    auto csi_scint         = std::vector<G4double>     {0.0 , 0.1, 1.0, 0.0};
    auto csi_rindex        =                            1.79;
    auto csi_abslength     =                            5*m ;

    // Values from "Temperature dependence of pure CsI: scintillation
    // light yield and decay time" by Amsler et al "cold" refers to
    // ~77K, i.e. liquid nitrogen temperature
    auto csi_scint_yield = config.cold ? 50000 / MeV : config.csi_scint_yield;
    auto csi_time_fast   = config.cold ? 1015 * ns   :  6 * ns;
    auto csi_time_slow   = config.cold ? 1015 * ns   : 28 * ns; // only one component at cold temps!
    G4MaterialPropertiesTable *csi_mpt = n4::material_properties()
      .add("RINDEX"                    , csi_energies   , csi_rindex     )
      .add("SCINTILLATIONCOMPONENT1"   , csi_energies   , csi_scint      )
      .add("SCINTILLATIONCOMPONENT2"   , csi_energies   , csi_scint      )
      .add("ABSLENGTH"                 , csi_energies   , csi_abslength  )
      .add("SCINTILLATIONTIMECONSTANT1",                  csi_time_fast  )
      .add("SCINTILLATIONTIMECONSTANT2",                  csi_time_slow  )
      .add("SCINTILLATIONYIELD"        ,                  csi_scint_yield)
      .add("SCINTILLATIONYIELD1"       ,                      0.57       )
      .add("SCINTILLATIONYIELD2"       ,                      0.43       )
      .add("RESOLUTIONSCALE"           ,                      1.0        )
      .done();

    csi -> GetIonisation() -> SetBirksConstant(0.00152 * mm/MeV);
    csi -> SetMaterialPropertiesTable(csi_mpt);
    return csi;
}

G4Material* air_with_properties() {
  static G4Material* air = nullptr;
  if (!air) {
    air = n4::material("G4_AIR");
    auto mpt = n4::material_properties()
      .add("RINDEX", OPTPHOT_ENERGY_RANGE, 1)
      .done();
    air -> SetMaterialPropertiesTable(mpt);
  }
  return air;
}

G4Material* teflon_with_properties() {
  static G4Material* teflon = nullptr;
  if (!teflon) {
    teflon = n4::material("G4_TEFLON");
    // Values could be taken from "Optical properties of Teflon AF
    // amorphous fluoropolymers" by Yang, French & Tokarsky (using
    // AF2400, Fig.6) but are also stated in the same paper as above
    auto mpt = n4::material_properties()
      .add("RINDEX", OPTPHOT_ENERGY_RANGE, 1.35)
      .done();
    teflon -> SetMaterialPropertiesTable(mpt);
  }
  return teflon;
}
