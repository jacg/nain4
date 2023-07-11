cmake_minimum_required(VERSION 3.25)

project(Client)

set(Nain4_DIR "$ENV{NAIN4_INSTALL}/lib/cmake/Nain4/")
find_package(Nain4 REQUIRED)

find_package(Geant4 REQUIRED)
include(${Geant4_USE_FILE})

add_executable(client_exe main.cxx)

target_link_libraries(
client_exe
PRIVATE
Nain4
${Geant4_LIBRARIES}
)
