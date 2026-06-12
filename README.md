# rbfx Third-Person Shooter Example

![License](https://img.shields.io/badge/license-MIT-blue.svg)
![Engine](https://img.shields.io/badge/engine-rbfx-green.svg)
![Language](https://img.shields.io/badge/language-C%2B%2B-blue.svg)

A complete **TPS style example** built with the **rbfx** game engine. Learn rbfx/Urho3D game development with working character controller, attacking mechanics, enemy AI, and UI.

## What is rbfx?

**rbfx** is a modern fork of the Urho3D game engine, designed for cross-platform 2D and 3D game development. It provides a lightweight, data-driven architecture with a powerful scene graph, physics integration, and a component-based entity system. rbfx is ideal for rapid prototyping and building production-ready games using C++ or AngelScript.

## Features

- [x] Third-person character controller with smooth camera follow
- [x] Attacking mechanics with melee combat
- [x] Enemy AI with pathfinding and state machines
- [x] Health and damage system
- [x] User interface (HUD, menus)
- [x] Physics-based movement and collisions
- [x] Animation system integration
- [x] Sound effects and music

## Build Instructions

### Prerequisites

- CMake 3.15+
- Visual Studio 2019+ / GCC 9+ / Clang 10+
- rbfx SDK (download from [rbfx GitHub](https://github.com/rokups/rbfx))

### Building with CMake

```bash
# Clone the repository
git clone https://github.com/ZkrAbbd690/ZkrAbbd690-rbfx-3d-fighting-Game-mockup.git
cd ZkrAbbd690-rbfx-3d-fighting-Game-mockup

# Configure
cmake -B build -DCMAKE_TOOLCHAIN_FILE=<path-to-rbfx>/rbfx.cmake

# Build
cmake --build build --config Release
```

### Running

After building, run the executable from the `bin` directory. Make sure the rbfx shared libraries are in your PATH or in the same directory as the executable.

## Controls

| Key | Action |
|-----|--------|
| W/A/S/D | Move character |
| Mouse | Look around |
| Space | Attack |
| Space | Jump |
| Shift | Sprint |
| R | Reload |
| Esc | Pause menu |

## Project Structure

```
├── Source/
│   ├── Main.cpp
│   ├── Game/
│   │   ├── GameApplication.h
│   │   ├── GameApplication.cpp
│   │   ├── Player/
│   │   │   ├── PlayerController.h
│   │   │   └── PlayerController.cpp
│   │   ├── Enemy/
│   │   │   ├── EnemyAI.h
│   │   │   └── EnemyAI.cpp
│   │   └── Weapon/
│   │       ├── Weapon.h
│   │       └── Weapon.cpp
│   └── UI/
│       ├── MainMenu.h
│       └── HUD.h
├── CMakeLists.txt
├── LICENSE
└── README.md
```

## Code Example

Here's a simple example of using rbfx APIs to create a scene and spawn a character:

```cpp
#include <rbfx/Engine.h>
#include <rbfx/Core/Context.h>
#include <rbfx/Scene/Scene.h>
#include <rbfx/Graphics/Octree.h>

using namespace Urho3D;

class TPSStyleGame : public Application
{
    void Setup() override
    {
        engineParameters_["FullScreen"] = false;
        engineParameters_["WindowResizable"] = true;
    }

    void Start() override
    {
        // Create scene
        auto scene = new Scene(context_);
        scene->CreateComponent<Octree>();

        // Create camera
        auto cameraNode = scene->CreateChild("Camera");
        cameraNode->SetPosition(Vector3(0, 5, -10));
        
        // Create player
        auto playerNode = scene->CreateChild("Player");
        playerNode->SetPosition(Vector3(0, 0, 0));
    }
};

URHO3D_DEFINE_APPLICATION(TPSStyleGame)
```

## Links

- [rbfx Official Repository](https://github.com/rokups/rbfx)
- [rbfx Documentation](https://rbfx.readthedocs.io/)
- [Urho3D Documentation](https://urho3d.io/documentation/)

## Contributing

Contributions are welcome! Please read [CONTRIBUTING.md](CONTRIBUTING.md) for guidelines on how to contribute to this rbfx example project.

## Star This Repo

If this rbfx TPS style example helps you learn game development with rbfx, please star this repository! It helps others discover this resource.

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.
