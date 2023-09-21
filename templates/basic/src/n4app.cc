// ANCHOR: full_file
// ANCHOR: includes
#include "nain4.hh"
#include "g4-mandatory.hh"
#include "n4_ui.hh"
#include "n4-volumes.hh"

#include <G4GenericMessenger.hh>

#include <G4PrimaryParticle.hh>
#include <G4String.hh>
#include <G4SystemOfUnits.hh>   // physical units such as `m` for metre
#include <G4Event.hh>           // needed to inject primary particles into an event
#include <G4Box.hh>             // for creating shapes in the geometry
#include <G4Sphere.hh>          // for creating shapes in the geometry
#include <FTFP_BERT.hh>         // our choice of physics list
#include <G4RandomDirection.hh> // for launching particles in random directions


#include <G4ThreeVector.hh>
#include <cstdlib>
// ANCHOR_END: includes

struct my {
  G4double       straw_radius{0.1 * m};
  G4double      bubble_radius{0.2 * m};
  G4double      socket_rot   {-90 * deg};
  G4String      particle_name{"geantino"};
  G4double      particle_energy{511 * keV};
  G4ThreeVector particle_dir {};
};

// ANCHOR: my_generator
auto my_generator(const my& my) {
  return [&](G4Event* event) {
    auto particle_type = n4::find_particle(my.particle_name);
    auto vertex = new G4PrimaryVertex();
    auto r = my.particle_dir.mag2() > 0 ? my.particle_dir : G4RandomDirection();
    vertex -> SetPrimary(new G4PrimaryParticle(
                           particle_type,
                           r.x(), r.y(), r.z(),
                           my.particle_energy
                         ));
    event  -> AddPrimaryVertex(vertex);
  };
}
// ANCHOR_END: my_generator

// ANCHOR: create_actions
n4::actions* create_actions(my& my, unsigned& n_event) {
  auto my_stepping_action = [&] (const G4Step* step) {
    auto pt = step -> GetPreStepPoint();
    auto volume_name = pt -> GetTouchable() -> GetVolume() -> GetName();
    if (volume_name == "straw" || volume_name == "bubble") {
      auto pos = pt -> GetPosition();
      std::cout << volume_name << " " << pos << std::endl;
    }
  };

  auto my_event_action = [&] (const G4Event*) {
     n_event++;
     std::cout << "end of event " << n_event << std::endl;
  };

  return (new n4::        actions{my_generator(my)  })
 -> set( (new n4::   event_action{                  }) -> end(my_event_action) )
 -> set(  new n4::stepping_action{my_stepping_action});
}
// ANCHOR_END: create_actions

// ANCHOR: my_geometry
auto my_geometry(const my& my) {
  auto r_bub = my.bubble_radius;
  auto r_str = my.straw_radius;
  auto water  = n4::material("G4_WATER");
  auto air    = n4::material("G4_AIR");
  auto steel  = n4::material("G4_STAINLESS-STEEL");
  auto world  = n4::box("world").cube(2*m).x(3*m).volume(water);

  n4::sphere("bubble").r(r_bub)         .place(air).in(world).at  (1.3*m, 0.8*m, 0.3*m).now();
  n4::tubs  ("straw" ).r(r_str).z(1.9*m).place(air).in(world).at_x(0.2*m              ).now();

  n4       ::sphere("socket-cap" ).r(0.3*m).phi_delta(180*deg)
    .sub(n4::box   ("socket-hole").cube(0.4*m))
    .name("socket")
    .place(steel).in(world).rotate_x(my.socket_rot).at(1*m, 0, 0.7*m).now();

  return n4::place(world).now();
}
// ANCHOR_END: my_geometry

// ANCHOR: pick_cli_arguments
int main(int argc, char* argv[]) {
  // ANCHOR_END: pick_cli_arguments
  unsigned n_event = 0;

  my my;

  // The trailing slash after '/my_geometry' is CRUCIAL: without it, the
  // messenger violates the principle of least surprise.
  auto messenger = new G4GenericMessenger{nullptr, "/my/", "docs: bla bla bla"};
  messenger -> DeclarePropertyWithUnit("straw_radius"      , "m"  , my. straw_radius  );
  messenger -> DeclarePropertyWithUnit("bubble_radius"     , "m"  , my.bubble_radius  );
  messenger -> DeclarePropertyWithUnit("socket_rot"        , "deg", my.socket_rot     );
  messenger -> DeclarePropertyWithUnit("particle_energy"   , "keV", my.particle_energy);
  messenger -> DeclareProperty        ("particle"          ,        my.particle_name  );
  messenger -> DeclareProperty        ("particle_direction",        my.particle_dir   );

  // ANCHOR: create_run_manager
  n4::run_manager::create()
    .ui("my-program-name", argc, argv)
    .macro_path("macs")
    .apply_command("/my/straw_radius 0.5 m")
    .apply_early_macro("early-hard-wired.mac")
    .apply_cli_early_macro() // CLI --early-macro executed at this point
    // .apply_command(...) // also possible after apply_early_macro

    .physics<FTFP_BERT>(0) // verbosity 0
    .geometry([&] { return my_geometry(my); })
    .actions(create_actions(my, n_event))

    .apply_command("/my/particle e-")
    .apply_late_macro("late-hard-wired.mac")
    .apply_cli_late_macro() // CLI --late-macro executed at this point
    // .apply_command(...) // also possible after apply_late_macro

    .run();


  // ANCHOR_END: create_run_manager

  // ANCHOR: build_minimal_framework
  // Important! physics list has to be set before the generator!
  // ANCHOR_END: build_minimal_framework

  // ANCHOR: run
  // ANCHOR_END: run
// ANCHOR: closing_bracket
}
// ANCHOR_END: closing_bracket
// ANCHOR_END: full_file
