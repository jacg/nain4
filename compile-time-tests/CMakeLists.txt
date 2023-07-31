cmake_minimum_required(VERSION 3.25)

project(compile-time-test)

set(Nain4_DIR "$ENV{NAIN4_INSTALL}/lib/cmake/Nain4/")
find_package(Nain4 REQUIRED)

find_package(Geant4 REQUIRED ui_all vis_all)
include(${Geant4_USE_FILE})

add_executable(compile_time_test main.cc)

target_link_libraries(
compile_time_test
PRIVATE
Nain4
${Geant4_LIBRARIES}
)
