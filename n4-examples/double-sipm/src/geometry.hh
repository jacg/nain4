#pragma once

#include "shared.hh"
#include <G4PVPlacement.hh>

G4PVPlacement* make_geometry(data& data, const config& config);
