#pragma once

#include <G4MaterialPropertiesTable.hh>
#include <G4MaterialPropertyVector.hh>
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


} // namespace nain4

namespace n4 { using namespace nain4; }
