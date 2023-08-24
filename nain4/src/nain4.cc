#include "nain4.hh"

#include <G4EmStandardPhysics_option4.hh>
#include <G4LogicalVolume.hh>
#include <G4MaterialPropertyVector.hh>
#include <G4OpticalPhysics.hh>
#include <G4Box.hh>
#include <FTFP_BERT.hh>
#include <G4PVPlacement.hh>
#include <G4String.hh>

#include <algorithm>
#include <initializer_list>
#include <iterator>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wshadow"


namespace nain4 {

bool place::global_check_overlaps_ = false;

G4PVPlacement* place::now() {
  // ----- Name --------------------------------------------------
  // + By default, the name is copied from the child volume.
  // + If a copy_number is specified, it is appended to the name.
  // + All of this is overriden if a name is provided explicitly.
  G4String the_name;
  if (this->label) {
    the_name = this->label.value();
  } else {
    the_name = this->child.value()->GetName();
    // TODO: G4 already appends the copy number to the name?
    if (this->copy_number) {
      auto suffix = "-" + std::to_string(copy_number.value());
      the_name += suffix;
    }
  }
  // TODO: Think about these later
  bool WTF_is_pMany   = false;

  return new G4PVPlacement{transformation,
                           child.value(),
                           the_name,
                           parent.value_or(nullptr),
                           WTF_is_pMany,
                           copy_number.value_or(0),
                           global_check_overlaps_ || local_check_overlaps_};
}

G4LogicalVolume* place::get_logical() {
  if (!child.has_value()) {
    auto name = label.value();
    std::cerr << "Called `n4::place::get_logical` on an incomplete"
              << " `n4::place` instance (" + name + "). Aborting."
              << std::endl;
    exit(EXIT_FAILURE);
  }

  return child.value();
}

std::vector<G4double> scale_by(G4double factor, std::initializer_list<G4double> const& data) {
  std::vector<G4double> out;
  out.reserve(data.size());
  std::transform(begin(data), end(data), back_inserter(out), [factor](auto d){ return d*factor; });
  return out;
}

std::vector<G4double> factor_over(G4double factor, std::initializer_list<G4double> const& data) {
  std::vector<G4double> out;
  out.reserve(data.size());
  std::transform(begin(data), end(data), back_inserter(out), [factor](auto d){ return factor/d; });
  return out;
}

// --------------------------------------------------------------------------------
// definition of material_properties
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

// --------------------------------------------------------------------------------
// stream redirection utilities

// redirect to arbitrary stream or buffer
redirect::redirect(std::ios& stream, std::streambuf* new_buffer)
: original_buffer(stream.rdbuf())
, stream(stream) {
  stream.rdbuf(new_buffer);
}

redirect::redirect(std::ios& stream, std::ios& new_stream) : redirect{stream, new_stream.rdbuf()} {}

redirect::~redirect() { stream.rdbuf(original_buffer); }

// redirect to /dev/null
silence::silence(std::ios& stream)
  : original_buffer{stream.rdbuf()}
  , stream{stream}
  , dev_null{"/dev/null"} {
  stream.rdbuf(dev_null.rdbuf());
}
silence::~silence() { stream.rdbuf(original_buffer); }

G4LogicalVolume* envelope_of(G4LogicalVolume* original) {
  return envelope_of(original, original -> GetName() + "-cloned");
}

G4LogicalVolume* envelope_of(G4LogicalVolume* original, G4String name) {
  return new G4LogicalVolume(
    original -> GetSolid(),
    original -> GetMaterial(),
    name);
}

} // namespace nain4

geometry_iterator begin(G4VPhysicalVolume& vol) { return geometry_iterator{&vol}; }
geometry_iterator   end(G4VPhysicalVolume&    ) { return geometry_iterator{    }; }
geometry_iterator begin(G4VPhysicalVolume* vol) { return begin(*vol); }
geometry_iterator   end(G4VPhysicalVolume* vol) { return   end(*vol); }

geometry_iterator begin(G4LogicalVolume& vol) { return geometry_iterator{&vol}; }
geometry_iterator   end(G4LogicalVolume&    ) { return geometry_iterator{    }; }
geometry_iterator begin(G4LogicalVolume* vol) { return begin(*vol); }
geometry_iterator   end(G4LogicalVolume* vol) { return   end(*vol); }

#pragma GCC diagnostic pop
