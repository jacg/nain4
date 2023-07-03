
# Avoid repeatedly including the targets
if(NOT TARGET Nain4::Nain4)
    list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}")
endif()

find_package(Geant4 REQUIRED)

include(${Geant4_USE_FILE})
