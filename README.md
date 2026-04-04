<div align="center">
  <img src="originlogo.png" alt="Origin Logo"  width="550"> 
  <h1 style="font-size: 3rem; margin-top: 10px;"><strong>Origin Engine</strong></h1>
</div>

* **[Origin Website](https://origin.aloyak.dev)**
* **[Demo Game](https://origin.aloyak.dev/game)**

## General Information
Origin is a true 3D pixel-art game engine written from scratch in modern C++. It uses OpenGL for rendering and is designed to be cross-platform, supporting Windows, MacOS, Linux, and Web Builds. Web builds are enabled through WebAssembly using Emscripten.

The engine features a custom-built editor called the Sandbox, which allows for scene editing, entity management, and component manipulation. Moreover, it uses ImGui for the editor's user interface.

Physics is handled by the Bullet3 Physics library, and the engine supports various effects and features such as dynamic lighting, and post-processing effects.

## Showcase
![Screenshot](assets/demo.png)

> Origin Engine 0.12.0 running on Linux. You can see a simple scene with the sponza model, a player with a camera component, and a dynamic point light.

![Sandbox Screenshot](assets/sandbox.png)

> Sandbox Editor running on Linux. Featuring the physics playground scene.

## Building

The project uses **CMake** to generate and build the engine. The repository is organized into three main modules:

* `engine` — the core engine code
* `game` — the game project that uses the engine
* `sandbox` — map editor with extra features

You may want to change your game's name in `game/CMakeLists.txt`.

* **Requirement:** CMake **3.23 or newer**. Cmake 4.0+ is **not completely supported**. 

* Note that the `sponza` model is **not included**. You can download it with `assets/models/download-sponza.sh` or clone it yourself from https://github.com/jimmiebergmann/Sponza

### Linux

To build the project on Linux:

```sh
chmod +x ./build.sh
./build.sh

# If you don't want to build the sandbox:
./build.sh --no-sandbox
```

This script will generate a build directory and compile the project.
The final executable can be found in `build/game/`

### Windows

To build the project on Windows, you can use the same `build.sh` script if you have a compatible environment (like Git Bash or WSL). Alternatively, you can use CMake directly:

* Tested using ninja but Visual Studio should work as well (recommended even!)

```sh
mkdir build
cd build
cmake -G "Ninja" .. # or -G "Visual Studio 17 2021" 
ninja
```

### Web

* **Requirement:** Emscripten sdk must be installed!

* Note that the web build is based on a minimal template static page at `game/web/shell.html` that you can modify as you like

```sh
chmod +x ./build.sh 
./build.sh --web # Sandbox isn't built for web
```
```sh
python3 -m http.server 8080 --directory build-web/game/ # suggestion!
```

This script will generate a build directory specific for web and compile the project.
Your build can be found at `build-web/game/game.html`, it should be opened with a server in order to work!

### MacOS

* Not yet tested! (origin uses OpenGL 4.1 as it is the last supported openGL version for MacOS, but **it is officially deprecated!**)

## License

This project is under the MIT License. Please see `LICENSE.md` for more information!

## Thanks and Inspirations
* [LearnOpenGL](https://learnopengl.com/) - the best OpenGL tutorial website out there
* [Texel-Splatting](https://dylanebert.com/texel-splatting/) by [Dylan Ebert](https://github.com/dylanebert) - gave me the idea to create this engine in the first place
* [Lite Engine](https://github.com/maritim/LiteEngine) - introduced me to game engines a while back, used then to test and try stuff

### HackClub

<a href="https://hackclub.com/"><img style="position: absolute; top: 0; left: 10px; border: 0; width: 256px; z-index: 999;" src="https://assets.hackclub.com/flag-orpheus-left.svg" href="https://flavortown.hackclub.com/projects/8834" alt="Hack Club"/></a>

Proud member of [Hack Club](https://hackclub.com/)!

This project was made as part of HackClub's Flavortown Event (2026): <https://flavortown.hackclub.com/projects/14868>