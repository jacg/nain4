#include "nain4.hh"

#include <G4EmStandardPhysics_option4.hh>
#include <G4OpticalPhysics.hh>
#include <G4Box.hh>
#include <FTFP_BERT.hh>
#include <G4PVPlacement.hh>
#include <G4String.hh>

#include <algorithm>
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

std::vector<G4double> scale_by(G4double factor, std::initializer_list<G4double> const& data) {
  std::vector<G4double> out;
  out.reserve(data.size());
  std::transform(begin(data), end(data), back_inserter(out), [factor](auto d){ return d*factor; });
  return out;
}

// --------------------------------------------------------------------------------
// definition of material_properties
material_properties& material_properties::add(G4String const& key, vec const& energies, vec const& values) {
  table -> AddProperty(key, energies, values); // es-vs size equality assertion done in AddProperty
  return *this;
}

material_properties& material_properties::add(G4String const& key, vec const& energies, G4double   value ) {
  return add(key, energies, vec(energies.size(), value));
}

material_properties& material_properties::add(G4String const& key, G4double value) {
  table->AddConstProperty(key, value);
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


// Set optical physics lists
void use_our_optical_physics(G4RunManager* run_manager, G4int verbosity) {
    auto physics_list = new FTFP_BERT{verbosity};
    physics_list -> ReplacePhysics(new G4EmStandardPhysics_option4());
    physics_list -> RegisterPhysics(new G4OpticalPhysics{});
    run_manager  -> SetUserInitialization(physics_list);
} // run_manager owns physics_list



G4VPhysicalVolume* combine_geometries(G4VPhysicalVolume* phantom, G4VPhysicalVolume* detector) {
  auto detector_envelope = detector -> GetLogicalVolume();
  auto phantom_envelope  =  phantom -> GetLogicalVolume();

  // TODO: not general enough: only uses translation ignores other transformations
  // TODO Can we avoid extracting logical and use physical/placement directly ?
  auto phantom_physical = phantom_envelope -> GetDaughter(0);
  auto phantom_logical  = phantom_physical -> GetLogicalVolume();
  auto phantom_translation = phantom_physical -> GetTranslation();

  // Check whether phantom envelope fits inside detector envelope, with margin.
  auto& pbox = dynamic_cast<G4Box&>(* phantom_envelope -> GetSolid());
  auto& dbox = dynamic_cast<G4Box&>(*detector_envelope -> GetSolid());
  auto expand = false;
  auto make_space = [&expand](auto p, auto d) {
    auto space_needed = std::max(p * 1.1, d);
    if (space_needed > d) { expand = true; }
    return space_needed;
  };
  auto x = make_space(pbox.GetXHalfLength(), dbox.GetXHalfLength());
  auto y = make_space(pbox.GetYHalfLength(), dbox.GetYHalfLength());
  auto z = make_space(pbox.GetZHalfLength(), dbox.GetZHalfLength());
  // Expand detector envelope if needed
  if (expand) {
    auto new_box = new G4Box(detector_envelope->GetName(), x, y, z);
    detector_envelope -> SetSolid(new_box);
  }

  n4::place(phantom_logical).in(detector_envelope).at(phantom_translation).now();
  return detector;
}

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
