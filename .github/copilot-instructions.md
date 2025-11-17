# Bleep Developer Guide
## Layout & dependencies
- Root `CMakeLists.txt` builds vendored Corrade/Magnum/Assimp/ImGui targets before `add_subdirectory(bleep)`, so treat `/corrade`, `/magnum*`, `/assimp`, `/imgui` as third-party — don't edit unless syncing upstream.
- Simulation code lives in `bleep/`: `include/` holds math/control layers, `src/` hosts the Magnum entry point, and `models/` stores STL geometry shipped into the binary dir during configure.
- `bleep/CMakeLists.txt` sets `MODELS_DIR` and copies `bleep/models` into `CMAKE_BINARY_DIR`; every `Graphics*` class expects that macro to resolve before instantiating `MeshDrawable`.
- UI layout persistence is driven by the repo-level `imgui.ini`, but `bleep/src/main.cpp` hardcodes its absolute path, so adjust `io.IniFilename` when cloning to a different location.

## Build & run
- Configure once with `cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug` (from repo root) to generate projects for Corrade/Magnum plugins plus the `bleep` executable.
- Build via `cmake --build build --target bleep`; this also compiles `MagnumPlugins::AssimpImporter`, required because `meshDrawable.h` loads STL meshes at runtime.
- Run `./build/bleep/bleep` so that the process cwd already contains the copied `models` folder; launching from elsewhere requires `MODELS_DIR` assets to be reachable.
- SDL must have joystick support enabled before `Controller::init()` runs; the app currently expects `SDL_Init(SDL_INIT_VIDEO|SDL_INIT_JOYSTICK|SDL_INIT_GAMECONTROLLER)` inside `Platform::Application`.

## Runtime architecture
- `bleep/src/main.cpp` defines `MyApplication`, wiring Magnum’s GL loop, `_timeline`, `ArcBallCamera`, ImGui windows, and global state objects (`debuggingLeg`, `GraphicsBody`, serial buffers, controller pointer).
- `Controller` (`bleep/include/controller.h`) wraps SDL joystick events plus macOS IOHID rumble activation for DualShock 3 pads; `controller->showGUI()` renders the on-screen sticks and exposes `init_gamepad()`.
- `Body`/`GraphicsBody` (`bleep/include/body.h`, `bleep/include/graphicsBody.h`) manage the hexapod: leg offsets, `_gaitOrder`, walking vs standing mode, and ImGui control panes for `_stepTime/_stepSize/_stepHeight`.
- `Leg`/`GraphicsLeg`/`Joint`/`GraphicsJoint` compose the kinematic chain (3 DOF per leg) and their drawables; `GraphicsLeg::CalculateIK()` mirrors `Leg::CalculateIK()` but also keeps STL meshes/cubes in sync.

## Data flow & hardware integration
- SDL events in `MyApplication::anyEvent` populate `Controller::leftJoystick/rightJoystick`; `GraphicsBody::WalkingMode()` consumes those vectors to move the body and issue `Leg::NewAnimation()` commands.
- After `GraphicsBody::getAllJointAngles()` packs servo targets, `MyApplication::drawEvent()` formats them via `Body::intArrayToString()` and optionally streams the `<v0:...:v17>\n` payload through `writeSerialData`.
- `renderGUI()` owns the “Serial Control” window: `parseSerialPorts(getSerialPorts())` lists `/dev/tty.*`, the “Connect” button prepends `/dev/tty.` before calling `openAndConfigureSerialPort`, and the scrolling region shows `receivedData`.
- `MacSerialPort/SerialPort.cpp` assumes macOS-style device naming and `termios`; if you need cross-platform support, isolate those calls behind new adapters instead of editing the existing implementation.

## Controls & debugging workflow
- Runtime ImGui windows (“Leg Debugging”, “Serial Control”, “Mode Selection”, “Number Input Fields”, `GraphicsBody::showPhases`) are created inside `renderGUI()`; keep ImGui state changes bracketed by `_imgui.newFrame()` and `_imgui.drawFrame()`.
- `GraphicsLeg::showDebugging()` supplies radio buttons to toggle mesh/cube visibility, edit joint lengths/scales, and print computed IK values, making it the first stop when leg motion looks wrong.
- The walking scheduler relies on `Phase` (three slots cycling `IDLE/SCHEDULED/ENGAGED`) plus `_gaitToggle`; watch the “Status” window to verify that legs finish animations before the next phase is armed.
- Controller troubleshooting: “Search...” in `Controller::showGUI()` runs `init_gamepad()` (DualShock 3 vendor 0x054C/product 0x0268) and the app polls one SDL `GameController`; expect `Controller::init()` to open indices 0/1.

## Conventions & tips
- Magnum math types (`Vector3`, `Quaternion`, `Color3`) are used everywhere; keep values in radians and leverage the `_degf` literal from `Math::Literals` when degrees are unavoidable.
- Movement tuning lives in the globals defined near the top of `bleep/src/main.cpp` (`_stepTime`, `_stepSize`, `_stepHeight`); ImGui exposes them, but they’re also read directly by `Leg::HandleAnimation()`, so update before animating.
- Serial packets are always 18 integers; `Number Input Fields` writes into the shared `values` array and calls `updateStringFromValues()` so manual overrides stay in the same `<...>` format expected by firmware.
- Mesh visibility and transforms are centralized in `GraphicsJoint::updateDrawObject()` and `CubeDrawable`; when adding hardware (e.g., sensors), prefer new drawables over embedding OpenGL calls in simulation code.
