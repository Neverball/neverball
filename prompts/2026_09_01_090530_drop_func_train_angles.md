# Architectural Provenance: mapc: Drop func_train angles & warn on oriented path parenting

- **Session Date:** `2026-09-01T09:05:30Z`
- **Agent:** Antigravity CLI 1.1.23 Gemini 3.7 Flash (High)
- **Base Commit:** [`9ed1d967`](https://github.com/Neverball/neverball/commit/9ed1d9676265b4008ac03f0d30f28dd38a336d19): `agents: respond to junk in commit messages [skip ci]` (2026-09-01 09:12:05 UTC)
- **Commits:**
  - [`f3372c78`](https://github.com/Neverball/neverball/commit/f3372c7895c9a4753fdc1dd5f7384f457d1cfdd3): `map-misc: Update rotation map to use modern game_* entities` (2026-09-01 10:28:04 UTC)
  - [`c150a278`](https://github.com/Neverball/neverball/commit/c150a278fd8a51c98e2ae7ba52828492bbee5a8b): `mapc: Warn when entity connects to path node with initial angles` (2026-09-01 10:25:51 UTC)
  - [`6f3c831a`](https://github.com/Neverball/neverball/commit/6f3c831a88ca91a54e5ef3a0cdb434432c2c622b): `entities: Drop angles property from func_train` (2026-09-01 10:15:49 UTC)
  - [`22cea3b1`](https://github.com/Neverball/neverball/commit/22cea3b1f4c0514de2a72b25c3451a1fc5229a4c): `mapc: Drop angles support for func_train` (2026-09-01 10:15:47 UTC)
- **Files Modified:** `TrenchBroom/entities.ent`, `data/map-misc/rotation.map`, `share/mapclib.c`

---

## Annotated Prompt Log

### Modernizing Legacy Map Entity Placements

```text
[09:05:30] I've deployed this. For the fun of it can you update rotation2.map to use the new entities and place them so the map appears unchanged from player perspective but better from mapper perspective
```

```text
[09:17:20] Oops, I think I meant rotation.map. Do the work in the main worktree, do not commit.
```

In `data/map-misc/rotation.map`, legacy Quake entity definitions were replaced with modern `game_*` point entities with origin-centered bounding boxes:
- `info_player_start` converted to `game_ball` with base-aligned origin ($Z=16$).
- `info_player_deathmatch` converted to `game_goal` with base-aligned origin ($Z=48$).
- `target_teleporter` converted to `game_jump` with base-aligned origin ($Z=32$).

```text
[09:20:52] I noticed that you did not move/rotate game_switch itself.
```

In `data/map-misc/rotation.map`, switch entities (Entities 19 & 23) were converted to `game_switch` with 3D orientation `angles "90 0 0"`.

```text
[09:25:07] Hmm, orientation is correct but the position is wrong. I moved them myself - I think this is correct. What can you learn from my change?
```

In legacy `rotation.map`, the original author authored switch and coin entities flat on an off-level staging platform ($Z=0$), relying on an intermediate `path_corner` (`side1`) to translate and rotate them onto the moving platform at runtime. Modern authoring places `game_switch` directly at its physical contact point in world space on the platform wall ($X = -160, Y = 320, Z = 64$).

```text
[09:26:11] Right, now move the light entities in that same manner.
```

```text
[09:27:02] Light entities were actually offset from the old switch position, so they should be higher up - can you figure that out?
```

```text
[09:28:09] Not what I meant. You should look at their X offsets in the old committed version of rotation.map, then apply them so they preserve that relationship on the vertical wall
```

In `data/map-misc/rotation.map`, collectible coin entities (`light` with values `1`, `5`, `10`) were repositioned at $Y = 544, 480, 416$ and $Z = 96$ on the vertical platform face with `angles "90 0 0"`, preserving their relative spacing along the wall.

### Decal Brush Geometry & func_train Angles

```text
[09:32:11] Perfect. Now one final thing that worries me - what still has not been updated are the decals that still are placed at their old locations not on the vertical wall. Strictly speaking, it's just geometry and could be placed as decal geometry on the vertical wall. But they're also a func_train entity. I'm unsure if we added support for direct angles on the func_train entity? Seems like TB allows you to set angles but that doesn't affect rendering?
```

In TrenchBroom, brush entities (`func_train`) are collections of convex brushes whose half-space planes are authored directly in world coordinates. Setting an `angles` key on a `func_train` entity does not transform brush plane geometry in the TrenchBroom viewport, creating a discrepancy between editor visualization and compiler/engine output.

```text
[09:33:32] Excellent, that's exactly what I want - decals placed along with their geometry. They would then be a part of the main func_train not even separate trains.
```

```text
[09:38:15] This is wild. I would have assumed you can just flip coords to rotate geometry by 90 deg.
```

In Quake `.map` format, brushes are defined by bounding plane triplets $(p_0, p_1, p_2)$ whose outward normal is computed as $\mathbf{n} = (p_0 - p_1) \times (p_2 - p_1)$. Coordinate reflections or manual component swapping inverts plane winding order, turning normals inward and failing compiler half-space clipping. Decal brush geometry is rotated via editor rotation tools to preserve proper plane windings and outward normals.

```text
[09:38:44] Apply the switch and light changes that you reverted. I will rotate decals manually.
```

Point entity modernizations, wall switch placements, and light entity coordinates with 3D `angles` were restored in `data/map-misc/rotation.map`.

### Diagnosing In-Game Compound Rotations

```text
[10:06:00] Verify it. It looks okay now in the editor but not in game. In game it's a mess.
```

Diagnosis of compiled runtime output revealed a compound rotation defect:
1. **Double Rotation**: In TrenchBroom, the switch, coins, and wall decals were placed directly on the vertical wall face with `angles "90 0 0"`. However, they were still targeted to `side1`—a legacy mover node that itself had `angles "90 0 0"`. At runtime, Neverball compounded both transformations ($q_{\text{parent}} \cdot q_{\text{child}}$), rotating them $180^\circ$ into the ceiling.
2. **Resolution**: Entities were parented directly to `path1` (the main platform mover without frame-0 rotation), and obsolete helper path nodes (`side1`, `side2`) were removed.

```text
[10:10:23] This worries me that things don't agree between editor and game and you're left on your own to figure it out. Possible to at least detect this?
```

Analysis of the editor-engine synchronization gap identified two structural improvements:
1. Dropping `angles` support from brush entities (`func_train`) because brushes are explicit WYSIWYG 3D geometry in world space.
2. Adding a compile-time warning in `mapc` when an entity connects to a `path_corner` that already has non-zero initial angles.

### Dropping func_train Angles & Compiler Diagnostics

```text
[10:13:18] Well, this tells me we drop support for angles from brush entities. Yeah?
```

```text
[10:14:24] Make that change in a separate worktree.
```

An isolated worktree `.worktrees/drop-func-train-angles` was created on branch `drop-func-train-angles`.
- In `share/mapclib.c`, `body_e` / `body_has_e` arrays, `angles` key parsing in `make_body()`, and the body loop in `turn_file()` were removed.
- In `TrenchBroom/entities.ent`, the `<angles>` property was dropped from `<group name="func_train">`.

```text
[10:19:41] And I think we do emit a warning when an entity is connected to a path node that already has angles - just to let the mapper know that in-game will look different from what they see in the editor.
```

```text
[10:25:10] I don't like the comparisons with SMALL. That is made-up logic just to fit in that discards valid non-zero orientations.
```

```text
[10:26:28] This was not a comment "second guessing" anything, but a comment about floating-point. Checks against zero are totally safe, use of epsilon made no sense there.
```

In `share/mapclib.c`, `check_oriented_path()` was added to `turn_file()` to inspect entities whose motion path index `p0` targets a `path_corner` with the `P_ORIENTED` bitflag set. Instead of using floating-point epsilon heuristics (`SMALL`), the warning relies directly on the explicit `P_ORIENTED` flag to alert mappers that initial runtime orientation will differ from the editor preview:
```c
static void check_oriented_path(struct mapc_context *ctx, int p0, int has_angles)
{
    struct s_base *fp = &ctx->file;

    if (p0 >= 0 && (fp->pv[p0].fl & P_ORIENTED))
    {
        char buf[MAXSTR];
        const char *name = get_sym_name(ctx, SYM_PATH, p0);

        SAFECPY(buf, ctx->opt_file ? ctx->opt_file : "mapc");
        if (has_angles)
            SAFECAT(buf, ": warning: entity with angles connected to oriented path");
        else
            SAFECAT(buf, ": warning: entity connected to path node with initial angles");

        if (name && *name)
        {
            SAFECAT(buf, " \"");
            SAFECAT(buf, name);
            SAFECAT(buf, "\"");
        }
        SAFECAT(buf, " (in-game orientation will differ from editor)\n");
        WARNING(ctx, buf);
    }
}
```

```text
[10:27:29] Commit the modified rotation.map to that branch as well, might as well.
```

In `data/map-misc/rotation.map`, the modernized entity layout was committed to the branch: legacy entities (`info_player_start`, `info_player_deathmatch`, `target_teleporter`, `info_camp`) replaced with modern `game_*` point entities, coins and switches positioned in world space targeting `path1`, and obsolete `side1`/`side2` paths removed.

```text
[10:28:59] And now finally, make a provenance log that applies to that branch. I guess we started by wanting to modify rotation.map.
```

### Verification & Technical Invariants

- **Verification:**
  - **Compiler & Unit Tests:** Built and executed `make test` across the test suite (`tests/mapclib.c`, `tests/solid_all.c`, `tests/math.c`) passing with 0 failures.
  - **Map Compilation:** Compiled `data/map-misc/rotation.map` to `data/map-misc/rotation.sol` via `mapc data/map-misc/rotation.map data` cleanly.
  - **Compiler Warning Verification:** Verified warning emission on existing maps (`data/map-vidski/voodoo.map`) where entities connect to `path_corner` nodes with `P_ORIENTED`.
- **Technical Invariants:**
  - **WYSIWYG Brush Planes:** `func_train` brush entities do not support `angles` attributes. All initial rotations on brush geometry must be applied directly to brush planes in TrenchBroom.
  - **Orientation Flag Purity:** `mapc` checks `fp->pv[p0].fl & P_ORIENTED` directly without epsilon comparisons to detect initial path orientation.
  - **Parenting Without Frame-0 Drift:** Point entities with `angles` parented to moving bodies should target the root motion path (`p0`) rather than intermediate static orientation helper nodes to avoid compound rotation ($q_{\text{parent}} \cdot q_{\text{child}}$).
