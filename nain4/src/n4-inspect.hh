#pragma once

#include <n4-run-manager.hh>

#include <G4LogicalVolumeStore.hh>
#include <G4PhysicalVolumeStore.hh>
#include <G4SolidStore.hh>
#include <G4ParticleTable.hh>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wshadow"

namespace nain4 {

// Utilies for concisely retrieving things from stores
// clang-format off
#define NAME     (G4String const& name)
#define NAME_VRB (G4String const& name, G4bool verbose=true)
#define IA inline auto
IA find_logical  NAME_VRB { return G4LogicalVolumeStore ::GetInstance()->GetVolume          (name, verbose); }
IA find_physical NAME_VRB { return G4PhysicalVolumeStore::GetInstance()->GetVolume          (name, verbose); }
IA find_solid    NAME_VRB { return G4SolidStore         ::GetInstance()->GetSolid           (name, verbose); }
IA find_particle NAME     { return G4ParticleTable:: GetParticleTable()->FindParticle       (name         ); }
IA event_number  ()       { return n4::run_manager::get().here_be_dragons() -> GetCurrentRun() -> GetNumberOfEvent(); }
#undef IA
#undef NAME
#undef NAME_VRB

// Remove all, logical/physical volumes, solids and assemblies.
inline void clear_geometry() { G4RunManager::GetRunManager() -> ReinitializeGeometry(true); }

} // nampsepace nain4

namespace n4 { using namespace nain4; }

#pragma GCC diagnostic pop
