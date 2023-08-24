cmake_minimum_required(VERSION 3.25)

project(nain4-tests)

set(TEST_EXECUTABLE nain4-tests)

# ----- Set installation variables
# Defaults to <top>/install, but can be overriden from CLI
if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
  set(CMAKE_INSTALL_PREFIX
      ${PROJECT_SOURCE_DIR}/../../install/nain4-tests
      CACHE PATH
      "Default installation directory"
      FORCE
  )
endif()

# ----- Generate compile_commands.json, unless otherwise instructed on CLI --
set(CMAKE_EXPORT_COMPILE_COMMANDS ON CACHE INTERNAL "")

# ----- Ensure that standard C++ headers are found by clangd
if(CMAKE_EXPORT_COMPILE_COMMANDS)
  set(CMAKE_CXX_STANDARD_INCLUDE_DIRECTORIES ${CMAKE_CXX_IMPLICIT_INCLUDE_DIRECTORIES})
endif()

include_directories(${PROJECT_SOURCE_DIR})

FILE(GLOB SOURCES ${PROJECT_SOURCE_DIR}/*.cc)
FILE(GLOB HEADERS ${PROJECT_SOURCE_DIR}/*.hh)

#----------------------------------------------------------------------------
# Find Geant4 package, activating all available UI and Vis drivers by default
# You can set WITH_GEANT4_UIVIS to OFF via the command line or ccmake/cmake-gui
# to build a batch mode only executable
#
option(WITH_GEANT4_UIVIS "Build example with Geant4 UI and Vis drivers" ON)
if(WITH_GEANT4_UIVIS)
  find_package(Geant4 REQUIRED ui_all vis_all)
else()
  find_package(Geant4 REQUIRED)
endif()


include(${Geant4_USE_FILE})

set(Nain4_TESTS
  ../test/test-nain4.cc
  ../test/test-run-manager.cc
  ../test/trivial-full-app-test.cc
)

# ----- Use Catch2 as C++ testing framework ---------------------------------
find_package(Catch2 3 REQUIRED)
set(ALL_TEST_SOURCES
  ../test/catch2-main-test.cc
  ${Nain4_TESTS}
)

# TODO including headers in add_executable apparently makes them show up in IDEs. Verify how?
add_executable(${TEST_EXECUTABLE} ${ALL_TEST_SOURCES})

if(APPLE)
  target_link_options(${TEST_EXECUTABLE} PRIVATE -undefined dynamic_lookup)
endif()


target_link_libraries(
  ${TEST_EXECUTABLE}
  PUBLIC
  Catch2::Catch2
  ${Geant4_LIBRARIES}
  Nain4
)

include(CTest)
include(Catch)
catch_discover_tests(${TEST_EXECUTABLE})

install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/${TEST_EXECUTABLE}
        DESTINATION ${CMAKE_INSTALL_PREFIX}/bin
)
