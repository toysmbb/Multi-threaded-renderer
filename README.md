# Multithreaded Particle Renderer

[![CI](../../actions/workflows/ci.yml/badge.svg)](../../actions/workflows/ci.yml)

A C++20 / OpenGL 3.3 demo that simulates **up to 1,000,000 particles on the CPU across multiple threads**
and renders the result with instanced drawing. The point of the project is the concurrency work:
a hand-written thread pool, a `ParallelFor` built on it, lock-free data partitioning, and measuring
how the simulation scales with the number of threads.

<!-- TODO: add a GIF or screenshot here, e.g. ![demo](docs/demo.gif) -->

## Highlights

- **Custom `ThreadPool`** (`core/ThreadPool.h`): task queue, `std::future` results, exception propagation,
  clean shutdown that drains the queue, and a pool with 0 workers that degrades to a serial loop.
- **`ParallelFor(count, fn(begin, end, chunk))`**: the calling thread processes one chunk itself instead of
  idling; always waits for every chunk before rethrowing an exception (tasks reference the caller's stack).
- **No locks in the hot path.** Each chunk reads/writes its own slice of the particle pool and appends to its
  own output vector. Output vectors are `alignas(64)` to avoid false sharing.
- **Deterministic by construction.** The concatenated output is identical for any thread count
  (checked by a unit test), so multithreading never changes what you see.
- **OpenGL only on the main thread**, as the API requires; worker threads never touch GL.
- **Tested under ThreadSanitizer and ASan/UBSan** in CI (`core/` has no OpenGL dependency, so this runs on any machine).
- **Headless benchmark** (`renderer_bench`) with a thread sweep, plus optional **Tracy** profiler zones.

## Architecture

```
core/  (no OpenGL, unit-tested, benchmarked)        src/  (OpenGL application)
 ├─ ThreadPool.h      pool + AddTask + ParallelFor    ├─ Application   main loop, input, timing
 ├─ ParticleSystem    ring-buffer pool, Update()      ├─ Renderer      draw calls, GPU uploads
 ├─ Random.h                                          ├─ Window        GLFW + context
 └─ Profiling.h       Tracy macros (optional)         └─ Shader / VertexBuffer / VertexArray / ...
```

### One frame

```
main thread                         worker threads
-----------                         --------------
emit new particles (serial, cheap)
ParticleSystem::Update ──────────►  chunk 1: simulate + collect visible particles ─┐
   chunk 0 runs here too            chunk 2: ...                                    │ each chunk has its
   wait for all chunks ◄─────────── chunk N: ...                                   ─┘ own output vector
upload chunks to the GPU instance buffer (glBufferSubData, main thread only)
draw cubes (instanced), draw particles (instanced, alpha blended)
```

Per particle, the data is split into a **hot** part (position, velocity, remaining life: 20 bytes, touched
every frame for all 1M slots) and a **cold** part (colors, sizes, lifespan: only read for particles that are alive).
That keeps the full-pool scan memory-light.

## Build

Requirements: CMake 3.20+, a C++20 compiler (MSVC 2019+, GCC 11+, Clang 14+), Git (dependencies are downloaded
with CMake `FetchContent`: GLFW, glad, glm, spdlog). On Linux install `xorg-dev libgl1-mesa-dev`.

```bash
cmake -B build
cmake --build build --config Release
./build/renderer            # Windows (MSVC): build\Release\renderer.exe
```

The default build type is Release (numbers from Debug builds are meaningless).

| CMake option | Default | Meaning |
|---|---|---|
| `RENDERER_BUILD_APP` | ON | OpenGL application (OFF = only core library, tests, benchmark; no GLFW needed) |
| `RENDERER_BUILD_BENCHMARK` | ON | headless `renderer_bench` |
| `RENDERER_BUILD_TESTS` | OFF | Catch2 unit tests |
| `RENDERER_ENABLE_TRACY` | OFF | Tracy profiler zones |
| `RENDERER_SANITIZE` | "" | e.g. `thread` or `address,undefined` (GCC/Clang) |

### Controls

`WASD` + mouse: camera · `[` / `]`: fewer / more threads (live, see the window title) · `V`: toggle vsync · `Esc`: quit.
The title bar shows FPS, thread count, alive particles and the simulation time per frame.
Command line: `--threads N`, `--particles N`, `--vsync`, `--help`.

## Tests

```bash
cmake -B build-test -DRENDERER_BUILD_APP=OFF -DRENDERER_BUILD_TESTS=ON
cmake --build build-test
ctest --test-dir build-test --output-on-failure

# with ThreadSanitizer (Linux/macOS, GCC/Clang)
cmake -B build-tsan -DCMAKE_BUILD_TYPE=RelWithDebInfo -DRENDERER_BUILD_APP=OFF \
      -DRENDERER_BUILD_TESTS=ON -DRENDERER_SANITIZE=thread
cmake --build build-tsan && ctest --test-dir build-tsan --output-on-failure
```

What is covered: pool results/arguments/move-only arguments, exception propagation, shutdown draining the queue,
zero-worker pool, `ParallelFor` coverage (every index exactly once, count smaller than chunk count, count 0,
exception waits for all chunks), particle lifetime and interpolation, ring-buffer overwrite, and
**single-thread vs multi-thread output equality**.

## Benchmark

```bash
./build/renderer_bench --sweep                                 # 1, 2, 4, ... hardware threads
./build/renderer_bench --threads 4 --particles 1000000 --active 0.1 --frames 300
```

It measures only `ParticleSystem::Update` (simulation + gathering visible particles), with a fixed RNG seed,
a warm-up phase and median-of-N timing. `--active` is the fraction of the pool that is alive
(the interactive app runs at about 9%).

### Results

<!-- TODO: run `renderer_bench --sweep` on your machine and paste the table below.
     State the CPU model, core/thread count, OS and compiler. Then write 2-3 sentences interpreting it:
     where does scaling stop and why? -->

| Threads | Median update (ms) | Speedup |
|---:|---:|---:|
| 1 | – | 1.00× |
| 2 | – | – |
| 4 | – | – |
| 8 | – | – |

*Machine: CPU model, cores/threads, RAM, OS, compiler.*

## Design notes and lessons learned

- **Profile before parallelizing.** The first version parallelized only the particle update and then gathered the
  visible particles in a *serial* pass (plus a ~28 MB allocation every frame). The serial part was as expensive
  as the parallel one, which caps the speedup at about 2× no matter how many cores you add (Amdahl's law).
  Fusing "update" and "gather" into one parallel pass with per-chunk output removed the serial section.
- **The workload is memory-bound.** Per particle there are only a few arithmetic operations, so scaling is limited
  by memory bandwidth, not by the number of cores. Splitting hot/cold data reduces the bytes read per frame.
- **False sharing is real.** Per-chunk output vectors sit on separate cache lines (`alignas(64)`).
- **Determinism is testable.** Because each particle is independent and chunks are concatenated in index order,
  results do not depend on thread count, which makes multithreaded code easy to verify.
- **Time-based simulation.** Emission and movement use the real frame time (clamped), so behavior does not depend on FPS.

## Limitations / possible next steps

- Particles are simulated on the CPU on purpose (to study CPU concurrency); the same work would be trivial on the GPU
  (compute shader or transform feedback).
- The pool uses one shared queue with a mutex. A work-stealing scheduler or a lock-free queue could reduce contention
  for very small tasks (not an issue at the current task size).
- Uploads use `glBufferSubData` with buffer orphaning; persistent-mapped buffers would avoid one copy.
- No frame pipelining: simulation of frame N+1 does not overlap with GPU rendering of frame N.

## Project layout

```
core/        thread pool, particle simulation (no OpenGL)
src/         OpenGL application
shaders/     GLSL 3.30 shaders (copied next to the executable at build time)
tests/       Catch2 tests
benchmark/   headless benchmark
.github/     CI: sanitizers for core, build check for the app on Windows + Linux
```
