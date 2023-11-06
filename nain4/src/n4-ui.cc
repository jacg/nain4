#include <algorithm>
#include <n4-ui.hh>
#include <n4-run-manager.hh>

#include <G4String.hh>
#include <G4UIExecutive.hh>
#include <G4UIcommandStatus.hh>
#include <G4UImanager.hh>
#include <G4VisExecutive.hh>
#include <G4VisManager.hh>

#include <argparse/argparse.hpp>

#include <cstdlib>
#include <initializer_list>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wshadow"

unsigned parse_beam_on(const std::string&  arg) {
  auto parsed = std::stoi(arg.c_str());
  if (parsed < 0) { throw std::runtime_error{std::string{"--beam-on requires an unsigned integer, you gave '"} + arg + "'"}; }
  return static_cast<unsigned>(parsed);

}

#define MULTIPLE nargs(argparse::nargs_pattern::at_least_one).append()
#define ANY      nargs(argparse::nargs_pattern::any         ).append()

const std::string default_vis_macro{"vis.mac"};

nain4::internal::cli_and_err nain4::internal::define_args(const std::string& program_name, int argc, char** argv) {
  auto args = std::make_unique<argparse::ArgumentParser>(program_name);
  args->add_argument("--beam-on" , "-n").metavar("N"    ).help("/run/beamOn N");
  args->add_argument("--early"   , "-e").metavar("ITEMS").help("execute ITEMS before run manager instantiation").MULTIPLE;
  args->add_argument("--late"    , "-l").metavar("ITEMS").help("execute ITEMS  after run manager instantiation").MULTIPLE;
  args->add_argument("--vis"     , "-g").metavar("ITEMS").help("switch from batch mode to GUI, executing ITEMS").ANY
    .default_value(std::vector<std::string>{default_vis_macro});
  args->add_argument("--macro-path", "-m").metavar("MACROPATHS").help("Add MACROPATHS to Geant4 macro search path").MULTIPLE;

  try {
    args->parse_args(argc, argv);
  } catch(const std::runtime_error& err) {
    return nain4::internal::cli_and_err{std::move(args), {err}};
  }
  return nain4::internal::cli_and_err{std::move(args), {}};
}

#undef MULTIPLE

bool is_macro(const std::string& e) { return e.ends_with(".mac"); };

void nain4::internal::exit_on_err(nain4::internal::may_err err) {
  if (err.has_value()) {
    std::cerr << "\n\n" << err.value().what() << "\n\n";
    exit(EXIT_FAILURE);
  }
}

n4::unique_argparse return_or_exit(nain4::internal::cli_and_err yyy) {
  if (yyy.err.has_value()) {
    std::cerr
      << yyy.cli
      << "\n\nCLI arguments error: " << yyy.err.value().what() << "\n\n";
    exit(EXIT_FAILURE);
  }
  return std::move(yyy.cli);
}

namespace nain4 {

ui::ui(const std::string& program_name, int argc, char** argv, bool warn_empty_run)
  : ui(program_name, argc, argv, return_or_exit(internal::define_args(program_name, argc, argv)), warn_empty_run)
{}

ui::ui(const std::string& program_name, int argc, char** argv, unique_argparse cli, bool warn_empty_run)
  : n_events{}
  , early{cli->get<std::vector<std::string>>("--early")}
  , late {cli->get<std::vector<std::string>>("--late" )}
  , vis  {cli->get<std::vector<std::string>>("--vis"  )}
  , use_graphics{cli->is_used("--vis")}
  , argc{argc}
  , argv{argv}
  , g4_ui{*G4UImanager::GetUIpointer()}
{
  if (auto n = cli->present("--beam-on")) { n_events  = parse_beam_on(n.value()); }

  // Here we use std::string because G4String does not work
  auto macro_paths = cli->get<std::vector<std::string>>("--macro-path");
  for (auto& path : macro_paths) {
    prepend_path(path);
  }

  if (cli->is_used("--vis")) {
    auto& items = vis; // = args.get<std::vector<std::string>>("--vis");

    bool macro_file_specified = std::find_if(begin(items), end(items), is_macro) != end(items);
    if (! macro_file_specified) { items.insert(begin(items), default_vis_macro); }
  }

  if (warn_empty_run && ! (n_events.has_value() || use_graphics)) {
    std::cerr << "'" + program_name + "' is not going to do anything interesting without some command-line arguments.\n\n";
    std::cerr << cli << std::endl;
  }

}

void ui::run() {
  if (n_events.has_value() && !use_graphics) {
    beam_on(n_events.value());
  }

  if (use_graphics) {
    G4UIExecutive ui_executive{argc, argv};
    G4VisExecutive vis_manager;
    vis_manager.Initialize();
    internal::exit_on_err(run_vis());
    if (n_events.has_value()) { beam_on(n_events.value()); }
    ui_executive.SessionStart();
  }
}

internal::may_err ui::run_many(const std::vector<std::string> macros_and_commands, const G4String& prefix) {
  for (const auto& item: macros_and_commands) {
    auto error = is_macro(item)                     ?
      run_macro(item, "CLI-"+prefix               ) :
      command  (item, "CLI-"+prefix, kind::command) ;
    if (error.has_value()) { return error; }
  }
  return {};
}

internal::may_err ui::run_macro(const G4String& filename, const G4String& prefix) {
  return command("/control/execute " + filename, prefix, kind::macro);
}


std::string ui::repr(const kind kind) {
  switch (kind) {
    case kind::command: return "command";
    case kind::macro  : return "macro";
    case kind::beam_on: return "beam_on";
  }
  return "UNREACHABLE";
}

internal::may_err ui::command (const G4String& command, const G4String& prefix, const kind kind) {
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
    std::string message{prefix + ' ' + repr(kind) + " rejected: (" + command + ") because: " + reason};
    std::cerr << message << std::endl;
    return std::runtime_error{message};
  }
  std::cout << "nain4::ui:"
            << std::setw(15) << prefix << ' '
            << std::setw( 7) << repr(kind)
            << " accepted: (" << command  << ')' << std::endl;
  return {};
}

test::argcv::argcv(std::initializer_list<std::string> args): argc{static_cast<int>(args.size())} {
  argv = new char*[argc+1];
  int i = 0;
  for (const auto& arg: args) {
    auto source = arg.c_str();
    auto copy_of_arg_owned_by_us = std::make_unique<char[]>(std::strlen(source)+1); // +1 for NULL terminator
    std::strcpy(copy_of_arg_owned_by_us.get(), source);
    argv[i++] = copy_of_arg_owned_by_us.get();
    owners.push_back(std::move(copy_of_arg_owned_by_us));
  }
  argv[i] = NULL;
  //assert(i == argc);
}

void test::report_args(std::ostream& out, int argc, char **argv) {
  out << "argc: " << argc << std::endl;
  for (int i = 0; i < argc; i++) {
    out << i << "  " << argv[i] << std::endl;
  }
}

} // namespace nain4

#pragma GCC diagnostic pop
