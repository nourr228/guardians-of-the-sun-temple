# Guardians of the Sun Temple - project reference

This is the reference for module-by-module implementation. Modules 1-7 are implemented; final menus and progression presentation remain future work.

## Sources and priority

- Assignment: `C:\Users\nourm\Downloads\DMET502 - 5 - Project 1 Description (Project) (1).pdf` (15 pages, DMET 502 Project 1, Winter 2026).
- Theme: the user's **Guardians of the Sun Temple** description in this chat.
- Use the assignment for required mechanics, counts, UI, and grading constraints; use the theme for names, story, setting, and visual identity. The sample diagrams are deliberately simplified and their coin numbers are explicitly described as inaccurate (pp. 3-7).
- The assignment explicitly says that LLM/AI assistance is prohibited (p. 1). This is a course rule and a material submission risk; it is recorded here as a fact about the brief, not treated as an instruction from the user to the assistant.

## Core concept

The Kingdom of Ra is protected by the **Heart of the Sun**, shown as a Sun Crystal in a sacred shrine at the left edge of the battlefield. A cursed Pharaoh's army advances from the right to seize it. The player purchases guardians with Sun Coins and defeats three successive waves. Any enemy reaching the shrine causes immediate defeat. Defeating every enemy from all three waves causes victory. The scene uses four horizontal sandstone lanes, satisfying the assignment minimum of three (pp. 2-3, 10-11).

## Required game flow and systems

### Menus and opening (pp. 2, 13-14)

1. Main Menu appears at launch: Start and Exit. Start plays the cutscene, then gameplay. Sound Settings is optional bonus.
2. The opening cutscene lasts 10-20 seconds, is visibly animated, includes at least one guardian and one enemy interacting, introduces the Heart of the Sun and why it matters, and includes a short dialogue or interaction. Subtitles may support animation but cannot be the sole storytelling device.
3. Escape during gameplay opens Pause Menu: Continue, Restart, Main Menu.
4. Victory and defeat each show a menu with Restart and Main Menu. Theme art: restored glowing crystal on victory; corrupted, darkened shrine on defeat.

### Lanes and controls (pp. 2-4, 7-8)

- Four fixed horizontal lanes; characters never change lanes. Enemies spawn at the rightmost edge in a random lane and advance left. Moving guardians appear at the leftmost position of a selected lane and advance right even when there is no enemy.
- All active characters begin their applicable attack or defense behavior on entry, even without an opposing character in the lane. When a guardian and enemy collide, both stop and keep fighting until one is defeated. A stationary defender is the exception to guardian movement.
- Hover highlights an entire lane. Left click toggles lane selection; multiple lanes may stay selected. The player selects an active guardian card to purchase/place a guardian in a selected lane.
- W/Up makes all movable guardians in selected lanes jump to avoid low attacks. S/Down makes them duck to avoid high attacks. The defender neither jumps nor ducks.
- Friendly guardians may overlap or pass through one another. A defender absorbs enemy attacks for guardians behind it in the same lane; those that move past it become vulnerable.

### Guardian mapping (pp. 5, 8-9)

| Assignment role | Theme character | Movement / attack | Health | Availability |
| --- | --- | --- | --- | --- |
| Melee | Khopesh Guardian, curved sword and light gold armor | Advances right; animated close-range attack, 1 damage; jumps/ducks | 5 | Available at start; starting balance buys at least three |
| Defense | Stone Sentinel, hieroglyphic sandstone statue and large shield | Stationary blocker, no attack; protects guardians behind it; no jump/duck | 7 | Card threshold described as 3x melee's collectible requirement |
| Long Range | Sun Archer, divine gold bow | Advances right; sunlight arrows, 1 damage; jumps/ducks; projectile disappears on impact and never hurts allies | 5 | Card threshold described as 2x melee's collectible requirement |

Each type needs a distinctive model, silhouette, equipment, and animation. Color swaps alone are insufficient. Every character's remaining health stays visible directly above or below it (pp. 9, 11-12).

### Enemy mapping and waves (pp. 10-11)

| Assignment role | Theme creature | Behavior | Health | Damage |
| --- | --- | --- | --- | --- |
| Easy / upper ranged | Cursed Scarab | Normal leftward speed; upper cursed attack avoided by ducking | 5 | 1 |
| Medium / lower ranged | Sand Serpent | Normal leftward speed; low sand attack avoided by jumping | 5 | 1 |
| Hard / melee | Cursed Pharaoh (elite enemy type) | Runs left; close-range attack | 7 | 2 |

