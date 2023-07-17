// clang-format off

#ifndef n4_run_manager_hh
#define n4_run_manager_hh

#include "g4-mandatory.hh"

#include <G4RunManager.hh>
#include <G4VUserActionInitialization.hh>
#include <G4VUserDetectorConstruction.hh>
#include <G4VUserPhysicsList.hh>

#include <G4RunManager.hh>
#include <G4RunManagerFactory.hh>
#include <G4VUserPrimaryGeneratorAction.hh>

namespace nain4 {


class run_manager {
public:
  run_manager(G4RunManagerType type = G4RunManagerType::SerialOnly)
      : manager_{G4RunManagerFactory::CreateRunManager(type)}
    {}
#define SET_U_INIT(METHOD, TYPE) run_manager& METHOD (TYPE* x) {    \
        manager_ -> SetUserInitialization(x);                       \
        user_init_set_count[#METHOD] += 1;                          \
        return *this;                                               \
}

  SET_U_INIT(actions , G4VUserActionInitialization)
  SET_U_INIT(geometry, G4VUserDetectorConstruction)
  SET_U_INIT(physics , G4VUserPhysicsList)

  template<class PHYSICS, class... ArgTypes>
  run_manager& physics(ArgTypes&&... args) { return this -> physics(new PHYSICS{std::forward<ArgTypes>(args)...});  }

  run_manager& geometry(n4::geometry::construct_fn   build) { return geometry(new n4::geometry{build}); }
  run_manager& actions (n4::generator::function      build) { return  actions(new n4::actions {build}); }
  run_manager& actions (G4VUserPrimaryGeneratorAction* gen) { return  actions(new n4::actions {gen  }); }

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
  std::map<std::string, unsigned> user_init_set_count{{"physics", 0}, {"actions", 0}, {"geometry", 0}};
};


inline G4RunManager* get_run_manager() { return G4RunManager::GetRunManager(); }


} // namespace nain4

#endif // n4_run_manager_hh
