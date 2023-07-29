#ifndef N4_VOLUMES_HH
#define N4_VOLUMES_HH

#include "nain4.hh"

#include <G4LogicalVolume.hh>
#include <G4Material.hh>
#include <G4String.hh>
#include <G4SystemOfUnits.hh>
#include <G4Types.hh>
#include <G4Box.hh>
#include <G4Cons.hh>
#include <G4Trd.hh>
#include <G4VSolid.hh>
#include <G4Sphere.hh>
#include <G4Tubs.hh>

#include <G4VSensitiveDetector.hh>
#include <G4VSolid.hh>
#include <optional>

#define G4D G4double
#define G4SensDet G4VSensitiveDetector

namespace nain4 {

#define OPT_DOUBLE std::optional<G4double>

enum class BOOL_OP { ADD, SUB, INT };

struct boolean_shape;

// ---- Base class for interfaces to G4VSolids --------------------------------------------------------
struct shape {
  G4LogicalVolume*  volume(G4Material* material) const;
  n4::place          place(G4Material* material) const { return n4::place(volume(material)); }
  shape&              name(G4String    name    ) { name_ = name; return *this; }
  shape&         sensitive(G4SensDet*  s       ) { sd    = s   ; return *this; }
  virtual G4VSolid*  solid(                    ) const = 0;
  virtual ~shape() {}

  // boolean operations
  boolean_shape add      (n4::shape& shape);
  boolean_shape subtract (n4::shape& shape);
  boolean_shape intersect(n4::shape& shape);
  boolean_shape add      (G4VSolid*  solid);
  boolean_shape subtract (G4VSolid*  solid);
  boolean_shape intersect(G4VSolid*  solid);

  // Alternative names
  template<class S> boolean_shape join (S shape);
  template<class S> boolean_shape sub  (S shape);
  template<class S> boolean_shape inter(S shape);

protected:
  shape(G4String name) : name_{name} {}
  std::optional<G4VSensitiveDetector*> sd;
  G4String                             name_;
};

// ---- Interface for constructing G4 boolean solids --------------------------------------------------
struct boolean_shape : shape {
  friend shape;
  G4VSolid* solid() const override;

