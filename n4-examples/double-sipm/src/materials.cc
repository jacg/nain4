#include "materials.hh"

#include <nain4.hh>

#include <G4SystemOfUnits.hh>

using vec_double = std::vector<G4double>;

// TODO: remove duplication of hc (defined in moth materials.cc and geometry.cc)
const G4double hc = CLHEP::h_Planck * CLHEP::c_light;
const vec_double OPTPHOT_ENERGY_RANGE{1*eV, 8.21*eV};

G4Material* csi_with_properties(const config& config) {
    auto csi = n4::material("G4_CESIUM_IODIDE");
    // csi_rindex: values taken from "Optimization of Parameters for a CsI(Tl) Scintillator Detector Using GEANT4-Based Monte Carlo..." by Mitra et al (mainly page 3)
    //  csi_scint: values from Fig. 2 in "A New Scintillation Material: Pure CsI with 10ns Decay Time" by Kubota et al (these are approximate...)
    // must be in increasing ENERGY order (decreasing wavelength) for scintillation to work properly
    auto      csi_energies = n4::scale_by(hc*eV, {1/0.55, 1/0.36, 1/0.3 , 1/0.26}); // denominator is wavelength in micrometres
    auto csi_energies_cold = n4::scale_by(hc*eV, {1/0.5 , 1/0.4 , 1/0.35, 1/0.27}); // denominator is wavelength in micrometres
    // auto     csi_energies = n4::scale_by(hc*eV, {1/0.9, 1/0.7, 1/0.54, 1/0.35});
    vec_double csi_rindex =                     {1.79  , 1.79  , 1.79 , 1.79  };  //vec_double csi_rindex = {2.2094, 1.7611};
    vec_double  csi_scint =                     {0.0   , 0.1   , 1.0  , 0.0   };
    auto    csi_abslength = n4::scale_by(m    , {5     , 5     , 5    , 5     });
    // Values from "Temperature dependence of pure CsI: scintillation light yield and decay time" by Amsler et al
    // "cold" refers to ~77K, i.e. liquid nitrogen temperature
    G4double csi_scint_yield      =  config.csi_scint_yield;
    G4double csi_scint_yield_cold = 50000 / MeV;
    G4double csi_time_fast        =     6 * ns;
    G4double csi_time_slow        =    28 * ns;
    G4double csi_time_fast_cold   =  1015 * ns; // only one component at cold temps!
    G4double csi_time_slow_cold   =  1015 * ns;
    G4MaterialPropertiesTable *csi_mpt = n4::material_properties()
        .add("RINDEX"                 , csi_energies, csi_rindex)
        .add("SCINTILLATIONCOMPONENT1", csi_energies,  csi_scint)
        .add("SCINTILLATIONCOMPONENT2", csi_energies,  csi_scint)
        .add("ABSLENGTH"              , csi_energies, csi_abslength)
        .add("SCINTILLATIONTIMECONSTANT1", csi_time_fast)
        .add("SCINTILLATIONTIMECONSTANT2", csi_time_slow)
        .add("SCINTILLATIONYIELD"        , csi_scint_yield)
        .add("SCINTILLATIONYIELD1"       ,     0.57   )
        .add("SCINTILLATIONYIELD2"       ,     0.43   )
        .add("RESOLUTIONSCALE"           ,     1.0    )
        .done();
    csi -> GetIonisation() -> SetBirksConstant(0.00152 * mm/MeV);
    csi -> SetMaterialPropertiesTable(csi_mpt);
    return csi;
}

G4Material* air_with_properties() {
    auto air = n4::material("G4_AIR");
    G4MaterialPropertiesTable *mpt_air = n4::material_properties()
        .add("RINDEX", OPTPHOT_ENERGY_RANGE, {1, 1})
        .done();
    air -> SetMaterialPropertiesTable(mpt_air);
    return air;
}

G4Material* teflon_with_properties() {
    auto teflon = n4::material("G4_TEFLON");
    // Values could be taken from "Optical properties of Teflon AF amorphous fluoropolymers" by Yang, French & Tokarsky (using AF2400, Fig.6)
    // but are also stated in the same paper as above
    G4MaterialPropertiesTable *mpt_teflon = n4::material_properties()
        .add("RINDEX", OPTPHOT_ENERGY_RANGE, {1.35, 1.35})
        .done();
    teflon -> SetMaterialPropertiesTable(mpt_teflon);
    return teflon;
}
