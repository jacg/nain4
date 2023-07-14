cmake_minimum_required(VERSION 3.25)

project(n4app)

set(Nain4_DIR "../install/lib/cmake/Nain4/")
message(${Nain4_DIR})
find_package(Nain4 REQUIRED)

find_package(Geant4 REQUIRED)
include(${Geant4_USE_FILE})

add_executable(n4app n4app.cc)

target_link_libraries(
n4app
PRIVATE
Nain4
${Geant4_LIBRARIES}
)
