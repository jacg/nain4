// clang-format off

#ifndef n4_run_manager_hh
#define n4_run_manager_hh

#include <G4RunManager.hh>
#include <G4VUserActionInitialization.hh>
#include <G4VUserDetectorConstruction.hh>
#include <G4VUserPhysicsList.hh>

#include <G4RunManager.hh>
#include <G4RunManagerFactory.hh>

namespace nain4 {


class run_manager {
public:
  run_manager(G4RunManagerType type = G4RunManagerType::SerialOnly)
      : manager_{G4RunManagerFactory::CreateRunManager(type)}
    {}
#define SET_U_INIT(METHOD, TYPE) run_manager& METHOD (TYPE* x) { manager_ -> SetUserInitialization(x); return *this; }

  SET_U_INIT(actions , G4VUserActionInitialization)
  SET_U_INIT(geometry, G4VUserDetectorConstruction)
  SET_U_INIT(physics , G4VUserPhysicsList)

  template<class PHYSICS, class... ArgTypes>
  run_manager& physics(ArgTypes&&... args) { return this -> physics(new PHYSICS{std::forward<ArgTypes>(args)...});  }

  run_manager& init();
#undef SET_U_INIT

  // G4RunManager has lots of methods. This is an escape hatch to enable use of
  // any that we haven't dealt with yet. But it does give the user the power to
  // cause lot of trouble.
  G4RunManager* here_be_dragons() { return manager_.get(); }

private:
  std::unique_ptr<G4RunManager>                manager_;
  std::unique_ptr<G4VUserActionInitialization> actions_;
  std::unique_ptr<G4VUserDetectorConstruction> geometry_;
  std::unique_ptr<G4VUserPhysicsList>          physics_;

};


inline G4RunManager* get_run_manager() { return G4RunManager::GetRunManager(); }


} // namespace nain4

#endif // n4_run_manager_hh
