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

ui::ui(int argc, char** argv)
:
  n_events{},
  early_macro{},
  late_macro{"macs/run.mac"},
  vis_macro{},
  argc{argc},
  argv{argv},
  g4_ui{*G4UImanager::GetUIpointer()}
{

}

  // // 1 argument on CLI: run in batch mode
  // //   +     integer: <ARGUMENT-FOR-BEAM-ON>
  // //   + non-integer: <MACRO-FILE>
  // if (argc == 2) {
  //   auto n         = parse_unsigned(argv[1]);
  //   auto file_name =                argv[1] ;
  //   if (n.has_value()) { beam_on  (n        ); }
  //   else               { run_macro(file_name); }
  // }
  // // 2 arguments on CLI: <MACRO-FILE> <ARGUMENT-FOR-BEAM-ON>
  // else if (argc == 3) {
  //   auto n         = parse_unsigned(argv[2]);
  //   auto file_name =                argv[1] ;
  //   if (n.has_value()) {
  //     run_macro(file_name);
  //     beam_on  (n        );
  //   }
  // } else {
  //   std::cerr << "Usage:\n\n"
  //             << argv[0] << "              # run GUI with macs/vis.mac\n"
  //             << argv[0] << " N            # batch mode: /run/beamOn N\n"
  //             << argv[0] << " MACRO-FILE   # batch mode: excecute MACRO-FILE\n"
  //             << argv[0] << " MACRO-FILE N # batch mode: excecute MACRO-FILE then /run/beamOn N\n"
  //             << std::endl;
  // }

void ui::run() {

  if (n_events.has_value()) {
    beam_on(n_events.value());
  }

  if (vis_macro.has_value()) {
    G4UIExecutive ui_executive{argc, argv};
    G4VisExecutive vis_manager;
    vis_manager.Initialize();
    run_macro("macs/vis.mac");
    ui_executive.SessionStart();
  }

}

} // namespace nain4
