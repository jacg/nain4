cmake_minimum_required(VERSION 3.25)

project(example-XXX)

set(EXE example-XXX)

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

# ----- Generate compile_commands.json, unless otherwise instructed on CLI --
set(CMAKE_EXPORT_COMPILE_COMMANDS ON CACHE INTERNAL "")

# ----- Ensure that standard C++ headers are found by clangd
if(CMAKE_EXPORT_COMPILE_COMMANDS)
  set(CMAKE_CXX_STANDARD_INCLUDE_DIRECTORIES ${CMAKE_CXX_IMPLICIT_INCLUDE_DIRECTORIES})
endif()

include_directories(${PROJECT_SOURCE_DIR})

FILE(GLOB SOURCES ${PROJECT_SOURCE_DIR}/*.cc)
FILE(GLOB HEADERS ${PROJECT_SOURCE_DIR}/*.hh)

set(Nain4_DIR "../../../install/nain4/lib/cmake/Nain4")

find_package(Nain4 REQUIRED)

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

add_executable(${EXE} ${HEADERS} ${SOURCES})

if(APPLE)
  target_link_options(${EXE} PRIVATE -undefined dynamic_lookup)
endif()


target_link_libraries(
  ${EXE}
  PUBLIC
  ${Geant4_LIBRARIES}
  Nain4
)

install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/${EXE}
        DESTINATION ${CMAKE_INSTALL_PREFIX}
)

add_custom_command(
  TARGET ${EXE} POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E create_symlink
  ${CMAKE_SOURCE_DIR}../macs $<TARGET_FILE_DIR:${EXE}>/macs
  COMMENT "Symlinking G4 macros directory" VERBATIM
)
