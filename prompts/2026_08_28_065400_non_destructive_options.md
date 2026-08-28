# Architectural Provenance: Make configuration screen non-destructive to game state

- **Session Date:** `2026-08-28T05:50:56Z`
- **Agent:** Antigravity CLI 1.1.22 Gemini 3.7 Flash (High)
- **Base Commit:** [`d1ddb472`](https://github.com/Neverball/neverball/commit/d1ddb47283819d4586004ad9792e839fb7d4107f): `ball/st_demo: smaller "Replay Paused" label` (2026-08-28 05:42:29 UTC)
- **Commits:**
  - [`32234315`](https://github.com/Neverball/neverball/commit/32234315de2c8df4a9c50880a3de0a5a55abd6f0): `share/part: Free coin_ebo in part_free` (2026-08-28 07:02:51 UTC)
  - [`7e707536`](https://github.com/Neverball/neverball/commit/7e707536d8af2347f100bf7e46939f6be691fd1e): `share/geom: Implement background stack` (2026-08-28 07:12:36 UTC)
  - [`b1f1d348`](https://github.com/Neverball/neverball/commit/b1f1d3483a1e7505e06cdc445d8e810878ffa292): `ball/game_client: Add game_client_state and GPU object reload` (2026-08-28 07:12:36 UTC)
  - [`81ea7559`](https://github.com/Neverball/neverball/commit/81ea7559ffbd2346dc8fea597e1ce55d11beed08): `ball/game_server: Add game_server_state` (2026-08-28 07:12:36 UTC)
  - [`4c8b6822`](https://github.com/Neverball/neverball/commit/4c8b68224ebb7348ec39761be887c2fb80f0b370): `ball/st_conf: Make configuration screen non-destructive` (2026-08-28 07:12:36 UTC)
  - [`cfe176a8`](https://github.com/Neverball/neverball/commit/cfe176a86ceb265dc927bf4c8308055bb16490b8): `ball/st_name: Do not free game client on name entry` (2026-08-28 07:23:16 UTC)
  - [`18449b6b`](https://github.com/Neverball/neverball/commit/18449b6b44eae63c606e7949f0ee47dbbdebf17c): `ball/st_ball: Support in-game ball model preview` (2026-08-28 07:23:16 UTC)
  - [`5fb03234`](https://github.com/Neverball/neverball/commit/5fb0323456e85fc8676ca2b3c7bb8d80a779f747): `ball/st_pause: Add Options button to pause menu` (2026-08-28 07:23:16 UTC)
  - [`f3dd27b1`](https://github.com/Neverball/neverball/commit/f3dd27b1bb15ef60df1c457295a3d5452fd3385e): `putt: Add Options button to pause menu and make configuration non-destructive` (2026-08-28 07:23:16 UTC)
- **Files Modified:** `ball/game_client.c`, `ball/game_client.h`, `ball/game_server.c`, `ball/game_server.h`, `ball/st_ball.c`, `ball/st_conf.c`, `ball/st_name.c`, `ball/st_pause.c`, `putt/game.c`, `putt/game.h`, `putt/st_all.c`, `putt/st_conf.c`, `share/geom.c`, `share/geom.h`, `share/part.c`, `share/st_common.c`, `share/st_common.h`

---

## Annotated Prompt Log

### Initial Problem & Hypothesis

```text
[05:50:56] /plan Investigate the changes required to make the options screen non-destructive to game state. E.g., if a player is in the middle of the game, they can freely pause the game, change settings, and return to the game without an issue. The focus here is on mapping the changes required to the options screen/setup/teardown.
```

```text
[06:22:05] /plan Investigate the changes required to make the options screen non-destructive to game state. E.g., if a player is in the middle of the game, they can freely pause the game, change settings, and return to the game without an issue. The focus here is on mapping the changes required to the options screen/setup/teardown.
```

```text
[06:22:42] /plan Investigate the changes required to make the options screen non-destructive to game state. E.g., if a player is in the middle of the game, they can freely pause the game, change settings, and return to the game without an issue. The focus here is on mapping the changes required to the options screen/setup/teardown.
```

Initial exploration across `ball/st_conf.c`, `ball/st_pause.c`, `ball/game_client.c`, `share/geom.c`, and OpenGL context management identified four primary architectural barriers to mid-game options navigation:
1. **Destructive Teardown in `st_conf` and `st_name`:** `conf_enter()` in `ball/st_conf.c` and `name_enter()` in `ball/st_name.c` unconditionally called `game_client_free(NULL)`, deallocating active level geometry meshes (`gd.draw`), physics simulation nodes, and vary interpolation data structures (`gd.vary`).
2. **Hardcoded State Transitions:** `conf_action()` in `ball/st_conf.c` and `putt/st_conf.c` hardcoded the exit transition to `exit_state(&st_title)`, preventing return to `st_pause`.
3. **GPU Resource Loss on Context Recreation:** Video mode changes, resolution adjustments, and language switches trigger `st_null` (`null_enter` / `null_leave`), deallocating OpenGL textures and mesh vertex buffers (`sol_free_draw`). Without explicit reload hooks for active level meshes, resuming gameplay caused corrupted rendering or crashes.
4. **Single-Slot Background Gradient State:** `share/geom.c` stored a single static background structure (`back_init` / `back_free`). Entering options invoked `back_init("back/gui.png")`, clobbering the active level's sky gradient texture.

The initial implementation plan artifact mapped state machine routing, caller preservation via `conf_back`, GPU reload hooks (`game_client_free_objects` / `game_client_load_objects`), and pause menu button integration.

### Constraint Discovery & Design Pivots

```text
[06:34:29] FYI, VS Code Mermaid parser complains: Parse error on line 9:
```

The Mermaid state machine diagram in the plan artifact was restructured with standard syntax, explicit subgraph identifiers, and `<br/>` linebreaks to ensure compatibility across Markdown parsers.

```text
[06:38:18] Give up goes to st_title? That seems wildly wrong, but I've never checked these diagrams before - maybe you don't even pay attention to them yourself?
```

In `ball/st_pause.c`, selecting "Give Up" (`PAUSE_EXIT`) invokes `goto_exit()` (`ball/st_level.c`), routing conditionally by game mode: `st_start` (Level Select) in standard play, `st_over` (Game Over) in Challenge mode, `st_done` upon set completion, or application exit in standalone mode. The plan diagram was corrected to reflect `goto_exit()` routing rather than `st_title`.

```text
[06:43:45] Regarding the ball model screen clobbering the loaded game, perhaps there is a possibility of showing that loaded level instead of the replay? Meaning, the ball is visible in-game so why not use that? Ultimately it would be good to re-architect ball rendering such that multiple balls (and level geometry) can be rendered simultaneously, but we're not there yet.
```

In `ball/st_ball.c`, the ball selection screen previously loaded a standalone demo replay (`gui/ball.nbr`) via `demo_replay_init()`, destroying active game simulation state.
The design was updated to support in-game preview:
- When entered during active gameplay, `ball_enter()` skips `load_ball_demo()`.
- `ball_paint()` draws the paused level geometry and player ball in-place with `game_client_draw(0, t)`.
- `ball_timer()` steps only `gui_timer()`, skipping replay playback.
- Cycling ball models with `GUI_PREV` / `GUI_NEXT` invokes `ball_free(); ball_init();`, immediately updating the rendered ball model in the paused level scene.

```text
[06:47:30] game_client_active -> game_client_state (follows convention of "state" representing a loaded state of a module)
```

The client state query helper was named `game_client_state()` across `ball/game_client.h` and `ball/game_client.c` to conform with codebase naming conventions (`audio_state()`, `geom_state()`, `server_state()`).

```text
[06:50:41] Possible to implement a simple stack for back_init? back_push/back_pop? So there's no need for an awkward game_client_restore_background
```

`share/geom.h` and `share/geom.c` were redesigned to maintain a LIFO background stack (`back_stack`) with capacity `BACK_STACK_MAX` (8):
- `back_push(name)` pushes a new background layer onto the stack.
- `back_pop()` pops and deallocates the top layer, reactivating the underlying background.
- Compatibility aliases `back_init` and `back_free` were retained as wrappers forwarding to `back_push` and `back_pop`.
- `back_free_objects()` and `back_load_objects()` iterate through active stack entries to recreate OpenGL textures and mesh draw objects across `st_null` context resets.

```text
[06:54:31] Looks good. One last change: observe vertical whitespace, use it to improve readability, don't lump additions with existing lines unless they belong together.
```

Vertical whitespace and visual grouping were adjusted across all code snippets and unified diff blocks in the plan artifact.

### Implementation & Atomic Commits

```text
[06:56:20] [Approved] non_destructive_options_screen_plan.md
```

Development commenced in isolated worktree `.worktrees/non-destructive-options`, landing the initial implementation across atomic commits:
1. `share/part.c`: Fixed a GPU buffer leak in `part_free()` by deleting `coin_ebo` alongside `coin_vbo` (`glDeleteBuffers_(1, &coin_ebo)`).
2. `share/geom.c`, `share/geom.h`: Implemented `back_push()`, `back_pop()`, `back_free_objects()`, and `back_load_objects()`.
3. `ball/game_client.c`, `ball/game_client.h`: Implemented `game_client_state()`, `game_client_free_objects()`, and `game_client_load_objects()`.
4. `ball/st_conf.c`: Added `conf_back` caller tracking, removed `game_client_free(NULL)`, and hooked `game_client_free_objects()` / `game_client_load_objects()` into `null_enter` / `null_leave`.
5. `ball/st_name.c`: Removed `game_client_free(NULL)` in `name_enter()`.
6. `ball/st_ball.c`: Added in-game level preview via `game_client_draw(0, t)`.
7. `ball/st_pause.c`: Added `PAUSE_OPTIONS` button to pause menu and protected `st_continue` from being overwritten when returning from options.
8. `putt/`: Added `game_free_objects()` and `game_load_objects()` to `putt/game.c`, made `putt/st_conf.c` non-destructive with `conf_back`, and added `PAUSE_OPTIONS` to `putt/st_all.c`.

### Iterative Diagnosis & Refinements

```text
[07:04:32] Intermission music is gone from the options menu when entering from the title screen. I feel like that should be preserved.
```

On the title screen, `st_title` initializes `gui/title.sol` via `game_client_init()`, causing `game_client_state()` to evaluate to `1`. Consequently, `conf_enter()`'s check `if (!game_client_state())` skipped `audio_music_fade_to(0.5f, "bgm/inter.ogg")`. Additionally, `share/st_common.c` contained an unconditional `audio_music_fade_to` call in `conf_common_init()`.
The unconditional fade was removed from `conf_common_init()`, and `conf_enter()` was temporarily adjusted to check `curr_mode() == MODE_NONE`.

```text
[07:06:55] Intermission music still doesn't play. Investigate further.
```

In `ball/progress.c`, `static int mode = MODE_NORMAL;` is statically initialized to `MODE_NORMAL` and is never set to `MODE_NONE`. Therefore, checking `curr_mode() == MODE_NONE` evaluated to `false` even on the title screen.
To distinguish active gameplay from title screen and menu contexts, `game_server_state()` (`return sd.state != 0;`) was added to `ball/game_server.h` and `ball/game_server.c`. Because `game_server_init()` is called only during real gameplay (`init_level()` in `ball/progress.c`):
- `ball/st_conf.c` checks `if (!game_server_state()) audio_music_fade_to(0.5f, "bgm/inter.ogg");` in `conf_enter()`.
- `ball/st_ball.c` uses `game_server_state()` to toggle between demo replay preview and in-game live level preview.
- Unused `progress.h` includes were removed from `st_conf.c` and `st_ball.c`.
- Changes were split into atomic commit `ball/game_server: Add game_server_state` and autosquashed into `ball/st_conf` and `ball/st_ball`.

```text
[07:11:45] Pausing, then going to options, then backing out thrashes the background gradient - trace the implementation.
```

Tracing `share/geom.c` and `share/mtrl.c` identified material cache aliasing:
1. `geom/back/back.sol` references the material `"grad.png"`.
2. `share/mtrl.c` caches and deduplicates materials by name globally in `mtrls[]`.
3. When a level loads, `game_client_init()` loads `back.sol` and sets `mp->o` to the level gradient texture (`grad_name`).
4. Entering options called `back_push("back/gui.png")`, which loaded `back.sol` and updated `mp->o` in the shared material cache slot to `"back/gui.png"`.
5. Upon `back_pop()`, `sol_free_full()` deallocated the mesh but left `mp->o` pointing to `"back/gui.png"`.
6. Returning to the paused level caused `back_draw()` to render `"back/gui.png"` instead of the level gradient.

To eliminate material cache mutation, `share/geom.c` was refactored:
- `geom/back/back.sol` is loaded once into `static struct s_full back;` during `geom_init()`.
- `back_stack` was changed to store only OpenGL texture handles (`GLuint tex`) and texture name strings.
- `back_push(name)` creates and pushes only the texture ID for `name`.
- `back_pop()` deletes the top texture via `glDeleteTextures(1, &tex)`.
- `back_draw()` sets `mp->o = back_stack[back_depth - 1].tex` directly before drawing `back.draw`.

```text
[07:17:21] As a test, I want to see what the conf screen would look like without its bg - so the game shows through.
```

In `ball/st_conf.c`, `conf_paint()` was implemented to conditionally render `game_client_draw(0, t)` and `hud_paint()` when `game_server_state()` is active, while skipping `back_push("back/gui.png")` in `conf_enter()` and `back_pop()` in `conf_leave()`.

```text
[07:19:07] Ah, but subscreens like Video still show the gui.png
```

To support transparent level passthrough across all configuration subscreens (Video, Display, Resolution, Joystick, Language, Gameplay):
1. Removed `back_push("back/gui.png")` and `back_free()` from `conf_common_init()` and `conf_common_leave()` in `share/st_common.c`.
2. Added `conf_common_bg_paint(void (*paint)(int id, float t))` callback mechanism to `share/st_common.h` and `share/st_common.c`.
3. Registered `conf_bg_paint()` from `ball/st_conf.c`, rendering `game_client_draw(0, t)` and `hud_paint()` when `game_server_state()` is active, and default menu backgrounds on the title screen.

```text
[07:22:49] This actually looks decent. Let's keep it. One nitpick: do not render the HUD while in options.
```

HUD rendering and timers were removed from options views:
- In `ball/st_conf.c`, removed `hud_paint()` from `conf_bg_paint()`.
- In `ball/st_ball.c`, removed `hud_paint()` from `ball_paint()`, removed `hud_timer()` from `ball_timer()`, and removed `#include "hud.h"`.
- Commits were autosquashed to maintain clean atomic history.

```text
[07:28:48] Generate the prompt log.
```

The session provenance and ADR generation workflow was initiated under `agy-session-provenance`.

### Verification & Technical Invariants

- **Verification:**
  - Clean compilation verified with `make clean && make -j$(sysctl -n hw.ncpu)`.
  - Full test suite verified with `make test` (`all tests passed`, exit code 0).
  - Git commit history verified with `git log` ensuring clean linear history and atomic single-concern commits.
- **Technical Invariants:**
  - **Texture-Only Background Stack:** `share/geom.c` maintains a LIFO texture stack (`back_stack`) over a single static `back.sol` mesh geometry instance, preventing material cache (`mtrl.c`) aliasing when pushing and popping background layers.
  - **Server vs Client State Semantics:** `game_client_state()` tracks client simulation allocations (`gd.state`), while `game_server_state()` strictly indicates active in-game server simulation (`sd.state`), distinguishing active gameplay from title/menu screens.
  - **GPU Object Lifecycle Isolation:** OpenGL context teardown (`st_null`) frees and reloads only GPU vertex/index buffers and textures (`game_client_free_objects` / `game_client_load_objects`, `back_free_objects` / `back_load_objects`, `game_free_objects` / `game_load_objects`) without resetting physics simulation, ball position, timers, or level progress.
  - **Decoupled Common Background Callback:** `conf_common_bg_paint()` provides a decoupled hook for host applications (`ball` vs `putt`) to supply custom background rendering for all configuration sub-menus without embedding game-specific dependencies into `share/st_common.c`.
