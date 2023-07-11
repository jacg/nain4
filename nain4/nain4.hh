// clang-format off

#ifndef nain4_hh
#define nain4_hh

#include <CLHEP/Geometry/Transform3D.h>
#include <G4LogicalVolume.hh>
#include <G4LogicalVolumeStore.hh>
#include <G4Material.hh>
#include <G4NistManager.hh>
#include <G4PVPlacement.hh>
#include <G4ParticleDefinition.hh>
#include <G4ParticleTable.hh>
#include <G4PhysicalVolumeStore.hh>
#include <G4RotationMatrix.hh>
#include <G4Run.hh>
#include <G4RunManager.hh>
#include <G4String.hh>
#include <G4SolidStore.hh>
#include <G4SDManager.hh>
#include <G4ThreeVector.hh>
#include <G4Transform3D.hh>
#include <G4VPhysicalVolume.hh>
#include <G4VSensitiveDetector.hh>
#include <G4VUserActionInitialization.hh>
#include <G4VUserDetectorConstruction.hh>
#include <G4VisAttributes.hh>

#include <string>
#include <utility>
#include <vector>
#include <tuple>
#include <optional>


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

using std::make_optional;
using std::nullopt;
using std::optional;

// Create logical volume from solid and material
template<class SOLID, class NAME, class... ArgTypes>
G4LogicalVolume* volume(NAME name, G4Material* material, ArgTypes&&... args) {
  auto solid = new SOLID{std::forward<NAME>(name), std::forward<ArgTypes>(args)...};
  return new G4LogicalVolume{solid, material, solid->GetName()};
}

// --------------------------------------------------------------------------------
// Utilies for concisely retrieving things from stores
// clang-format off
#define NAME     (G4String const& name)
#define NAME_VRB (G4String const& name, G4bool verbose=true)
#define IA inline auto
IA material      NAME     { return G4NistManager        ::   Instance()->FindOrBuildMaterial(name         ); }
IA element       NAME     { return G4NistManager        ::   Instance()->FindOrBuildElement (name         ); }
IA find_logical  NAME_VRB { return G4LogicalVolumeStore ::GetInstance()->GetVolume          (name, verbose); }
IA find_physical NAME_VRB { return G4PhysicalVolumeStore::GetInstance()->GetVolume          (name, verbose); }
IA find_solid    NAME_VRB { return G4SolidStore         ::GetInstance()->GetSolid           (name, verbose); }
IA find_particle NAME     { return G4ParticleTable:: GetParticleTable()->FindParticle       (name         ); }

IA event_number  ()       { return G4RunManager::GetRunManager()->GetCurrentRun()->GetNumberOfEvent(); }
#undef IA
#undef NAME
#undef NAME_VRB

// Remove all, logical/physical volumes, solids and assemblies.
inline void clear_geometry() { G4RunManager::GetRunManager() -> ReinitializeGeometry(true); }


// --------------------------------------------------------------------------------
template<class SENSITIVE>
auto fully_activate_sensitive_detector(SENSITIVE* detector) {
  detector -> Activate(true);
  G4SDManager::GetSDMpointer() -> AddNewDetector(detector);
  return detector;
}

// --------------------------------------------------------------------------------
// TODO make a builder for this (if more methods are added?)
// TODO: needs tests
// TODO: belongs with the actions in g4-mandatory
class sensitive_detector : public G4VSensitiveDetector {
public:
  using process_hits_fn = std::function<bool(G4Step*)>;
  using end_of_event_fn = std::function<void(G4HCofThisEvent*)>;

  sensitive_detector(G4String name, process_hits_fn process_hits, end_of_event_fn end_of_event);
  G4bool ProcessHits(G4Step* step, G4TouchableHistory*) override;
  void   EndOfEvent (G4HCofThisEvent* hc)               override;
private:
  process_hits_fn process_hits;
  end_of_event_fn end_of_event;
};

// --------------------------------------------------------------------------------
// The G4Material::AddElement is overloaded on double/int in the second
// parameter. Template argument deduction doesn't seem to be able to resolve
// this, when the values are nested inside an std::initializer_list argument.
// This forces the caller to specify the template argument explicitly, so we
// provide wrappers (material_from_elements_N and material_from_elements_F) with
// the hope that it's a slightly nicer interface.
template<typename NUMBER>
G4Material* material_from_elements(std::string name, G4double density, G4State state,
                                   std::vector<std::tuple<std::string, NUMBER>> components,
                                   bool warn = false)
{
  auto the_material = G4Material::GetMaterial(name, warn);
  if (!the_material) {
    auto n_components = static_cast<G4int>(components.size());
    the_material = new G4Material{name, density, n_components, state};
    for (auto [the_element, n_atoms]: components) {
      the_material -> AddElement(element(the_element), n_atoms);
    }
  }
  return the_material;
}


// Wrapper for material_from_elements<G4int>
inline
G4Material* material_from_elements_N(std::string name, G4double density, G4State state,
                                     std::vector<std::tuple<std::string, G4int>> components,
                                     bool warn = false) {
  return material_from_elements<G4int>(name, density, state, components, warn);
}

