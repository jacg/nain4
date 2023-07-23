// clang-format off

#ifndef n4_run_manager_hh
#define n4_run_manager_hh

#include "g4-mandatory.hh"

#include <G4Run.hh>
#include <G4RunManager.hh>
#include <G4VUserActionInitialization.hh>
#include <G4VUserDetectorConstruction.hh>
#include <G4VUserPhysicsList.hh>

#include <G4RunManager.hh>
#include <G4RunManagerFactory.hh>
#include <G4VUserPrimaryGeneratorAction.hh>

#include <cstdlib>


namespace nain4 {

// Type-state pattern to ensure that the run manager is fully
// configured before initialization.
// Usage:
// auto run_manager = n4::run_manager::create()
//                       .physics (...)
//                       .geometry(...)
//                       .actions (...);

// The last step implicitly initializes the run manager. At each step
// we provide three alternative styles of providing the required
// information (using physics as an example)
// .physics(a_physics_list_instance)            // implemented by NEXT_STATE_BASIC
// .physics<a_physics_list_type>(args...)       // implemented by NEXT_CONSTRUCT
// .physics(zero_arg_fn_returning_physics_list) // implemented by NEXT_BUILD_FN

// Geant4 requires that the physics list be set **BEFORE** a primary
// generator class is **INSTANTIATED**. This requirement cannot be
// fully enforced, but we try to make it less likely by suppressing
// the NEXT_STATE_BASIC option in .actions(...).

// We impose greater restrictions than strictly necessary: G4 allows
// geometry to commute with actions and physics. However our
// constraint imposes no loss of generality.

class run_manager {
  using RM = std::unique_ptr<G4RunManager>;

  run_manager(RM manager) : manager{std::move(manager)} {
    // TODO: Error on second call!
    rm_instance = this;
  }

  RM manager;
  static run_manager* rm_instance;

// Each state needs temporarily owns the G4RunManager and hands over
// ownership to the next state. The constructor is private to ensure
// that clients cannot create their own; by making run_manager a
// friend, each state can build the next because the states are
// members of run_manager as is the create method.
#define CORE(THIS_STATE)                                       \
    friend run_manager;                                        \
  private:                                                     \
    RM manager;                                                \
    THIS_STATE(RM manager) : manager{std::move(manager)} {}    \
  public:


// Transition to the next state by providing an instance of the
// required G4VUser* class. In the last step (actions -> initialized),
// the run manager is initialized implicitly, hence, this macro needs
// the EXTRA parameter.
#define NEXT_STATE_BASIC(NEXT_STATE, METHOD, TYPE, EXTRA) \
  NEXT_STATE METHOD(TYPE* x) {                            \
      manager -> SetUserInitialization(x);                \
      EXTRA;                                              \
      return NEXT_STATE{std::move(manager)};              \
    }

// Transition to the next state by specifying a class and its
// construction arguments.
#define NEXT_CONSTRUCT(NEXT_STATE, METHOD)                             \
    template<class G4VUSERTYPE, class... ArgTypes>                     \
    inline NEXT_STATE METHOD(ArgTypes&&... args) {                     \
      return METHOD(new G4VUSERTYPE{std::forward<ArgTypes>(args)...}); \
    }

// Transition to the next state by providing a zero-argument function
// which returns an instance of the required G4VUser* class.
#define NEXT_BUILD_FN(NEXT_STATE, METHOD, FN_TYPE, BODY) \
  NEXT_STATE METHOD(FN_TYPE build) {                     \
    return METHOD(BODY);                                 \
  }

  struct set_actions {
    CORE(set_actions)
    using fn_type = n4::generator::function;
  private: // To reduce the possibility of instantiating generator before setting physics
    NEXT_STATE_BASIC(run_manager, actions, G4VUserActionInitialization, manager -> Initialize())
  public:
    NEXT_CONSTRUCT  (run_manager, actions)
    NEXT_BUILD_FN   (run_manager, actions, fn_type, new n4::actions{new n4::generator{build}})
    //TODO: Implement these methods
    // NEXT_BUILD_FN   (run_manager, actions, fn_type, new n4::actions{build})
    // NEXT_BUILD_FN   (run_manager, actions, fn_type, build())
  };

  struct set_geometry {
    CORE(set_geometry)
    using fn_type = n4::geometry::construct_fn;
    NEXT_STATE_BASIC(set_actions, geometry, G4VUserDetectorConstruction,)
    NEXT_CONSTRUCT  (set_actions, geometry)
    NEXT_BUILD_FN   (set_actions, geometry, fn_type, new n4::geometry{build})
    //TODO: Implement this method
    // NEXT_BUILD_FN   (set_actions, geometry, fn_type, build())
  };

  struct set_physics {
    CORE(set_physics)
    using fn_type = std::function<G4VUserPhysicsList* ()>;
    NEXT_STATE_BASIC(set_geometry, physics, G4VUserPhysicsList,)
    NEXT_CONSTRUCT  (set_geometry, physics)
    NEXT_BUILD_FN   (set_geometry, physics, fn_type, build())
  };


public:
  static set_physics create(G4RunManagerType type=G4RunManagerType::SerialOnly) {
    // TODO
    // if (rm_instance) {
    //   std::cerr << "MAL" << std::endl;
    //   exit(EXIT_FAILURE);
    // }
    auto manager = std::unique_ptr<G4RunManager>{G4RunManagerFactory::CreateRunManager(type)};
    return set_physics{std::move(manager)};
  }

  static run_manager* get() {
    if (!rm_instance) {
      std::cerr << "MAL" << std::endl;
      exit(EXIT_FAILURE);
    }
    return rm_instance;
  }

  G4RunManager* here_be_dragons() { return manager.get(); }
};

} // namespace nain4

#endif // n4_run_manager_hh
