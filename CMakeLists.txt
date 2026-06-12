cmake_minimum_required(VERSION 3.15)
project(FighterProject)

set(CMAKE_CXX_STANDARD 17)

# Point to the installed CMake config files
set(Urho3D_DIR "D:/urho3d-evolution/rbfx/install/lib/cmake/Urho3D")

find_package(Urho3D REQUIRED)

# Add include directories for rbfx
include_directories("${Urho3D_DIR}/../../include")
include_directories("${Urho3D_DIR}/../../include/Urho3D/ThirdParty")

# UPDATED: Added all implementation source modules to the target executable pool
add_executable(FighterProject
    main.cpp
    Fighter.cpp
    Zombie.cpp
    ZombieAI.cpp
    ZombieManager.cpp
    CombatSystem.cpp
    CameraController.cpp
)

target_link_libraries(FighterProject PRIVATE Urho3D)

set_target_properties(FighterProject PROPERTIES 
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)