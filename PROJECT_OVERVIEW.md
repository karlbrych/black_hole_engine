# 🕳️ Black Hole Engine — Project Overview

> Generated: 2026-06-21
> Repository: `black_hole_engine`
> Language: C++20 + GLSL + Rust
> Build System: CMake 3.15+

---

## 1. Project Description

An OpenGL-based 3D graphics engine that visualizes and simulates celestial bodies (planets, stars, black holes) in real time. Created as a school project focusing on 3D graphics, GPU programming, and shader-based visual effects.

**Key capabilities:**
- Real-time 3D rendering with Phong lighting (ambient + diffuse + specular)
- N-body gravitational simulation with object merging / accretion
- Free-fly perspective camera (WASD + mouse look)
- Rotating skybox with preprocessed cubemap textures
- ImGui-based start menu (Start Game / Exit / Save / Load)
- ImGui-based object editor (spawn new bodies, adjust parameters, preview)
- Custom binary `.bhtx` texture format via a Rust preprocessor CLI
- Save / load simulation state to binary files (`saves/save.dat`)

---

## 2. Project Structure

```
/home/kajabrych/black_hole_engine/
├── CMakeLists.txt                  # Root build script (CMake 3.15+, C++20)
├── CMakeSettings.json              # VS CMake settings (Ninja, x64-Debug)
├── README.md                       # Original project readme (Czech)
├── PROJECT_OVERVIEW.md             # ← This file
├── .gitignore
│
├── src/                            # Application source code
│   ├── main.cpp                    # Entry point — creates Game, calls game.run()
│   ├── game.h                      # Game class declaration
│   ├── game.cpp                    # Game lifecycle: init, scene, input, render, cleanup
│   │
│   ├── models/                     # Core engine modules
│   │   ├── camera.h                # OrthoCamera & PerspectiveCamera structs
│   │   ├── core.h                  # Object struct + Plane struct + gravity API
│   │   ├── core.cpp                # N-body gravity, save/load, draw logic
│   │   ├── shader.h                # Shader class (compile / link GLSL programs)
│   │   ├── shader.cpp              # Shader implementation + uniform setters
│   │   ├── sphere.h / sphere.cpp   # UV-sphere mesh generation (VAO/VBO/EBO)
│   │   ├── skybox.h / skybox.cpp   # Rotating cubemap skybox
│   │   ├── texture.h / texture.cpp # Texture cache, .bhtx loader, STBI fallback
│   │   ├── startmenu.h/.cpp        # ImGui start menu (Start / Exit / Save / Load)
│   │   ├── object_editor.h/.cpp    # ImGui object editor (spawn / preview)
│   │   ├── stb_image.h / stb_image.cpp  # stb_image v2.30 (single-translation-unit)
│   │
│   ├── shaders/                    # GLSL shader sources
│   │   ├── vertex.glsl             # Object vertex shader (pos + UV)
│   │   ├── fragment.glsl           # Object fragment shader (Phong lighting)
│   │   ├── light_source_fragment.glsl  # Light-source (emissive) fragment shader
│   │   ├── skybox_vertex.glsl      # Skybox vertex shader
│   │   └── skybox_fragment.glsl    # Skybox cubemap fragment shader
│   │
│   ├── assets/                     # Game assets
│   │   ├── planet.png              # Planet diffuse texture
│   │   ├── poop-texture.jpg        # Alternate planet texture
│   │   ├── sun-texture.jpg         # Sun / light-source texture
│   │   ├── sun-text.png            # (unused / extra)
│   │   ├── 1000023844.jpg          # (unused / extra)
│   │   ├── wood.jpg                # (unused / extra)
│   │   └── skybox/                 # Cubemap face textures
│   │       ├── right.png, left.png, top.png, bottom.png, front.png, back.png
│   │
│   └── rust/                       # Rust tooling
│       └── texture-preprocessor/   # CLI tool: image → .bhtx binary format
│           ├── Cargo.toml
│           ├── Cargo.lock
│           └── src/                # Rust source (image pipeline)
│
├── include/                        # Third-party headers
│   └── ImGui/                      # Dear ImGui (docking branch)
│
├── libs/                           # (empty / placeholder)
├── build/                          # CMake build output
│   ├── _deps/glfw-src/…            # GLFW 3.4 fetched by CMake FetchContent
│   └── src/assets/ …               # Copied assets + preprocessed .bhtx files
│
└── .cache/                         # (cache artifacts)
```
---

