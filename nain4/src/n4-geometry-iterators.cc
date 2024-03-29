#include <n4-geometry-iterators.hh>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wshadow"

using namespace n4::test;

geometry_iterator begin(G4VPhysicalVolume& vol) { return geometry_iterator{&vol}; }
geometry_iterator   end(G4VPhysicalVolume&    ) { return geometry_iterator{    }; }
geometry_iterator begin(G4VPhysicalVolume* vol) { return begin(*vol); }
geometry_iterator   end(G4VPhysicalVolume* vol) { return   end(*vol); }

geometry_iterator begin(G4LogicalVolume& vol) { return geometry_iterator{&vol}; }
geometry_iterator   end(G4LogicalVolume&    ) { return geometry_iterator{    }; }
geometry_iterator begin(G4LogicalVolume* vol) { return begin(*vol); }
geometry_iterator   end(G4LogicalVolume* vol) { return   end(*vol); }

#pragma GCC diagnostic pop
