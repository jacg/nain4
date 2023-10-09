#include <n4-defaults.hh>
#include <n4-mandatory.hh>
#include <n4-run-manager.hh>
#include <n4-shape.hh>

// Solids
#include <FTFP_BERT.hh>
#include <G4Box.hh>
#include <G4Cons.hh>
#include <G4PVPlacement.hh>
#include <G4RunManager.hh>
#include <G4Trd.hh>

// Managers
#include <G4NistManager.hh>

// Units
#include <G4SystemOfUnits.hh>
#include <G4UnitsTable.hh>

// Other G4
#include <G4Gamma.hh>
#include <G4Material.hh>
#include <G4VUserDetectorConstruction.hh>

#include <catch2/catch_approx.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_contains.hpp>

#include <algorithm>
#include <initializer_list>
#include <memory>
#include <stdexcept>

using Catch::Approx;

// Many of the tests below check physical quantities. Dividing physical
// quantities by their units gives raw numbers which are easily understandable
// by a human reader, which is important test failures are reported. Sometimes
// this gives rise to the apparently superfluous division by the same unit on
// both sides of an equation, in the source code.

#include <numeric>

char *fake_argv[] = { (char*)"progname-aaa", NULL };

using namespace n4::test;

TEST_CASE("nain run_manager build_fn initialization", "[nain][run_manager]") {
  auto hush = n4::silence{std::cout};

  auto rm = n4::run_manager::create()
     .ui("progname", 1, fake_argv, false)
     .physics(default_physics_lists)
     .geometry(water_box)
     .actions(do_nothing);
}


struct dummy_geometry : G4VUserDetectorConstruction {
  dummy_geometry(double, double, double) {}
  G4PVPlacement* Construct() {
    return water_box();
  }
};

struct dummy_actions : G4VUserActionInitialization {
  dummy_actions(int) {}
  void Build() const override {SetUserAction(new n4::generator([] (auto) {}));}
};

TEST_CASE("nain run_manager construct initialization", "[nain][run_manager]") {
  auto hush = n4::silence{std::cout};

  n4::run_manager::create()
     .ui("progname", 1, fake_argv, false)
     .physics<FTFP_BERT>(0) // verbosity 0
     .geometry<dummy_geometry>(1., 2., 3.)
     .actions<dummy_actions>(10)
     .run();
}

TEST_CASE("nain run_manager basic initialization", "[nain][run_manager]") {
  auto hush = n4::silence{std::cout};

  n4::run_manager::create()
     .ui("progname", 1, fake_argv, false)
     .physics (new FTFP_BERT{0}) // verbosity 0
     .geometry(new dummy_geometry{1., 2., 3.})
     .actions<dummy_actions>(10)
     .run();
}


// TEST_CASE("nain run_manager get", "[nain][run_manager]") {
//   default_run_manager().run();
//   auto& rm_reference = n4::run_manager::get();

//   CHECK(&rm_value == &rm_reference);
// }

// These tests are commented out because we still don't know how to get Catch2
// to assert failures.

// TEST_CASE("nain run_manager no_world_volume", "[nain][run_manager]") {
//   auto my_geometry = [] {
//     auto air = n4::material("G4_AIR");
//     auto box_world    = n4::box{"world"   }.cube(1).volume(air);
//     auto box_daughter = n4::box{"daughter"}.cube(1).volume(air);

//     // We place the daughter
//     // But we forget to place the mother
//     // And we should get an error
//     return n4::place(box_daughter).in(box_world).now();
//     //     n4::place(box_mother  ).             .now();
//   };

//   auto hush = n4::silence{std::cout};
//   n4::run_manager::create()
//      .physics<FTFP_BERT>(0)
//      .geometry(my_geometry)
//      .actions(do_nothing)
//      .initialize();
// }

// TEST_CASE("nain run_manager too_many_world_volumes", "[nain][run_manager]") {
//   auto my_geometry = [] {
//     auto air = n4::material("G4_AIR");
//     auto box_world_1 = n4::box{"world-1"}.cube(1).volume(air);
//     auto box_world_2 = n4::box{"world-2"}.cube(1).volume(air);

//     // No `.in` call defaults to world volume
//            n4::place(box_world_1).now();
//     return n4::place(box_world_2).now();
//   };

//   auto hush = n4::silence{std::cout};
//   n4::run_manager::create()
//      .physics<FTFP_BERT>(0)
//      .geometry(my_geometry)
//      .actions(do_nothing)
//      .initialize();

