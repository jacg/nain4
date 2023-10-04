#include <nain4.hh>

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

#pragma GCC diagnostic pop
