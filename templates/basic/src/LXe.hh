#pragma once

#include <G4Types.hh>
#include <G4Material.hh>

G4double LXe_Scintillation(G4double energy);

G4double LXe_refractive_index(G4double energy);

G4MaterialPropertiesTable* LXe_optical_material_properties();

G4Material* LXe_with_properties();
