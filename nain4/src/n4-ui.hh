#pragma once

#include <G4String.hh>
#include <G4Types.hh>
#include <G4UImanager.hh>

#include <argparse/argparse.hpp>

#include <algorithm>
#include <memory>
#include <optional>


namespace nain4 {

using unique_argparse = std::unique_ptr<argparse::ArgumentParser>;

namespace test {
struct query;
// Utility for construction of argc/argv combination for use in n4::ui CLI tests
struct argcv {
  int    argc;
  char** argv;
  argcv(std::initializer_list<std::string> args);
private:
  std::vector<std::unique_ptr<char[]>> owners;
};

void report_args(std::ostream&, int argc, char** argv);
} // namespace test

namespace internal {
struct cli_and_err { unique_argparse cli; std::optional<std::runtime_error> err; };
cli_and_err define_args(const std::string& program_name, int argc, char** argv);
using may_err = std::optional<std::exception>;
void exit_on_err(may_err);
} // namespace internal

class ui {
private:
  using may_err = internal::may_err;
public:
  enum class kind { command, macro, beam_on };
  std::string repr(const kind kind);
private: ui(const std::string& program_name, int argc, char** argv, unique_argparse, bool warn_empty_run);
public : ui(const std::string& program_name, int argc, char** argv,                  bool warn_empty_run);

  static ui fake_ui() {
    test::argcv args{"prog-name"};
    return ui("prog-name", args.argc, args.argv, false);
  }

  void run(std::optional<unsigned> n = std::nullopt);
  may_err run_many (const std::vector<std::string> macros_and_commands, const G4String& prefix);
  may_err run_macro(const G4String& filename, const G4String& prefix);
  may_err command  (const G4String& command , const G4String& prefix, const kind kind);
  may_err beam_on  (      G4int     n       ) { return command("/run/beamOn " + std::to_string(n), "", kind::beam_on); }
  may_err run_early() { return run_many(early, "early"); }
  may_err run_late () { return run_many(late , "late" ); }
  may_err run_vis  () { return run_many(vis  , "vis"  ); }

  // Parsing the macro search path every time something is prepended
  // to the search path is technically unnecessary and introduces some
  // overhead but not doing so is error-prone and can cause many
  // headaches. Since this function will only be called a reduced
  // number of times, I think in the benefits outweight the cost.
  void     set_path(G4String const& path) {                       g4_ui.SetMacroSearchPath(path) ; g4_ui.ParseMacroSearchPath();}
  void prepend_path(G4String const& path) { set_path(path + ":" + g4_ui.GetMacroSearchPath(    ));}

private:
  friend test::query;

  std::optional<G4int>     n_events;
  std::vector<std::string> early;
  std::vector<std::string> late;
  std::vector<std::string> vis;
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
    , vis         {ui.vis}
    , use_graphics{ui.use_graphics}
  {}
  std::optional<G4int>    n_events;
  std::vector<std::string>early;
  std::vector<std::string>late;
  std::vector<std::string>vis;
  bool                    use_graphics;
};

} // namespace test
} // namespace nain4

namespace n4 { using namespace nain4; }
