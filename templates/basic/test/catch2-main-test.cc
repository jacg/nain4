#include <catch2/catch_session.hpp>

int main(int argc, char** argv) {

  // ----- Catch2 session --------------------------------------------------
  int result = Catch::Session().run(argc, argv);

  // ----- Post-test cleanup -----------------------------------------------

  // Smart pointers should clean up all the stuff we made for G4

  // ----- Communicate test result to OS -----------------------------------
  return result;
}
