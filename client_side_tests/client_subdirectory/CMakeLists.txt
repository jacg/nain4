cmake_minimum_required(VERSION 3.25)

project(Client)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_EXTENSIONS OFF)

# ANCHOR: add_subdir
add_subdirectory(nain4/nain4/src)
# ANCHOR_END: add_subdir

find_package(Geant4 REQUIRED ui_all vis_all)
include(${Geant4_USE_FILE})

add_executable(client_exe main.cc)

target_link_libraries(
client_exe
PRIVATE
Nain4
${Geant4_LIBRARIES}
)