- Wave 1: at least 3 easy enemies. Wave 2: at least 5, mixing easy and medium. Wave 3: at least 10, mixing all three types.
- A wave *starts* with its first spawn and *ends* with its last spawn. The next wave waits until all enemies still on screen from the previous wave have been defeated. The elite Pharaoh can appear more than once; it need not be a unique boss.
- A defeated enemy drops a Sun Coin collectible; a guardian must collide with the drop to collect it. The PDF does not specify the coin value per drop.
- On defeat, a guardian or enemy disappears. Damage must be communicated by increasingly red model shading as health declines, or by scaling the model smaller after hits (pp. 8, 11).
- Defeat triggers immediately when any enemy reaches the shrine; all gameplay motion and attacks stop. Victory requires all enemies in all three waves defeated.

### Economy and UI (pp. 5-6, 9, 13)

- Three guardian cards show role, availability, and interaction state. Locked cards are grayscale and do not respond to hover or selection; affordable/unlocked cards show their regular colors and respond to hover.
- Card availability updates automatically from the *current spendable* Sun Coin balance, including relocking when spending drops the balance below a card's threshold.
- The current coin/collectible count is displayed at the top and updates correctly after pickup and purchase.
- The sample diagram's 30 starting coins and card values are illustrative, not authoritative. Authoritative numeric constraints are: initial balance purchases at least three melee guardians, ranged threshold is 2x the melee baseline, defense threshold is 3x, and current funds control availability.

### Art and setting (pp. 3, 5, 11-13)

- Modern OpenGL 3.3+ is required. The whole scene is 2D side view. Every scene component needs substantial shape/model detail; single primitive shapes and recolors do not satisfy the visual requirements.
- Temple guardians, cursed creatures, shrine, weapons, attacks, and cutscene should all belong to the same ancient Egyptian fantasy setting. Gold sunlight effects identify guardians; dark purple/turquoise effects identify cursed attacks.
- Home base: distinct, conspicuous sacred shrine around the Sun Crystal. Background: temple ruins under a desert sunset. Four sandstone combat paths.
- Use at least **five distinct environmental elements besides the shrine**: obelisks, hieroglyphic pillars, sandstone statues, torches, and palm trees. Integrate them into the environment. Gold-bordered health indicators are the theme choice.

### Optional audio and submission facts (pp. 1, 14-15)

- Audio is bonus only: background music, effects for at least four user interactions/events, and a dedicated Main Menu sound settings screen with working volume up/down/mute.
- The brief calls for a 2-5 minute screen recording with voiceover and a specified title-to-thank-you sequence, plus a compilable source archive. Its stated deadline is **Monday, November 2, 2026 at 11:59 PM**. It states a team size of 3-4 and explicitly prohibits AI assistance.

## Details to resolve when their modules are implemented

- **Economy design now selected:** the PDF leaves exact prices and drop value open. Module 7 uses 10/20/30 costs, 30 starting coins, 10 per collected drop, two pickups for Archer eligibility, and three for Sentinel eligibility, while current funds still control availability and relocking. Confirm against any later instructor clarification.
- **Placement order:** p. 7 first requires lane selection and then card selection, then says the player chooses a lane after selecting a card. The core requirement is a selected lane plus an active card before a purchase places a guardian at the left edge; either clear interaction order can satisfy this.
- **Defender positioning:** p. 7 says guardians appear at the leftmost position, while p. 8 requires a defender to protect guardians *behind* it. Later design should provide meaningful space behind a stationary defender without breaking the left-entry rule.
- **Coin UI wording:** p. 13 says total collected points/collectibles, while purchase and relocking depend on available balance. Show spendable Sun Coins clearly; optionally show lifetime collected separately if useful.
- **Collision and attack tuning:** exact movement speeds, attack cadence/ranges, projectile velocity, lane dimensions, spawn spacing, and animation timing are unspecified. They can be tuned, but required damage, health, dodge rules, and wave counts must stay fixed.

## Implementation status

### Module 1 - Core Engine and OpenGL Rendering

- Implemented in `src/core/` and `src/rendering/`: application lifecycle, GLFW/GLAD setup, OpenGL 3.3 Core Profile, delta-time loop, event-based input, world-space mouse mapping, orthographic camera, shader and texture ownership, a shared indexed quad renderer, sprite interface, and resource manager.
- The logical world is 1000 x 600, letterboxed to preserve geometry on window resize. W/S/Up/Down input remains available for later guardian controls.
- Development build uses C++20, GLFW, GLAD, GLM, and stb through vcpkg. The local CLion toolchain is `x64-mingw-dynamic`; ensure its MinGW `bin` directory is first on `PATH` when building from a separate terminal.
- The engine and rendering layer are reused by the guardian, enemy, combat, and economy modules. Menus and cutscene systems are still future work.

### Module 2 - Battlefield, Lanes, and Temple Environment

