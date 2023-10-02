#include "geometry.hh"
#include "materials.hh"

#include <nain4.hh>
#include <n4-volumes.hh>
#include <g4-mandatory.hh>

#include <CLHEP/Units/PhysicalConstants.h>
#include <CLHEP/Units/SystemOfUnits.h>
#include <CLHEP/Vector/ThreeVector.h>
#include <FTFP_BERT.hh>
#include <G4EmStandardPhysics_option4.hh>
#include <G4HCofThisEvent.hh>
#include <G4LogicalBorderSurface.hh>
#include <G4LogicalVolume.hh>
#include <G4Material.hh>
#include <G4MaterialPropertiesTable.hh>
#include <G4OpticalPhysics.hh>
#include <G4OpticalSurface.hh>
#include <G4PVPlacement.hh>
#include <G4RandomDirection.hh>
#include <G4RunManagerFactory.hh>
#include <G4SubtractionSolid.hh>
#include <G4SystemOfUnits.hh>
#include <G4ThreeVector.hh>
#include <G4Tubs.hh>
#include <Randomize.hh>

using vec_double = std::vector<G4double>;
using vec_int    = std::vector<G4int>;

const G4double hc = CLHEP::h_Planck * CLHEP::c_light;

G4double detection_probability(G4double energy, vec_double& energies, vec_double& scintillation);
void place_csi_teflon_border_surface_between(G4PVPlacement* one, G4PVPlacement* two);
n4::sensitive_detector* sensitive_detector(G4int nb_detectors_per_side, std::vector<vec_double>& times_of_arrival);

G4PVPlacement* make_geometry(std::vector<std::vector<G4double>>& times_of_arrival, const config& config) {
    auto csi    =    csi_with_properties(config);
    auto air    =    air_with_properties();
    auto teflon = teflon_with_properties();
    auto plastic = n4::material("G4_POLYCARBONATE"); // probably wrong

    auto world = n4::box{"World"}.cube(100*mm).volume(air);

    auto source_ring = n4::tubs("SourceRing").r_inner(9.5*mm).r(12.5*mm).z(3*mm).volume(plastic);
    n4::place(source_ring).in(world).rot_y(90*deg).at({0, 0, 0}).now();

    // ---- Teflon-coated scintillators ----------------------------------------------------------------------
    auto scint_xy = 3*mm, scint_z = 20*mm, coating_thck = 0.25*mm,  scintillator_offset = 23*mm;

    auto coating_log  = n4::box("Coating"     ).xy(scint_xy + coating_thck*2).z(scint_z + coating_thck).volume(teflon);
    auto scintillator = n4::box("Scintillator").xy(scint_xy                 ).z(scint_z               ).place (csi)
        .in(coating_log).at(0, 0, coating_thck/2).now();
    // Make a reusable (place many times) logical volume, containing: scintillator, coating and border-surface-between-them
    auto coated_scint_log = n4::envelope_of(coating_log, "Coated-scintillator");
    auto coating = n4::place(coating_log).in(coated_scint_log).now();
    place_csi_teflon_border_surface_between(scintillator, coating);
    // Reuse coated scintillator with optical border, however many times you need
    n4::place(coated_scint_log).in(world).at(0, 0, scintillator_offset).rot_y(  0*deg).copy_no(0).now();
    n4::place(coated_scint_log).in(world).at(0, 0, scintillator_offset).rot_y(180*deg).copy_no(1).now();

    // ---- Detector geometry --------------------------------------------------------------------------------
    G4int nb_detectors_per_side = 3;
    G4double detector_width = scint_xy / nb_detectors_per_side; // assumes the detectors are square
    G4double detector_depth = detector_width; // this will make the detectors cubes
    auto detector = n4::box{"Detector"}.xy(detector_width).z(detector_depth)
        .sensitive(sensitive_detector(nb_detectors_per_side, times_of_arrival))
        .volume(air); // material doesn't matter: everything entering the detector is stopped immediately

    for (G4int side=0; side<2; side++) {
        for (G4int i=0; i<nb_detectors_per_side; i++) {
            for (G4int j=0; j<nb_detectors_per_side; j++) {
                G4double xpos = (i - ((float) nb_detectors_per_side/2 - 0.5)) * detector_width;
                G4double ypos = (j - ((float) nb_detectors_per_side/2 - 0.5)) * detector_width;
                G4double zpos;
                if (side == 0) { zpos =   scintillator_offset + coating_thck/2 + scint_z/2 + detector_depth/2;  }
                else           { zpos = -(scintillator_offset + coating_thck/2 + scint_z/2 + detector_depth/2); }
                n4::place(detector)
                    .in(world)
                    .at({xpos, ypos, zpos})
                    .copy_no(side*pow(nb_detectors_per_side, 2) + i*nb_detectors_per_side + j)
                    .now();
            }
        }
    }
    // -------------------------------------------------------------------------------------------------------
    return n4::place(world).now();
}

