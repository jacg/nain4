// clang-format off

#include "nain4.hh"
#include "test_utils.hh"

#include <G4Box.hh>

#include <G4EventManager.hh>
#include <G4LogicalVolumeStore.hh>
#include <G4ParticleGun.hh>
#include <G4String.hh>
#include <G4SystemOfUnits.hh>
#include <G4TouchableHistory.hh>
#include <G4UserRunAction.hh>
#include <G4UserEventAction.hh>
#include <G4UserSteppingAction.hh>
#include <G4VSensitiveDetector.hh>
#include <G4VUserActionInitialization.hh>
#include <G4VUserDetectorConstruction.hh>
#include <G4VUserPrimaryGeneratorAction.hh>
#include <FTFP_BERT.hh>
#include <G4EmStandardPhysics_option4.hh>
#include <G4OpticalPhysics.hh>


#include <Randomize.hh>

#include <catch2/catch_test_macros.hpp>

#include <tuple>
#include <algorithm>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wshadow"

// ----- Fundamental requirements of Geant4 --------------------------------------

// Geant4 apps need to register 3 mandatory user-defined classes with the Geant4
// run manager, before any Geant4 app can run. These classes must be subclasses
// of abstract base classes provided by Geant4:
//
// - G4VUserDetectorConstruction : geometry of the detector
// - G4VUserPhysicsList          : physical processes
// - G4VUserActionInitialization : primary particles + other hooks
//
// The last of these MUST also define a source of particles by implementing a
// subclass of
//
// - G4VUserPrimaryGeneratorAction
//
// and MAY also hook into various stages of the Geant4 simulation process by
// providing subclasses of
//
// - G4UserEventAction
// - G4UserRunAction
// - G4UserSteppingAction
//
// Note that these last three are concrete (not abstract) classes (classes which
// have a 'V' in their name just after 'G4' are abstract), which means that they
// can be used without the need to define a subclass: they satisfy the
// requirements of the framework, but do nothing interesting: they define the
// interface which can be used to inject interesting behaviour into the process.

// ----- Description of this example ---------------------------------------------

// This example contains two thin, rectangular slabs. One acts as a source of
// geantinos (particles which do not interact with anything, merely propagate);
// the other acts as a detector.
//
// They are both normal to the x-axis.
//
// + The detector is centred on the x-axis and has an area of 1m x 1m.
// + The source's size and position are chosen when the geometry is constructed.
//
// The source is located at x = -50 cm; the detector at x = +50 cm.
//
// Geantinos emerge from random positions in the source, and propagate along the
// x-axis. Thus, all hits in the detector should be limited to the y-z-extent of
// the source. This property is verified by the test.

// ----- Sensitive Detector ------------------------------------------------------

// Sensitive detector: can be attached to volumes that make up the geometry, and
// used to collect information about particles which reach those volumes
class sensitive : public G4VSensitiveDetector {
public:
  sensitive(G4String name) : G4VSensitiveDetector{name} { reset_observations(); }

  // G4 will call this method whenever a simulation step takes place in a volume
  // to which an instance of this class has been attached by
  // SetSensitiveDetector(). Thus, this method is used to collect information
  // about particles which reach the detector.
  G4bool ProcessHits(G4Step* step, G4TouchableHistory*) override {
    // Store the min and max y and z positions of particles reaching the detector
    auto pos = step -> GetPreStepPoint() -> GetPosition();
    auto y = pos.y();
    auto z = pos.z();
    y_min = std::min(y, y_min);
    y_max = std::max(y, y_max);
    z_min = std::min(z, z_min);
    z_max = std::max(z, z_max);

    ++detected_particles;

    return true; // TODO what is the meaning of this?
  }

  void reset_observations() {
    y_min =   std::numeric_limits<G4double>::infinity();
    y_max = - std::numeric_limits<G4double>::infinity();
    z_min =   std::numeric_limits<G4double>::infinity();
    z_max = - std::numeric_limits<G4double>::infinity();
    detected_particles = 0;
  }

public:
  G4double y_min;
  G4double y_max;
  G4double z_min;
  G4double z_max;
  unsigned detected_particles;
};

// ----- The geometry ------------------------------------------------------------

// Both the source and the detector are 1cm thick slabs, oriented normal to the
// x-axis, separated by 1m.
//
// The detector is 1m x 1m; the source's size and orientation is specified in
// the constructor of the geometry.
//
// The source slab and the detector slab are contained in a cuboidal world
// volume with a 10% margin around its contents.
class geometry : public G4VUserDetectorConstruction {
public:
  geometry(G4double y_min, G4double y_max, G4double z_min, G4double z_max )
    : y_min{y_min}, y_max{y_max}, z_min{z_min}, z_max{z_max} {}

