# Architectural Provenance: ball/st_game_link: Add game link loading state and asset launcher

- **Session Date:** `2026-08-27T16:02:31Z`
- **Agent:** Antigravity CLI 1.1.22 Gemini 3.7 Flash (High)
- **Base Commit:** [`329a96e4`](https://github.com/Neverball/neverball/commit/329a96e40fcc28c8f6d30d3fcc0d2ca03914ee27): `doc: Instruct agents to query harness version dynamically` (2026-08-27 08:59:03 UTC)
- **Commits:**
  - [`80c57778`](https://github.com/Neverball/neverball/commit/80c57778b3b49a5834e34e1b907c13abfa6c6b94): `ball/st_game_link: Add game link loading state and asset launcher` (2026-08-27 21:25:06 UTC)
  - [`c46666d4`](https://github.com/Neverball/neverball/commit/c46666d4461d301cc7685281db29dd22f7169097): `ball/main: Delegate --link argument handling to game_link_handle` (2026-08-27 21:25:06 UTC)
- **Files Modified:** `Makefile`, `ball/main.c`, `ball/st_game_link.c`, `ball/st_game_link.h`, `emscripten/ball.mk`

---

## Annotated Prompt Log

### Initial Problem & Hypothesis

```text
[16:02:31] /plan Neverball supports linking to assets (mostly just levels and level sets) via "deep links" e.g. URL parameter `?link=set-easy/wakka` translated to command line argument `--link set-easy/wakka` and has the game open that level if that level is found. If not found, it looks at the available packages and opens the installation page for that package, if found. It primary exists to enable linking to levels and level sets in the web version, but falls flat on its face because at best you end at the addon list. It would be significantly more useful if such a deep link automatically installed the addon still went to the actual level or level set after installation, --link acting as a sort of a "do what I mean" shorthand that tries its best to get and present the requested asset all-in-one. I imagine the installation step happening in a number of ways: 1) extending st_loading to install the missing package while showing download progress, 2) adding separate set of screens that explain they got there via deep link and need to install an addon to proceed, have them confirm, then install with a progress bar/indicator and present the asset. In both cases some of the functionality overlaps with what already exists in the addon management system/UI and would be nice to reuse those components where possible.
```

In `ball/main.c`, `--link` arguments (and web URL query parameters `?link=...`) were previously handled by `link_handle()`. If the referenced set or level was already installed locally, it launched directly into `st_level` or `st_start`. If missing locally, it queried `package_search()` and navigated to the addon browser item (`goto_package()` in `share/st_package.c`), leaving the user to manually click download without automated launch of the targeted level.

The initial planning phase investigated `ball/main.c`, `share/st_package.c`, `share/package.h`, `share/set.h`, and `ball/st_loading.c`. An initial plan artifact (`deep_link_addon_installer_plan.md`) was created evaluating three architectural options: extending `st_loading` directly, creating a dedicated preview and confirmation screen, or implementing a dedicated link state (`st_link`) managing download callbacks and state transitions.

### Constraint Discovery: Terminology & Addon UI Decoupling

```text
[20:07:20] Let's clarify terminology: we'll call them "game links" not "deep links". st_link sounds good, but I don't see any reuse of components, did no opportunities arise?
```

Terminology was standardized from "deep links" to "game links". The implementation plan in `deep_link_addon_installer_plan.md` was updated to factor out GUI and thumbnail download routines from `share/st_package.c` into reusable public helper functions (`struct package_card_ids`, `gui_package_card()`, `gui_package_card_update()`, and `package_fetch_thumbnail()`) for shared use between `share/st_package.c` and a dedicated `st_link` state.

```text
[20:14:02] [Approved] deep_link_addon_installer_plan.md
```

A dedicated git worktree (`.worktrees/game-links`) was created on branch `game-links`. Initial implementation proceeded:
1. `share/st_package.h` and `share/st_package.c` were refactored to export package card widget hierarchy construction and thumbnail download callbacks.
2. A new state `st_link` (`ball/st_link.h` and `ball/st_link.c`) was implemented with a full confirmation card GUI, thumbnail fetching, download progress bar, and automated level launching.
3. `ball/main.c` was updated to delegate `--link` argument handling to `game_link_handle()`, and build configurations were updated in `Makefile` and `emscripten/ball.mk`.

### Interim Feedback & Visual Refinement

```text
[20:22:34] No prompt log until I say so.
```

The automated subagent invocation was suppressed and a feature walkthrough artifact (`walkthrough.md`) was created.

```text
[20:27:39] Ha, that looks terrible. Have a look at the screenshot.
```

In `ball/st_link.c`, the dialog layout was restructured:
- Reordered header elements so the title aligned left and the cancel button aligned right.
- Centered the addon preview thumbnail at half-screen width.
- Wrapped description text and action buttons in a standard Neverball dialog container.
- Corrected the asynchronous lifecycle ordering so GUI widget IDs were allocated before thumbnail downloads initiated callbacks.

### Architectural Pivot to Automatic Loading

```text
[21:00:17] /plan Let's pivot: downloading and installation is all automatic under the "Loading" screen, no separate confirmation screen.
```

The design pivoted away from interactive confirmation screens and addon card UI reuse toward a fully automatic, non-interactive loading state. An implementation plan (`game_link_automatic_installer_plan.md`) was prepared to handle downloading, progress tracking, and asset launch directly on state entry.

```text
[21:01:49] st_link may remain if that's easier than dumping all functionality into st_loading, but the concept remains: it's a loading screen and all automatic.
```

The plan was updated to keep the state isolated in `ball/st_link.[ch]` rather than overloading `ball/st_loading.c`, maintaining separation of concerns while matching `st_loading`'s visual and operational behavior.

```text
[21:03:45] [Approved] game_link_automatic_installer_plan.md
```

Implementation transitioned `st_link` into an automatic loading state:
1. **State Transition & Download Lifecycle:** On `st_link` entry, `package_fetch()` is initiated immediately with progress and completion callbacks (`game_link_download_progress` and `game_link_download_done`).
2. **Post-Download Target Launch:** Upon download completion, `package_fetch` mounts the downloaded `.pk3` file via PhysicsFS. `game_link_launch_target()` initializes level sets with `set_init()`, finds the set with `set_find()`, selects the target level with `set_find_level()`, and transitions to `st_level` (or `st_start` for full sets). If the set is not found after download, it falls back to `st_title`.
3. **Reversion of Unused Addon GUI Helpers:** The earlier UI helper exports in `share/st_package.h` and `share/st_package.c` were discarded, reverting `share/st_package` to its pristine upstream state.
4. **Main Delegation:** `ball/main.c` delegates `--link` resolution in `refresh_packages_done()` and `main_preload()` to `game_link_handle()`.

### Visual Consistency, Naming Discipline & Final Polish

```text
[21:12:43] Visually needs to match st_loading (black bg). Label box must be large enough initially to hold a 100% progress.
```

In `ball/st_link.c`:
- Removed decorative background rendering and background audio to match `st_loading`'s minimalist black screen.
- Pre-dimensioned the status label widget by creating it with `_("Loading (100%)...")` before calculating layout geometry with `gui_layout()`, then immediately setting the label text to `_("Loading...")`. This prevented text clipping and widget jitter as percentage numbers grew from single to double and triple digits.

```text
[21:16:19] For consistency's sake rename st_link to st_game_link
```

Renamed files and identifiers from `st_link` to `st_game_link`:
- Renamed `ball/st_link.h` to `ball/st_game_link.h` (guard `ST_GAME_LINK_H`) and `ball/st_link.c` to `ball/st_game_link.c`.
- Updated references in `ball/main.c`, `Makefile`, and `emscripten/ball.mk`.

```text
[21:20:18] Update all symols in st_game_link to use the new prefix.
```

```text
[21:20:24] Update all symbols in st_game_link to use the new prefix.
```

In `ball/st_game_link.h` and `ball/st_game_link.c`, all static and extern symbols were renamed to use the `game_link_` prefix (`struct state st_game_link`, `game_link_ctx`, `game_link_handle`, `game_link_launch_target`, `game_link_download_progress`, `game_link_download_done`, `game_link_start_download`, `game_link_action`, `game_link_gui`, `game_link_enter`, `game_link_leave`, `game_link_paint`). Changes were autosquashed into the commit history.

```text
[21:24:48] Using sprintf to insert a static 100% is an odd choice.
```

In `ball/st_game_link.c` (`game_link_gui()`), replaced the temporary `sprintf(buf, _("Loading (%d%%)..."), 100)` layout sizing call with direct passing of the string literal `_("Loading (100%)...")` into `gui_label()`. Changes were autosquashed into [`80c57778`](https://github.com/Neverball/neverball/commit/80c57778b3b49a5834e34e1b907c13abfa6c6b94).

```text
[21:27:45] Generate a prompt log (provenance)
```

The final provenance synthesis was executed to capture the architectural decisions, design pivots, and technical invariants across the `329a96e4..c46666d4` commit range.

### Verification & Technical Invariants

- **Verification:**
  - Full test suite verified passing (`make test`: all test suites exit code 0).
  - Native binary builds verified passing (`make neverball`).
  - Help flag and CLI argument parsing verified (`./neverball -h`).
- **Technical Invariants:**
  - **Single Responsibility & State Isolation:** `ball/st_game_link` encapsulates the entire download, progress monitoring, and asset presentation lifecycle without adding ad-hoc download logic into `ball/st_loading.c` or modifying `share/st_package.c`.
  - **Dynamic Package Mounting & Launch Order:** `package_fetch()` automatically mounts downloaded archive `.pk3` files onto the PhysFS search path upon successful transfer completion. Therefore, `game_link_launch_target()` must re-run `set_init()` to rescan available sets before querying `set_find()` and `set_find_level()`.
  - **Layout Sizing Stability:** Status labels displaying dynamic percentage text (`"Loading (X%)..."`) must be initialized with the maximum expected width (`"Loading (100%)..."`) prior to `gui_layout()` computation to avoid widget reallocation or horizontal text jitter during progress updates.
  - **Link Prefix Matching:** Game link parsing in `game_link_handle()` handles both bare set identifiers (`set-name` / `set-name.txt`) and level-qualified set paths (`set-name/level` / `set-name/level.sol`), ensuring `.txt` and `.sol` extensions are appended idempotently via `strbuf`.
