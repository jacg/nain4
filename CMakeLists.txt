cmake_minimum_required(VERSION 3.10 FATAL_ERROR)

project(explore-G4)

add_executable(doit main.cc)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED true)

find_package(Geant4 REQUIRED ui_all vis_all)
