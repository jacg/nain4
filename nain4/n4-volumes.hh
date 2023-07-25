#ifndef N4_VOLUMES_HH
#define N4_VOLUMES_HH

#include <G4LogicalVolume.hh>
#include <G4Material.hh>
#include <G4PVPlacement.hh>
#include <G4String.hh>
#include <G4SystemOfUnits.hh>
#include <G4Types.hh>
#include <G4VGraphicsScene.hh>

#include <optional>

namespace nain4 {

struct box {
  box(G4String name) : name{name} {}
  box&      x(G4double l) { half_x_ = l / 2; return *this; }
  box&      y(G4double l) { half_y_ = l / 2; return *this; }
  box&      z(G4double l) { half_z_ = l / 2; return *this; }
  box& half_x(G4double l) { half_x_ = l    ; return *this; }
  box& half_y(G4double l) { half_y_ = l    ; return *this; }
  box& half_z(G4double l) { half_z_ = l    ; return *this; }
  box& cube_size(G4double l);
  box& cube_half_size(G4double l);
  G4Box* solid() const;
  G4LogicalVolume* logical(G4Material* material) const;
private:
  G4String name;
  G4double half_x_;
  G4double half_y_;
  G4double half_z_;
};

struct sphere {
  sphere(G4String name) : name{name} {}
  sphere& r_inner     (G4double x) { r_inner_     = x; return *this; };
  sphere& r           (G4double x) { r_           = x; return *this; };
  sphere& phi_start   (G4double x) { phi_start_   = x; return *this; };
  sphere& phi_end     (G4double x) { phi_end_     = x; return *this; };
  sphere& phi_delta   (G4double x) { phi_delta_   = x; return *this; };
  sphere& theta_start (G4double x) { theta_start_ = x; return *this; };
  sphere& theta_end   (G4double x) { theta_end_   = x; return *this; };
  sphere& theta_delta (G4double x) { theta_delta_ = x; return *this; };
  G4Sphere* solid() const;
  G4LogicalVolume* logical(G4Material* material) const;
private:
  G4String name;
  G4double r_inner_                    = 0;
  G4double r_                          = 1 * m;
  G4double                phi_start_   = 0;
  std::optional<G4double> phi_end_;
  std::optional<G4double> phi_delta_;
  G4double                theta_start_ = 0;
  std::optional<G4double> theta_end_;
  std::optional<G4double> theta_delta_;
  const static constexpr G4double   phi_full = 360 * deg;
  const static constexpr G4double theta_full = 180 * deg;
};

}; // namespace nain4

namespace n4 { using namespace nain4; }

#endif // N4_VOLUMES_HH
