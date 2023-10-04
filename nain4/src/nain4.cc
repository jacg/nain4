#include <nain4.hh>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wshadow"


namespace nain4 {

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
