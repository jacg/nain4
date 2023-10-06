cmake_minimum_required(VERSION 3.25)

project(Client)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_EXTENSIONS OFF)

set(USERNAME $ENV{GITHUB_USERNAME})

execute_process(
  COMMAND bash -c "git log -1 | head -1 | cut -d ' ' -f 2 | tr -d '\n'"
  OUTPUT_VARIABLE COMMIT_HASH
)

message("Using username: <${USERNAME}>")
message("Using commit: <${COMMIT_HASH}>")

# ANCHOR: fetch
include(FetchContent)

set(FETCHCONTENT_UPDATES_DISCONNECTED ON CACHE BOOL "Cache package contents to avoid unnecessary downloads")

FetchContent_Declare(
  Nain4
  GIT_REPOSITORY https://github.com/${USERNAME}/nain4.git
  GIT_TAG        ${COMMIT_HASH}
  # make sure that no other nain4 installation is used
  OVERRIDE_FIND_PACKAGE
  SOURCE_SUBDIR nain4/src
)

FetchContent_MakeAvailable(Nain4)
# ANCHOR_END: fetch

find_package(Geant4 REQUIRED ui_all vis_all)
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
