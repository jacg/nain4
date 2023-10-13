cmake_minimum_required(VERSION 3.25)

project(Client)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_EXTENSIONS OFF)

# ANCHOR: find_package
set(nain4_DIR "$ENV{NAIN4_INSTALL}/lib/cmake/nain4/")

find_package(nain4 REQUIRED)
# ANCHOR_END: find_package

find_package(Geant4 REQUIRED ui_all vis_all)
include(${Geant4_USE_FILE})

add_executable(client_exe main.cc)

target_link_libraries(
client_exe
PRIVATE
nain4
${Geant4_LIBRARIES}
)
