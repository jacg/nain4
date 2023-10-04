// clang-format off
#pragma once

#include <n4-run-manager.hh>

#include <G4LogicalVolumeStore.hh>
#include <G4PhysicalVolumeStore.hh>
#include <G4SolidStore.hh>
#include <G4VisAttributes.hh>



// Disable GCC's shadow warnings
// This happens in constructors where it is a C++ idiom to have
// constructor parameters match memeber names.
// This has been reported to gcc's bug tracker
// https://gcc.gnu.org/bugzilla/show_bug.cgi?id=78147
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wshadow"

////////////////////////////////////////////////////////////////////////////////////////////////////
// Note 1
//
// + G4 forces you to use its G4Exception function, whose return type is void.
//
// + C++'s ternary operator treats throw expressions as a special case.
//
// + Hiding the throw expression inside G4Exception, disables this special
//   treatment, which results in a type mismatch between the void and whatever
//   values are present in the rest of the ternary operator
//
// + So we go through the following convolutions to satisfy the type system:
//
//   1. use throw at the top-level
//   2. use G4Exception in the argument to throw
//   3. but throw does not accept void
//   4. so use comma operator to give acceptable overall expression type (c-string)
//   5. but the actual value of the string doesn't matter, just its type.

#define FATAL(description) G4Exception((__FILE__ + (":" + std::to_string(__LINE__))).c_str(), "666", FatalException, description)

namespace nain4 {

// Here for now, as it admits volumes. Needs to find a better home if we ever overload it.
G4LogicalVolume* envelope_of(G4LogicalVolume* original);
G4LogicalVolume* envelope_of(G4LogicalVolume* original, G4String name);

// Remove all, logical/physical volumes, solids and assemblies.
inline void clear_geometry() { G4RunManager::GetRunManager() -> ReinitializeGeometry(true); }

// --------------------------------------------------------------------------------
// keyword-argument construction of G4VisAttributes
// TODO: G4VisAttributes does NOT have virtual destructor, do we get UB by subclassing?
class vis_attributes : public G4VisAttributes {
public:
  using G4VisAttributes::G4VisAttributes; // Inherit ALL constructors
  using MAP = std::map<G4String, G4AttDef>;
#define FORWARD(NEW_NAME, TYPE, OLD_NAME) vis_attributes& NEW_NAME(TYPE v){ this->OLD_NAME(v); return *this; }
  FORWARD(visible                       , bool     , SetVisibility)
  FORWARD(daughters_invisible           , bool     , SetDaughtersInvisible)
  FORWARD(colour                        , G4Colour , SetColour)
  FORWARD(color                         , G4Color  , SetColor)
  FORWARD(line_style                    , LineStyle, SetLineStyle)
  FORWARD(line_width                    , G4double , SetLineWidth)
  FORWARD(force_wireframe               , bool     , SetForceWireframe)
  FORWARD(force_solid                   , bool     , SetForceSolid)
  FORWARD(force_aux_edge_visible        , bool     , SetForceAuxEdgeVisible)
  FORWARD(force_line_segments_per_circle, G4int    , SetForceLineSegmentsPerCircle)
  FORWARD(start_time                    , G4double , SetStartTime)
  FORWARD(  end_time                    , G4double , SetEndTime)
  FORWARD(att_values, const std::vector<G4AttValue>*, SetAttValues)
  FORWARD(att_defs  , const                     MAP*, SetAttDefs)
#undef FORWARD
};

// --------------------------------------------------------------------------------
// stream redirection utilities

// redirect to arbitrary stream or buffer
class redirect {
public:
  redirect(std::ios& stream, std::streambuf* new_buffer);
  redirect(std::ios& stream, std::ios&       new_stream);
  ~redirect();
private:
  std::streambuf* original_buffer;
  std::ios&       stream;
};

// redirect to /dev/null
class silence {
public:
  explicit silence(std::ios& stream);
  ~silence();
private:
  std::streambuf* original_buffer;
  std::ios&       stream;
  std::ofstream   dev_null;
};

} // namespace nain4

namespace n4 { using namespace nain4; }

#pragma GCC diagnostic pop
