cmake_minimum_required(VERSION 3.25)

project(Client)

# ANCHOR: fetch
include(FetchContent)

set(FETCHCONTENT_UPDATES_DISCONNECTED ON CACHE BOOL "Cache package contents to avoid unnecessary downloads")

FetchContent_Declare(
  Nain4
  GIT_REPOSITORY https://github.com/jacg/nain4.git
  GIT_TAG        v0.0.2
  # make sure that no other nain4 installation is used
  OVERRIDE_FIND_PACKAGE
  SOURCE_SUBDIR nain4
)

FetchContent_MakeAvailable(Nain4)
# ANCHOR_END: fetch

find_package(Geant4 REQUIRED)
include(${Geant4_USE_FILE})

add_executable(client_exe main.cc)

# ANCHOR: link
target_link_libraries(
client_exe
PRIVATE
Nain4
${Geant4_LIBRARIES}
)
# ANCHOR_END: link