## 3. Build System — CMake (CMakeLists.txt)

| Setting              | Value                          |
|----------------------|--------------------------------|
| `cmake_minimum_required` | 3.15                        |
| Project name         | `Blackhole`                    |
| C++ standard         | C++20 (required, no extensions) |
| Generator            | Ninja (per CMakeSettings.json) |

### External Dependencies

| Library     | How acquired         | Purpose                          |
|-------------|----------------------|----------------------------------|
| OpenGL      | `find_package`       | GPU rendering API                |
| GLFW 3.4    | `FetchContent` (GH)  | Window, input, OpenGL context    |
| GLM         | System / find_package| 3D math (vectors, matrices)      |
| GLAD        | Bundled (`src/glad.c`)| OpenGL function loader           |
| ImGui       | Bundled (`include/`) | GUI (menu + object editor)       |
| stb_image   | Bundled (`src/models/`)| Texture file loading             |

### Build Steps
```bash
cmake -B build
cmake --build build
./build/Blackhole
```

Pre-build: all textures in `src/assets/` are preprocessed via `cargo run --release` (Rust CLI) into `.bhtx` files placed in `build/assets/`. This is handled by a `PreprocessTextures` custom target, which is a dependency of the `Blackhole` executable.

### Release Flags
```
-O3 -march=native -ffast-math   (compile)
-flto                           (link)
```

---

## 4. Application Architecture

### 4.1 Entry Point — `main.cpp`

```cpp
int main() {
    Game game;
    return game.run();
}
```

### 4.2 Game Lifecycle — `Game` class (`game.h` / `game.cpp`)

```
run()
  ├── initialize()
  │     ├── glfwInit()
  │     ├── glfwCreateWindow()
  │     ├── gladLoadGLLoader()
  │     ├── glEnable(GL_DEPTH_TEST | GL_MULTISAMPLE)
  │     ├── set GLFW callbacks (framebuffer, cursor pos, mouse button)
  │     └── initializeScene()
  │
  ├── [main loop: while !glfwWindowShouldClose]
  │     ├── processInput()                  ← WASD movement, ESC/TAB
  │     ├── startMenu_.beginFrame()         ← ImGui new frame
  │     │
  │     ├── if gameStopped_:
  │     │     └── renderMenuFrame()         ← StartMenu UI
  │     │         ├── "Start Game" → gameStopped_ = false
  │     │         ├── "Exit"       → close window
  │     │         ├── "Save"       → save_to_binary(plane, "saves/save.dat")
  │     │         └── "Load"       → load_from_binary(plane, "saves/save.dat")
  │     │
  │     ├── else:
  │     │     └── renderRunningFrame()
  │     │         ├── compute deltaTime, update camera.view via glm::lookAt
  │     │         ├── clear buffers
  │     │         ├── skybox_->render()
  │     │         ├── plane_.rotate(time)   ← orbital rotation (cos/sin)
  │     │         ├── plane_.draw()         ← render all objects
  │     │         ├── DoGravity()           ← N-body step
  │     │         └── objectEditor_.render() ← if TAB pressed, show editor
  │     │
  │     ├── startMenu_.endFrame()           ← ImGui render
  │     ├── glfwSwapBuffers()
  │     └── glfwPollEvents()
  │
  └── cleanup()
        ├── ImGui shutdown
        ├── delete GL buffers (VAO/VBO/EBO)
        ├── delete all Plane objects
        ├── skybox_.reset()
        └── glfwTerminate()
```
---

## 6. Rendering Pipeline

Per frame (not in menu):

1. **Skybox** — `GL_LEQUAL`, no translation, rotating around Y-axis at 0.13 rad/s
2. **Objects** — each `Object::draw()` binds texture, sets uniforms, draws indexed sphere mesh; light sources use `lightSourceShader`, others use `objectShader` (Phong)
3. **Gravity step** — `DoGravity()` updates velocities and positions
4. **Object Editor** — if open, renders ImGui panel + depth-less preview sphere

### Shaders

