#include "n4_ui.hh"
#include "n4_run_manager.hh"

#include <G4VisManager.hh>
#include <G4VisExecutive.hh>
#include <G4UImanager.hh>
#include <G4UIExecutive.hh>

#include <memory>
#include <optional>
#include <stdexcept>
#include <string>

std::optional<unsigned> parse_unsigned(char* arg) {
  try {
    auto parsed = std::stoi(arg);
    if (parsed < 0) { return std::nullopt; }
    return static_cast<unsigned>(parsed);
  } catch (std::invalid_argument&) {
    return std::nullopt;
  }
}


namespace nain4 {

void ui(int argc, char** argv) {
  if (!nain4::run_manager::initialize_called) {
    std::cerr << "\n\n\n"
              << "n4::ui called before n4::run_manager was initialized.\n"
              << "Make sure to call `.initialize()` on the run manager before "
              << "invoking this function.\n\n\n"
              << std::endl;
    exit(EXIT_FAILURE);
  }

  G4UImanager& ui_manager = *G4UImanager::GetUIpointer();
  G4String execute = "/control/execute ";

  auto run_macro = [&] (auto file_name) { ui_manager.ApplyCommand(execute + file_name); };
  auto beam_on   = [&] (auto N        ) { ui_manager.ApplyCommand("/run/beamOn " + std::to_string(N.value())); };

  // Zero arguments on CLI: run interactively in GUI with macs/vis.mac
  if (argc == 1) {
    G4UIExecutive ui_executive{argc, argv};
    G4VisExecutive vis_manager;
    vis_manager.Initialize();
    run_macro("macs/vis.mac");
    ui_executive.SessionStart();
  }

  // 1 argument on CLI: run in batch mode
  //   +     integer: <ARGUMENT-FOR-BEAM-ON>
  //   + non-integer: <MACRO-FILE>
  if (argc == 2) {
    auto n         = parse_unsigned(argv[1]);
    auto file_name =                argv[1] ;
    if (n.has_value()) { beam_on  (n        ); }
    else               { run_macro(file_name); }
  }
  // 2 arguments on CLI: <MACRO-FILE> <ARGUMENT-FOR-BEAM-ON>
  else if (argc == 3) {
    auto n         = parse_unsigned(argv[2]);
    auto file_name =                argv[1] ;
    if (n.has_value()) {
      run_macro(file_name);
      beam_on  (n        );
    }
  } else {
    std::cerr << "Usage:\n\n"
              << argv[0] << "              # run GUI with macs/vis.mac\n"
              << argv[0] << " N            # batch mode: /run/beamOn N\n"
              << argv[0] << " MACRO-FILE   # batch mode: excecute MACRO-FILE\n"
              << argv[0] << " MACRO-FILE N # batch mode: excecute MACRO-FILE then /run/beamOn N\n"
              << std::endl;
  }
}
} // namespace nain4
