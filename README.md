<div align="center">
  <img src="originlogo.png" alt="Origin Logo"  width="750"> 
  <h1 style="font-size: 3rem; margin-top: 10px;"><strong>Origin Engine</strong></h1>
</div>

## General Information
Origin is a true 3D pixel-art game engine written from scratch in modern C++. It uses OpenGL for rendering and is designed to be cross-platform, supporting Windows, macOS, Linux, and the Web. Web builds are enabled through WebAssembly using Emscripten.

## Building

The project uses **CMake** to generate and build the engine. The repository is organized into two main modules:

* `engine` — the core engine code
* `game` — the game project that uses the engine
* `sandbox` **(not yet!)** — planned map editor with extra features

You may want to change your game's name in `game/CMakeLists.txt`.

> **Requirement:** CMake **3.23 or newer**.

> Note that the `sponza` model is **not included**. You can download it with `assets/models/download-sponza.sh` or clone it yourself from [this repository](git clone https://github.com/jimmiebergmann/Sponza)

### Linux

To build the project on Linux:

```sh
chmod +x ./build.sh
./build.sh
```

This script will generate a build directory and compile the project.
The final executable can be found in `build/game/`

### Other Platforms

Support for other platforms is planned but currently limited:

* Windows — not yet tested (recommended using Visual Studio)
* macOS — not yet tested
* Web (WebAssembly) — not yet implemented in the build system

## License

This project is under the MIT License. Please see `LICENSE.md` for more information!