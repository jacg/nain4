#include "n4-volumes.hh"

#include "nain4.hh"
#include <G4LogicalVolume.hh>
#include <G4String.hh>
#include <G4VGraphicsScene.hh>


using opt_double = std::optional<G4double>;

G4double compute_angle_delta(G4String name, const opt_double& delta, const opt_double& end, G4double start, G4double full) {
  if (delta.has_value() && end.has_value()) {
    throw "You cannot provide both end-angle and angle-delta for the " + name + " coordinate.";
  }
  return delta.value_or(end.value_or(full) - start);
}

static const G4String r_usage = "Usage:\n"
".r      (...)\n"
".r_inner(...).r      (...)\n"
".r_inner(...).r_delta(...)\n"
".r      (...).r_delta(...)\n"
".r_delta(...)\n";

void fail0(){ throw "You must provide some information about the radius. \n" + r_usage; }
void fail3(){ throw "You may not provide more than two radius parameters.\n" + r_usage; }
void faili(){ throw "You provided only inner radius, also need outer or delta.  \n" + r_usage; }

std::tuple<G4double, G4double> compùte_r_range(opt_double min, opt_double max, opt_double del) {
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
  auto [r_inner, r_outer] = compùte_r_range(r_inner_, r_outer_, r_delta_);
  auto   phi_delta = compute_angle_delta("phi"  ,   phi_delta_,   phi_end_,   phi_start_,   phi_full);
  auto theta_delta = compute_angle_delta("theta", theta_delta_, theta_end_, theta_start_, theta_full);
  return new G4Sphere{name, r_inner, r_outer, phi_start_, phi_delta, theta_start_, theta_delta};
}

 G4Tubs* tubs::solid() const {
  auto [r_inner, r_outer] = compùte_r_range(r_inner_, r_outer_, r_delta_);
  auto phi_delta = compute_angle_delta("phi", phi_delta_, phi_end_, phi_start_, phi_full);
  return new G4Tubs{name, r_inner, r_outer, half_z_, phi_start_, phi_delta};
}

G4Box* box::solid() const { return new G4Box(name, half_x_, half_y_, half_z_); }

box& box::cube_size     (G4double l) { return this ->      x(l)     .y(l)     .z(l); }
box& box::cube_half_size(G4double l) { return this -> half_x(l).half_y(l).half_z(l); }

} // namespace nain4
