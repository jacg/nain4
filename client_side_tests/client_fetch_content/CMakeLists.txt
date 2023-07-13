cmake_minimum_required(VERSION 3.25)

project(Client)

# ANCHOR: fetch
include(FetchContent)

set(FETCHCONTENT_UPDATES_DISCONNECTED ON CACHE BOOL "Cache package contents to avoid unnecessary downloads")

FetchContent_Declare(
  Nain4
  GIT_REPOSITORY https://github.com/gonzaponte/nain4.git
  GIT_TAG        04cac7ba64f3a832cd1e05ec0b0a8f73d1b0288d
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
