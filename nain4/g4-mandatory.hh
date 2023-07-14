#ifndef nain4_g4_mandatory_hh
#define nain4_g4_mandatory_hh

#include <G4Box.hh>
#include <G4ClassificationOfNewTrack.hh>
#include <G4ParticleGun.hh>
#include <G4StackManager.hh>
#include <G4Track.hh>
#include <G4UserEventAction.hh>
#include <G4UserRunAction.hh>
#include <G4UserStackingAction.hh>
#include <G4UserSteppingAction.hh>
#include <G4UserTrackingAction.hh>
#include <G4VUserActionInitialization.hh>
#include <G4VUserEventInformation.hh>
#include <G4VUserDetectorConstruction.hh>
#include <G4VUserPrimaryGeneratorAction.hh>

#include <globals.hh>

#include <vector>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wshadow"

namespace nain4 {

// ----- run_action -----------------------------------------------------------------
struct run_action : public G4UserRunAction {
  using action_t   = std::function<void   (G4Run const*)>;
  using generate_t = std::function<G4Run* (            )>;

  G4Run* GenerateRun() override {
    if (generate_) { return generate_(); }
    else { return G4UserRunAction::GenerateRun(); }
  }
  void BeginOfRunAction(const G4Run* run) override { if (begin_) begin_(run); }
  void   EndOfRunAction(const G4Run* run) override { if   (end_)   end_(run); }

  run_action* generate(generate_t action) { generate_ = action; return this; }
  run_action*    begin(action_t   action) {    begin_ = action; return this; }
  run_action*      end(action_t   action) {      end_ = action; return this; }
private:
  generate_t generate_;
  action_t   begin_;
  action_t   end_;
};

// ----- event_action ---------------------------------------------------------------
struct event_action : public G4UserEventAction {
  using action_t = std::function<void (G4Event const*)>;

  void BeginOfEventAction(G4Event const* event) override { if (begin_) begin_(event); }
  void   EndOfEventAction(G4Event const* event) override { if   (end_)   end_(event); }

  event_action* begin(action_t action) { begin_ = action; return this; }
  event_action*   end(action_t action) {   end_ = action; return this; }
private:
  action_t begin_;
  action_t end_;
};

// ----- stacking_action ------------------------------------------------------------
struct stacking_action : public G4UserStackingAction {
  using classify_t = std::function<G4ClassificationOfNewTrack (G4Track const *)>;
  using stage_t    = std::function<void              (G4StackManager * const  )>;
  using voidvoid_t = std::function<void              (                        )>;

  G4ClassificationOfNewTrack ClassifyNewTrack(G4Track const* track) override {
    if (classify_) {return classify_(track); }
    else { return G4UserStackingAction::ClassifyNewTrack(track); }
  }
  void NewStage       () override { if   (stage_)   stage_(stackManager); }
  void PrepareNewEvent() override { if (prepare_) prepare_(); }

  stacking_action*   classify(classify_t a) { classify_ = a; return this; }
  stacking_action* next_stage(   stage_t a) {    stage_ = a; return this; }
  stacking_action* next_event(voidvoid_t a) {  prepare_ = a; return this; }
private:
  classify_t classify_;
     stage_t stage_;
  voidvoid_t prepare_;
};

// ----- tracking_action ------------------------------------------------------------
struct tracking_action : public G4UserTrackingAction {
  using action_t = std::function<void (const G4Track*)>;
  void  PreUserTrackingAction(const G4Track* track) override { if ( pre_)  pre_(track); }
  void PostUserTrackingAction(const G4Track* track) override { if (post_) post_(track); }

  tracking_action* pre (action_t a) { pre_  = a; return this; }
  tracking_action* post(action_t a) { post_ = a; return this; }
private:
  action_t pre_;
  action_t post_;
  };

// ----- stepping_action ------------------------------------------------------------
struct stepping_action : public G4UserSteppingAction {
  using action_t = std::function<void (const G4Step*)>;
  // Only one method, so set it in constructor.
  stepping_action(action_t action) : action{action} {}
  void UserSteppingAction(const G4Step* step) override { action(step); }
private:
  action_t action;
};

// ----- primary generator ----------------------------------------------------------
struct generator : public G4VUserPrimaryGeneratorAction {
  using function = std::function<void (G4Event*)>;
  generator(function fn = geantino_along_x) : doit{fn} {}
  void GeneratePrimaries(G4Event* event) override { doit(event); };
private:
  function const doit;
  static void geantino_along_x(G4Event*);
};

// ----- actions --------------------------------------------------------------------
struct actions : public G4VUserActionInitialization {
  actions(G4VUserPrimaryGeneratorAction* generator) : generator_{generator} {}
  actions(generator::function fn) : generator_{new generator(fn)} {}
  // See B1 README for explanation of the role of BuildForMaster in multi-threaded mode.
  //void BuildForMaster() const override;
  void Build() const override;

  actions* set(G4UserRunAction     * a) { run_   = a; return this; }
  actions* set(G4UserEventAction   * a) { event_ = a; return this; }
  actions* set(G4UserSteppingAction* a) { step_  = a; return this; }
  actions* set(G4UserTrackingAction* a) { track_ = a; return this; }
  actions* set(G4UserStackingAction* a) { stack_ = a; return this; }

private:
  G4VUserPrimaryGeneratorAction* generator_;
  G4UserRunAction              * run_   = nullptr;
  G4UserEventAction            * event_ = nullptr;
  G4UserSteppingAction         * step_  = nullptr;
  G4UserTrackingAction         * track_ = nullptr;
  G4UserStackingAction         * stack_ = nullptr;
};

// ----- geometry -------------------------------------------------------------------
// Quickly implement G4VUserDetectorConstruction: just instantiate this class
// with a function which returns the geometry
struct geometry : public G4VUserDetectorConstruction {
  using construct_fn = std::function<G4VPhysicalVolume* ()>;
  geometry(construct_fn f) : construct{f} {}
  G4VPhysicalVolume* Construct() override { return construct(); }
private:
  construct_fn construct;
};

// --------------------------------------------------------------------------------
// TODO Currently has a hard-wired storing of steps: generalize :
// The subclass via which G4 insists that you manage the information that
// interests you about an event.
struct event_data : public G4VUserEventInformation {
  event_data(std::vector<G4Step>&& hits) : G4VUserEventInformation(), hits{std::move(hits)} {}
  ~event_data() override {};
  void Print() const override {/* purely virtual in superclass */};
  void set_hits(std::vector<G4Step>&& sensor_hits) { hits = std::move(sensor_hits); }
  std::vector<G4Step>& get_hits() { return hits; }
private:
  std::vector<G4Step> hits;
};

} // namespace nain4

namespace n4 { using namespace nain4; }

#pragma GCC diagnostic pop

#endif