// }


TEST_CASE("nain run_manager exactly_one_world_volumes", "[nain][run_manager]") {
  auto my_geometry = [] {
    auto air = n4::material("G4_AIR");
    auto box_daughter = n4::box{"daughter"}.cube(1).volume(air);
    auto box_world    = n4::box{"world"   }.cube(1).volume(air);

    // No `.in` call defaults to world volume
    /*   */n4::place(box_daughter).in(box_world).now();
    return n4::place(box_world   ).now();
  };

  auto hush = n4::silence{std::cout};
  n4::run_manager::create()
     .ui("progname", 1, fake_argv, false)
     .physics<FTFP_BERT>(0)
     .geometry(my_geometry)
     .actions(do_nothing)
     .run();
}

TEST_CASE("cli macropath with values", "[nain][cli][macropath]") {
  auto hush = n4::silence{std::cout};

  char *argv[] = { (char*)"progname-aaa"
                 , (char*)"--macro-path"
                 , (char*)"path-aaa"
                 , (char*)"--macro-path"
                 , (char*)"path-bbb"
                 , (char*)"path-ccc"
                 , NULL
                 };

  auto rm = n4::run_manager::create().ui("progname", 6, argv, false);

  auto search_path = G4UImanager::GetUIpointer() -> GetMacroSearchPath();
  std::cerr << search_path << std::endl;

  CHECK(search_path.find("path-aaa") != std::string::npos);
  CHECK(search_path.find("path-bbb") != std::string::npos);
  CHECK(search_path.find("path-ccc") != std::string::npos);
}


TEST_CASE("cli without macropath", "[nain][cli][macropath]") {
  auto hush = n4::silence{std::cout};

  char *argv[] = {(char*)"progname-aaa", NULL};

  auto rm = n4::run_manager::create().ui("progname", 1, argv, false);
  auto search_path = G4UImanager::GetUIpointer() -> GetMacroSearchPath();

  CHECK(search_path == "");
}