// Wrapper for material_from_elements<G4double>
inline
G4Material* material_from_elements_F(std::string name, G4double density, G4State state,
                                     std::vector<std::tuple<std::string, G4double>> components,
                                     bool warn = false) {
  return material_from_elements<G4double>(name, density, state, components, warn);
}

// --------------------------------------------------------------------------------

class place {
public:
  place(G4LogicalVolume* child)  : child(child ? make_optional(child) : nullopt) {}
  place(place const&) = default;

  place& rotate(G4RotationMatrix& rot)     { transformation = HepGeom::Rotate3D{rot}      * transformation; return *this; }
  place& at(double x, double y, double z)  { transformation = HepGeom::Translate3D{x,y,z} * transformation; return *this; }
  place& at(G4ThreeVector    p)            { return at(p.x(), p.y(), p.z()); }
  place& copy_no(int         n)            { copy_number = n      ; return *this; }
  place& in(G4LogicalVolume* parent_)      { parent      = parent_; return *this; }
  place& name(G4String       label_)       { label       = label_ ; return *this; }

  place  clone() const                                            { return *this; }
  G4PVPlacement* operator()()                                     { return now(); }
  G4PVPlacement* now();

private:
  optional<G4LogicalVolume*>  child;
  optional<G4LogicalVolume*>  parent;
  optional<G4String>          label;
  optional<int>               copy_number;
  G4Transform3D               transformation = HepGeom::Transform3D::Identity;
};

// --------------------------------------------------------------------------------
// Utility for creating a vector of physical quantity data, without having to
// repeat the physical unit in each element.

// TODO const version?
std::vector<G4double> scale_by(G4double factor, std::initializer_list<G4double> const& data);

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
// definition of material properties

class material_properties {
  using vec = std::vector<G4double>;
public:
  material_properties& add(G4String const& key, vec const& energies, vec const& values);
  material_properties& add(G4String const& key, vec const& energies, G4double   value );
  material_properties& add(G4String const& key, G4double value);
  G4MaterialPropertiesTable* done() { return table; }
private:
  G4MaterialPropertiesTable* table = new G4MaterialPropertiesTable;
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

// --------------------------------------------------------------------------------
// Use our flavour of optical physics: jump through 3 hoops in one go
void use_our_optical_physics(G4RunManager* run_manager, G4int verbosity=0);

// --------------------------------------------------------------------------------
// Not really nain4, as it's kinda specific to PET / NEMA?
G4VPhysicalVolume* combine_geometries(G4VPhysicalVolume* phantom, G4VPhysicalVolume* detector);

} // namespace nain4

namespace n4 { using namespace nain4; }

// --------------------------------------------------------------------------------

#include <iterator>
#include <queue>

// TODO Move into nain4 namespace
// TODO This is breadth-first; how about depth-first?
// TODO This is an input iterator; how about output/forward?
// TODO Swich to C++ 20 and do it with concepts
class geometry_iterator {
public:
  geometry_iterator() {}
  geometry_iterator(G4VPhysicalVolume* v) { this->q.push(v); }
  geometry_iterator(G4LogicalVolume  * v) { queue_daughters(*v); }
  geometry_iterator(geometry_iterator const &) = default;
  geometry_iterator(geometry_iterator      &&) = default;

  using iterator_category = std::input_iterator_tag;
  using value_type        = G4VPhysicalVolume;
  using pointer           = value_type *; // TODO should this be pointer to pointer?
  using reference         = value_type * const &;
  using difference_type   = std::ptrdiff_t;

  geometry_iterator  operator++(int) { auto tmp = *this; ++(*this); return tmp; }
  geometry_iterator& operator++(   ) {
    if (!this->q.empty()) {
      auto current = this->q.front();
      this->q.pop();
      queue_daughters(*current->GetLogicalVolume());
    }
    return *this;
  }

  pointer   operator->()       { return this->q.front(); }
  reference operator* () const { return this->q.front(); }

  friend bool operator== (const geometry_iterator& a, const geometry_iterator& b) { return a.q == b.q; };
  friend bool operator!= (const geometry_iterator& a, const geometry_iterator& b) { return a.q != b.q; };

private:
  std::queue<G4VPhysicalVolume*> q{};

  void queue_daughters(G4LogicalVolume const& logical) {
    for(size_t d=0; d<logical.GetNoDaughters(); ++d) {
      this->q.push(logical.GetDaughter(d));
    }
  }
};

// By overloading begin and end, we can make G4PhysicalVolume
// usable with the STL algorithms and range-based for loops.
geometry_iterator begin(G4VPhysicalVolume&);
geometry_iterator   end(G4VPhysicalVolume&);
geometry_iterator begin(G4VPhysicalVolume*);
geometry_iterator   end(G4VPhysicalVolume*);

geometry_iterator begin(G4LogicalVolume&);
geometry_iterator   end(G4LogicalVolume&);
geometry_iterator begin(G4LogicalVolume*);
geometry_iterator   end(G4LogicalVolume*);


#pragma GCC diagnostic pop

#endif
