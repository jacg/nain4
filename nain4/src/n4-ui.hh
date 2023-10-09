#pragma once

#include <G4String.hh>
#include <G4Types.hh>
#include <G4UImanager.hh>

#include <argparse/argparse.hpp>

#include <optional>


namespace nain4 {

namespace test { struct query; }

class ui {
public:
  ui(const std::string& program_name, int argc, char** argv, bool warn_empty_run);
  void run();
  void run_many (const std::vector<std::string> macros_and_commands, const G4String& prefix);
  void run_macro(const G4String& filename, const G4String& prefix);
  void command  (const G4String& command , const G4String& prefix, const G4String& kind);
  void beam_on  (      G4int     n       ) { command("/run/beamOn " + std::to_string(n), "", "beam-on"); }
  void run_early() { run_many(early, "early"); }
  void run_late () { run_many(late , "late" ); }
  void run_vis_macro() { if ( vis_macro.has_value()) { run_macro(vis_macro.value(), "vis macro"); }}

  // Parsing the macro search path every time something is prepended
  // to the search path is technically unnecessary and introduces some
  // overhead but not doing so is error-prone and can cause many
  // headaches. Since this function will only be called a reduced
  // number of times, I think in the benefits outweight the cost.
  void     set_path(G4String const& path) {                       g4_ui.SetMacroSearchPath(path) ; g4_ui.ParseMacroSearchPath();}
  void prepend_path(G4String const& path) { set_path(path + ":" + g4_ui.GetMacroSearchPath(    ));}

private:
  friend test::query;
  argparse::ArgumentParser args;

  std::optional<G4int>     n_events;
  std::vector<std::string> early;
  std::vector<std::string> late;
  std::optional<G4String>  vis_macro;
  bool                     use_graphics;

  int    argc;
  char** argv;

  G4UImanager& g4_ui;
};

namespace test {
struct query {
  query(const ui& ui)
    : n_events    {ui.n_events}
    , early       {ui.early}
    , late        {ui.late}
    , vis_macro   {ui.vis_macro}
    , use_graphics{ui.use_graphics}
  {}
  std::optional<G4int>    n_events;
  std::vector<std::string>early;
  std::vector<std::string>late;
  std::optional<G4String> vis_macro;
  bool                    use_graphics;
};

} // namespace test
} // namespace nain4

namespace n4 { using namespace nain4; }
