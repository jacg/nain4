#ifndef n4_run_manager_hh
#define n4_run_manager_hh

#include <G4RunManager.hh>
#include <G4RunManagerFactory.hh>

namespace nain4 {
// --------------------------------------------------------------------------------
// Utilities for concise creation and finding of run manager
// clang-format off
inline
std::unique_ptr<G4RunManager> run_manager(G4RunManagerType type = G4RunManagerType::SerialOnly) {
  return std::unique_ptr<G4RunManager>{G4RunManagerFactory::CreateRunManager(type)};
}

inline G4RunManager* get_run_manager() { return G4RunManager::GetRunManager(); }

} // namespace nain4

#endif // n4_run_manager_hh