| Shader Pair               | Purpose                        |
|---------------------------|--------------------------------|
| `vertex.glsl` + `fragment.glsl` | Main Phong-lit objects    |
| `vertex.glsl` + `light_source_fragment.glsl` | Emissive sources |
| `skybox_vertex.glsl` + `skybox_fragment.glsl` | Cubemap skybox  |

---

## 7. Texture System

### Custom Binary Format — `.bhtx`

- **Magic**: `0x42585458` ("BXTX" ASCII)
- **Header**: version (u32), width (u32), height (u32), channels (u32), mipLevels (u32)
- **Mip levels**: per-level: width (u32), height (u32), dataSize (u32), pixel data
- **Pipeline**: `*.png/.jpg` → Rust CLI → `*.bhtx` in build dir

### Texture Loading (`Texture` class)

- Singleton cache (`std::unordered_map<std::string, GLuint>`)
- `LoadTexture()`: checks cache → tries `.bhtx` → falls back to stb_image
- `CleanupAll()` deletes all OpenGL texture IDs from cache

---

## 8. Save / Load System

Binary serialization format:
```
[version: size_t] [objectCount: size_t] [dt: double] [G: double]
[objects: Object[] via Object::serialize()]
```
Version check (`plane.version = 1`). Saves to `saves/save.dat`.

---

## 9. ImGui Integration

| Panel          | Class           | Toggle          | Features                                |
|----------------|-----------------|-----------------|------------------------------------------|
| Start Menu     | `StartMenu`     | On launch       | Start, Exit, Save, Load                 |
| Object Editor  | `Object_Editor` | TAB key         | Spawn objects, pos/vel/mass/radius/tex, blackhole/lightsource, preview, teleport |

Shared ImGui context created in `StartMenu::init()`.

---

## 10. Controls

| Input          | Action                                  |
|----------------|------------------------------------------|
| W / A / S / D  | Move camera forward / left / back / right |
| Mouse          | Look around (yaw/pitch)                  |
| ESC            | Close application                        |
| TAB            | Toggle Object Editor (when game running) |
| Right Mouse    | (reserved)                               |

Camera speed: `1000.0` u/s, mouse sensitivity: `0.1`.

---

## 11. Key Observations & Technical Notes

1. **C++ Standard**: C++20 required, code uses mainly C++17 patterns.
2. **Singleton Pattern**: Raw static `Game* instance_` for GLFW callbacks.
3. **Non-copyable Shader**: `shader` class is move-only.
4. **Memory**: Raw `new/delete` for `Object*`; manual cleanup.
5. **Rust tooling**: Texture preprocessor uses Rust edition 2024 with `image`, `anyhow`, `clap`.
6. **Linux focus**: Links `dl`, `pthread`, `m` on Unix; Windows separate section in CMake.
7. **No tests**: Project has no unit test framework.
8. **Scale**: `scale_ = 100000.0` when scaled; solar system spans ~3,000,000 units. Camera starts at `(300000, 0, 0)`.
9. **setupSolarSystem()**: Was commented out on startup, now enabled via uncommented call.

---

## 12. Git History (20 most recent commits)

```
909c713 — leak fixes
6a81b12 — idk snad to funguje
d41643a — Editor done (#12)
e9a0b5d — Editor done
edcaf3b — README Update
1a41a34 — simple MSAA antialiasing
49ba737 — BRUTALNI REFACTOR MAIN !!!!!!
be2857c — fix
3d42678 — preprocessed skybox nigiri
1441a30 — MRDKA VOLE
0fbffa7 — u
e332292 — bro
98cb095 — please
2035f68 — pokus 2
e78b0cd — added new line hopefully NOT on local account
7562d8d — Remove extra newline in vertex shader
82ed918 — changed structure for better lighting impl
336af95 — Kaja optimalizace (#10)
e9a34d9 — preprocessor & opti
6ea6b8a — improvements
```

---

## 13. Possible Future Improvements

- GPU-accelerated N-body via compute shaders
- Ray-marching for black hole gravitational lensing
- Post-processing: bloom, HDR, motion blur
- Better collision / merger physics
- Unit test framework
- Texture hot-reloading
- Config file for simulation parameters
### 4.3 Scene Initialization — `Game::initializeScene()`

