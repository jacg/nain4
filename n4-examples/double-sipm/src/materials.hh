#pragma once

#include "shared.hh"

#include <G4Material.hh>

G4Material*    csi_with_properties(const config& config);
G4Material*    air_with_properties();
G4Material* teflon_with_properties();
