#ifndef GEOMETRY_HH
#define GEOMETRY_HH

#include "shared.hh"

class G4VPhysicalVolume;

G4VPhysicalVolume* make_geometry(data& data, const config& config);

#endif // GEOMETRY_HH
