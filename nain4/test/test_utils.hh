#pragma once

#include <n4-material.hh>
#include <n4-shapes.hh>
#include <n4-stream.hh>

#include <FTFP_BERT.hh>
#include <G4EmStandardPhysics_option4.hh>
#include <G4OpticalPhysics.hh>


inline auto water_box() {
  return n4::box("box").cube(1).place(n4::material("G4_WATER")).now();
}

inline void do_nothing(G4Event*) {}


inline auto default_physics_lists() {
  auto verbosity = 0;
  auto physics_list = new FTFP_BERT{verbosity};
  physics_list -> ReplacePhysics(new G4EmStandardPhysics_option4());
  physics_list -> RegisterPhysics(new G4OpticalPhysics{});
  return physics_list;
}

inline auto default_run_manager_with_ui_args(int argc, char** argv) {
  // Redirect G4cout to /dev/null while Geant4 makes noise
  auto hush = n4::silence{std::cout};

  // Construct the default run manager
  return n4::run_manager::create()
    .ui("progname", argc, argv, false)
    .apply_cli_early()
    .physics(default_physics_lists)
    .geometry(water_box)
    .actions(do_nothing)
    .apply_cli_late();
}

inline auto default_run_manager() {
  char *fake_argv[] = { (char*)"progname-ccc", NULL };
  return default_run_manager_with_ui_args(1, fake_argv);
}
