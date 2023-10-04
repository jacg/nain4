#pragma once

#include <n4-shapes.hh>

namespace nain4 {

enum class BOOL_OP { ADD, SUB, INT };

// ---- Interface for constructing G4 boolean solids --------------------------------------------------
struct boolean_shape : shape {
  friend shape;
  G4VSolid* solid() const override;

  boolean_shape& rotate_x(double delta         )  { auto rot = G4RotationMatrix{}; rot.rotateX(delta);             return rotate(rot);}
  boolean_shape& rotate_y(double delta         )  { auto rot = G4RotationMatrix{}; rot.rotateY(delta);             return rotate(rot);}
  boolean_shape& rotate_z(double delta         )  { auto rot = G4RotationMatrix{}; rot.rotateZ(delta);             return rotate(rot);}
  boolean_shape& rotate  (G4RotationMatrix& r  )  { transformation = HepGeom::Rotate3D{r}        * transformation; return *this; }
  boolean_shape& rot     (G4RotationMatrix& r  )  { return rotate(r); }
  boolean_shape& rot_x   (double delta         )  { return rotate_x(delta); }
  boolean_shape& rot_y   (double delta         )  { return rotate_y(delta); }
  boolean_shape& rot_z   (double delta         )  { return rotate_z(delta); }
  boolean_shape& at  (double x, double y, double z) { transformation = HepGeom::Translate3D{x,y,z} * transformation; return *this; }
  boolean_shape& at_x(double x                    ) { return at(x, 0, 0); }
  boolean_shape& at_y(          double y          ) { return at(0, y, 0); }
  boolean_shape& at_z(                    double z) { return at(0, 0, z); }
  boolean_shape& at(G4ThreeVector    p)           { return at(p.x(), p.y(), p.z()); }
  boolean_shape& name(G4String name)              { name_ = name; return *this; }
private:
  boolean_shape(G4VSolid* a, G4VSolid* b, BOOL_OP op) : shape{a -> GetName()}, a{a}, b{b}, op{op}  {}
  G4VSolid* a;
  G4VSolid* b;
  BOOL_OP   op;
  G4Transform3D transformation = HepGeom::Transform3D::Identity;
};

// Clear and prominent error message
#define CLEAR_ERROR_MSG(METHOD, TYPE_DESCRIPTION)                       \
"\n\n\n\n"                                                              \
"[n4::boolean_shape::" METHOD "]\n"                                     \
"Attempted to create a boolean shape using " TYPE_DESCRIPTION ".\n"     \
"Only n4::shape and G4VSolid* are accepted.\n"                          \
"For more details, please check\n"                                      \
"https://jacg.github.io/nain4/explanation/boolean_solid_input_types.md" \
"\n\n\n\n"

#define SUBCLASS_OF(SUPER, SUB)  std::is_base_of_v<SUPER, std::remove_pointer_t<SUB>>

// Rejects specific types
#define REJECT_KNOWN_TYPE(SUPER, SUB, METHOD)                                     \
  static_assert( !SUBCLASS_OF(SUPER, SUB), CLEAR_ERROR_MSG(METHOD, "a " #SUPER));

// Rejects any other type. The condition here is unnecessary, but it
// describes better what we want to achieve
#define REJECT_UNKNOWN_TYPE(SUB, METHOD)                                    \
  static_assert( SUBCLASS_OF(n4::shape, SUB) || SUBCLASS_OF(G4VSolid, SUB)  \
               , CLEAR_ERROR_MSG(METHOD, "an invalid type"));

// A catch-all template so we give a better error for any type that is
// not appropriate. The template resolution order takes care of
// finding the valid input types first
#define CHECK_INVALID(METHOD)                                                                 \
template<class SUBTYPE> boolean_shape shape::METHOD(SUBTYPE x) {                              \
  /**/ if constexpr (SUBCLASS_OF(n4::shape, SUBTYPE)) { return shape::METHOD##_(x.solid()); } \
  else if constexpr (SUBCLASS_OF(G4VSolid , SUBTYPE)) { return shape::METHOD##_(x        ); } \
  else {                                                                                      \
    REJECT_KNOWN_TYPE(G4LogicalVolume, SUBTYPE, #METHOD);                                     \
    REJECT_KNOWN_TYPE(G4PVPlacement  , SUBTYPE, #METHOD);                                     \
    REJECT_KNOWN_TYPE(n4::place      , SUBTYPE, #METHOD);                                     \
    if constexpr ( !SUBCLASS_OF(G4LogicalVolume, SUBTYPE) &&                                  \
                   !SUBCLASS_OF(G4PVPlacement  , SUBTYPE) &&                                  \
                   !SUBCLASS_OF(n4::place      , SUBTYPE) ) {                                 \
      REJECT_UNKNOWN_TYPE(             SUBTYPE, #METHOD);                                     \
    }                                                                                         \
  }                                                                                           \
}

CHECK_INVALID(add)
CHECK_INVALID(sub)
CHECK_INVALID(inter)
CHECK_INVALID(join)
CHECK_INVALID(subtract)
CHECK_INVALID(intersect)

#undef CHECK_INVALID
#undef REJECT_UNKNOWN_TYPE
#undef REJECT_KNOWN_TYPE
#undef SUBCLASS_OF
#undef CLEAR_ERROR_MSG

} // namespace nain4

namespace n4 { using namespace nain4; }
