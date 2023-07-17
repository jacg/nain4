#include "n4_ui.hh"

#include <G4VisManager.hh>
#include <G4VisExecutive.hh>
#include <G4UImanager.hh>
#include <G4UIExecutive.hh>

#include <memory>

namespace nain4 {

void ui(int argc, char** argv) {

  // Initialize visualization
  std::unique_ptr<G4VisManager> visManager = std::make_unique<G4VisExecutive>();
  // G4VisExecutive can take a verbosity argument - see /vis/verbose guidance.
  // G4VisManager* visManager = new G4VisExecutive("Quiet");
  visManager -> Initialize();

  // Get the pointer to the User Interface manager
  G4UImanager* UImanager = G4UImanager::GetUIpointer();
  std::unique_ptr<G4UIExecutive> ui;
  if ( argc == 1 ) { ui = std::make_unique<G4UIExecutive>(argc, argv); }
  if ( ! ui ) {
    // batch mode
    G4String command = "/control/execute ";
    G4String fileName = argv[1];
    UImanager->ApplyCommand(command+fileName);
  } else {
    // interactive mode
    UImanager->ApplyCommand("/control/execute macs/vis.mac");
    ui->SessionStart();
  }
}

} // namespace nain4