```cpp
void Game::initializeScene() {
    skybox_ = std::make_unique<skybox>();
    objectShader_   = shader("vertex.glsl", "fragment.glsl");
    lightSourceShader_ = shader("vertex.glsl", "light_source_fragment.glsl");
    sphereMesh_ = createSphere(1.0f, 64, 32);
    textures.push_back(Texture::LoadTexture("assets/planet.png"));
    textures.push_back(Texture::LoadTexture("assets/poop-texture.jpg"));
    textures.push_back(Texture::LoadTexture("assets/sun-texture.jpg"));
    skybox_->loadPreprocessedTextures({6 cubemap faces});
    scale_ = scaleSystem ? 100000.0 : 1.0;
    plane_.G = 4.0 * M_PI * M_PI;
    plane_.dt = 0.0001;
    setupSolarSystem();  // Spawn Sun + 8 planets
}
```

### 4.4 Solar System Setup — `Game::setupSolarSystem()`

Creates 9 `Object` instances with realistic relative distances, masses (scaled by `scale_³`), and orbital velocities from `√(G / r)`.

| Object  | Distance (AU) | Relative Mass | Radius (× scale) |
|---------|---------------|---------------|-------------------|
| Sun     | 0             | 1.0 (scaled)  | 0.25 × scale      |
| Mercury | 0.387         | 1.65e-7       | 0.01 × scale      |
| Venus   | 0.723         | 2.45e-6       | 0.02 × scale      |
| Earth   | 1.0           | 3.00e-6       | 0.02 × scale      |
| Mars    | 1.524         | 3.23e-7       | 0.015 × scale     |
| Jupiter | 5.203         | 9.54e-4       | 0.08 × scale      |
| Saturn  | 9.537         | 2.86e-4       | 0.07 × scale      |
| Uranus  | 19.191        | 4.37e-5       | 0.05 × scale      |
| Neptune | 30.07         | 5.15e-5       | 0.05 × scale      |

Sun is flagged as `IsLightSource = true`.

---

## 5. Core Data Models

### 5.1 `Object` (core.h)

```cpp
struct Object {
    glm::mat4 modelMatrix;
    double xv, yv, zv;           // Velocity components
    glm::vec3 pos;                // Position
    GLuint VAO, VBO, EBO;
    double mass;
    size_t indexCount;
    float radius;
    GLuint textureId;
    bool IsBlackHole;
    bool IsLightSource = false;
    bool IsPreview = false;

    void draw(const shader&, glm::mat4 proj, glm::mat4 view) const;
    void rotate(float time);
    void serialize(std::ofstream&) const;
    bool deserialize(std::istream&);
    static Object* deserializeFrom(std::istream&);
};
```

### 5.2 `Plane` (core.h) — Simulation Container

```cpp
struct Plane {
    double G = 0.00675;
    double dt = 0.01;
    std::vector<Object*> objs;
    std::vector<glm::vec3> accelBuffer;
    std::vector<int> mergeTargetBuffer;
    const size_t version = 1;

    void draw(...) const;
    void rotate(float time);
};
```

### 5.3 Gravity — `DoGravity()` (core.cpp)

- O(n²) pairwise N-body integration
- Acceleration: `a = G * m * inv(r²+ε²)³ᐟ²` with softening ε = 0.05
- Object merging / accretion when bodies overlap (volume + mass summed)
- Black holes excluded from velocity updates (static attractors)
- Dead objects (mass ≤ 0 & radius ≤ 0) cleaned up after each step

### 5.4 Camera

Two camera types in `camera.h`:

| Type             | Default Position | Projection                      |
|------------------|------------------|----------------------------------|
| OrthoCamera      | (0, 200, 0.001)  | Orthographic (top-down)          |
| PerspectiveCamera| (0, 0, 5)        | Perspective 45° FOV              |

`Game` uses `PerspectiveCamera camera_`. View computed per-frame via `glm::lookAt()`. Far plane = `scale_ * 50.0`.
---

## 6. Rendering Pipeline

Per frame (not in menu):

1. **Skybox** — `GL_LEQUAL`, no translation, rotating around Y-axis at 0.13 rad/s
2. **Objects** — each `Object::draw()` binds texture, sets uniforms, draws indexed sphere mesh; light sources use `lightSourceShader`, others use `objectShader` (Phong)
3. **Gravity step** — `DoGravity()` updates velocities and positions
4. **Object Editor** — if open, renders ImGui panel + depth-less preview sphere

