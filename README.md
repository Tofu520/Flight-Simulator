# Flight Simulator

A simple 3D flight simulator built with OpenGL. Fly an airplane through a skybox environment using keyboard controls.

---

## Features

- Real-time 3D rendering with OpenGL
- Skybox environment for sky/horizon visuals
- 3D airplane model loaded via TinyObjLoader
- GLSL shaders for lighting and texturing
- Camera and flight controls via GLFW input

---

## Dependencies

Make sure the following are installed on your system before building:

- **OpenGL** — GPU rendering API (usually bundled with your graphics drivers)
- **GLFW3** — Window creation and input handling
- **GLM** — OpenGL Mathematics library (vectors, matrices, transforms)
- **TinyObjLoader** — Lightweight `.obj` model loader (header included in `include/`)
- **CMake** (version 3.14 or higher)
- A C++17-compatible compiler (e.g., GCC, Clang, MSVC)

### Installing dependencies (Ubuntu/Debian)

```bash
sudo apt update
sudo apt install cmake libglfw3-dev libglm-dev libgl1-mesa-dev
```

### Installing dependencies (macOS with Homebrew)

```bash
brew install cmake glfw glm
```

---

## Building

```bash
# Clone the repository
git clone https://github.com/Tofu520/Flight-Simulator.git
cd Flight-Simulator

# Create a build directory
mkdir build && cd build

# Configure and build
cmake ..
make
```

---

## Running

From inside the `build/` directory, run:

```bash
./FlightSimulator
```

> Make sure to run the executable from the project root or `build/` directory so that relative paths to `shaders/`, `textures/`, `assets/`, and `Skybox/` resolve correctly.

---

## Controls

| Key | Action |
|-----|--------|
| `W` | Pitch up (nose up) |
| `S` | Pitch down (nose down) |
| `A` | Roll left |
| `D` | Roll right |
| `Q` | Increase throttle |
| `E` | Decrease throttle |

The camera follows the airplane automatically — no manual camera control needed.

---

## Project Structure

```
Flight-Simulator/
├── assets/Airplane/    # .obj airplane model files
├── include/            # Header files (including TinyObjLoader)
├── shaders/            # GLSL vertex and fragment shaders
├── Skybox/             # Skybox cubemap textures
├── src/                # C++ source files
├── textures/           # Texture images
└── CMakeLists.txt      # Build configuration
```
