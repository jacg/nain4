cmake_minimum_required(VERSION 3.25)

project(n4app-00-basic)

# ----- Generate compile_commands.json, unless otherwise instructed on CLI --
set(CMAKE_EXPORT_COMPILE_COMMANDS ON CACHE INTERNAL "")
# ----- Ensure that standard C++ headers are found by clangd
if(CMAKE_EXPORT_COMPILE_COMMANDS)
  set(CMAKE_CXX_STANDARD_INCLUDE_DIRECTORIES ${CMAKE_CXX_IMPLICIT_INCLUDE_DIRECTORIES})
endif()

set(Nain4_DIR "../../install/lib/cmake/Nain4/")
message(${Nain4_DIR})
find_package(Nain4 REQUIRED)

find_package(Geant4 REQUIRED)
include(${Geant4_USE_FILE})

set(EXE n4app-00-basic)
add_executable(${EXE} n4app.cc)

target_link_libraries(
${EXE}
PRIVATE
Nain4
${Geant4_LIBRARIES}
)