### Shaders

| Shader Pair               | Purpose                        |
|---------------------------|--------------------------------|
| `vertex.glsl` + `fragment.glsl` | Main Phong-lit objects    |
| `vertex.glsl` + `light_source_fragment.glsl` | Emissive sources |
| `skybox_vertex.glsl` + `skybox_fragment.glsl` | Cubemap skybox  |

---

## 7. Texture System

### Custom Binary Format — `.bhtx`

- **Magic**: `0x42585458` ("BXTX" ASCII)
- **Header**: version (u32), width (u32), height (u32), channels (u32), mipLevels (u32)
- **Mip levels**: per-level: width (u32), height (u32), dataSize (u32), pixel data
- **Pipeline**: `*.png/.jpg` → Rust CLI → `*.bhtx` in build dir

### Texture Loading (`Texture` class)

- Singleton cache (`std::unordered_map<std::string, GLuint>`)
- `LoadTexture()`: checks cache → tries `.bhtx` → falls back to stb_image
- `CleanupAll()` deletes all OpenGL texture IDs from cache

---

## 8. Save / Load System

Binary serialization format:
```
[version: size_t] [objectCount: size_t] [dt: double] [G: double]
[objects: Object[] via Object::serialize()]
```
Version check (`plane.version = 1`). Saves to `saves/save.dat`.

---

## 9. ImGui Integration

| Panel          | Class           | Toggle          | Features                                |
|----------------|-----------------|-----------------|------------------------------------------|
| Start Menu     | `StartMenu`     | On launch       | Start, Exit, Save, Load                 |
| Object Editor  | `Object_Editor` | TAB key         | Spawn objects, pos/vel/mass/radius/tex, blackhole/lightsource, preview, teleport |

Shared ImGui context created in `StartMenu::init()`.

---

## 10. Controls

| Input          | Action                                  |
|----------------|------------------------------------------|
| W / A / S / D  | Move camera forward / left / back / right |
| Mouse          | Look around (yaw/pitch)                  |
| ESC            | Close application                        |
| TAB            | Toggle Object Editor (when game running) |
| Right Mouse    | (reserved)                               |

Camera speed: `1000.0` u/s, mouse sensitivity: `0.1`.

---

## 11. Key Observations & Technical Notes

1. **C++ Standard**: C++20 required, code uses mainly C++17 patterns.
2. **Singleton Pattern**: Raw static `Game* instance_` for GLFW callbacks.
3. **Non-copyable Shader**: `shader` class is move-only.
4. **Memory**: Raw `new/delete` for `Object*`; manual cleanup.
5. **Rust tooling**: Texture preprocessor uses Rust edition 2024 with `image`, `anyhow`, `clap`.
6. **Linux focus**: Links `dl`, `pthread`, `m` on Unix; Windows separate section in CMake.
7. **No tests**: Project has no unit test framework.
8. **Scale**: `scale_ = 100000.0` when scaled; solar system spans ~3,000,000 units. Camera starts at `(300000, 0, 0)`.
9. **setupSolarSystem()**: Was commented out on startup, now enabled via uncommented call.

---

## 12. Git History (20 most recent commits)

```
909c713 — leak fixes
6a81b12 — idk snad to funguje
d41643a — Editor done (#12)
e9a0b5d — Editor done
edcaf3b — README Update
1a41a34 — simple MSAA antialiasing
49ba737 — BRUTALNI REFACTOR MAIN !!!!!!
be2857c — fix
3d42678 — preprocessed skybox nigiri
1441a30 — MRDKA VOLE
0fbffa7 — u
e332292 — bro
98cb095 — please
2035f68 — pokus 2
e78b0cd — added new line hopefully NOT on local account
7562d8d — Remove extra newline in vertex shader
82ed918 — changed structure for better lighting impl
336af95 — Kaja optimalizace (#10)
e9a34d9 — preprocessor & opti
6ea6b8a — improvements
```

---

## 13. Possible Future Improvements

- GPU-accelerated N-body via compute shaders
- Ray-marching for black hole gravitational lensing
- Post-processing: bloom, HDR, motion blur
- Better collision / merger physics
- Unit test framework
- Texture hot-reloading
- Config file for simulation parameters