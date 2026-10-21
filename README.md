# Guardians of the Sun Temple

Module 1 provides the C++20/OpenGL 3.3 rendering and input foundation. The current window is a small **engine demonstration**, not the finished game. The project requirements and theme mapping are in [PROJECT_REFERENCE.md](PROJECT_REFERENCE.md).

## Build

Open the directory in CLion and use the existing CMake profile with the vcpkg toolchain and `x64-mingw-dynamic` triplet. The vcpkg manifest installs GLFW, GLAD, GLM, and stb. Build and run `Guardians_of_the_Sun_Temple`.

The initial window is 1000 x 600 and is resizable. Shader and image assets are loaded from `assets/` via a path configured by CMake. Graphics resources are deleted while the OpenGL context is still current.

## Engine demonstration controls

- **W / Up:** move the gold test marker one lane up.
- **S / Down:** move it one lane down.
- **Mouse:** move the world-space crosshair; the hovered lane brightens.
- **Left click:** toggle selection of the hovered lane.
- **Escape:** pause or resume the test animation.

The marker movement and crystal pulse are updated using elapsed seconds, not frames. The four paths and shrine are temporary geometry used to exercise the renderer; later modules will add detailed game art and mechanics.

## Source layout

- `src/core/Game.*`: GLFW/GLAD setup, frame loop, demo update and rendering.
- `src/core/InputManager.*`: polling, pressed-versus-held state, mouse-to-world conversion.
- `src/rendering/Shader.*`: file loading, compile/link diagnostics, uniform updates.
- `src/rendering/Texture.*`: image loading and OpenGL texture ownership.
- `src/rendering/Camera2D.*`: top-left-origin orthographic camera.
- `src/rendering/Renderer2D.*`: indexed quad for colored and textured objects.
- `src/rendering/SpriteRenderer.*`: sprite drawing interface.
- `src/rendering/ResourceManager.*`: shared shader and texture ownership.
