#include "n4-volumes.hh"

#include "nain4.hh"
#include <G4LogicalVolume.hh>
#include <G4Sphere.hh>
#include <G4String.hh>
#include <G4VGraphicsScene.hh>


G4double xxx(G4String name, const std::optional<G4double>& delta, const std::optional<G4double>& end, G4double start, G4double full) {
  if (delta.has_value() && end.has_value()) {
      name;
      // TODO error
  }
  if (!delta.has_value() && !end.has_value()) { return full; }
  return delta.value_or(end.value() - start);
  // Option 1
  // return delta.value_or(end.value_or(0.) - start);

  // Option 2
  return delta.has_value() ? delta.value() : end.value() - start;
}

namespace nain4 {

G4Sphere* sphere::solid() const {
  auto   phi_delta = xxx("phi"  ,   phi_delta_,   phi_end_,   phi_start_,   phi_full);
  auto theta_delta = xxx("theta", theta_delta_, theta_end_, theta_start_, theta_full);
  return new G4Sphere(name, r_inner_, r_, phi_start_, phi_delta, theta_start_, theta_delta);
}

G4LogicalVolume* sphere::logical(G4Material* material) const { return n4::logical(solid(), material); }

G4Box* box::solid() const { return new G4Box(name, half_x_, half_y_, half_z_); }

G4LogicalVolume* box::logical(G4Material* material) const {
  return volume<G4Box>(name, material, half_x_, half_y_, half_z_);
}

box& box::cube_size     (G4double l) { return this ->      x(l)     .y(l)     .z(l); }
box& box::cube_half_size(G4double l) { return this -> half_x(l).half_y(l).half_z(l); }

} // namespace nain4
