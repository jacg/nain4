// clang-format off
#pragma once

#include <g4-mandatory.hh>
#include <n4_ui.hh>

#include <G4Run.hh>
#include <G4RunManager.hh>
#include <G4String.hh>
#include <G4UserRunAction.hh>
#include <G4VUserActionInitialization.hh>
#include <G4VUserPhysicsList.hh>

#include <G4RunManager.hh>
#include <G4RunManagerFactory.hh>
#include <G4VUserPrimaryGeneratorAction.hh>

#include <cstdlib>


class G4VUserDetectorConstruction;

namespace nain4 {

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wshadow"

// Type-state pattern to ensure that the run manager is fully
// configured before initialization.
// Usage:
// auto run_manager = n4::run_manager::create()
//                       .physics (...)
//                       .geometry(...)
//                       .actions (...)
//                       .run();

// At each step we provide three alternative styles of providing the required
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
  using G4RM = std::unique_ptr<G4RunManager>;

  run_manager(G4RM g4_manager) : g4_manager{std::move(g4_manager)} {
    rm_instance = this;
  }

public:
  run_manager(run_manager& ) = delete;
  run_manager(run_manager&&) = default;

private:
  G4RM g4_manager;
  static run_manager*   rm_instance;
  static bool         create_called;

// Each state needs temporarily owns the G4RunManager and hands over
// ownership to the next state. The constructor is private to ensure
// that clients cannot create their own; by making run_manager a
// friend, each state can build the next because the states are
// members of run_manager as is the create method.
#define CORE(THIS_STATE)                     \
    friend run_manager;                      \
  private:                                   \
    G4RM g4_manager;                         \
    n4::ui ui;                               \
    THIS_STATE(G4RM g4_manager, n4::ui ui) : \
      g4_manager{std::move(g4_manager)},     \
      ui     {std::move(ui     )}            \
      { }                                    \
  public:


// Transition to the next state by providing an instance of the
// required G4VUser* class.
#define NEXT_STATE_BASIC(NEXT_STATE, METHOD, TYPE)             \
  NEXT_STATE METHOD(TYPE* x) {                                 \
      g4_manager -> SetUserInitialization(x);                  \
      return NEXT_STATE{std::move(g4_manager), std::move(ui)}; \
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
    run_manager* run() {
      g4_manager -> Initialize();
      check_world_volume();
      run_manager::rm_instance = new run_manager{std::move(g4_manager)};
      ui.run();
      return run_manager::rm_instance;
    }
    ready apply_command   (const G4String& command ) { ui.command  (command , "late", "command" ); return std::move(*this); }
    ready apply_late_macro(const G4String& filename) { ui.run_macro(filename, "late"            ); return std::move(*this); }
    ready apply_cli_late  (                        ) { ui.run_late (                            ); return std::move(*this); }
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
    set_physics apply_command    (const G4String& command ) { ui.command  (command , "early", "command"); return std::move(*this); }
    set_physics apply_early_macro(const G4String& filename) { ui.run_macro(filename, "early"           ); return std::move(*this); }
    set_physics apply_cli_early  (                        ) { ui.run_early(                            ); return std::move(*this); }
    set_physics macro_path       (const G4String& path    ) { ui.prepend_path(path)                     ; return std::move(*this); }

    using fn_type = std::function<G4VUserPhysicsList* ()>;
    NEXT_STATE_BASIC(set_geometry, physics, G4VUserPhysicsList)
    NEXT_CONSTRUCT  (set_geometry, physics)
    NEXT_BUILD_FN   (set_geometry, physics, fn_type, build())
  };

  struct initialize_ui {
    friend run_manager;

  private:
    G4RM g4_manager;
    initialize_ui(G4RM g4_manager) : g4_manager{std::move(g4_manager)} { }

  public:
    set_physics ui(const std::string& program_name, int argc, char** argv, bool warn_empty_run=true) {
      auto ui = n4::ui(program_name, argc, argv, warn_empty_run);
      return {std::move(g4_manager), std::move(ui)};
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

    auto g4_manager = std::unique_ptr<G4RunManager>{G4RunManagerFactory::CreateRunManager(type)};
    return initialize_ui{std::move(g4_manager)};
  }

  static void exit_if_too_early(const G4String& method);

  // TODO: Consider returning pointer to make compiler error message
  // clearer, in cases such as
  // auto rm = run_manager::get()
  // where the auto requires an `&` for compilation to succeed.
  static run_manager& get() {
    exit_if_too_early("run_manager::get");
    return *rm_instance;
  }

  static const G4VUserDetectorConstruction& get_geometry();
  static const G4UserRunAction            & get_run_action();
  static const G4UserEventAction          & get_event_action();
  static const G4UserTrackingAction       & get_tracking_action();
  static const G4UserStackingAction       & get_stacking_action();
  static const G4UserSteppingAction       & get_stepping_action();

#define DOWNCAST(METHOD)                        \
  template <class DOWN>                         \
  static const DOWN& METHOD() {                 \
    exit_if_too_early("run_manager::" #METHOD); \
    return static_cast<const DOWN&>(METHOD());  \
  }

  DOWNCAST(get_geometry);
  DOWNCAST(get_run_action);
  DOWNCAST(get_event_action);
  DOWNCAST(get_tracking_action);
  DOWNCAST(get_stacking_action);
  DOWNCAST(get_stepping_action);

#undef DOWNCAST

  G4RunManager* here_be_dragons() { return g4_manager.get(); }
};


#pragma GCC diagnostic pop

} // namespace nain4
