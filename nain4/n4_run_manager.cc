#include "n4_run_manager.hh"
#include "G4VPhysicalVolume.hh"
#include "G4PhysicalVolumeStore.hh"

namespace nain4 {

run_manager* run_manager::rm_instance   = nullptr;
bool         run_manager::create_called = false;


void check_for_multiple_world_volumes() {
  auto store = G4PhysicalVolumeStore::GetInstance();
  std::vector<G4String> names{};

  for (auto phys : *store) {
    // Mother is nullptr when phys is the world volume
    if (! phys -> GetMotherLogical()) {
      names.push_back(phys -> GetName());
    }
  }
  auto count = names.size();
  if (count == 1) return;

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

}
