#pragma once

#include <G4MaterialPropertiesTable.hh>
#include <G4Material.hh>
#include <G4MaterialPropertyVector.hh>
#include <G4NistManager.hh>
#include <G4Types.hh>

#include <vector>

namespace nain4 {
// --------------------------------------------------------------------------------
// definition of material properties

class material_properties {
  using vec = std::vector<G4double>;
public:
  material_properties& add(G4String const& key, vec const& energies, vec const& values);
  material_properties& add(G4String const& key, vec const& energies, G4double   value );
  material_properties& add(G4String const& key, G4double value);
  material_properties& add(G4String const& key, G4MaterialPropertyVector* value);
  // Quick hack to get around Itsaso's complilation problems. Need to implement a proper interface for this
  material_properties& NEW(G4String const& key, vec const& energies, vec const& values);
  material_properties& NEW(G4String const& key, vec const& energies, G4double   value );
  material_properties& NEW(G4String const& key, G4double value);
  material_properties& NEW(G4String const& key, G4MaterialPropertyVector* value);
  material_properties& copy_from    (G4MaterialPropertiesTable const * const other, std::vector<std::string> const& keys);
  material_properties& copy_NEW_from(G4MaterialPropertiesTable const * const other, std::vector<std::string> const& keys);
  material_properties& copy_from    (G4MaterialPropertiesTable const * const other, std::initializer_list<std::string> const& keys);
  material_properties& copy_NEW_from(G4MaterialPropertiesTable const * const other, std::initializer_list<std::string> const& keys);
  material_properties& copy_from    (G4MaterialPropertiesTable const * const other,             std::string  const& key );
  material_properties& copy_NEW_from(G4MaterialPropertiesTable const * const other,             std::string  const& key );
  G4MaterialPropertiesTable* done() { return table; }
private:
  G4MaterialPropertiesTable* table = new G4MaterialPropertiesTable;
};

// --------------------------------------------------------------------------------

inline auto material (G4String const& name){ return G4NistManager::Instance()->FindOrBuildMaterial(name); }
inline auto element  (G4String const& name){ return G4NistManager::Instance()->FindOrBuildElement (name); }

// An element identifier + element count.
// The element can be identified by
// + G4Element*
// + a string containing the name of the element
// Comes in two varieties, N/F: see discussion of material_from_elements_{N,F} below
struct element_spec_N {
  element_spec_N(G4Element const * const element, G4int n): element{               element      }, n{n} {}
  element_spec_N(std::string const& element_name, G4int n): element{nain4::element(element_name)}, n{n} {}
  G4Element const * const element;
  G4int const n;
};
struct element_spec_F {
  element_spec_F(G4Element const * const element, G4double f): element{               element      }, n{f} {}
  element_spec_F(std::string const& element_name, G4double f): element{nain4::element(element_name)}, n{f} {}
  G4Element const * const element;
  G4double const n;
};

struct material_options {
  G4State  state   {kStateUndefined};
  G4double temp    {       NTP_Temperature};
  G4double pressure{CLHEP::STP_Pressure};
};

// --------------------------------------------------------------------------------
// The G4Material::AddElement is overloaded on double/int in the second
// parameter. Template argument deduction doesn't seem to be able to resolve
// this, when the values are nested inside an std::initializer_list argument.
// This forces the caller to specify the template argument explicitly, so we
// provide wrappers (material_from_elements_N and material_from_elements_F) with
// the hope that it's a slightly nicer interface.
template<typename ELEMENT_SPEC>
G4Material* material_from_elements(
  std::string name,
  G4double density,
  material_options const& opts,
  std::vector<ELEMENT_SPEC> components,
  bool warn = false
) {
  auto the_material = G4Material::GetMaterial(name, warn);
  if (!the_material) {
    auto n_components = static_cast<G4int>(components.size());
    the_material = new G4Material{name, density, n_components, opts.state, opts.temp, opts.pressure};
    for (auto const& spec: components) {
      the_material -> AddElement((G4Element*)spec.element, spec.n);
    }
  }
  return the_material;
}


// Wrapper for material_from_elements<G4int>
inline
G4Material* material_from_elements_N(std::string name, G4double density, material_options const& opts,
                                     std::vector<element_spec_N> components,
                                     bool warn = false) {
  return material_from_elements<element_spec_N>(name, density, opts, components, warn);
}

// Wrapper for material_from_elements<G4double>
inline
G4Material* material_from_elements_F(std::string name, G4double density, material_options const& opts,
                                     std::vector<element_spec_F> components,
                                     bool warn = false) {
  return material_from_elements<element_spec_F>(name, density, opts, components, warn);
}


} // namespace nain4

namespace n4 { using namespace nain4; }
