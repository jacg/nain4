#include <n4-run-manager.hh>

#include "G4VPhysicalVolume.hh"
#include "G4PhysicalVolumeStore.hh"
#include <G4ThreeVector.hh>
#include <G4UserRunAction.hh>
#include <G4VUserDetectorConstruction.hh>

namespace nain4 {

run_manager* run_manager::  rm_instance = nullptr;
bool         run_manager::create_called = false;


void check_world_volume() {
  auto store = G4PhysicalVolumeStore::GetInstance();
  std::vector<G4String> names{};
  bool placed_at_origin = false;

  for (auto phys : *store) {
    // Mother is nullptr when phys is the world volume
    if (! phys -> GetMotherLogical()) {
      names.push_back(phys -> GetName());
      placed_at_origin = phys -> GetObjectTranslation()   ==   G4ThreeVector{};
    }
  }
  auto count = names.size();
  if (count == 1) {
    if (placed_at_origin) { return; }
    else                  {
      std::cerr << "The world volume (the only volume without a mother volume) cannot be placed off origin."
                << std::endl;
    }
  }

  else if (count == 0) {
    std::cerr << "No world volume provided. ";
  }

  else {
    std::cerr   << "Too many world volumes provided:";
    for (auto& name : names)
      std::cerr << "\n ->" << name;
  }

  std::cerr     << "\nYou must provide exactly one world volume."
                << std::endl;
  exit(EXIT_FAILURE);
}


void run_manager::exit_if_too_early(const G4String& method) {
  if (!run_manager::rm_instance) {
    std::cerr << method << " called before run_manager configuration completed. "
                 "Configure the run_manager with:\n"
                 "n4::run_manager::create()\n"
                 "  .physics (...)\n"
                 "  .geometry(...)\n"
                 "  .actions (...)\n"
                 "  .run();\n\n"
              << std::endl;
    exit(EXIT_FAILURE);
  }
}

#define GET_USER_DEFINED(N4_METHOD, G4_METHOD, RETURN_TYPE) \
  const RETURN_TYPE& run_manager::N4_METHOD() {             \
    exit_if_too_early("run_manager::" #N4_METHOD);          \
    return *get().here_be_dragons() -> G4_METHOD();         \
  }

  GET_USER_DEFINED(get_geometry       , GetUserDetectorConstruction, G4VUserDetectorConstruction);
  GET_USER_DEFINED(get_run_action     , GetUserRunAction           , G4UserRunAction            );
  GET_USER_DEFINED(get_event_action   , GetUserEventAction         , G4UserEventAction          );
  GET_USER_DEFINED(get_tracking_action, GetUserTrackingAction      , G4UserTrackingAction       );
  GET_USER_DEFINED(get_stacking_action, GetUserStackingAction      , G4UserStackingAction       );
  GET_USER_DEFINED(get_stepping_action, GetUserSteppingAction      , G4UserSteppingAction       );

} // namespace nain4
