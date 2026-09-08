# Architectural Provenance: Add config UI for camera presets

- **Session Date:** `2026-07-30T09:47:33Z`
- **Agent:** Antigravity CLI 1.1.17 Gemini 3.7 Flash (High)
- **Base Commit:** [`5b8cd372`](https://github.com/Neverball/neverball/commit/5b8cd372c10a726c4bb155e41f9713f64cf54e34): `Add configurable rotate_max option for velocity-based manual rotation scaling` (2026-07-29 12:29:18 UTC)
- **Commits:**
  - [`01ab984b`](https://github.com/Neverball/neverball/commit/01ab984ba37ec69cba5876ad1d8a442d80ea4cf9): `Add config UI for camera presets` (2026-07-30 18:16:04 UTC)
- **Files Modified:** `ball/game_common.c`, `ball/game_common.h`, `ball/st_conf.c`, `ball/st_conf.h`, `share/st_common.h`

---

## Annotated Prompt Log

### Initial Problem & Hypothesis

```text
[09:47:33] /plan Would be neat if players could set some camera presets from in-game instead of having to go into config. I am specifically referring to Chase camera presets "1.4 Classic", "1.5 Classic" and the current hybrid style (which might be called simply "Default" for now until superseded). These could be set from the options menu, perhaps.
```

The initial planning phase investigated `ball/game_common.c`, `ball/game_common.h`, and `ball/st_conf.c` to determine Chase camera (`CAM_1`) configuration parameters and identify menu integration points in the options system.

### Initial Implementation: In-Menu Camera Presets

```text
[09:53:09] [Approved] camera_presets_plan.md
```

In `ball/game_common.h` and `ball/game_common.c`, preset enum identifiers (`CAM_PRESET_DEFAULT`, `CAM_PRESET_1_4`, `CAM_PRESET_1_5`, and `CAM_PRESET_CUSTOM`) and helper functions `cam_preset_get(int c)` and `cam_preset_set(int c, int preset)` were introduced to inspect and apply camera configurations to Chase camera (`CAM_1`):
- `CAM_PRESET_1_4`: `speed=250`, `torque=1`, `free_rotate=0`, `velocity_xz=0`, `rotate_max=100`.
- `CAM_PRESET_1_5`: `speed=250`, `torque=0`, `free_rotate=1`, `velocity_xz=1`, `rotate_max=150`.
- `CAM_PRESET_DEFAULT`: `speed=250`, `torque=1`, `free_rotate=1`, `velocity_xz=1`, `rotate_max=150`.

In `ball/st_conf.c`, an inline `CONF_CAMERA_PRESET` option row was initially integrated into the main Options menu (`conf_gui()`).

### Design Pivot: Dedicated Gameplay Options Sub-Screen

```text
[09:55:46] /plan Tweak the camera preset selection, so that it's a separate screen - there is not enough space in the options menu to place three buttons side by side in the right column as is.
```

```text
[10:00:39] I'm actually thinking that it's time for a "Gameplay: configure" section in the main options screen. Under that section, you'd have another conf screen: left side says "Camera Preset", right side has three buttons one after another. Doable?
```

```text
[10:02:27] resume if you haven't
```

```text
[10:03:23] [Approved] gameplay_options_plan.md
[Approved] camera_preset_subscreen_plan.md
[Approved] camera_presets_plan.md
[Approved] walkthrough.md
```

In `ball/st_conf.h` and `ball/st_conf.c`, a dedicated configuration sub-screen state `st_conf_gameplay` was implemented. The main Options screen (`conf_gui()`) was updated to add a `"Gameplay"` row with action token `CONF_GAMEPLAY` transitioning via `goto_state(&st_conf_gameplay)`. In `share/st_common.h`, `struct conf_option.text` was widened from 8 to 16 bytes (`char text[16]`) to support longer option labels.

### GUI Layout Iterations: Vertical Stack & Symmetrical Columns

```text
[10:08:28] Preset buttons should have been laid out vertically, not side by side.
```

```text
[10:09:56] Left column: single label "Camera Preset". Right column: three buttons stacked vertically with preset name as button text.
```

In `ball/st_conf.c`, `gameplay_gui()` was restructured using a horizontal array `gui_harray(id)` containing two columns: a left column for the `"Camera Preset"` label and a right column `gui_vstack` stacking the three preset buttons (`"Default"`, `"1.4 Classic"`, `"1.5 Classic"`). Active preset highlighting was applied using `gui_set_hilite()`.

```text
[10:12:33] This is really good, but left side should be laid out same as right side (I guess it's a stack?) but second and third slots should be empty spaces.
```

```text
[10:13:32] Hmm, that didn't quite work. Maybe not gui_space but a gui_label with a single empty space?
```

```text
[10:17:13] That worked, but not liking it visually. Any way we can achieve the same effect but so that those two placeholders are invisible (no bg, no text, just for alignment).
```

In `ball/st_conf.c`, the left column was placed in a `gui_vstack(jd)` to match the right column. Dummy space labels were added and styled with `gui_clr_rect()` to eliminate background quad rendering while matching line heights.

```text
[10:20:20] Rename GAMEPLAY_PRESET enums, they're more like GAMEPLAY_CAMERA_* enums.
```

In `ball/st_conf.c`, the GUI action token enums were renamed from `GAMEPLAY_PRESET_*` to `GAMEPLAY_CAMERA_DEFAULT`, `GAMEPLAY_CAMERA_1_4`, and `GAMEPLAY_CAMERA_1_5`.

### GUI Polish: Layout Cleanup & HUD String Refactoring

```text
[17:53:58] I had an epiphany: for the left column, we should have replaced the two empty labels with a gui_fill (or maybe gui_filler, I forget).
```

In `ball/st_conf.c`, the dummy labels and `gui_clr_rect()` calls in `gameplay_gui()` were replaced with `gui_filler(ld)` under `gui_label(ld, _("Camera Preset"), GUI_SML, 0, 0)`. In Neverball's GUI layout engine, `gui_filler()` expands into remaining vertical space within `gui_vstack`, aligning the top label without generating dummy widgets.

```text
[17:54:52] Replace "Gameplay Options" header with "Gameplay".
```

In `ball/st_conf.c`, the sub-screen header was updated to `conf_header(id, _("Gameplay"), GUI_BACK)`.

```text
[17:56:32] Okay, now match spacing between header row and rest of the screen to that used in the main conf screen.
```

In `ball/st_conf.c`, the redundant `gui_space(id)` after `conf_header()` was removed from `gameplay_gui()`, matching the vertical spacing in `conf_gui()`.

```text
[17:59:40] Can we use cam_preset_get in cam_to_str?
```

In `ball/game_common.c`, `cam_to_str(int c)` was refactored to query `cam_preset_get(c)` in a switch statement, mapping `CAM_PRESET_1_4` to `_("1.4 Classic")`, `CAM_PRESET_1_5` to `_("1.5 Classic")`, and `CAM_PRESET_DEFAULT` to `_("Chase Camera")`.

### Final Sub-State Naming & Subsystem Decoupling

```text
[18:13:01] rename st_gameplay to st_conf_gameplay for consistency
```

```text
[18:14:14] resume
```

```text
[18:40:32] Hmm, should "1.4 Classic" also switch the keyboard response to 1.4.0 style? I have gotten so many complaints about that. Personally, I hate that behavior of instant max tilt - feels like cheating. But keyboard players swear by it.
```

```text
[18:41:03] Cancel the rename, I did it already. Hmm, should "1.4 Classic" also switch the keyboard response to 1.4.0 style? I have gotten so many complaints about that. Personally, I hate that behavior of instant max tilt - feels like cheating. But keyboard players swear by it.
```

In `ball/st_conf.h` and `ball/st_conf.c`, `st_conf_gameplay` was finalized as the state struct name, consistent with `st_conf_video` and `st_conf_display`. Camera presets remain strictly scoped to camera properties (`cam_torque`, `cam_free_rotate`, `cam_velocity_xz`, `cam_rotate_max`), keeping input filtering (`joystick_response`) decoupled from camera behavior.

### Verification & Technical Invariants

- **Verification:** Built cleanly via `make -j$(sysctl -n hw.ncpu)` with zero compiler warnings or errors. Verified menu navigation (`Options` -> `Gameplay: Configure` -> `Camera Preset` -> `Default`, `1.4 Classic`, `1.5 Classic`), preset switching for `CAM_1`, and string synchronization across `cam_to_str()` and `cam_preset_get()`.
- **Technical Invariants:**
  - **Camera Preset Scope:** Camera presets (`cam_preset_set`) mutate only `CONFIG_CAMERA_1_SPEED`, `CONFIG_CAMERA_1_TORQUE`, `CONFIG_CAMERA_1_FREE_ROTATE`, `CONFIG_CAMERA_1_VELOCITY_XZ`, and `CONFIG_CAMERA_1_ROTATE_MAX` for `CAM_1`, without altering unrelated input response parameters (`CONFIG_JOYSTICK_RESPONSE`).
  - **GUI Vertical Alignment:** Symmetrical two-column layouts where only one column contains multiple stacked elements must use `gui_filler()` below single labels in `gui_vstack` rather than dummy empty labels or manual offsets.
  - **Option Struct Capacity:** `struct conf_option` in `share/st_common.h` requires a 16-byte `text` buffer (`char text[16]`) to prevent truncation of descriptive preset names.
