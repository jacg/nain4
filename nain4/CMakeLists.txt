cmake_minimum_required(VERSION 3.25)

project(Nain4)

# ----- Generate compile_commands.json, unless otherwise instructed on CLI --
set(CMAKE_EXPORT_COMPILE_COMMANDS ON CACHE INTERNAL "")

# ----- Ensure that standard C++ headers are found by clangd
if(CMAKE_EXPORT_COMPILE_COMMANDS)
  set(CMAKE_CXX_STANDARD_INCLUDE_DIRECTORIES ${CMAKE_CXX_IMPLICIT_INCLUDE_DIRECTORIES})
endif()

include_directories(${PROJECT_SOURCE_DIR})

add_library(Nain4 nain4.cc g4-mandatory.cc)

find_package(Geant4 REQUIRED)

include(${Geant4_USE_FILE})

set(Nain4_TESTS
  test/test-nain4.cc
)

# ----- Use Catch2 as C++ testing framework ---------------------------------
find_package(Catch2 REQUIRED)
set(ALL_TEST_SOURCES
  test/catch2-main-test.cc
  # ${Nain4_SOURCES}
  # ${Nain4_HEADERS}
  ${Nain4_TESTS}
)

# TODO including headers in add_executable apparently makes them show up in IDEs. Verify how?
add_executable(nain4-tests ${ALL_TEST_SOURCES})
target_link_libraries(
  nain4-tests
  PUBLIC
  Catch2::Catch2
  ${Geant4_LIBRARIES}
  Nain4
)
include(CTest)
include(Catch)
catch_discover_tests(nain4-tests)
