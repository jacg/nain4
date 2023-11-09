// ANCHOR: full_file
// ANCHOR: includes
#include <my-lib.hh>

#include <G4GenericMessenger.hh>

#include <cstdlib>
// ANCHOR_END: includes

// ANCHOR: pick_cli_arguments
int main(int argc, char* argv[]) {
  // ANCHOR_END: pick_cli_arguments
  unsigned n_event = 0;

  my my;

  G4int physics_verbosity = 0;

  // ANCHOR: create_run_manager
  n4::run_manager::create()
    .ui("my-program-name", argc, argv)
    .macro_path("macs")
    .apply_command("/my/straw_radius 0.5 m")
    .apply_early_macro("early-hard-wired.mac")
    .apply_cli_early() // CLI --early executed at this point
    // .apply_command(...) // also possible after apply_early_macro

    .physics<FTFP_BERT>(physics_verbosity)
    .geometry([&] { return my_geometry(my); })
    .actions(create_actions(my, n_event))

    .apply_command("/my/particle e-")
    .apply_late_macro("late-hard-wired.mac")
    .apply_cli_late() // CLI --late executed at this point
    // .apply_command(...) // also possible after apply_late_macro

    .run();
  // ANCHOR_END: create_run_manager

  // ANCHOR: build_minimal_framework
  // Important! physics list has to be set before the generator!
  // ANCHOR_END: build_minimal_framework

  // ANCHOR: run
  // ANCHOR_END: run
// ANCHOR: closing_bracket
}
// ANCHOR_END: closing_bracket
// ANCHOR_END: full_file
