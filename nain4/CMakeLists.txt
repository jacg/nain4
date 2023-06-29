cmake_minimum_required(VERSION 3.25)

project(Nain4)

# ----- Generate compile_commands.json, unless otherwise instructed on CLI --
set(CMAKE_EXPORT_COMPILE_COMMANDS ON CACHE INTERNAL "")

# ----- Ensure that standard C++ headers are found by clangd
if(CMAKE_EXPORT_COMPILE_COMMANDS)
  set(CMAKE_CXX_STANDARD_INCLUDE_DIRECTORIES ${CMAKE_CXX_IMPLICIT_INCLUDE_DIRECTORIES})
endif()

add_library(Nain4 nain4.cc g4-mandatory.cc)

find_package(Geant4 REQUIRED)

include(${Geant4_USE_FILE})

target_link_libraries(Nain4 PUBLIC ${Geant4_LIBRARIES})
