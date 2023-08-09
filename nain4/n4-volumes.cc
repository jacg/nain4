#include "n4-volumes.hh"

#include "nain4.hh"
#include <G4CSGSolid.hh>
#include <G4Cons.hh>
#include <G4LogicalVolume.hh>
#include <G4String.hh>
#include <G4UnionSolid.hh>
#include <G4SubtractionSolid.hh>
#include <G4IntersectionSolid.hh>
#include <G4VGraphicsScene.hh>
#include <G4VPVParameterisation.hh>
#include <G4Orb.hh>
#include <G4VSolid.hh>

#include <cstdlib>

using opt_double = std::optional<G4double>;

std::tuple<G4double, G4double> compute_angles( G4String name
                                             , const opt_double& start
                                             , const opt_double& delta
                                             , const opt_double& end
                                               , G4double full
                                             ) {
  if (start.has_value() && delta.has_value() && end.has_value()) {
    throw "You cannot provide all start, delta and end angles for the " + name + " coordinate.";
  }

  if (                     delta.has_value() && end.has_value()) { return {end.value() - delta.value(), delta.value()                       }; }
  if (                     delta.has_value()                   ) { return {start.value_or(0)          , delta.value()                       }; }
  if (start.has_value()                                        ) { return {start.value()              , end  .value_or(full) - start.value()}; }
  return {0, end.value_or(full)};
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

std::tuple<G4double, G4double> compute_r_range(opt_double min, opt_double max, opt_double del) {
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

boolean_shape shape::add      (n4::shape& shape) { return add      (shape.solid()); }
boolean_shape shape::subtract (n4::shape& shape) { return subtract (shape.solid()); }
boolean_shape shape::intersect(n4::shape& shape) { return intersect(shape.solid()); }

boolean_shape shape::add      (G4VSolid* solid) { return boolean_shape{this -> solid(), solid, BOOL_OP::ADD}; }
boolean_shape shape::subtract (G4VSolid* solid) { return boolean_shape{this -> solid(), solid, BOOL_OP::SUB}; }
boolean_shape shape::intersect(G4VSolid* solid) { return boolean_shape{this -> solid(), solid, BOOL_OP::INT}; }

G4VSolid* boolean_shape::solid() const {
  if (op == BOOL_OP::ADD) { return new G4UnionSolid       {name_, a, b, transformation}; }
  if (op == BOOL_OP::SUB) { return new G4SubtractionSolid {name_, a, b, transformation}; }
  if (op == BOOL_OP::INT) { return new G4IntersectionSolid{name_, a, b, transformation}; }
  // Unreachable
  return nullptr;
}

template<class... Args>
void check_mandatory_args(G4String type, G4String name, Args&&... args) {
  for(const auto arg : {args...}) {
    if (!arg.has_value()) {
      std::cerr << "Attempted to construct an instance of n4::"
                << type
                << " with name "
                << name
                << " without specifying all arguments."
                << std::endl;
      exit(EXIT_FAILURE);
    }
  }
}

G4Box* box::solid() const {
  check_mandatory_args("box", name_, half_x_, half_y_, half_z_);
  return new G4Box(name_, half_x_.value(), half_y_.value(), half_z_.value());
}

G4VSolid* sphere::solid() const {
  auto [r_inner, r_outer]         = compute_r_range(r_inner_, r_outer_, r_delta_);
  auto [  phi_start,   phi_delta] = compute_angles("phi"  ,   phi_start_,   phi_delta_,   phi_end_,   phi_full);
  auto [theta_start, theta_delta] = compute_angles("theta", theta_start_, theta_delta_, theta_end_, theta_full);

  if (r_inner == 0 && phi_delta == phi_full && theta_delta == theta_full) {
    return new G4Orb{name_, r_outer};
  }
  return new G4Sphere{name_, r_inner, r_outer, phi_start, phi_delta, theta_start, theta_delta};
}

G4Tubs* tubs::solid() const {
  check_mandatory_args("tubs", name_, half_z_);
  auto [r_inner, r_outer]     = compute_r_range(r_inner_, r_outer_, r_delta_);
  auto [phi_start, phi_delta] = compute_angles("phi", phi_start_, phi_delta_, phi_end_, phi_full);
  return new G4Tubs{name_, r_inner, r_outer, half_z_.value(), phi_start, phi_delta};
}

G4Cons* cons::solid() const {
  check_mandatory_args("cons", name_, half_z_);
  auto [r1_inner, r1_outer]   = compute_r_range(r1_inner_, r1_outer_, r1_delta_);
  auto [r2_inner, r2_outer]   = compute_r_range(r2_inner_, r2_outer_, r2_delta_);
  auto [phi_start, phi_delta] = compute_angles("phi", phi_start_, phi_delta_, phi_end_, phi_full);
  return new G4Cons{name_, r1_inner, r1_outer, r2_inner, r2_outer, half_z_.value(), phi_start, phi_delta};
}

G4Trd* trd::solid() const {
  check_mandatory_args("trd", name_, half_x1_, half_x2_, half_y1_, half_y2_, half_z_);
  return new G4Trd(name_, half_x1_.value(), half_x2_.value(), half_y1_.value(), half_y2_.value(), half_z_.value());
}

G4LogicalVolume* shape::volume(G4Material* material) const {
  auto vol = n4::volume(solid(), material);
  if (sd.has_value()) { vol -> SetSensitiveDetector(sd.value()); }
  return vol;
}


} // namespace nain4
