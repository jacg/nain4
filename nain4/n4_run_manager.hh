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


namespace nain4 {

#define NEXT_STATE(NAME, TYPE)                       \
  inline run_manager_##NAME NAME(TYPE* x) {          \
    manager -> SetUserInitialization(x);             \
    return run_manager_##NAME{ std::move(manager) }; \
}

#define NEXT_STATE_TEMPLATED(NAME)                            \
  template<class OBJECT, class... ArgTypes>                   \
  inline run_manager_##NAME NAME(ArgTypes&&... args) {        \
    return NAME(new OBJECT{std::forward<ArgTypes>(args)...}); \
}

#define NEXT_STATE_N4(NAME, TYPE)                       \
  inline run_manager_##NAME NAME(TYPE arg) {            \
    return NAME(new n4::NAME{std::forward<TYPE>(arg)}); \
}

  
class run_manager {
public:
  run_manager(std::unique_ptr<G4RunManager> manager) : manager(manager.release()) {}
  run_manager(run_manager&& other) = default;

  inline bool is_valid() { return manager != nullptr; }

  inline G4RunManager* please_ask_the_developers_to_implement_this() { return manager.get(); }

protected:
  std::unique_ptr<G4RunManager> manager;
};


class run_manager_initialized : public run_manager {
public:
  using run_manager::run_manager;
  inline auto beam_on(unsigned n_events) { manager -> BeamOn(n_events);}
  inline auto run()    { return manager -> GetCurrentRun(); }
  inline auto evt_no() { return run()   -> GetNumberOfEvent();}
};

static auto RM = std::shared_ptr<run_manager_initialized>();


class run_manager_actions : public run_manager {
public:
  using run_manager::run_manager;
  inline run_manager_initialized init() {
    manager -> Initialize();
    auto rm = run_manager_initialized{ std::move(manager) };
    RM.reset(&rm);
    return rm;
  }
};


class run_manager_geometry : public run_manager {
public:
  using run_manager::run_manager;
  NEXT_STATE          (actions, G4VUserActionInitialization)
  NEXT_STATE_TEMPLATED(actions)
  NEXT_STATE_N4(actions, n4::generator::function)
  NEXT_STATE_N4(actions, G4VUserPrimaryGeneratorAction*)
};


class run_manager_physics : public run_manager {
public:
  using run_manager::run_manager;
  NEXT_STATE          (geometry, G4VUserDetectorConstruction)
  NEXT_STATE_TEMPLATED(geometry)
  NEXT_STATE_N4(geometry, n4::geometry::construct_fn)
};


class run_manager_blank : public run_manager {
public:
  run_manager_blank(G4RunManagerType type = G4RunManagerType::SerialOnly)
    : run_manager(std::unique_ptr<G4RunManager>{G4RunManagerFactory::CreateRunManager(type)}) {}

  NEXT_STATE          (physics, G4VUserPhysicsList)
  NEXT_STATE_TEMPLATED(physics)
};

#undef NEXT_STATE
#undef NEXT_STATE_TEMPLATED
#undef IMPLEMENT_NEXT_STATE

template<class... ArgTypes>
inline run_manager_blank make_run_manager(ArgTypes&&... args) {
  if (RM)
    std::cerr << "Calling to n4::make_run_manager more than "
              << "once is not allowed." << std::endl;

  return run_manager_blank{std::forward<ArgTypes>(args)...};
}

inline std::shared_ptr<run_manager_initialized> get_run_manager() {
  if (!RM)
    std::cerr << "Calling to n4::get_run_manager before the run manager "
              << "is not fully initialized is not allowed." << std::endl;

  return RM;
}

} // namespace nain4

#endif // n4_run_manager_hh
