#include <n4_ui.hh>
#include <n4_run_manager.hh>

#include <G4String.hh>
#include <G4UIExecutive.hh>
#include <G4UIcommandStatus.hh>
#include <G4UImanager.hh>
#include <G4VisExecutive.hh>
#include <G4VisManager.hh>

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

#define MULTIPLE nargs(argparse::nargs_pattern::at_least_one).append()

argparse::ArgumentParser define_args(const std::string& program_name, int argc, char** argv) {
  argparse::ArgumentParser args{program_name};
  args.add_argument("--beam-on" , "-n").metavar("N"    ).help("/run/beamOn N");
  args.add_argument("--early"   , "-e").metavar("ITEMS").help("execute ITEMS before run manager instantiation").MULTIPLE;
  args.add_argument("--late"    , "-l").metavar("ITEMS").help("execute ITEMS  after run manager instantiation").MULTIPLE;
  args.add_argument("--vis"     , "-g").metavar("MACRO").help("switch from batch mode to GUI, executing MACRO").implicit_value(std::string{"vis.mac"});
  args.add_argument("--macro-path", "-m").metavar("MACROPATHS").help("Add MACROPATHS to Geant4 macro search path").MULTIPLE; // TODO metavar does not appear in help

  try {
    args.parse_args(argc, argv);
  } catch(const std::runtime_error& err) {
    std::cerr << err.what() << std::endl;
    std::cerr << args;
    throw err;
  }

  return args;
}

#undef MULTIPLE

namespace nain4 {

// TODO forward to private constructor that accepts parser as parameter, for direct initialization
ui::ui(const std::string& program_name, int argc, char** argv, bool warn_empty_run)
:
  args{define_args(program_name, argc, argv)},
  n_events{},
  early{args.get<std::vector<std::string>>("--early")},
  late {args.get<std::vector<std::string>>("--late" )},
  vis_macro{},
  argc{argc},
  argv{argv},
  g4_ui{*G4UImanager::GetUIpointer()}
{
  if (auto n = args.present("--beam-on")) { n_events = parse_beam_on(n.value()); }
  if (args.is_used("-g")) { vis_macro = args.get("-g"); }

  // Here we use std::string because G4String does not work
  auto macro_paths = args.get<std::vector<std::string>>("--macro-path");
  for (auto& path : macro_paths) {
    prepend_path(path);
  }

  if (warn_empty_run && ! (n_events.has_value() || vis_macro.has_value())) {
    std::cerr << "'" + program_name + "' is not going to do anything interesting without some command-line arguments.\n\n";
    std::cerr << args << std::endl;
  }

}

void ui::run() {
  if (n_events.has_value() && !vis_macro.has_value()) {
    beam_on(n_events.value());
  }

  if (vis_macro.has_value()) {
    G4UIExecutive ui_executive{argc, argv};
    G4VisExecutive vis_manager;
    vis_manager.Initialize();
    run_vis_macro();
    if (n_events.has_value()) { beam_on(n_events.value()); }
    ui_executive.SessionStart();
  }

}

void ui::run_many(const std::vector<std::string> macros_and_commands, const G4String& prefix) {
  for (const auto& item: macros_and_commands) {
    if (item.ends_with(".mac")) { run_macro(item, "CLI-"+prefix           ); }
    else                        { command  (item, "CLI-"+prefix, "command"); }
  }
}

void ui::run_macro(const G4String& filename, const G4String& prefix) {
  command("/control/execute " + filename, prefix, "macro");
}

void ui::command (const G4String& command, const G4String& prefix, const G4String& kind) {
  auto status = g4_ui.ApplyCommand(command);
  if (status != fCommandSucceeded) {
    std::string reason =
      (status == fCommandNotFound)          ? "command not found"             :
      (status == fIllegalApplicationState)  ? "illegal application state"     :
      (status == fParameterOutOfRange)      ? "parameter out of range"        :
      (status == fParameterUnreadable)      ? "parameter unreadable"          :
      (status == fParameterOutOfCandidates) ? "parameter not in accepted set" :
      (status == fAliasNotFound)            ? "alias not found"               :
                                              "this should not have happened!";
    std::string message{prefix + ' ' + kind + " failed: (" + command + ") because: " + reason};
    std::cerr << message << std::endl;
    throw std::runtime_error{message};
  }
  std::cout << "nain4::ui:"
            << std::setw(15) << prefix << ' '
            << std::setw( 7) << kind
            << " succeeded: (" << command  << ')' << std::endl;
}


} // namespace nain4
