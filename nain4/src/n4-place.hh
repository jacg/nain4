#pragma once

#include <G4PVPlacement.hh>

#include <optional>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wshadow"

namespace nain4 {

class place {
public:
  place(G4LogicalVolume* child)  : child(child ? std::make_optional(child) : std::nullopt) {}
  place(place const&) = default;

  place& trans    (G4Transform3D& transform_){ return transform(transform_);                                    }
  place& transform(G4Transform3D& transform_){ transformation =      transform_ * transformation; return *this; }

  place& rotate  (G4RotationMatrix& r)  { transformation = HepGeom::Rotate3D{r} * transformation; return *this; }
  place& rotate_x(double delta       )  { auto rot = G4RotationMatrix{}; rot.rotateX(delta); return rotate(rot);}
  place& rotate_y(double delta       )  { auto rot = G4RotationMatrix{}; rot.rotateY(delta); return rotate(rot);}
  place& rotate_z(double delta       )  { auto rot = G4RotationMatrix{}; rot.rotateZ(delta); return rotate(rot);}

  place& rot     (G4RotationMatrix& r)  { return rotate      (r); }
  place& rot_x   (double delta       )  { return rotate_x(delta); }
  place& rot_y   (double delta       )  { return rotate_y(delta); }
  place& rot_z   (double delta       )  { return rotate_z(delta); }

  place& at  (double x, double y, double z) { transformation = HepGeom::Translate3D{x,y,z} * transformation; return *this; }
  place& at  (G4ThreeVector p             ) { return at(p.x(), p.y(), p.z()); }
  place& at_x(double x                    ) { return at(  x  ,   0  ,   0  ); }
  place& at_y(          double y          ) { return at(  0  ,   y  ,   0  ); }
  place& at_z(                    double z) { return at(  0  ,   0  ,   z  ); }

  place& copy_no(int         n)           { copy_number = n      ; return *this; }
  place& in(G4LogicalVolume* parent_)     { parent      = parent_; return *this; }
  place& in(G4PVPlacement*   parent_)     { return in(parent_ -> GetLogicalVolume()); }
  place& in(place&           parent_)     { return in(parent_ .  get_logical()     ); }
  place& name(G4String       label_)      { label       = label_ ; return *this; }

  place&      check_overlaps           () {  local_check_overlaps_ = true ; return *this; }
  void static check_overlaps_switch_on () { global_check_overlaps_ = true ; }
  void static check_overlaps_switch_off() { global_check_overlaps_ = false; }

  place  clone() const                                           { return *this; }
  G4PVPlacement* operator()()                                    { return now(); }
  G4PVPlacement* now();

  G4LogicalVolume* get_logical();

private:
  std::optional<G4LogicalVolume*>  child;
  std::optional<G4LogicalVolume*>  parent;
  std::optional<G4String>          label;
  std::optional<int>               copy_number;
  G4Transform3D                    transformation = HepGeom::Transform3D::Identity;
  bool         local_check_overlaps_ = false;
  static bool global_check_overlaps_;
};

} // namespace nain4

namespace n4 { using namespace nain4; }

#pragma GCC diagnostic pop
