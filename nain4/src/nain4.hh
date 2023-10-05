// clang-format off
#pragma once

#include <n4-run-manager.hh>

#include <G4LogicalVolumeStore.hh>
#include <G4PhysicalVolumeStore.hh>
#include <G4SolidStore.hh>
#include <G4VisAttributes.hh>



// Disable GCC's shadow warnings
// This happens in constructors where it is a C++ idiom to have
// constructor parameters match memeber names.
// This has been reported to gcc's bug tracker
// https://gcc.gnu.org/bugzilla/show_bug.cgi?id=78147
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wshadow"

////////////////////////////////////////////////////////////////////////////////////////////////////
// Note 1
//
// + G4 forces you to use its G4Exception function, whose return type is void.
//
// + C++'s ternary operator treats throw expressions as a special case.
//
// + Hiding the throw expression inside G4Exception, disables this special
//   treatment, which results in a type mismatch between the void and whatever
//   values are present in the rest of the ternary operator
//
// + So we go through the following convolutions to satisfy the type system:
//
//   1. use throw at the top-level
//   2. use G4Exception in the argument to throw
//   3. but throw does not accept void
//   4. so use comma operator to give acceptable overall expression type (c-string)
//   5. but the actual value of the string doesn't matter, just its type.

#define FATAL(description) G4Exception((__FILE__ + (":" + std::to_string(__LINE__))).c_str(), "666", FatalException, description)

namespace nain4 {

} // namespace nain4

namespace n4 { using namespace nain4; }

#pragma GCC diagnostic pop
