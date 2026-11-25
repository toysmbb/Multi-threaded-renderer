# Multithreaded Renderer

An OpenGL 3.3 renderer written in C++20 that moves its heaviest per-frame computation onto a custom thread pool. The particle simulation is split across multiple CPU threads using a custom thread pool, and the thread count can be changed while the program is running to see how it affects performance.

## About
The goal of this project is to show how multithreading can improve performance in a real-time renderer.

The scene has 100 cubes and a particle system with around 90,000 particles alive at any moment. The particle simulation is the heavy CPU work, and it is divided between threads every frame. 
The window title shows: FPS, thread count, number of alive particles, and simulation time per frame (ms).

## How it works
- **Custom thread pool** (`ThreadPool.h`): a task queue with worker threads, `std::future` results and exception handling.
- **ParallelFor**: splits the particle array into chunks, one per thread. The main thread works on a chunk too instead of waiting.
- **No locks while simulating.** Every thread updates its own part of the particle array and writes visible particles into its own output buffer.
- **OpenGL stays on the main thread.** Worker threads only do the simulation. Uploading data to the GPU and drawing is done by the main thread, because OpenGL requires it.
- **Instanced drawing** is used for both the cubes and the particles.

## Controls

| Key | Action |
|---|---|
| `W` `A` `S` `D` + mouse | Move the camera |
| `[` / `]` | Fewer / more threads |
| `V` | Toggle vsync |
| `Esc` | Quit |

## Build
Requirements: CMake, a C++20. Dependencies (GLFW, glad, glm, spdlog) are handled by CMake.

```bash
cmake -B build
cmake --build build --config Release
```

## Project structure
```
core/      thread pool, particle system
src/       window, renderer, camera, shaders and OpenGL wrappers
shaders/   GLSL shaders
```

## Built with
C++20 · OpenGL 3.3 · GLFW · glad · glm · CMake
