#include <n4-material.hh>


#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wshadow"

namespace nain4 {

material_properties& material_properties::add(G4String const& key, vec const& energies, vec const& values) {
  table -> AddProperty(key, energies, values); // es-vs size equality assertion done in AddProperty
  return *this;
}

material_properties& material_properties::add(G4String const& key, vec const& energies, G4double   value ) {
  return add(key, {energies.front(), energies.back()}, {value, value});
}

material_properties& material_properties::add(G4String const& key, G4double value) {
  table -> AddConstProperty(key, value);
  return *this;
}

material_properties& material_properties::add(G4String const& key, G4MaterialPropertyVector* value) {
  table -> AddProperty(key, value);
  return *this;
}

material_properties& material_properties::NEW(G4String const& key, vec const& energies, vec const& values) {
  table -> AddProperty(key, energies, values, true); // es-vs size equality assertion done in AddProperty
  return *this;
}

material_properties& material_properties::NEW(G4String const& key, vec const& energies, G4double   value ) {
  return NEW(key, {energies.front(), energies.back()}, {value, value});
}

material_properties& material_properties::NEW(G4String const& key, G4double value) {
  table -> AddConstProperty(key, value, true);
  return *this;
}
material_properties& material_properties::NEW(G4String const& key, G4MaterialPropertyVector* value) {
  table -> AddProperty(key, value, true);
  return *this;
}

material_properties& material_properties::copy_from(
  G4MaterialPropertiesTable const * const other,
  std::vector<std::string> const& keys
) {
  for (auto key : keys) { copy_from(other, key); }
  return *this;
}

material_properties& material_properties::copy_NEW_from(
  G4MaterialPropertiesTable const * const other,
  std::vector<std::string> const& keys
) {
  for (auto key : keys) { copy_NEW_from(other, key); }
  return *this;
}

material_properties& material_properties::copy_from(
  G4MaterialPropertiesTable const * const other,
  std::initializer_list<std::string> const& keys
) {
  for (auto key : keys) { copy_from(other, key); }
  return *this;
}

material_properties& material_properties::copy_NEW_from(
  G4MaterialPropertiesTable const * const other,
  std::initializer_list<std::string> const& keys
) {
  for (auto key : keys) { copy_NEW_from(other, key); }
  return *this;
}

material_properties& material_properties::copy_from(
  G4MaterialPropertiesTable const * const other,
  std::string const& key
) {
  if (other -> ConstPropertyExists(key)) { add(key, other -> GetConstProperty(key)); }
  else                                   { add(key, other ->      GetProperty(key)); }
  return *this;
}

material_properties& material_properties::copy_NEW_from(
  G4MaterialPropertiesTable const * const other,
  std::string const& key
) {
  if (other -> ConstPropertyExists(key)) { NEW(key, other -> GetConstProperty(key)); }
  else                                   { NEW(key, other ->      GetProperty(key)); }
  return *this;
}

} // namespace nain4

#pragma GCC diagnostic pop
