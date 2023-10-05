#pragma once

#include <G4LogicalVolume.hh>
#include <G4Material.hh>
#include <G4VSolid.hh>

namespace nain4 {

inline auto volume(G4VSolid* solid, G4Material* material) { return new G4LogicalVolume{solid, material, solid->GetName()}; }

// Create logical volume from solid and material
template<class SOLID, class NAME, class... ArgTypes>
G4LogicalVolume* volume(NAME name, G4Material* material, ArgTypes&&... args) {
  auto solid = new SOLID{std::forward<NAME>(name), std::forward<ArgTypes>(args)...};
  return volume(solid, material);
}

// Here for now, as it admits volumes. Needs to find a better home if we ever overload it.
G4LogicalVolume* envelope_of(G4LogicalVolume* original);
G4LogicalVolume* envelope_of(G4LogicalVolume* original, G4String name);

} // namespace nain4

namespace n4 { using namespace nain4; }
