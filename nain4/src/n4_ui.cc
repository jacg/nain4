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
  late_macro{},
  vis_macro{"macs/vis.mac"},
  argc{argc},
  argv{argv},
  g4_ui{*G4UImanager::GetUIpointer()}
{

}

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
    run_macro(vis_macro.value());
    ui_executive.SessionStart();
  }

  if (late_macro.has_value()) {
    run_macro(late_macro.value());
  }

}

} // namespace nain4