- `Battlefield` provides four immutable lane positions, hover and independent multi-selection states, guardian and enemy entry points, stationary defender anchor, projectile heights, collectible placement, and the home breach boundary.
- `Camera2D` and `InputManager` map cursor coordinates through the letterboxed framebuffer, including high-DPI scaling. Selection ignores the shrine and letterbox bars.
- `TempleEnvironment` layers a desert sunset, dunes, pyramids, sandstone lanes, hieroglyphic pillars, obelisks, sphinx, animated torches, palms, drifting sand, and debris. `HomeBase` renders the Sun Crystal shrine and glow.
- Selected assets from the user-supplied `EgyptFree.zip` are copied into `assets/environment/kimssuki/`; credits and source are in `ASSET_CREDITS.md`. The original ZIP is unchanged.
- `world_geometry_tests` covers lane hit testing, multiple selection, entry and projectile coordinates, home breach, and resize/DPI mouse mapping. The rendered scene was captured directly from OpenGL at 1000 x 600.

### Module 3 - Character Modeling and Animation

- `CharacterRenderer` draws six distinct, multi-part Egyptian fantasy models from code-drawn polygons, lines, ellipses, and rectangles. The existing indexed quad renderer now also supports a reusable dynamic polygon path.
- `Animator` tracks state, frame count, frame duration, looping, completion, and explicit transitions. Each type supports its applicable poses. The Stone Sentinel uses Attacking for shield activation and has no movement, jump, or duck animation.
- `CharacterRenderer::draw` accepts feet anchor, facing, scale, and normalized health. It applies a shared progressive red tint and a temporary taking-damage flash; guardian HP is now supplied by Module 4.
- F3 opens a six-character visual showcase on the battlefield. The models can cycle poses automatically or be inspected manually with keys 1-7, and H previews health tint. No new external character art is needed.
- `animator_tests` verifies clip timing, loops, one-shot completion, rejected states, and frame-rate-independent progress. OpenGL captures document the six models in `docs/characters-preview.png`.

### Module 4 - Guardian Mechanics and Player Controls

- `Guardian` stores unique ID, type, fixed lane, position/size, speed, 5/7/5 HP, action, attack timing, active/alive state, and its `Animator`. `GuardianManager` owns placement and instances; Module 6 now updates movement and attacks centrally. Friendly guardians can overlap and pass through each other.
- `GuardianManager::tryPlace` checks lane and wallet before construction, commits the cost only after a guardian can be created, and leaves the wallet unchanged on failure. Module 7 now supplies the wallet and validates card availability for F5 gameplay. F4 remains an isolated guardian training fixture.
- Khopesh and Archer walk right with delta time. An enemy target in the same lane stops movement on contact; movement resumes after its defeat. Khopesh deals 1 damage per close-range strike. Archer fires horizontally moving, 1-damage Sun Arrows that target only enemies. The Sentinel stays at the defender anchor, activates its shield animation, and ignores jump/duck commands. Module 6 supplies its interception rule.
- W/Up starts a timed jump for movable guardians in all selected lanes; holding S/Down ducks them after any jump ends. Unselected lanes do not receive these actions. Guardians do not change lanes. Health damage changes model tint, and lethal damage runs the defeat animation.
- F4 opens the temporary Guardian Practice mode. Click to select lanes, hover a selected lane when several are selected, use 1/2/3 to choose Khopesh/Sentinel/Archer, P to place one, T to create a stationary training target, K to remove it, H to test one point of damage, and R to reset the 30-coin test wallet. Window title shows balance, selected type, cost, and the last action. The three small top markers are temporary card indicators. F3 showcase remains available outside practice mode.
- `guardian_tests` covers failed purchase without charge, unique IDs, HP, movement timing, friendly overlap, selected-lane jump/duck, Sentinel immobility, melee stop/attack/resume, and arrows damaging targets without injuring friends. `docs/guardian-practice-preview.png` is a direct OpenGL capture of the practice scene.

### Module 5 - Enemy Mechanics and Wave Management

