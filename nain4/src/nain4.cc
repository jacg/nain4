#include <nain4.hh>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wshadow"


namespace nain4 {

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
