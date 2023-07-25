#include "n4-volumes.hh"

#include "nain4.hh"
#include <G4LogicalVolume.hh>
#include <G4Sphere.hh>
#include <G4String.hh>
#include <G4VGraphicsScene.hh>


G4double xxx(G4String name, std::optional<G4double> delta, std::optional<G4double> end, G4double start, G4double full) {
  if (delta.has_value() && end.has_value()) {
      name;
      // TODO error
  }
  return delta.value_or(end.value_or(full) - start);
}

static const G4String sphere_r_usage = "Usage:\n"
".r      (...)\n"
".r_inner(...).r      (...)\n"
".r_inner(...).r_delta(...)\n"
".r      (...).r_delta(...)\n"
".r_delta(...)\n";

void fail0(){ throw "You must provide some information about the sphere radius. \n" + sphere_r_usage; }
void fail3(){ throw "You may not provide more than two sphere radius parameters.\n" + sphere_r_usage; }
void faili(){ throw "You provided only inner radius, also need outer or delta.  \n" + sphere_r_usage; }

std::tuple<G4double, G4double> yyy(std::optional<G4double> min, std::optional<G4double> max, std::optional<G4double> del) {
  // Disallowed cases
  if ( min.has_value() &&  max.has_value() &&  del.has_value()) { fail3(); }
  if (!min.has_value() && !max.has_value() && !del.has_value()) { fail0(); }
  if ( min.has_value() && !max.has_value() && !del.has_value()) { faili(); }

  // Allowed cases
  if ( min.has_value() && max.has_value()) { return {min.value()              , max.value()               }; }
  if ( min.has_value() && del.has_value()) { return {min.value()              , min.value() + del.value() }; }
  if ( del.has_value() && max.has_value()) { return {max.value() - del.value(), max.value()               }; }
  if ( max.has_value()                   ) { return {                        0, max.value()               }; }
  if ( del.has_value()                   ) { return {                        0, del.value()               }; }

  // Unreachable
  return {0, 0};
}

namespace nain4 {

G4Sphere* sphere::solid() const {
  auto [r_inner, r_outer] = yyy(r_inner_, r_outer_, r_delta_);
  auto   phi_delta = xxx("phi"  ,   phi_delta_,   phi_end_,   phi_start_,   phi_full);
  auto theta_delta = xxx("theta", theta_delta_, theta_end_, theta_start_, theta_full);
  return new G4Sphere{name, r_inner, r_outer, phi_start_, phi_delta, theta_start_, theta_delta};
}

G4LogicalVolume* sphere::logical(G4Material* material) const { return n4::logical(solid(), material); }

G4Box* box::solid() const { return new G4Box(name, half_x_, half_y_, half_z_); }

G4LogicalVolume* box::logical(G4Material* material) const {
  return volume<G4Box>(name, material, half_x_, half_y_, half_z_);
}

box& box::cube_size     (G4double l) { return this ->      x(l)     .y(l)     .z(l); }
box& box::cube_half_size(G4double l) { return this -> half_x(l).half_y(l).half_z(l); }

} // namespace nain4