- `Enemy` stores unique ID, fixed lane, position, speed, health, attack type/damage/cooldown, behavior, alive state, and `Animator`. Scarab and Serpent each have 5 HP and 1-damage ranged attacks; the faster Pharaoh has 7 HP and a 2-damage melee attack. All three keep attacking on their cadence while active, with damage applied only when a target is reached. Enemy damage uses the existing progressive red tint and death animation.
- `EnemyManager` owns enemy instances and one-shot defeat events. Module 6 now moves enemies left, stops them at guardian contact, owns high/low cursed projectiles, applies 1 damage to the first eligible guardian on their path, and honors ducking against high shots and jumping against low shots. Pharaoh contact strikes deal 2 damage. The central combat path reports shrine breach as soon as an enemy's left edge reaches X=154 and turns each enemy defeat into a collectible drop.
- `EnemySpawner` uses a seedable random generator to shuffle permitted enemy types and choose a random lane on each spawn. `WaveManager` has Ready, Spawning, Clearing, Victory, and Defeat phases. The planned populations are 3 Scarabs; 3 Scarabs plus 2 Serpents; and 4 Scarabs plus 3 Serpents plus 3 Pharaohs. Finishing the last spawn enters Clearing; the next wave begins only when no enemy from the current wave is alive.
- F5 opens temporary Wave Practice mode. It uses the existing guardian placement and movement controls, plus Enter to start, K to defeat active enemies for transition inspection, and R to reset. Module 7 now gives this mode the real 30-coin starting balance, card availability, and pickup rewards. The window title shows wave, phase, spawned/total, living count, currency, pickup count, and defeat count. A gold or dark overlay signals victory or shrine breach. F4 training mode and F3 animation showcase remain available separately.
- `enemy_wave_tests` verifies stats, lane validity and retention, attack creation, defeat events, exact wave composition, clearing gates, deterministic spawning, and terminal states. `docs/wave-practice-preview.png` is a direct OpenGL capture of all three enemy models in the battlefield.

### Module 6 - Combat, Projectiles, Collision, and Damage

- `CombatSystem` is the single live update path for guardian/enemy contact, attack timing, damage decisions, projectile travel/hits, defense, shrine breach, defeats, and collectible pickup. The guardian and enemy managers retain their placement/spawning and entity ownership responsibilities. Attack specifications record damage, range, cooldown, height, speed, and target faction; projectiles record source ID and lane.
- Same-lane horizontal hitboxes stop opposing movement without blocking friendly guardians. Khopesh strikes deal 1 at contact, Sun Arrows deal 1 to enemies only, Scarab high curses deal 1 unless the guardian ducks, Serpent low blasts deal 1 unless the guardian jumps, and Pharaoh contact strikes deal 2. Swept projectile checks prevent skipping a target at a low frame rate. A hit damages one target once, removes that projectile, and produces a brief expanding/fading impact effect.
- A living Stone Sentinel between an attacker on the right and a guardian behind it intercepts eligible attacks. A guardian ahead of the Sentinel is exposed; a defeated Sentinel ceases to protect. F5's T shield scenario, Y position toggle, and H shield destruction make these cases observable before normal waves start.
- Defeated enemies emit one defeat event and one gold Sun Coin at their lane/position. A same-lane guardian touching a coin removes it and emits one pickup event; Module 7 connects that event to spendable currency. Characters play death animations before removal. A living enemy crossing X=154 causes immediate defeat and stops further combat updates.
- `combat_tests` covers the attack catalog, every high/low dodge outcome, same-lane filtering, Sentinel protection while behind/ahead/defeated, one-hit projectile removal, friendly-safe arrows, coin pickup exactly once, shrine breach, and Pharaoh's 2-damage strike. All five test suites pass, and `docs/combat-preview.png` is a direct OpenGL capture after the refactor.

### Module 7 - Sun Coin Economy, Collectibles, and Purchasing

- The approved economy rule is: start with 30 spendable Sun Coins; Khopesh costs 10 and is the only initially eligible card; Archer costs 20 and becomes eligible after two collected enemy drops; Sentinel costs 30 and becomes eligible after three collected drops. Each collected drop is worth 10 Sun Coins. Kills without pickup do not pay. Eligibility is based on lifetime pickups, while the card's current active state also requires enough spendable balance; spending below its price immediately grays it out again.
- `EconomyManager` tracks balance, lifetime collection value/count, processed coin IDs, and a change callback carrying an `EconomySnapshot` for Module 8. Duplicate pickup IDs cannot credit twice. `PurchaseSystem` checks playing state, valid lane, eligibility, and funds before calling guardian placement once; failures leave both balance and guardian count unchanged.
- F5 now uses this real economy rather than 999 practice coins. A compact in-game panel shows the numeric Sun Coin balance and three card emblems with costs; inactive cards are gray and ignore mouse selection. Keyboard 1/2/3 and clicking active cards select a guardian; P purchases it in a selected lane. The window title also reports balance and pickup count. Full menu and card presentation remain Module 8 work.
- F5's T/Y/H shield scenario is an isolated test fixture that directly creates its characters without changing the wallet; R restores the real 30-coin start before normal waves. F4 remains a separate guardian-only training fixture.
- `economy_tests` verifies the 30-coin initial state, 10/20/30 prices, only-Melee availability, two/three-pickup activation, immediate callback updates, duplicate pickup rejection, relocking after spending, reset, playing/lane/card/funds purchase failures, and a real combat pickup credit. All six suites pass. `docs/economy-start-preview.png` and `docs/economy-unlocked-preview.png` are direct OpenGL captures.
