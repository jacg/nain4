#ifndef GEOMETRY_HH
#define GEOMETRY_HH

#include "shared.hh"

class G4PVPlacement;

G4PVPlacement* make_geometry(data& times_of_arrival, const config& config);

#endif // GEOMETRY_HH
