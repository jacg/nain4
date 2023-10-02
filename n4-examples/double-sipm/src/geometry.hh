#ifndef GEOMETRY_HH
#define GEOMETRY_HH

#include "shared.hh"

class G4PVPlacement;

G4PVPlacement* make_geometry(std::vector<std::vector<G4double>>& times_of_arrival, const config& config);

#endif // GEOMETRY_HH
