# Flight Simulator

A simple 3D flight simulator built with OpenGL. The aircraft is modelled after the **Piper PA-28 Cherokee** — a real-world single-engine trainer — using its published mass, thrust, cruise speed, and airfoil data to drive the flight physics.

---

## Features

- Real-time 3D rendering with OpenGL
- Skybox environment for sky/horizon visuals
- 3D airplane model loaded via glTF (GLB) using a custom glTF loader
- GLSL shaders for lighting and texturing
- Camera and flight controls via GLFW input
- Physically-based flight model (lift, drag, thrust, torque per surface)
- ISA-based atmosphere: air density decreases with altitude, reducing engine thrust and enforcing a natural service ceiling

---

## Dependencies

Make sure the following are installed on your system before building:

- **OpenGL** — GPU rendering API (usually bundled with your graphics drivers)
- **GLFW3** — Window creation and input handling
- **GLM** — OpenGL Mathematics library (vectors, matrices, transforms)
- **Freetype** — Font rendering for the HUD overlay
- **CMake** (version 3.14 or higher)
- A C++17-compatible compiler (e.g., GCC, Clang, MSVC)

### Installing dependencies (Ubuntu/Debian)

```bash
sudo apt update
sudo apt install cmake libglfw3-dev libglm-dev libgl1-mesa-dev libfreetype-dev
```

### Installing dependencies (macOS with Homebrew)

```bash
brew install cmake glfw glm freetype
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
├── assets/Airplane/    # GLB airplane model
├── include/            # Header files (flight physics, ISA atmosphere, glTF loader)
├── shaders/            # GLSL vertex and fragment shaders
├── Skybox/             # Skybox cubemap textures
├── src/                # C++ source files
├── textures/           # Texture images
└── CMakeLists.txt      # Build configuration
```
