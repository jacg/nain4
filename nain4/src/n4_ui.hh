#ifndef nain4_ui_hh
#define nain4_ui_hh

#include <G4String.hh>
#include <G4Types.hh>
#include <G4UImanager.hh>

#include <argparse/argparse.hpp>

#include <optional>


namespace nain4 {

class ui {
public:
  ui(const std::string& program_name, int argc, char** argv, bool warn_empty_run);
  void run();
  void run_many (const std::vector<std::string> macros_and_commands);
  void run_macro(const G4String& filename);
  void command  (const G4String& command );
  void beam_on  (      G4int     n       ) { command("/run/beamOn " + std::to_string(n)); }
  void run_early() { run_many(early); }
  void run_late () { run_many(late ); }
  void run_vis_macro() { if ( vis_macro.has_value()) { run_macro(vis_macro.value()); }}

  // Parsing the macro search path every time something is prepended
  // to the search path is technically unnecessary and introduces some
  // overhead but not doing so is error-prone and can cause many
  // headaches. Since this function will only be called a reduced
  // number of times, I think in the benefits outweight the cost.
  void     set_path(G4String const& path) {                       g4_ui.SetMacroSearchPath(path) ; g4_ui.ParseMacroSearchPath();}
  void prepend_path(G4String const& path) { set_path(path + ":" + g4_ui.GetMacroSearchPath(    ));}

private:
  argparse::ArgumentParser args;

  std::optional<G4int>     n_events;
  std::vector<std::string> early;
  std::vector<std::string> late;
  std::optional<G4String>  vis_macro;

  int    argc;
  char** argv;

  G4UImanager& g4_ui;
};

} // namespace nain4

namespace n4 { using namespace nain4; }

#endif // nain4_ui_hh
