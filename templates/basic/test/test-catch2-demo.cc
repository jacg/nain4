#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <catch2/matchers/catch_matchers.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

// REQUIRE is an assertion which aborts the whole test case immediately
TEST_CASE("require", "[require]") {
  REQUIRE( 6*9 == 42 ); // REQUIRE will stop at first failure, c.f. CHECK
  REQUIRE( 6*7 == 42 ); // So this one will not be executed because of the failure above
}

// CHECK is an assertion which causes the test case to fail overall, but allows
// the test case to continue executing and perform and report further assertions
TEST_CASE("check", "[check]") {
  CHECK( 6*9 == 42 ); // CHECK will continue on first failure, c.f. REQUIRE
  CHECK( 6*7 == 42 ); // So this one will be executed in spite of the above failure
}

// GENERATE is a utility for parametrizing a test, allowing the same test to be
// applied to many different values
TEST_CASE("generate", "[generate]") {
  auto number = GENERATE(2, 4, 6, 7, 8);
  auto is_even = [] (auto n) { return n % 2 == 0; };
  CHECK(is_even(number));
}

// Floating point numbers need special care, because of the inherent errors that
// appear in floating point arithmetic. Try
//
//    just catch2-demo 'float' -s
//
// to see the explicit passes and failures in the following test case
TEST_CASE("float", "[float]") {
  // This will fail, because of floating point arithmetic errors
  CHECK(0.1 + 0.2 == 0.3);
  // Catch2 provides matchers for dealing with this sort of situation
  using namespace Catch::Matchers;
  REQUIRE_THAT(0.1 + 0.2, WithinRel(0.3, 1e-8)); // Relative tolerance
  REQUIRE_THAT(0.1 + 0.2, WithinAbs(0.3, 1e-6)); // Absolute tolerance
  REQUIRE_THAT(0.1 + 0.2, WithinULP(0.3, 1));    // Units in Last Place
  // For a concise summary of these three approaches to comparing floating point
  // numbers, see
  //
  //   https://jtempest.github.io/float_eq-rs/book/background/float_comparison_algorithms.html
  //
  // Note, the code samples on this page are written in Rust, not C++, but the
  // description has a good information content to length ratio.
}

// Catch2 tests can use sections to share setup and teardown code between test
// code. For example (adapted from the Catch2 documentation)
TEST_CASE("vectors can be sized and resized", "[vector]") {

  // For each SECTION below, this TEST_CASE is executed from the start. This
  // means that each section is entered with a freshly constructed vector v,
  // that we know has size INITIAL and capacity at least INITIAL, because the
  // two assertions are also checked before the section is entered. Each run
  // through a test case will execute one, and only one, leaf section.
  unsigned const INITIAL = 5, BIGGER = 10, SMALLER = 0;
  std::vector<int>      v(INITIAL);
  REQUIRE(v.size()     == INITIAL);
  REQUIRE(v.capacity() >= INITIAL);

  SECTION("resizing to smaller changes size but not capacity") {
    v.resize               (SMALLER);
    REQUIRE(v.size()     == SMALLER);
    REQUIRE(v.capacity() >= INITIAL);
  }

  SECTION("resizing to bigger changes size and capacity") {
    v.resize               (BIGGER);
    REQUIRE(v.size()     == BIGGER);
    REQUIRE(v.capacity() >= BIGGER);
  }

  SECTION("reserving to smaller does not change size or capacity") {
    v.reserve               (SMALLER);
    REQUIRE(v.size()     == INITIAL);
    REQUIRE(v.capacity() >= INITIAL);
  }

  // SECTION can also be nested, in which case the parent section can be entered
  // multiple times, once for each leaf section. Nested sections are most useful
  // when you have multiple tests that share part of the set up. So you could
  // add a check that std::vector::reserve does not remove unused excess
  // capacity, like this:
  SECTION("reserving to bigger changes capacity but not size") {
    v.reserve              (BIGGER);
    REQUIRE(v.size()     == INITIAL);
    REQUIRE(v.capacity() >= BIGGER);
    const unsigned LITTLE_BIT = GENERATE(1,2,3,4);
    SECTION("reserving down unused capacity does not change capacity") {
      v.reserve              (BIGGER - LITTLE_BIT);
      REQUIRE(v.size()     == INITIAL);
      REQUIRE(v.capacity() >= BIGGER);
    }
  }
}
// Another way to look at sections is that they are a way to define a tree of
// paths through the test. Each section represents a node, and the final tree is
// walked in depth-first manner, with each path only visiting only one leaf
// node.


// Catch2 also supports Behaviour-Driven Development (BDD) style testing. The
// previous TEST_CASE would be written in BDD style like this:
SCENARIO("vectors can be sized and resized BDD", "[vector][BDD]") {
  GIVEN("A vector with some items") {
    unsigned const INITIAL = 5, BIGGER = 10, SMALLER = 0;
    std::vector<int>      v(INITIAL);
    REQUIRE(v.size()     == INITIAL);
    REQUIRE(v.capacity() >= INITIAL);

    WHEN("the size is reduced") {
      v.resize                 (SMALLER);
      THEN("the size increases but the capacity does not") {
        REQUIRE(v.size()     == SMALLER);
        REQUIRE(v.capacity() >= INITIAL);
      }
    }

    WHEN("the size is increased") {
      v.resize                 (BIGGER);
      THEN("the size and capacity increase") {
        REQUIRE(v.size()     == BIGGER);
        REQUIRE(v.capacity() >= BIGGER);
      }
    }

    WHEN("a lower capacity is reserved") {
      v.reserve                (SMALLER);
      THEN("neither size nor capacity change") {
        REQUIRE(v.size()     == INITIAL);
        REQUIRE(v.capacity() >= INITIAL);
      }
    }

    WHEN("a bigger capacity is reserved") {
      v.reserve                (BIGGER);
      THEN("the capacity changes but the size does not") {
        REQUIRE(v.size()     == INITIAL);
        REQUIRE(v.capacity() >= BIGGER);
      }
      WHEN("reserving down unused capacity") {
        const unsigned LITTLE_BIT = GENERATE(1,2,3,4);
        v.reserve                  (BIGGER - LITTLE_BIT);
        THEN("capacity does not change capacity") {
          REQUIRE(v.size()     == INITIAL);
          REQUIRE(v.capacity() >= BIGGER);
        }
      }
    }
  }
}
