#ifndef GEOMETRY_HH
#define GEOMETRY_HH

#include "shared.hh"

class G4PVPlacement;

G4PVPlacement* make_geometry(data& data, const config& config);

#endif // GEOMETRY_HH