  boolean_shape& rotate_x(double delta         )  { auto rot = G4RotationMatrix{}; rot.rotateX(delta);             return rotate(rot);}
  boolean_shape& rotate_y(double delta         )  { auto rot = G4RotationMatrix{}; rot.rotateY(delta);             return rotate(rot);}
  boolean_shape& rotate_z(double delta         )  { auto rot = G4RotationMatrix{}; rot.rotateZ(delta);             return rotate(rot);}
  boolean_shape& rotate(G4RotationMatrix& rot)    { transformation = HepGeom::Rotate3D{rot}      * transformation; return *this; }
  boolean_shape& at(double x, double y, double z) { transformation = HepGeom::Translate3D{x,y,z} * transformation; return *this; }
  boolean_shape& at(G4ThreeVector    p)           { return at(p.x(), p.y(), p.z()); }
  boolean_shape& name(G4String name)              { name_ = name; return *this; }
private:
  boolean_shape(G4VSolid* a, G4VSolid* b, BOOL_OP op) : shape{a -> GetName()}, a{a}, b{b}, op{op}  {}
  G4VSolid* a;
  G4VSolid* b;
  BOOL_OP   op;
  G4Transform3D transformation = HepGeom::Transform3D::Identity;
};

template<class S> boolean_shape shape::join (S shape){ return add      (shape); }
template<class S> boolean_shape shape::sub  (S shape){ return subtract (shape); }
template<class S> boolean_shape shape::inter(S shape){ return intersect(shape); }

// ---- Macros for reuse of members and setters of orthogonal directions ------------------------------

#define COMMON(N4_TYPE, G4_TYPE)          \
public:                                   \
  N4_TYPE(G4String name) : shape{name} {} \
  G4_TYPE* solid() const;

#define HAS_R(TYPE, N)                                                         \
public:                                                                        \
  TYPE& r ## N ## _inner     (G4D x) { r ## N ## _inner_ = x; return *this; }; \
  TYPE& r ## N               (G4D x) { r ## N ## _outer_ = x; return *this; }; \
  TYPE& r ## N ## _delta     (G4D x) { r ## N ## _delta_ = x; return *this; }; \
private:                                                                       \
  OPT_DOUBLE r ## N ## _inner_;                                                \
  OPT_DOUBLE r ## N ## _delta_;                                                \
  OPT_DOUBLE r ## N ## _outer_;

#define HAS_PHI(TYPE, N)                                                           \
public:                                                                            \
  TYPE& phi ## N ## _start   (G4D x) { phi ## N ## _start_   = x; return *this; }; \
  TYPE& phi ## N ## _end     (G4D x) { phi ## N ## _end_     = x; return *this; }; \
  TYPE& phi ## N ## _delta   (G4D x) { phi ## N ## _delta_   = x; return *this; }; \
private:                                                                           \
  G4D        phi ## N ## _start_ = 0;                                              \
  OPT_DOUBLE phi ## N ## _end_;                                                    \
  OPT_DOUBLE phi ## N ## _delta_;                                                  \
  const static constexpr G4D phi ## N ## _full = 360 * deg;

#define HAS_THETA(TYPE, N)                                                         \
public:                                                                            \
  TYPE& theta ## N ## _start (G4D x) { theta ## N ## _start_ = x; return *this; }; \
  TYPE& theta ## N ## _end   (G4D x) { theta ## N ## _end_   = x; return *this; }; \
  TYPE& theta ## N ## _delta (G4D x) { theta ## N ## _delta_ = x; return *this; }; \
private:                                                                           \
  G4D   theta ## N ## _start_ = 0;                                                 \
  OPT_DOUBLE theta ## N ## _end_;                                                  \
  OPT_DOUBLE theta ## N ## _delta_;                                                \
  const static constexpr G4D theta ## N ## _full = 180 * deg;

#define HAS_X(TYPE, N)                                                 \
public:                                                                \
  TYPE&      x ## N(G4D l) { half_x ## N ## _ = l / 2; return *this; } \
  TYPE& half_x ## N(G4D l) { half_x ## N ## _ = l    ; return *this; } \
private:                                                               \
  OPT_DOUBLE half_x ## N ## _;

#define HAS_Y(TYPE, N)                                                 \
public:                                                                \
  TYPE&      y ## N(G4D l) { half_y ## N ## _ = l / 2; return *this; } \
  TYPE& half_y ## N(G4D l) { half_y ## N ## _ = l    ; return *this; } \
private:                                                               \
  OPT_DOUBLE half_y ## N ## _;

#define HAS_Z(TYPE, N)                                                  \
public:                                                                 \
  TYPE&      z ## N (G4D l) { half_z ## N ## _ = l / 2; return *this; } \
  TYPE& half_z ## N (G4D l) { half_z ## N ## _ = l    ; return *this; } \
private:                                                                \
  OPT_DOUBLE half_z ## N ## _;

#define HAS_XYZ(TYPE)                                                        \
  HAS_X(TYPE,)                                                               \
  HAS_Y(TYPE,)                                                               \
  HAS_Z(TYPE,)                                                               \
public:                                                                      \
  TYPE&      xyz(G4D x, G4D y, G4D z) { return this ->     x(x).y(y).z(z); } \
  TYPE& half_xyz(G4D x, G4D y, G4D z) { return this -> xyz(x*2, y*2, z*2); }

// ---- Interfaces for specific G4VSolids -------------------------------------------------------------
struct box : shape {
  COMMON(box, G4Box)
  HAS_XYZ(box)
public:
  box&      cube(G4double l) { return this ->      xyz(l,l,l); }
  box& half_cube(G4double l) { return this -> half_xyz(l,l,l); }
  box&      xy  (G4double l) {      x(l);      y(l); return *this;}
  box& half_xy  (G4double l) { half_x(l); half_y(l); return *this;}
  box&      xz  (G4double l) {      x(l);      z(l); return *this;}
  box& half_xz  (G4double l) { half_x(l); half_z(l); return *this;}
  box&      yz  (G4double l) {      y(l);      z(l); return *this;}
  box& half_yz  (G4double l) { half_y(l); half_z(l); return *this;}
};

struct sphere : shape {
  COMMON(sphere, G4VSolid)
  HAS_R    (sphere,)
  HAS_PHI  (sphere,)
  HAS_THETA(sphere,)
};

struct tubs : shape {
  COMMON(tubs, G4Tubs)
  HAS_R  (tubs,)
  HAS_PHI(tubs,)
  HAS_Z  (tubs,)
};

// TODO: give n4::cons a default value of 0 for r1 ???
struct cons : shape {
  COMMON(cons, G4Cons)
  HAS_R  (cons, 1)
  HAS_R  (cons, 2)
  HAS_Z  (cons,  )
  HAS_PHI(cons,  )
public:
  // G4 disallows the inner radius of a G4Cons to be less than this value.
  // 1e3 * G4GeometryTolerance::GetInstance() -> GetRadialTolerance(); which is one nm
  const static constexpr G4double eps = 1 * CLHEP::nm;
};

struct trd : shape {
  COMMON(trd, G4Trd)
  HAS_X (trd, 1)
  HAS_X (trd, 2)
  HAS_Y (trd, 1)
  HAS_Y (trd, 2)
  HAS_Z (trd,  )
public:
  trd&      xy1(G4D l) {      x1(l);      y1(l); return *this;}
  trd&      xy2(G4D l) {      x2(l);      y2(l); return *this;}
  trd& half_xy1(G4D l) { half_x1(l); half_y1(l); return *this;}
  trd& half_xy2(G4D l) { half_x2(l); half_y2(l); return *this;}
};


// ---- Ensure that local macros don't leak out -------------------------------------------------------
#undef G4D
#undef G4SensDet
#undef OPT_DOUBLE
#undef COMMON
#undef HAS_R
#undef HAS_PHI
#undef HAS_THETA
#undef HAS_X
#undef HAS_Y
#undef HAS_Z
#undef HAS_XYZ

}; // namespace nain4

namespace n4 { using namespace nain4; }

#endif // N4_VOLUMES_HH
