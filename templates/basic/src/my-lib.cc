#include "G4PVPlacement.hh"
#include <my-lib.hh>

my::my()
  // The trailing slash after '/my_geometry' is CRUCIAL: without it, the
  // messenger violates the principle of least surprise.
  : messenger{new G4GenericMessenger{nullptr, "/my/", "docs: bla bla bla"}}
{
  messenger -> DeclarePropertyWithUnit("straw_radius"      , "m"  , this ->  straw_radius  );
  messenger -> DeclarePropertyWithUnit("bubble_radius"     , "m"  , this -> bubble_radius  );
  messenger -> DeclarePropertyWithUnit("socket_rot"        , "deg", this -> socket_rot     );
  messenger -> DeclarePropertyWithUnit("particle_energy"   , "keV", this -> particle_energy);
  messenger -> DeclareProperty        ("particle"          ,        this -> particle_name  );
  messenger -> DeclareProperty        ("particle_direction",        this -> particle_dir   );
  messenger -> DeclareProperty        ("physics_verbosity" ,        this -> phys_verbosity );
}


// ANCHOR: my_generator
std::function<void (G4Event*)> my_generator(const my& my) {
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
G4PVPlacement* my_geometry(const my& my) {
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
