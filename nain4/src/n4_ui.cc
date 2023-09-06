#include "n4_ui.hh"
#include "n4_run_manager.hh"

#include <G4VisManager.hh>
#include <G4VisExecutive.hh>
#include <G4UImanager.hh>
#include <G4UIExecutive.hh>

#include <argparse/argparse.hpp>

#include <cstdlib>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>


unsigned parse_beam_on(const std::string&  arg) {
  auto parsed = std::stoi(arg.c_str());
  if (parsed < 0) { throw std::runtime_error{std::string{"--beam-on requires an unsigned integer, you gave '"} + arg + "'"}; }
  return static_cast<unsigned>(parsed);

}


namespace nain4 {

// TODO forward to private constructor that accepts parser as parameter, for direct initialization
ui::ui(const std::string& program_name, int argc, char** argv)
:
  n_events{},
  early_macro{},
  late_macro{},
  vis_macro{"macs/vis.mac"},
  argc{argc},
  argv{argv},
  g4_ui{*G4UImanager::GetUIpointer()}
{
  argparse::ArgumentParser args(program_name);

  args.add_argument("--beam-on"    , "-n", "-b").metavar("N-EVENTS").help("run simulation with given number of events");
  args.add_argument("--early-macro", "-e"      ).metavar("FILENAME").help("execute before run manager instantiation");
  args.add_argument( "--late-macro", "-l"      ).metavar("FILENAME").help("execute after  run manager instantiation");
  args.add_argument(  "--vis-macro", "-g"      ).metavar("FILENAME").help("switch from batch mode to GUI, executing this macro");

  try {
    args.parse_args(argc, argv);
  } catch(const std::runtime_error& err) {
    std::cerr << err.what() << std::endl;
    std::cerr << args;
    std::exit(EXIT_FAILURE);
  }

  if (auto n = args.present("--beam-on")) { n_events = parse_beam_on(n.value()); }
  early_macro = args.present("--early-macro");
  late_macro  = args.present( "--late-macro");
  vis_macro   = args.present(  "--vis-macro");

}

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