  G4VPhysicalVolume* Construct() {
    // --- Use meaningless materials in this example ---------------------------
    auto air = nain4::material("G4_AIR");

    // --- Dimensions of the component volumes ---------------------------------
    auto source_y    =  y_max - y_min;
    auto source_z    =  z_max - z_min;
    auto source_cy   = (y_min + y_max) / 2;
    auto source_cz   = (z_min + z_max) / 2;
    auto dx          =   1*cm;
    auto detector_yz = 100*cm;
    auto sep         = 100*cm;
    auto world_x     = 1.1 * sep + dx;
    auto world_yz    = 1.1 * detector_yz;

    // --- The logical volumes that make up the geometry
    auto detector = nain4::volume<G4Box>("detector", air,      dx/2, detector_yz/2, detector_yz/2);
    auto source   = nain4::volume<G4Box>("source"  , air,      dx/2,   source_y /2,    source_z/2);
    auto world    = nain4::volume<G4Box>("world"   , air, world_x/2,    world_yz/2,    world_yz/2);

    // --- Make the detector volume sensitive ----------------------------------
    detector->SetSensitiveDetector(new sensitive("/sensitive"));

    // --- Establish the geometrical relationship between the volumes ----------
    nain4::place(source)  .in(world).at({-sep/2, source_cy, source_cz}).now();
    nain4::place(detector).in(world).at({ sep/2,         0,         0}).now();
    return nain4::place(world)                                         .now();
  }

private:
  G4double y_min, y_max, z_min, z_max;
};

// ----- Primary particle generator ----------------------------------------------

// Particles are generated at random points in the source slab, and shot along
// the x axis, towards the target slab.
class primary_generator : public G4VUserPrimaryGeneratorAction {
public:
  primary_generator(unsigned n_gun, unsigned n_loop)
  : gun{(G4int)n_gun}
  , primaries_to_generate{n_loop}
  {
    // Geantinos don't interact with anything, so we can easily predict their
    // trajectory, which is very useful for writing the test.
    gun.SetParticleDefinition(nain4::find_particle("geantino"));
    // Shoot along the x-axis, so the y and z coordinates won't change during
    // flight; again, very useful for the test.
    gun.SetParticleMomentumDirection({1, 0, 0});
  }

  // G4 calls this when it needs primary particles
  void GeneratePrimaries(G4Event* event) override {
    if (!source) {
      auto source_physical = nain4::find_physical("source");
      auto source_logical = source_physical -> GetLogicalVolume();
      auto box = dynamic_cast<G4Box*>(source_logical->GetSolid());
      if (box) {
        auto t = source_physical -> GetTranslation();
        auto dx = box -> GetXHalfLength() * 2;
        auto dy = box -> GetYHalfLength() * 2;
        auto dz = box -> GetZHalfLength() * 2;
        source = std::make_tuple(t.x(), t.y(), t.z(), dx, dy, dz);
      }
    }
    auto [tx, ty, tz, dx, dy, dz] = source.value();
    for (unsigned n=0; n<primaries_to_generate; ++n) {
      auto x = tx + dx * (G4UniformRand() - 0.5);
      auto y = ty + dy * (G4UniformRand() - 0.5);
      auto z = tz + dz * (G4UniformRand() - 0.5);
      gun.SetParticlePosition({x, y, z});
      gun.GeneratePrimaryVertex(event);
    }
  }

private:
  G4ParticleGun gun;
  std::optional<std::tuple<G4double, G4double, G4double, G4double, G4double, G4double>> source;
  unsigned primaries_to_generate;
};

// ----- Particle Generator and other hooks --------------------------------------

// The primary particle generator MUST be set here in the Build method.
// Optionally we MAY also set other user hooks.
class actions : public G4VUserActionInitialization {
public:
  actions(unsigned n_gun, unsigned n_loop)
    : G4VUserActionInitialization()
    , n_gun{n_gun}
    , n_loop{n_loop}
  {}

  // This one is relevant for multi-threaded mode TODO discuss
 void BuildForMaster() const override {
    //SetUserAction(new G4UserRunAction);
  };

 void Build() const override {
    // Geant4 will crash at runtime without this
    SetUserAction(new primary_generator{n_gun, n_loop});

    // Other possibilities include subclasses of the following. The base classes
    // provided by G4 do nothing, so leaving them out altogether has the same
    // effect as setting them.
    SetUserAction(new G4UserRunAction);
    SetUserAction(new G4UserEventAction);
    SetUserAction(new G4UserSteppingAction);
  }
private:
  unsigned n_gun;
  unsigned n_loop;
};

// ----- TESTS -------------------------------------------------------------------

TEST_CASE("trivial app", "[app]") {

  // Position and extent of the source slab
  auto y_min =  5*cm;  auto y_max =  7*cm;
  auto z_min = 10*cm;  auto z_max = 15*cm;

  // The total number of particles generated can be influenced in 3 different places
  unsigned n_gun = 3; unsigned n_beam_on = 5; unsigned n_inside_generator = 7;
  auto expected_hits = n_gun * n_beam_on * n_inside_generator;


  auto hush = std::make_unique<n4::silence>(std::cout);
  auto run_manager = n4::make_run_manager()
    .physics(default_physics_lists())
    .geometry<geometry>(y_min, y_max, z_min, z_max)
    .actions <actions >(n_gun, n_inside_generator)
    .init();

  run_manager.beam_on(n_beam_on);

  hush = nullptr;

  // Verify that all the geantinos coming out from the source, hit the detector
  // in the source's x-axis-projection onto the detector
  auto sd = dynamic_cast<sensitive*>(nain4::find_logical("detector") -> GetSensitiveDetector());
  CHECK(sd->y_min >= y_min);
  CHECK(sd->y_max <= y_max);
  CHECK(sd->z_min >= z_min);
  CHECK(sd->z_max <= z_max);

  // Verify that all generated particles arrived at the detector
  CHECK(sd->detected_particles == expected_hits);
}

#pragma GCC diagnostic pop
