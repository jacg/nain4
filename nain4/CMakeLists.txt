cmake_minimum_required(VERSION 3.25)

project(Nain4)

# ----- Set installation variables
# Defaults to <top>/install, but can be overriden from CLI
if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
  set(CMAKE_INSTALL_PREFIX
      ${PROJECT_SOURCE_DIR}/../install/
      CACHE PATH
      "Default installation directory"
      FORCE
  )
endif()

set(CMAKE_INSTALL_LIBDIR
    "lib"
    CACHE PATH
    "Library installation directory"
)

set(CMAKE_INSTALL_INCLUDEDIR
    "include"
    CACHE PATH
    "Header installation directory"
)

# ----- Generate compile_commands.json, unless otherwise instructed on CLI --
set(CMAKE_EXPORT_COMPILE_COMMANDS ON CACHE INTERNAL "")

# ----- Ensure that standard C++ headers are found by clangd
if(CMAKE_EXPORT_COMPILE_COMMANDS)
  set(CMAKE_CXX_STANDARD_INCLUDE_DIRECTORIES ${CMAKE_CXX_IMPLICIT_INCLUDE_DIRECTORIES})
endif()

set(NAIN4_INCLUDE_DIRS ${PROJECT_SOURCE_DIR})
set(NAIN4_LIB_DIRS ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR})
set(NAIN4_LIBRARIES Nain4)


include_directories(${PROJECT_SOURCE_DIR})

FILE(GLOB SOURCES ${PROJECT_SOURCE_DIR}/*.cc)
FILE(GLOB HEADERS ${PROJECT_SOURCE_DIR}/*.hh)

add_library(Nain4 SHARED ${SOURCES} ${HEADERS})

#set_target_properties(Nain4 PROPERTIES ENABLE_EXPORTS ON)

find_package(Geant4 REQUIRED)

include(${Geant4_USE_FILE})

set(Nain4_TESTS
  test/test-nain4.cc
  test/trivial-full-app-test.cc
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

target_include_directories(
  Nain4
  PUBLIC
  $<BUILD_INTERFACE:${NAIN4_INCLUDE_DIRS}>
  $<INSTALL_INTERFACE:include>
)

include(CTest)
include(Catch)
catch_discover_tests(nain4-tests)

include(GNUInstallDirs)
set(NAIN4_CONFIG_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/Nain4)

install(TARGETS Nain4
        EXPORT Nain4Targets
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

install(FILES ${HEADERS}
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(EXPORT Nain4Targets
        FILE Nain4Targets.cmake
        NAMESPACE nain4::
        DESTINATION ${NAIN4_CONFIG_DESTINATION}
)

include(CMakePackageConfigHelpers)
configure_package_config_file(
    ${CMAKE_CURRENT_LIST_DIR}/../Nain4Config.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/Nain4Config.cmake
    INSTALL_DESTINATION ${NAIN4_CONFIG_DESTINATION}
)


install(FILES ${CMAKE_CURRENT_BINARY_DIR}/Nain4Config.cmake
        DESTINATION ${NAIN4_CONFIG_DESTINATION}
)
