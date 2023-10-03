#include "shared.hh"

#include <Randomize.hh>

//#include <G4UnitsTable.hh>

config::config() : msngr{new G4GenericMessenger{this, "/my/", "MY configuration variables"}} {
  // The trailing slash after '/my' is CRUCIAL: without it, the msngr
  // violates the principle of least surprise.

  //new G4UnitDefinition("1/MeV","1/MeV", "1/Energy", 1/MeV);
  //msngr -> DeclarePropertyWithUnit("csi_scint_yield", "1/MeV", csi_scint_yield);
  msngr -> DeclareProperty("csi_scint_yield", csi_scint_yield);
  msngr -> DeclareProperty("cold", cold);
  msngr -> DeclareMethod("random_seed", &config::set_random_seed);
}

void config::set_random_seed(G4long seed) { G4Random::setTheSeed(seed); }