n4::sensitive_detector* sensitive_detector(G4int nb_detectors_per_side, std::vector<vec_double>& times_of_arrival) {
    auto process_hits = [nb_detectors_per_side, &times_of_arrival](G4Step* step) {
        G4Track* track = step -> GetTrack();
        track -> SetTrackStatus(fStopAndKill);

        auto pre             = step -> GetPreStepPoint();
        auto copy_nb         = pre  -> GetTouchable() -> GetCopyNumber();
        auto time            = pre  -> GetGlobalTime(); // This will be written to file: what are the units?
        auto photon_momentum = pre -> GetMomentum();
        auto photon_energy   = photon_momentum.mag();

        // Pixel pitch 25 um
        // auto sipm_energies = n4::scale_by(hc*eV, {1/0.9 , 1/0.7, 1/0.5  , 1/0.46 , 1/0.4 , 1/0.32});
        // std::vector<G4double> sipm_pdes =        {  0.03,   0.1,   0.245,   0.255,   0.23,   0.02};
        auto sipm_energies = n4::scale_by(hc*eV, {1/0.9 , 1/0.7, 1/0.5  , 1/0.46 , 1/0.4 , 1/0.36, 1/0.34, 1/0.3 , 1/0.28});
        std::vector<G4double> sipm_pdes =        {  0.03,   0.1,   0.245,   0.255,   0.23,   0.18,   0.18,   0.14,   0.02};


        if (G4UniformRand() < detection_probability(photon_energy, sipm_energies, sipm_pdes)) {
            if (copy_nb < pow(nb_detectors_per_side, 2)) { times_of_arrival[0].push_back(time); }
            else                                         { times_of_arrival[1].push_back(time); }
        }

        return true;
    };

    auto end_of_event = [](G4HCofThisEvent* what) {};
    return (new n4::sensitive_detector{"Detector", process_hits}) -> end_of_event(end_of_event);
}

void place_csi_teflon_border_surface_between(G4PVPlacement* one, G4PVPlacement* two) {
    static G4OpticalSurface* csi_teflon_surface = nullptr;
    auto name = "CsI-TeflonSurface";
    if (! csi_teflon_surface) {
        csi_teflon_surface = new G4OpticalSurface(name);
        // Values from same paper as above ("Optimization of Parameters...")
        // "groundfrontpainted" (I think) only considers whether the photon is reflected or absorbed, so there will be no shine through visible in the simulation
        csi_teflon_surface -> SetType(dielectric_dielectric);
        csi_teflon_surface -> SetModel(unified);
        csi_teflon_surface -> SetFinish(groundfrontpainted);
        csi_teflon_surface -> SetSigmaAlpha(0.0);

        vec_double pp = {2.038*eV, 4.144*eV};
        // According to the docs, for UNIFIED, dielectric_dielectric surfaces only the Lambertian reflection is turned on
        csi_teflon_surface -> SetMaterialPropertiesTable(
            n4::material_properties{}
            .add("REFLECTIVITY", pp, {1.0 , 1.0})
            .done());
    }
    new G4LogicalBorderSurface(name, one, two, csi_teflon_surface);
}

G4double detection_probability(G4double energy, vec_double& energies, vec_double& scintillation) {
    // Detection probablity = 0 if energy lies outside of range
    if (! (energies.front() <= energy && energy <= energies.back())) { return 0; }

    // Find index of first point above desired energy
    size_t index = 0;
    for (size_t i=1; i<energies.size(); i++) {
        if (energy < energies[i]) {
            index = i;
            break;
        }
    }
    G4double y0 = scintillation[index-1]; G4double y1 = scintillation[index];
    G4double x0 = energies     [index-1]; G4double x1 = energies     [index];
    return y0 + ((y1 - y0) / (x1 - x0)) * (energy - x0);
}
