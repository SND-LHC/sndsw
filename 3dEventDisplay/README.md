# SND 3D Event Display

A 3D visualization tool for the **SND@LHC** (Scattering and Neutrino Detector at the LHC) experiment.

## Table of Contents
- [Background](#background)
- [Build Requirements](#build-requirements)
- [Dependencies](#dependencies)
- [Build](#build)
- [Run Instructions](#run-instructions)

## Background

This tool was developed as part of a Bachelor's thesis project in Computer Science and Engineering 
at the University of Bologna (Alma Mater Studiorum), focused on designing and implementing a new 
interactive 3D event display for the SND@LHC experiment.

- **Thesis** (Italian): [*Progettazione e sviluppo di un Event Display 3D interattivo basato su OpenGL per il rivelatore SND@LHC*](https://amslaurea.unibo.it/id/eprint/39325/) — Licensed under [CC BY-SA 4.0](http://creativecommons.org/licenses/by-sa/4.0/)
  - Author: Enrico Bartocetti
  - Supervisor: Prof. Damiana Lazzaro
  - Co-supervisors: Prof. Luigi Guiducci, Dr. Giulia Paggi
  - University of Bologna, Bachelor's Degree in Computer Science and Engineering, July 2026

## Build Requirements
* **C++17** compatible compiler
* **CMake** (version 3.15 or higher)
* **OpenGL** development headers and drivers

## Dependencies
The project utilizes a hybrid approach for dependency management, using **CMake FetchContent** for automatic downloads and local storage for core UI components.

### Automatically Managed (via CMake)
These are downloaded and compiled automatically during the first build:
* [**GLFW**](https://github.com/glfw/glfw): Window management and input handling.
* [**GLM**](https://github.com/g-truc/glm): Mathematics library for graphics operations.
* [**Assimp**](https://github.com/assimp/assimp): Open Asset Import Library for loading 3D models (glTF).
* [**ImGuiFileDialog**](https://github.com/aiekick/ImGuiFileDialog): A feature-rich file selection dialog for ImGui.
* [**stb_image**](https://github.com/nothings/stb): Image library for exporting screenshots.

### Local External Libraries
Located in the `/external` directory:
* [**ImGui**](https://github.com/ocornut/imgui): Bloat-free Graphical User interface for C++.
* [**GLAD**](https://glad.dav1d.de/): OpenGL loader-generator.

### Assets & Fonts
* [**JetBrains Mono**](https://www.jetbrains.com/lp/mono/): Licensed under the **SIL Open Font License 1.1**.
    * The font is **embedded** directly into the binary via `include/ui/JetBrainsMono.h` using a compressed byte array.

## Build
This module is built as part of the sndsw framework via aliBuild (see the main sndsw documentation).

## Run Instructions

> **Note:** Plain X11 forwarding (`ssh -Y`) works but is very slow (~3 FPS). 
> For a responsive experience, run the display via **VNC** instead (see below).

### Step 1: Get a display

**Option A: VNC (recommended)**

1. Log in to `lxplus`:
```bash
   ssh <user>@lxplus.cern.ch
```
   Once connected, check which specific node you landed on (e.g. `lxplus980.cern.ch`) — you'll need this for the tunnel in step 3.

2. Start a VNC server session on that node:
```bash
   vncserver -geometry 1920x1080
```
   Note the display number it returns (e.g. `:1`). The corresponding port is 
   `5900 + display number` (e.g. `:1` → port `5901`, `:2` → port `5902`, etc.).

3. From your local machine, open an SSH tunnel to the **same specific node**, 
   using the port matching your display number:
```bash
   ssh -L <port>:localhost:<port> <user>@lxplus980.cern.ch
```
   > **Important:** Use the specific node hostname (e.g. `lxplus980.cern.ch`), 
   > not the generic `lxplus.cern.ch`, otherwise the load balancer may route 
   > you to a different node and the tunnel will fail to connect.

4. Connect using a VNC viewer of your choice to `localhost:<port>` 
   (matching the port used in step 3).

**Option B: X11 forwarding (slow, not recommended)**

Simply connect with:
```bash
ssh -Y <user>@lxplus.cern.ch
```

### Step 2: Set up the environment and run

Set up the environment from CVMFS:

```bash
source /cvmfs/sndlhc.cern.ch/SNDLHC-2025/Jan30/setUp.sh
```

Load your local sndsw environment:

```bash
alienv enter sndsw/latest
```

Run the 3D event display:

```bash
cd sw/BUILD/sndsw-latest/sndsw/bin/
./3dEventDisplay
```