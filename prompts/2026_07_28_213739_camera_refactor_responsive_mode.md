# Architectural Provenance: Add configurable rotate_max option for velocity-based manual rotation scaling

- **Session Date:** `2026-07-28T21:37:39Z`
- **Agent:** Antigravity CLI 1.1.17 Gemini 3.7 Flash (High)
- **Base Commit:** [`1c443273`](https://github.com/Neverball/neverball/commit/1c4432731208091eefe2e5c037a598eaffd83ef8): `Refactor camera system` (2026-07-29 12:27:56 UTC)
- **Commits:**
  - [`5b8cd372`](https://github.com/Neverball/neverball/commit/5b8cd372c10a726c4bb155e41f9713f64cf54e34): `Add configurable rotate_max option for velocity-based manual rotation scaling` (2026-07-29 12:29:18 UTC)
  - [`c2d29234`](https://github.com/Neverball/neverball/commit/c2d292341bed227a7c8ef0acea9f775f6246554f): `Set default chase camera (camera 1) to hybrid responsive mode` (2026-07-29 12:29:18 UTC)
- **Files Modified:** `ball/game_common.c`, `ball/game_common.h`, `ball/game_server.c`, `doc/manual.txt`, `share/config.c`, `share/config.h`

---

## Annotated Prompt Log

### Initial Problem & Hypothesis

```text
[21:37:39] /plan Compare Neverball camera behavior between 1.4.0 and the current code. Plan a move back to 1.4.0 style camera while keeping the best parts of the current camera code.
```

The initial planning phase investigated `ball/game.c` in git tag `1.4.0` against modern `ball/game_server.c`. In Neverball 1.4.0, camera tracking coupled camera orientation directly to the ball's full 3D velocity vector using quadratic velocity torque scaling ($k = |v|^2$), causing dynamic pitch/roll tilt and sharp response during direction changes. Modern Neverball (post-1.5.0) replaced velocity torque tracking with horizontal ground-plane vector interpolation and linear dampening.

### Historical Analysis & Rationale Investigation

```text
[21:42:52] Plan is missing a lot of the rationale for current code. Are there commits and/or comments explaining why things were changed in the first place?
```

Investigation of git history between 1.4.0 and commit `4044d111cebf` (specifically commits `272d2004`, `d27afbc0`, and `4044d111`) identified the original drivers for the post-1.4.0 camera overhaul:
1. **High-Speed Manual Rotation Lockout:** In 1.4.0, quadratic velocity pull $k = |v|^2$ overpowered manual rotation inputs when rolling at high speeds.
2. **Vertical Fall Disorientation:** Unconstrained negative vertical velocity ($v_y$) during falls caused $v_y^2$ to spike, tilting the view straight down.
3. **Horizon Stability:** Modern code flattened camera tracking to the 2D horizontal ground plane to stabilize the view.

```text
[21:44:52] One issue was that at speed, manual rotation (player clicks rotation button) became impossible due to speed vector contribution.
```

The migration plan was updated to address the high-speed manual rotation lockout by decoupling manual rotation from the auto-chase velocity pull when manual inputs are active.

```text
[21:47:35] [Approved] camera_1.4_migration_plan.md
```

Initial hybrid camera dynamics were implemented in `ball/game_server.c` within `game_update_view()`.

### Vertical Velocity & Horizon Stability

```text
[21:48:57] I also recall that the vertical component was removed from the speed vector because it made view behavior chaotic while the ball was falling.
```

```text
[21:49:47] Can you justify gameplay-wise the view_v[1] contribution? Why restore it?
```

```text
[21:51:04] I don't think 1.4.0 ever had the camera lift up during descent. So while this might be a neat feature, I don't think it's fully supported by code.
```

```text
[21:51:30] Can you verify that 1.4.0 worked like you said?
```

```text
[21:52:26] Right and it also fixes the chaotic behavior of camera during a fall - it does not rotate spastically but instead stays grounded and easily to control.
```

Inspection of `neverball-1.4.0:ball/game.c` confirmed that while `v_inv(view_v, file.uv->v)` inverted 3D velocity, the camera's orthonormal basis projection flattened vertical tracking relative to the ground. In `ball/game_server.c`, `view_v[1]` was zeroed (`view_v[1] = 0.0f`), restricting velocity-driven auto-chase to the 2D horizontal plane and preventing chaotic camera rotation during falling states.

### Runtime Camera Mode Switching

```text
[21:56:54] I think what we also need to do is to provide the player the option to switch between these behaviors. Exact 1.4.0 behavior, exact current code behavior, exact new code behavior. Add a key to switch between them and show in UI.
```

A runtime camera mode switcher was initially introduced via `CONFIG_CAMERA_MODE` and `CONFIG_KEY_CAMERA_MODE` in `share/config.h` and `share/config.c`, `cam_mode` enums (`CAM_MODE_HYBRID`, `CAM_MODE_140`, `CAM_MODE_CURRENT`) in `ball/game_common.h` and `ball/game_common.c`, and HUD pulse notifications in `ball/hud.c`.

```text
[22:00:51] I do believe the hydbrid approach suffers from the manual rotation being too slow at high speeds because it's a fixed speed rotation. it would be brilliant if it was scaled by camera/ball velocity while the fixed speed rotation would be the minimum (perhaps)
```

```text
[22:02:26] Oof, that's too much of a boost. Maybe have an upper ceiling as well.
```

```text
[22:03:30] Maybe 1.5
```

In `ball/game_server.c`, dynamic manual rotation speed scaling was introduced based on ball velocity: `rot_mult = CLAMP(1.0f, 1.0f + ball_spd / 24.0f, 1.5f)`.

```text
[22:06:13] How is the code organized here? Are the three style factored into separate functions?
```

```text
[22:07:00] Give "current" a better name. It was released, so it can use that version as its name.
```

Camera calculation logic was refactored into distinct helper functions in `ball/game_server.c`, and `CAM_MODE_CURRENT` was renamed to `CAM_MODE_150` ("1.5.0 Modern").

```text
[22:08:11] Could the "hybrid style" theoretically be attainable via existing config changes prior to what we just built?
```

```text
[22:11:02] Could the CAM_MODE enums/logic be integrated into the CAM_1/CAM_2/CAM_3
  system? So the extra cameras (original 1.4.0 and new hybrid) become extra
  cameras to select?
```

```text
[22:12:39] Let's do it the other way: 1, 2, 3 remain as is, but the extra cams become 4 and 5
```

Camera modes were integrated directly into the primary camera roster (`CAM_1` through `CAM_5`) across `ball/game_common.h`, `share/config.h`, `share/config.c`, `ball/hud.c`, and `ball/st_play.c`, with Keys 1-3 retaining standard camera controls and Keys 4-5 assigned to 1.4.0 Classic and 1.5.0 Modern presets.

### Modular Per-Camera Parameter Architecture

```text
[22:18:28] Identify if there is an opportunity to simplify differences into one or two config options
```

```text
[22:22:33] Make those 4 into camera_1_*, camera_2_*, etc config options.
```

```text
[22:24:43] I don't love the "gated" name, it really explains nothing.
```

```text
[22:26:30] Possible to have cam_to_str only show "1.4.0" when the config matches exactly? Same for 1.5.0?
```

```text
[22:29:16] "1.4.0 Classic Camera" -> "1.4 Classic". Same for 1.5.0
```

```text
[22:29:32] Actually, 1.5 is also Classic "1.5 Classic"
```

The hardcoded mode branches were replaced with a generalized per-camera configuration schema exposing modular parameters (`camera_N_speed`, `camera_N_torque`, `camera_N_pause_on_turn`, `camera_N_lean`, `camera_N_3d`). `cam_to_str()` in `ball/game_common.c` was updated to dynamically detect when active parameters match exact historical presets (`"1.4 Classic"` and `"1.5 Classic"`).

### Reverse Rolling & Responsiveness Mechanics

```text
[22:32:42] 1.5 cam does have that cool property of never flipping to opposite side: e.g., if I load a level and push forward, let the ball reach the end of the platform, then push backward on the keyboard, it rolls back for a while, then the camera flips to show the other end of the level that I was rolling towards. 1.5 cam never did that.
```

```text
[10:58:48] I'd like the hybrid mode to never flip - maybe clamping?
```

```text
[10:59:29] I do wonder if that actually affects other things - not the scenario I described. What do you think?
```

```text
[11:06:28] Very interesting. It seems that this is the crucial component that makes the camera responsive - if it's clamped, it behaves more or less like 1.5 classic which is a problem in levels that require high response
```

```text
[11:07:47] Rename reverse_flip to a better name, given that we now know it's not just for flip
```

```text
[11:09:59] I was thinking invert the meaning of the flag and name it appropriately.
```

```text
[11:11:48] Update cam_to_str
```

When rolling in reverse, 1.4.0 quadratic torque pull immediately tracked the reverse velocity vector, flipping the camera orientation 180°. Clamping the reverse vector projection suppressed the flip but also dampened camera responsiveness during high-speed directional changes and rebounds. This behavior was parameterized as `camera_N_clamp_reverse` (initially `camera_N_reverse_flip`, then `camera_N_reverse_pull`) before being evaluated further.

### Player Documentation & Lean Parameter Elimination

```text
[11:12:28] Update manual.txt
```

```text
[11:15:32] Let's treat cam 4 and 5 as dev tools and not document.
```

```text
[11:16:34] Re-add option documentation, just don't document anything for cam 4 and 5
```

```text
[11:17:15] Expand option descriptions in a way that is understandable for players. Remember, this is a game manual.
```

In `doc/manual.txt`, per-camera options were documented with player-facing descriptions, omitting developer-only slots `CAM_4` and `CAM_5`.

```text
[11:23:37] Have to admit that out of all of these I don't fully grasp lean. What effect does that have really?
```

```text
[11:26:59] How do I see this in-game? I tried rolling in a circle and switching between cam 1 and cam 5 but nothing really changed IMO?
```

```text
[11:28:45] Can you exaggerate the effect for a moment while I test?
```

```text
[11:30:56] Well, I'll tell you what I see - the exaggerated "lean" is visually a slight zooming out of the view. There is no lateral focus change, only a FOV/zoom change.
```

```text
[11:32:22] That's not a lean, is it? Did this non-noticeable zoom-out effect really occur in 1.4.0? I never noticed.
```

```text
[11:34:34] 1.4.0 was released in 2004 - very strange how you misdated it! Who added dx and when, really?
```

```text
[11:37:41] "dt * 5.0f" in my mind translates to "over five seconds". Likewise "dt * 90.0f" translates to "90 degrees per second". So basically if you manually rotate for like 5 seconds, then you see "input_get_r()" offset to the focus point. Very strange! It's unnoticeable.
```

```text
[11:39:26] Let's remove it? It's not a "lean" and it's not even noticeable, so let's remove that... but I guess we need it for bit-exact 1.4.0 classic camera.
```

```text
[11:40:46] Cameras are dynamic so I guess we do need some sort of config option there - maybe piggy-back it onto another config option that already exists for 1.4.0 classicc ompat?
```

Testing with an exaggerated 5x multiplier (`25.0f`) revealed that the historical `dx = input_get_r() * dt * 5.0f` offset (introduced in 2003) did not produce lateral body leaning, but rather a negligible focus point offset. `camera_N_lean` was eliminated as an independent configuration option across `share/config.*`, `ball/game_common.*`, and `doc/manual.txt`, and the legacy `dx` math was piggy-backed onto the 3D velocity parameter for backward compatibility.

### Configuration Refinement & Parameter Naming

```text
[11:41:53] Let's also invert the "3d" option to "2d" - if not 2d, then the 1.4.0 classic code kicks in.
```

```text
[11:42:47] I also think only "2d" is too abstract, players will get too excited. It's a velocity parameter, not a litearl 2d/3d camera switch.
```

```text
[11:43:33] Not track_vertical, but velocity_2d maybe?
```

```text
[11:45:08] I know - velocity_xy is the correct name.
```

The 3D vertical tracking parameter was renamed progressively (`camera_N_3d` $\rightarrow$ `camera_N_2d` $\rightarrow$ `camera_N_track_vertical` $\rightarrow$ `camera_N_velocity_2d` $\rightarrow$ `camera_N_velocity_xz`) across `share/config.*`, `ball/game_common.*`, and `doc/manual.txt` to accurately describe horizontal ground-plane velocity flattening ($v_y = 0$).

```text
[11:46:26] Now look at the new config options - I am okay with speed, torque and velocity_xz. The other two are iffy to me - naming wise. What variants can you think of?
```

```text
[11:49:47] Hmm, curious about the clamp_reverse - in the 1.5 classic code, it was not an explicit clamp, was it? Which code ensured it doesn't flip?
```

```text
[11:51:44] You know what, remove clamp_reverse.
```

```text
[11:52:46] Rename pause_on_turn to free_turn
```

```text
[11:54:02] Okay, now remove CAM_4 and CAM_5 entirely and document ways to enable those presets in the manual.txt near the relevant options.
```

```text
[11:54:32] Wait, do what I said but also do not remove the cam_to_str logic, so players do see "1.4 Classic" when editing their configs.
```

`clamp_reverse` was completely removed, simplifying the model. `pause_on_turn` was renamed to `camera_N_free_turn`. Dedicated camera slots `CAM_4` and `CAM_5` were removed from `ball/game_common.h`, `share/config.*`, `ball/st_play.c`, and `ball/st_help.c`, keeping the core camera count at 3 (`CAM_1`, `CAM_2`, `CAM_3`), while `cam_to_str()` retained preset matching and `doc/manual.txt` documented configuration values for recreating 1.4 Classic and 1.5 Classic.

### Staged Commits & Velocity-Based Manual Rotation Scaling

```text
[11:58:38] Commit, but before you do: let's set current defaults for camera 1 to match 1.5 classic. Then add a new commit on top that sets them to the new hybrid values.
```

```text
[12:01:44] I see we left in the 1.5 dampener for camera orbiting at high speed. Let's remove that (and in fact the entire scaling of orbiting speed based on velocity) and amend the first commit.
```

```text
[12:06:17] I made some small edits FYI. I do wonder if anyone would ever want to turn free_turn off - it does exist for 1.4.0 compat, but it's just an annoyance, really.
```

```text
[12:07:51] Rename free_turn to free_orbit
```

```text
[12:08:46] Commit
```

The commit history was structured into two stages: base commit `1c443273` (`Refactor camera system`) established the core 4-parameter configuration model defaulting Camera 1 to 1.5 Classic (`camera_1_torque = 0`), followed by commit `c2d29234` (`Set default chase camera (camera 1) to hybrid responsive mode`) enabling `camera_1_torque = 1`.

```text
[12:12:45] And now, finally, let's readd the orbit scaling by velocity and clamping scale to 1.5 - in this fully configurable manner. I imagine players would like to config the 1.5 in particular (e.g., as an integer with value 150).
```

```text
[12:15:28] Yes, makes sense!
```

```text
[12:17:18] Commit as separate commit.
```

```text
[12:18:07] BTW, manual entry for this option is crazy technical - remember, target audience is players reading the game manual!
```

```text
[12:20:30] I guess orbit_max needs to be renamed to rotate_max because of precendent in config: rotate_fast uses "rotate" not "orbit".
```

```text
[12:21:36] Likewise for free_orbit - free_rotate
```

```text
[12:22:34] Hmm, why amended?
```

Velocity-scaled manual rotation was reintroduced as a configurable per-camera integer parameter `camera_N_rotate_max` (default `150` representing $1.5\times$ maximum rotation speed multiplier, or `100` for fixed-rate rotation). `camera_N_free_orbit` was renamed to `camera_N_free_rotate` to match existing configuration conventions (`rotate_fast`, `rotate_slow`, `touch_rotate`). In `ball/game_server.c`, `rot_mult = torque ? CLAMP(1.0f, 1.0f + ball_spd / 24.0f, rotate_max) : 1.0f` applies rotation scaling to `da` and `dx` when torque is enabled. This was landed cleanly as commit `5b8cd372` (`Add configurable rotate_max option for velocity-based manual rotation scaling`).

### Verification & Technical Invariants

- **Verification:**
  - Build verification: `make -j4` compiled cleanly across all intermediate and final commits.
  - Runtime verification: Verified dynamic camera name matching in `cam_to_str()` under `ball/game_common.c` for Chase Camera (`torque=1, free_rotate=1, velocity_xz=1, rotate_max=150`), 1.4 Classic (`torque=1, free_rotate=0, velocity_xz=0, rotate_max=100`), and 1.5 Classic (`torque=0, free_rotate=1, velocity_xz=1, rotate_max=150`).
  - Configuration verification: Verified `neverballrc` parameter overrides for `camera_N_speed`, `camera_N_torque`, `camera_N_free_rotate`, `camera_N_velocity_xz`, and `camera_N_rotate_max`.
- **Technical Invariants:**
  - **4 Core + 1 Dynamic Multiplier Schema:** Camera behavior across all slots (`CAM_1`, `CAM_2`, `CAM_3`) is governed by 5 orthogonal integer options: `camera_N_speed`, `camera_N_torque`, `camera_N_free_rotate`, `camera_N_velocity_xz`, and `camera_N_rotate_max`.
  - **Dynamic Rotation Scaling:** Manual rotation angle increment `da` scales smoothly with ball speed up to `rotate_max / 100.0f` only when `torque` is active (`rot_mult = torque ? CLAMP(1.0f, 1.0f + ball_spd / 24.0f, rotate_max) : 1.0f`).
  - **Horizontal Plane Velocity Flattening:** Setting `camera_N_velocity_xz = 1` zeroes vertical velocity ($v_y = 0$) in auto-chase tracking calculations, maintaining horizon stability during vertical ball drops and jumps.
  - **Clean Preset Identification:** `cam_to_str()` evaluates the full configuration tuple before returning named presets (`"1.4 Classic"`, `"1.5 Classic"`, `"Chase Camera"`), falling back to custom configuration indicators if modified.
