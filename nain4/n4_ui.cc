#include "n4_ui.hh"

#include <G4VisManager.hh>
#include <G4VisExecutive.hh>
#include <G4UImanager.hh>
#include <G4UIExecutive.hh>

#include <memory>

namespace nain4 {

void ui(int argc, char** argv) {
  G4UImanager& ui_manager = *G4UImanager::GetUIpointer();
  G4String execute = "/control/execute ";

  if (argc == 1) { // No .mac file specified on CLI: run interactively in GUI
    G4UIExecutive ui_executive{argc, argv};
    G4VisExecutive vis_manager;
    vis_manager.Initialize();
    ui_manager.ApplyCommand(execute + "macs/vis.mac");
    ui_executive.SessionStart();
  } else { // Run in batch mode with the .mac file specified on CLI
    G4String file_name = argv[1];
    ui_manager.ApplyCommand(execute + file_name);
  }
}

} // namespace nain4
