#include <n4-all.hh>

#include <catch2/catch_test_macros.hpp>


TEST_CASE("nain vis_attributes", "[nain][vis_attributes]") {
  // Utility for more convenient configuration of G4VisAttributes
  // TODO could do with more extensive testing
  using nain4::vis_attributes;
  auto convenient = vis_attributes{}
    .visible(true)
    .colour({1,0,0})
    .start_time(1.23)
    .end_time(4.56)
    .force_line_segments_per_circle(20)
    .force_solid(true)
    .force_wireframe(false);
  auto pita = G4VisAttributes{};
  pita.SetVisibility(true);
  pita.SetColour({1,0,0});
  pita.SetStartTime(1.23);
  pita.SetEndTime(4.56);
  pita.SetForceLineSegmentsPerCircle(20);
  pita.SetForceSolid(true);
  pita.SetForceWireframe(false);
  CHECK(convenient == pita);

  // The meaning of the different constructors

  // Default constructor sets colour: white, visibility: true
  CHECK(vis_attributes{} == vis_attributes{}.colour({1,1,1}).visible(true));
  // Can set colour via constructor
  CHECK(vis_attributes         {{0,1,0}} ==
        vis_attributes{}.colour({0,1,0}));
  // Can set visibility via constructor
  CHECK(vis_attributes          {true} ==
        vis_attributes{}.visible(true));

  CHECK(vis_attributes          {false} ==
        vis_attributes{}.visible(false));
  // Can set both visibility and colour via constructor
  CHECK(vis_attributes          {false ,       {1,1,0}} ==
        vis_attributes{}.visible(false).colour({1,1,0}));
}

TEST_CASE("nain vis_attributes shape", "[nain][vis_attributes]") {
  auto attrs = n4::vis_attributes{}.visible(true).colour(G4Colour::Red());
  auto check_props = [] (auto vol) {
    auto vis = vol -> GetVisAttributes();
    CHECK(vis -> IsVisible());
    CHECK(vis -> GetColour()    == G4Colour::Red());
  };
  auto air   = n4::material("G4_AIR");
  auto box1 = n4::box("box1").cube(1).vis( attrs               ).volume(air);
  auto box2 = n4::box("box2").cube(1).vis(&attrs               ).volume(air);
  auto box3 = n4::box("box3").cube(1).vis(true, G4Colour::Red()).volume(air);

  check_props(box1);
  check_props(box2);
  check_props(box3);
}
