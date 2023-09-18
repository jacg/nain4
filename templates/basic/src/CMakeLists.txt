cmake_minimum_required(VERSION 3.25)

# CHANGEME-project-name: Not important, but you might want to change it
project(my-nain4-project)
set(CMAKE_CXX_STANDARD 20)

set(EXE CHANGEME-my-n4-prog)

# ----- Generate compile_commands.json, unless otherwise instructed on CLI --
set(CMAKE_EXPORT_COMPILE_COMMANDS ON CACHE INTERNAL "")
# ----- Ensure that standard C++ headers are found by clangd
if(CMAKE_EXPORT_COMPILE_COMMANDS)
  set(CMAKE_CXX_STANDARD_INCLUDE_DIRECTORIES ${CMAKE_CXX_IMPLICIT_INCLUDE_DIRECTORIES})
endif()

find_package(Nain4 REQUIRED)

#----------------------------------------------------------------------------
# Find Geant4 package, activating all available UI and Vis drivers by default
# You can set WITH_GEANT4_UIVIS to OFF via the command line or ccmake/cmake-gui
# to build a batch mode only executable
option(WITH_GEANT4_UIVIS "Build example with Geant4 UI and Vis drivers" ON)
if(WITH_GEANT4_UIVIS)
  find_package(Geant4 REQUIRED ui_all vis_all)
else()
  find_package(Geant4 REQUIRED)
endif()

include(${Geant4_USE_FILE})

# CHANGEME-n4app.cc: change the file name n4app.cc on the following line and
# rename the file in this directory
add_executable(${EXE} n4app.cc)

target_link_libraries(
  ${EXE}
  PRIVATE
  Nain4
  ${Geant4_LIBRARIES}
)

#----------------------------------------------------------------------------
# Link macro files directory to the build directory, i.e. the directory in which
# we build ${EXE}. This is so that we can run the executable directly
# because it relies on these macros being in the current working directory.
#
add_custom_command(
  TARGET ${EXE} POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E create_symlink
  ${CMAKE_SOURCE_DIR}../macs $<TARGET_FILE_DIR:${EXE}>/macs
  COMMENT "Symlinking G4 macros directory" VERBATIM
)


install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/${EXE}
        DESTINATION ${CMAKE_INSTALL_PREFIX}/bin
)