// Utility for construction of argc/argv combination for use in n4::ui CLI tests
struct argcv {
  int    argc;
  char** argv;
  argcv(std::initializer_list<std::string> args): argc{static_cast<int>(args.size())} {
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
private:
  std::vector<std::unique_ptr<char[]>> owners;
};

TEST_CASE("cli no args", "[nain][cli]") {
  auto hush = n4::silence{std::cout};
  char *argv[] = {(char*)"progname", NULL};
  std::cerr << "                                       progname **no args**" << std::endl;
  auto rm = n4::run_manager::create().ui("progname", 1, argv, false);
}

TEST_CASE("cli implicit vis macro", "[nain][cli]") {
  auto hush = n4::silence{std::cout};
  char *argv[] = {(char*)"progname", (char*)"-g", NULL};
  std::cerr << "                                       progname -g" << std::endl;
  auto rm = n4::run_manager::create().ui("progname", 2, argv, false);
}

TEST_CASE("cli explicit vis macro", "[nain][cli]") {
  auto hush = n4::silence{std::cout};
  char *argv[] = {(char*)"progname", (char*)"-g", (char*)"aaa", NULL};
  std::cerr << "                                       progname -g aaa" << std::endl;
  auto rm = n4::run_manager::create().ui("progname", 3, argv, false);
}

TEST_CASE("cli implicit vis macro not last", "[nain][cli]") {
  auto hush = n4::silence{std::cout};
  char *argv[] = {(char*)"progname", (char*)"-g", (char*)"-n", (char*)"1", NULL};
  std::cerr << "                                       progname -g -n 1" << std::endl;
  auto rm = n4::run_manager::create().ui("progname", 4, argv, false);
}



TEST_CASE("macropath without value", "[nain][run_manager][macropath]") {
  auto hush = n4::silence{std::cout};

  char *argv[] = {(char*)"progname-aaa", (char*)"--macro-path", NULL};

  using Catch::Matchers::Contains;
  // We tried
  //   REQUIRE_THROWS_WITH( n4::run_manager::create().ui("progname", 2, argv, false)
  //                      , Contains("Too few arguments") && Contains("--macro-path"));
  // and variations on the there but nothing worked sensibly.
  REQUIRE_THROWS_AS(n4::run_manager::create().ui("progname", 2, argv, false), std::runtime_error);
}

TEST_CASE("apply command failure stops", "[nain][run_manager][command]") {
  auto hush = n4::silence{std::cout};

  char *argv[] = {(char*)"progname-aaa", (char*)"--early", (char*)"/some/rubbish", NULL};

  using Catch::Matchers::Contains;
  // We tried
  //   REQUIRE_THROWS_WITH( n4::run_manager::create().ui("progname", 2, argv, false)
  //                      , Contains("Too few arguments") && Contains("--macro-path"));
  // and variations on the there but nothing worked sensibly.
  REQUIRE_THROWS_AS(default_run_manager_with_ui_args(3, argv).run(), std::runtime_error);

}

TEST_CASE("run manager get geometry", "[run_manager][get_geometry]") {
  auto my_geometry = [] {
    auto air = n4::material("G4_AIR");
    auto box_daughter = n4::box{"daughter"}.cube(1).volume(air);
    auto box_world    = n4::box{"world"   }.cube(1).volume(air);

    // No `.in` call defaults to world volume
    /*   */n4::place(box_daughter).in(box_world).now();
    return n4::place(box_world   ).now();
  };

  auto my_generator = [] (auto) {
    auto geo_from_n4_rm = &n4::run_manager::get_geometry<n4::geometry>();
    auto geo_from_g4_rm = static_cast<const n4::geometry*>(G4RunManager::GetRunManager() -> GetUserDetectorConstruction());
    CHECK(geo_from_g4_rm == geo_from_n4_rm);
  };

  char *argv[] = {(char*)"progname-aaa", (char*)"-n", (char*)"1", NULL};
  auto hush = n4::silence{std::cout};
  n4::run_manager::create()
     .ui("progname", 3, argv, false)
     .physics<FTFP_BERT>(0)
     .geometry(my_geometry)
     .actions(my_generator)
     .run();

}


TEST_CASE("run manager get run action", "[run_manager][get_run_action]") {
  auto generator_with_check = [] (auto) {
    auto      run_action_from_n4_rm = &n4::run_manager::     get_run_action<n4::     run_action>();
    auto    event_action_from_n4_rm = &n4::run_manager::   get_event_action<n4::   event_action>();
    auto tracking_action_from_n4_rm = &n4::run_manager::get_tracking_action<n4::tracking_action>();
    auto stacking_action_from_n4_rm = &n4::run_manager::get_stacking_action<n4::stacking_action>();
    auto stepping_action_from_n4_rm = &n4::run_manager::get_stepping_action<n4::stepping_action>();

    auto      run_action_from_g4_rm = static_cast<const n4::     run_action*>(G4RunManager::GetRunManager() ->      GetUserRunAction());
    auto    event_action_from_g4_rm = static_cast<const n4::   event_action*>(G4RunManager::GetRunManager() ->    GetUserEventAction());
    auto tracking_action_from_g4_rm = static_cast<const n4::tracking_action*>(G4RunManager::GetRunManager() -> GetUserTrackingAction());
    auto stacking_action_from_g4_rm = static_cast<const n4::stacking_action*>(G4RunManager::GetRunManager() -> GetUserStackingAction());
    auto stepping_action_from_g4_rm = static_cast<const n4::stepping_action*>(G4RunManager::GetRunManager() -> GetUserSteppingAction());

    CHECK(     run_action_from_g4_rm ==      run_action_from_n4_rm);
    CHECK(   event_action_from_g4_rm ==    event_action_from_n4_rm);
    CHECK(tracking_action_from_g4_rm == tracking_action_from_n4_rm);
    CHECK(stacking_action_from_g4_rm == stacking_action_from_n4_rm);
    CHECK(stepping_action_from_g4_rm == stepping_action_from_n4_rm);
  };

  auto actions = [&generator_with_check] {
    return (new n4::actions(generator_with_check))
      -> set(new n4::     run_action)
      -> set(new n4::   event_action)
      -> set(new n4::tracking_action)
      -> set(new n4::stacking_action)
      -> set(new n4::stepping_action{[] (auto) {}});
  };

  char *argv[] = {(char*)"progname-aaa", (char*)"-n", (char*)"1", NULL};
  auto hush = n4::silence{std::cout};
  n4::run_manager::create()
     .ui("progname", 3, argv, false)
     .physics<FTFP_BERT>(0)
     .geometry(water_box)
     .actions(actions)
     .run();

}
