# CPU Scheduler Simulator

A C++ educational project that simulates classic CPU scheduling algorithms.  You can run it from a **console** build or from an interactive **GUI** desktop app with visualization and controls.

## Algorithms


| Algorithm            | Type                                               |
| -------------------- | -------------------------------------------------- |
| **FCFS**             | First-Come, First-Served                           |
| **SJF**              | Shortest Job First (preemptive and non-preemptive) |
| **Round Robin (RR)** | Time-quantum based                                 |
| **Priority**         | Preemptive and non-preemptive                      |


## Dependencies (install yourself — not downloaded by CMake)

CMake **does not** install these; you need them on your machine before building.


| Tool              | Purpose                                                     | Download                                                                                                                                                                                                                                                                                                                                                                                           |
| ----------------- | ----------------------------------------------------------- | -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| **CMake**         | Generates build files and drives the build                  | [cmake.org/download](https://cmake.org/download/)                                                                                                                                                                                                                                                                                                                                                  |
| **Git**           | CMake `FetchContent` clones GLFW and ImGui during configure | [git-scm.com/downloads](https://git-scm.com/downloads)                                                                                                                                                                                                                                                                                                                                             |
| **C++ toolchain** | Compiles C++17 code                                         | **Windows:** [Visual Studio](https://visualstudio.microsoft.com/downloads/) (Desktop development with C++) or [Build Tools for Visual Studio](https://visualstudio.microsoft.com/downloads/#build-tools-for-visual-studio-2022); **LLVM/Clang:** [LLVM releases](https://github.com/llvm/llvm-project/releases); **MinGW-w64:** [MSYS2](https://www.msys2.org/) or [WinLibs](https://winlibs.com/) |
| **OpenGL**        | GUI uses OpenGL 3                                           | **Windows:** system `opengl32` is enough; **Linux:** Mesa / distro GL dev packages (e.g. Debian/Ubuntu: `libgl1-mesa-dev` — [Mesa](https://www.mesa3d.org/)); **macOS:** OpenGL framework (deprecated but still available)                                                                                                                                                                         |


Fetched **automatically** when you run CMake (no separate install):

- [GLFW](https://github.com/glfw/glfw) 3.4 — window and OpenGL context  
- [Dear ImGui](https://github.com/ocornut/imgui) v1.91.9 — UI (GLFW + OpenGL3 backends)

## Build and run (recommended — PowerShell)

From the **repository root** on Windows (PowerShell 5.1+):


| Script | What it does |
| ------ | ------------ |
| [scripts/build.ps1](scripts/build.ps1) | Configures CMake in `build/` and builds (default **Release**). |
| [scripts/run-gui.ps1](scripts/run-gui.ps1) | Builds (unless `-SkipBuild`) and runs **`cpu_scheduler_gui`** (ImGui app). |
| [scripts/run-console.ps1](scripts/run-console.ps1) | Compiles **`main.cpp`** and all `src/**/*.cpp` **without CMake** (compiler in `PATH` or `CXX`) into `out/console/` and runs the console demo. |


Examples:

```powershell
.\scripts\build.ps1
.\scripts\build.ps1 -Config Debug

.\scripts\run-gui.ps1
.\scripts\run-gui.ps1 -SkipBuild
.\scripts\run-gui.ps1 -Config Debug

.\scripts\run-console.ps1
.\scripts\run-console.ps1 -SkipBuild
```

With **Visual Studio** multi-config generators, binaries are under `build/Release/` or `build/Debug/`. The run scripts search common output layouts.

**Linux / macOS:** Install [PowerShell](https://github.com/PowerShell/PowerShell/releases) to use the same scripts, or use the manual CMake commands below.

## Manual build (without scripts)

From the project root:

```powershell
cmake -S . -B build
cmake --build build --config Release --parallel
```

On single-config generators (Ninja, Unix Makefiles), executables are under `build/`. With Visual Studio multi-config, use `build/Release/` (or `Debug/`).

Run the GUI or console binary from the build output directory, or use the scripts above.

## Contributors

Algorithm implementations:

- **Taqi Ahmed** — SJF preemptive
- **Omar Wael** — SJF non-preemptive
- **Moahmmed Ehab** — RR
- **Mohamed Sherif** — Priority non-preemptive
- **Kareem Nasser**— Priority preemptive
- **Mina Emad**— FCFS

GUI and project packaging:

- **Seif Zayed**