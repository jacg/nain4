// clang-format off

#ifndef n4_run_manager_hh
#define n4_run_manager_hh

#include "g4-mandatory.hh"
#include "n4_ui.hh"

#include <G4Run.hh>
#include <G4RunManager.hh>
#include <G4String.hh>
#include <G4VUserActionInitialization.hh>
#include <G4VUserDetectorConstruction.hh>
#include <G4VUserPhysicsList.hh>

#include <G4RunManager.hh>
#include <G4RunManagerFactory.hh>
#include <G4VUserPrimaryGeneratorAction.hh>

#include <cstdlib>


namespace nain4 {

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wshadow"

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

void check_world_volume();

class run_manager {
  using RM = std::unique_ptr<G4RunManager>;

  run_manager(RM manager) : manager{std::move(manager)} {
    rm_instance = this;
  }

public:
  run_manager(run_manager& ) = delete;
  run_manager(run_manager&&) = default;

  RM manager;
  static run_manager*   rm_instance;
  static bool         create_called;

// Each state needs temporarily owns the G4RunManager and hands over
// ownership to the next state. The constructor is private to ensure
// that clients cannot create their own; by making run_manager a
// friend, each state can build the next because the states are
// members of run_manager as is the create method.
#define CORE(THIS_STATE)                \
    friend run_manager;                 \
  private:                              \
    RM manager;                         \
    n4::ui ui;                          \
    THIS_STATE(RM manager, n4::ui ui) : \
      manager{std::move(manager)},      \
      ui     {std::move(ui     )}       \
      { }                               \
  public:


// Transition to the next state by providing an instance of the
// required G4VUser* class.
#define NEXT_STATE_BASIC(NEXT_STATE, METHOD, TYPE)          \
  NEXT_STATE METHOD(TYPE* x) {                              \
      manager -> SetUserInitialization(x);                  \
      return NEXT_STATE{std::move(manager), std::move(ui)}; \
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

  struct ready {
    CORE(ready)
    void run() {
      manager -> Initialize();
      check_world_volume();
      run_manager::rm_instance = new run_manager{std::move(manager)};
      ui.run();
    }
    ready& apply_command(const G4String& command) { ui.apply(command); return *this; }
  };

  struct set_actions {
    CORE(set_actions)
    using fn_type = n4::generator::function;
    using gn_type = std::function<n4::generator*()>;
    using ac_type = std::function<n4::actions  *()>;

    NEXT_STATE_BASIC(ready, actions, G4VUserActionInitialization)
    NEXT_CONSTRUCT  (ready, actions)
    NEXT_BUILD_FN   (ready, actions, fn_type, new n4::actions{new n4::generator{build}})
    NEXT_BUILD_FN   (ready, actions, gn_type, new n4::actions{build()})
    NEXT_BUILD_FN   (ready, actions, ac_type, build())
  };

  struct set_geometry {
    CORE(set_geometry)
    using fn_type = n4::geometry::construct_fn;
    NEXT_STATE_BASIC(set_actions, geometry, G4VUserDetectorConstruction)
    NEXT_CONSTRUCT  (set_actions, geometry)
    NEXT_BUILD_FN   (set_actions, geometry, fn_type, new n4::geometry{build})
    //TODO: Implement this method
    // NEXT_BUILD_FN   (set_actions, geometry, fn_type, build())
  };

  struct set_physics {
    CORE(set_physics)
    set_physics& apply_command    (const G4String& command ) { ui.apply    (command ); return *this; }
    set_physics& apply_early_macro(const G4String& filename) { ui.run_macro(filename); return *this; }
    using fn_type = std::function<G4VUserPhysicsList* ()>;
    NEXT_STATE_BASIC(set_geometry, physics, G4VUserPhysicsList)
    NEXT_CONSTRUCT  (set_geometry, physics)
    NEXT_BUILD_FN   (set_geometry, physics, fn_type, build())
  };

  struct initialize_ui {
    friend run_manager;

  private:
    RM manager;
    initialize_ui(RM manager) : manager{std::move(manager)} { }

  public:
    set_physics ui(int argc, char** argv) {
      auto ui = n4::ui(argc, argv);
      return {std::move(manager), std::move(ui)};
    }
  };


public:
  static initialize_ui create(G4RunManagerType type=G4RunManagerType::SerialOnly) {
    if (run_manager::create_called) {
      std::cerr << "run_manager::create has already been called. "
                << "It makes no sense to call it more than once."
                << std::endl;
      exit(EXIT_FAILURE);
    }

    run_manager::create_called = true;

    auto manager = std::unique_ptr<G4RunManager>{G4RunManagerFactory::CreateRunManager(type)};
    return initialize_ui{std::move(manager)};
  }

  // TODO: Consider returning pointer to make compiler error message
  // clearer, in cases such as
  // auto rm = run_manager::get()
  // where the auto requires an `&` for compilation to succeed.
  static run_manager& get() {
    if (!rm_instance) {
      std::cerr << "run_manager::get called before run_manager configuration completed. "
                << "Configure the run_manager with:\n"
                << "auto run_manager = n4::run_manager::create()\n"
                << "                      .physics (...)\n"
                << "                      .geometry(...)\n"
                << "                      .actions (...);\n\n"
                << "[...]\n\n"
                << "run_manager.initialize();\n"
                << std::endl;
      exit(EXIT_FAILURE);
    }
    return *rm_instance;
  }

  G4RunManager* here_be_dragons() { return manager.get(); }
};

#pragma GCC diagnostic pop

} // namespace nain4

#endif // n4_run_manager_hh
