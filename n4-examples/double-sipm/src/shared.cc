#include "shared.hh"

#include <Randomize.hh>
#include <G4UnitsTable.hh>

config::config() : msngr{new G4GenericMessenger{this, "/my/", "MY configuration variables"}} {
  // The trailing slash after '/my' is CRUCIAL: without it, the msngr
  // violates the principle of least surprise.

  // BuildUnitsTable *must* be called before `new G4UnitDefinition`, otherwise
  // the standard units disappear. We're looking into how this can be mitigated
  // in a nani4 messenger interface.
  G4UnitDefinition::BuildUnitsTable();

  new G4UnitDefinition("1/MeV","1/MeV", "1/Energy", 1/MeV);
  msngr -> DeclarePropertyWithUnit("csi_scint_yield", "1/MeV", csi_scint_yield);

#define PROPERTY(name) msngr -> DeclareProperty(#name, name)
  PROPERTY(cold);
  PROPERTY(debug);
  PROPERTY(verbosity_phys_list);
  PROPERTY(verbosity_em);
  PROPERTY(verbosity_optical);
#undef PROPERTY
  msngr -> DeclareMethod("random_seed", &config::set_random_seed);
}

void config::set_random_seed(G4long seed) { G4Random::setTheSeed(seed); }

G4bool config::debug = false;
