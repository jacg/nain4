#ifndef N4_VOLUMES_HH
#define N4_VOLUMES_HH

#include <G4LogicalVolume.hh>
#include <G4Material.hh>
#include <G4PVPlacement.hh>
#include <G4String.hh>
#include <G4SystemOfUnits.hh>
#include <G4Types.hh>
#include <G4VGraphicsScene.hh>

namespace nain4 {

struct sphere {
  sphere(G4String name) : name{name} {}
  sphere& r_min       (G4double x) { r_min_       = x; return *this; };
  sphere& r_max       (G4double x) { r_max_       = x; return *this; };
  sphere& phi_start   (G4double x) { phi_start_   = x; return *this; };
  sphere& phi_delta   (G4double x) { phi_delta_   = x; return *this; };
  sphere& theta_start (G4double x) { theta_start_ = x; return *this; };
  sphere& theta_delta (G4double x) { theta_delta_ = x; return *this; };
  G4Sphere* solid() const;
  G4LogicalVolume* logical(G4Material* material) const;
private:
  G4String name;
  G4double r_min_       = 0;
  G4double r_max_       = 0;
  G4double phi_start_   = 0;
  G4double phi_delta_   = 360 * deg;
  G4double theta_start_ = 0;
  G4double theta_delta_ = 360 * deg;
};

}; // namespace nain4

namespace n4 { using namespace nain4; }

#endif // N4_VOLUMES_HH
