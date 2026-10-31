# Guardians of the Sun Temple

Modules 1-6 provide the C++20/OpenGL 3.3 engine, four-lane battlefield, six animated character models, three guardian types, three enemy waves, and unified combat. The final spendable coin economy, menus, and progression presentation remain for later modules. The assignment requirements and theme mapping are in [PROJECT_REFERENCE.md](PROJECT_REFERENCE.md).

![Battlefield preview](docs/battlefield-preview.png)

![Six character models](docs/characters-preview.png)

![Guardian practice preview](docs/guardian-practice-preview.png)

![Enemy wave preview](docs/wave-practice-preview.png)

![Unified combat preview](docs/combat-preview.png)

## Build and run

Open this directory in CLion using the existing CMake profile, vcpkg toolchain, and `x64-mingw-dynamic` triplet. Build and run `Guardians_of_the_Sun_Temple`. The vcpkg manifest installs GLFW, GLAD, GLM, and stb.

On Windows, the build copies GLFW and the MinGW runtime DLLs beside the executable, so the built game can also launch directly from `cmake-build-debug/`.

The window begins at 1000 x 600 and can be resized. The scene keeps a 1000 x 600 logical world with letterboxing. Mouse input is converted through the active viewport, so lane selection continues to align after resizing and on high-DPI displays.

## Current controls

- Move the mouse over a path to highlight its full playable area.
- Left click a path to select or deselect it. Several paths can stay selected.
- Escape pauses or resumes environment animation and lane selection.
- F3 shows or hides the six-character animation showcase.
- In the showcase, 1-7 selects Idle, Moving, Attacking, Jumping, Ducking, TakingDamage, or Dying. Unsupported poses stay Idle.
- In the showcase, 0 resumes automatic pose cycling; H steps through visual health levels, and R restores full visual health.

## Test the guardians (F4)

- Press F4 to open or close Guardian Practice mode.
- Click a lane to select it. Several lanes can stay selected; hover the intended lane before placing if several are selected.
- Press 1 for Khopesh (10 coins), 2 for Stone Sentinel (30), or 3 for Sun Archer (20). Press P to place one guardian in the chosen lane. The temporary wallet starts with 30 coins; its balance and action messages appear in the window title.
- Press T to add a stationary training target in the chosen lane. Khopesh should stop at it, strike it, and continue after it falls. Archer should fire Sun Arrows that hit it without hurting friendly guardians. Press K to remove the target.
- Press W or Up to make all movable guardians in selected lanes jump; hold S or Down to duck. The Sentinel stays still and never jumps or ducks.
- Press H to damage all guardians in the chosen lane by 1 point; their gold-bordered health bars shrink and models redden. Press R to reset guardians, targets, and wallet so each type can be tried from the initial balance.

This is a temporary test mode. Training targets, wallet, and card markers will be replaced by later modules. F4 now uses the same combat collision and damage rules as F5.

## Test the enemy waves (F5)

- Press F5 to open Wave Practice mode. It is separate from F4; its 999 Sun Coins are a test allowance, not the final starting balance.
- Click a lane, press 1/2/3 to choose Khopesh/Sentinel/Archer, and press P to place one. Repeat in several lanes; you can continue placing during a wave.
- Press Enter to start Wave 1. The window title shows the wave number, whether it is spawning or clearing, spawned/total count, and living enemy count. Wave 1 has 3 Scarabs; Wave 2 has 3 Scarabs and 2 Serpents; Wave 3 has 4 Scarabs, 3 Serpents, and 3 Pharaohs.
- Use W/Up to jump over low Serpent attacks and hold S/Down to duck under high Scarab attacks. Pharaohs strike at close range for 2 damage. Enemy and guardian health bars show their remaining HP.
- To inspect wave transitions quickly, press K to defeat current enemies. If the wave is still spawning, later enemies will still appear. The next wave begins only after its last spawn and all its enemies are defeated. Press R to reset the mode.
- If any enemy reaches the shrine, the action stops and the scene darkens. Clearing all three waves produces a gold victory overlay. F5 closes the mode.

## Test combat and Sentinel protection (F5)

- For a repeatable shield test, press **R** in F5, then **T**. This creates a stationary Khopesh Guardian behind a Stone Sentinel and a nearby Scarab in lane 1. The first high cursed shots should reduce the Sentinel's health while the Khopesh remains at 5 HP.
- Press **Y** to move the Khopesh ahead of the Sentinel. Later shots should damage the Khopesh directly. Press Y again to return it behind the shield.
- Press **H** to destroy the Sentinel. Subsequent shots can now hit the guardian behind it. Press **R** to clear this test before starting the normal waves.
- In normal waves, defeated enemies leave gold Sun Coins at their defeat positions. A guardian collects a coin by touching it; the window title shows the pickup count. This pickup event is ready for Module 7, where it will increase spendable currency.

## World coordinates

World coordinates start at the upper-left corner. X increases to the right and Y increases downward. `Battlefield` owns four fixed lanes, each 100 logical units high. Its API supplies lane bounds, guardian and enemy entry points, a stationary defender anchor, high/middle/low projectile positions, collectible positions, hover state, and selected lane IDs. `HomeBase::reachedBy` checks the loss boundary at X = 154.

## Scene and assets

`TempleEnvironment` renders layered sky, dunes, pyramids, lanes, sandstone details, moving sand, torch flames, obelisks, pillars, a sphinx, and palms. `HomeBase` renders the Sun Crystal shrine with a pulsing glow. The props from the free Egypt pack are credited in [ASSET_CREDITS.md](ASSET_CREDITS.md). The original ZIP remains untouched; only the files used by the scene were copied into `assets/environment/kimssuki/`.

The six characters are built from code-drawn shapes and articulated parts, so no new character image assets are required. `CharacterRenderer::draw` accepts a character type, animator, feet position, facing, scale, and normalized health from the guardian and enemy systems. The Stone Sentinel's Attacking pose is its shield activation, not a damage-dealing attack.

## Tests

Run `ctest --test-dir cmake-build-debug --output-on-failure`. The five suites cover lane geometry, animation, guardian placement and movement, exact wave composition and gating, and combat damage, dodge, shielding, projectile removal, pickups, and shrine breach.

For a one-frame OpenGL capture, set `GOST_CAPTURE_PATH` to a PNG path before running the game. The application renders one frame, saves it, then exits.

For repeatable showcase captures, set `GOST_PREVIEW_STATE` to 0-6, optionally `GOST_PREVIEW_TIME` to seconds into that clip and `GOST_PREVIEW_DAMAGE` to 0-3, along with `GOST_CAPTURE_PATH`.

Set `GOST_PRACTICE_PREVIEW=1` with `GOST_CAPTURE_PATH` to capture a sample F4 guardian scene.

Set `GOST_WAVE_PREVIEW=1` with `GOST_CAPTURE_PATH` to capture a sample F5 enemy scene.

## Source layout

- `src/core/`: GLFW/GLAD lifecycle, delta-time loop, and input state.
- `src/rendering/`: shader and texture ownership, indexed quads and polygon renderer, sprites, orthographic camera, character models, and animator.
- `src/world/`: lane geometry and selection, shrine boundary, and temple environment.
- `src/entities/` and `src/systems/`: guardian/enemy state, placement, random spawning, and wave phases.
- `src/combat/`: central contact, attack, projectile, damage, protection, and coin pickup rules.
- `src/demo/`: character showcase and temporary guardian/wave practice modes.
