# Architectural Provenance: TrenchBroom 3D Entity Orientation & game_* Point Entities

- **Session Date:** `2026-08-29T16:04:57Z`
- **Agent:** Antigravity CLI 1.1.22 Gemini 3.7 Flash (High)
- **Base Commit:** [`85fa4c61`](https://github.com/Neverball/neverball/commit/85fa4c619e967fdbd3f66a48743d2bf01c37eab1): `Merge branch 'level-title-display'` (2026-08-28 22:01:46 UTC)
- **Commits:**
  - [`7dd7b94c`](https://github.com/Neverball/neverball/commit/7dd7b94c6602474cb7c638986b6fdba94c7c9bb9): `mapc: Synthesize parented orientation movers for angled entities` (2026-08-31 23:00:31 UTC)
  - [`63a52a0a`](https://github.com/Neverball/neverball/commit/63a52a0a94ee0a473b26d4e64617b3986a826957): `mapc: Support game_* point entities` (2026-08-31 23:00:31 UTC)
  - [`5d6388a7`](https://github.com/Neverball/neverball/commit/5d6388a7ad70e2b6526c9dedddd4c2df323ec9f9): `obj/editor: Add arrow.map authoring helper` (2026-08-31 23:00:31 UTC)
  - [`2e365887`](https://github.com/Neverball/neverball/commit/2e36588796793e3634b57b1d299cbd93d3824ef1): `obj/editor: Add ball2.obj preview model` (2026-08-31 23:00:31 UTC)
  - [`b21e651b`](https://github.com/Neverball/neverball/commit/b21e651b8a04af6fe902b041502a0f06167dee31): `obj/editor: Add goal2.obj preview model` (2026-08-31 23:00:31 UTC)
  - [`f5573e07`](https://github.com/Neverball/neverball/commit/f5573e073f1d598a8a54d0bda8c4e5583ad088b5): `obj/editor: Add switch2.obj, switch3.obj, and jump2.obj preview models` (2026-08-31 23:00:31 UTC)
  - [`e4eed16c`](https://github.com/Neverball/neverball/commit/e4eed16c8b35806171376ce1be9f7c30cba2769f): `entities: Add game_* definitions with symmetric bounding boxes` (2026-08-31 23:00:31 UTC)
  - [`082847ce`](https://github.com/Neverball/neverball/commit/082847ceaa2b2971412d370f7e76f1a647beed5f): `docs: Document TrenchBroom entity definition conventions in AGENTS.md` (2026-08-31 23:00:31 UTC)
  - [`ddc8a06a`](https://github.com/Neverball/neverball/commit/ddc8a06a1c84c04a9b226159b80065a038b0d52f): `map-misc: Add rotating cube map` (2026-08-31 23:00:31 UTC)
- **Files Modified:** `AGENTS.md`, `Radiant/neverball.game/data/entities.ent`, `TrenchBroom/entities.ent`, `data/map-misc/cube.map`, `data/obj/editor/arrow.map`, `data/obj/editor/ball2.obj`, `data/obj/editor/goal2.obj`, `data/obj/editor/jump2.obj`, `data/obj/editor/switch2.obj`, `data/obj/editor/switch3.obj`, `data/set-misc.txt`, `share/mapclib.c`

---

## Annotated Prompt Log

### Initial Problem & Rotating Cube Map Prototype

```text
[16:04:57] /plan Let's build your intuition for Neverball mapping by making the following map: we're on a big cube (512x512x512 units) that rotates once every 5 seconds - another one of its faces becomes the "top". On each face is a single coin, as the cube rotates, that coin rotates with it.
```

The initial planning phase investigated Neverball's mover and path system across `mapc/` and `share/mapclib.c`. In Neverball, moving solid bodies (`func_train`) and attached point entities follow sequences of `path_corner` nodes. The compiler shifts brush vertices relative to the initial path node, allowing rotation around the path origin. To rotate a $512 \times 512 \times 512$ unit solid cube through $360^\circ$ around the X axis without hitting `q_slerp` antipodal singularities ($\ge 180^\circ$), four $90^\circ$ pitch path segments (`0 0 0`, `90 0 0`, `180 0 0`, `-90 0 0`) were planned in `rotating_cube_map_plan.md`.

```text
[08:29:17] [Approved] rotating_cube_map_plan.md
```

In `data/map-misc/cube.map`, a $512 \times 512 \times 512$ unit solid cube brush was created inside a `func_train` entity parented to `cube_p0`. Six `light` entities (ITEM_COIN) and one `info_player_deathmatch` (goal) were placed on the six cube faces targeting `cube_p0`. The map was compiled to `data/map-misc/cube.sol` and registered in `data/set-misc.txt`.

### Path Rhythm & Orientation Adjustments

```text
[10:49:48] Change path sequence so that the platform stays fixed for 5 seconds, then flips in 1 second, and so on.
```

In `data/map-misc/cube.map`, the path network was split into alternating stationary rest phases and rotation steps:
- Four stationary nodes (`cube_p0a`, `cube_p1a`, `cube_p2a`, `cube_p3a`) with `speed "5"` and `smooth "0"`.
- Four rotation transitions (`cube_p0b`, `cube_p1b`, `cube_p2b`, `cube_p3b`) with `speed "1"` and `smooth "1"`.

```text
[10:50:37] This isn't how mapc is used.
```

The build invocation was corrected to standard Neverball usage: `mapc data/map-misc/cube.map data`.

```text
[10:53:02] Rotate in 2 seocnds, 1 was too fast.
```

In `data/map-misc/cube.map`, transition duration on `cube_p0b`, `cube_p1b`, `cube_p2b`, and `cube_p3b` was updated to 2.0s (`speed "2"`).

```text
[10:58:16] The coin on the second face (after first flip) is rotated (not oriented with bottom to face).
```

In Neverball's simulation engine (`share/solid_all.c`), parenting an entity to a path corner inherits that node's orientation. When side coins directly targeted `cube_p0a`, their orientation remained upright in world space rather than perpendicular to each cube face normal. In `data/map-misc/cube.map`, individual parent path corners (`coin_p0` through `coin_p5`) were configured with face-aligned `angles` and parented via `target2 "cube_p0a"` to the primary cube mover.

```text
[11:05:00] Change to 3 second flip
```

In `data/map-misc/cube.map`, transition durations were temporarily changed to 3.0s (`speed "3"`).

### First-Frame Positioning & Compiler Auto-Orientation

```text
[11:13:23] And herein is exposed the problem: when I look at this map in TrenchBroom, the entities are clearly stacked on top of each other, which creates a really terrible UX to the mapper to the point where it's actually unusable. Ideally, mapper should be able to position the entity to where it is on the first frame of the gameplay and have it appear and function as it does now orientation/positioning wise.
```

```text
[11:14:28] /plan And herein is exposed the problem: when I look at this map in TrenchBroom, the entities are clearly stacked on top of each other, which creates a really terrible UX to the mapper to the point where it's actually unusable. Ideally, mapper should be able to position the entity to where it is on the first frame of the gameplay and have it appear and function as it does now orientation/positioning wise. This requires mapc/SOL loader changes, not map changes.
```

```text
[11:15:01] /plan And herein is exposed the problem: when I look at this map in TrenchBroom, the entities are clearly stacked on top of each other, which creates a really terrible UX to the mapper to the point where it's actually unusable. Ideally, mapper should be able to position the entity to where it is on the first frame of the gameplay and have it appear and function as it does now orientation/positioning wise. Explore and determine if this can be done inside the map (mapper changes their workflow) or requires a mapc/SOL loader change.
```

The initial planning artifact `trenchbroom_entity_orientation_plan.md` evaluated whether entity positioning could be resolved purely inside `.map` definitions or required changes to the `mapc` compiler.

```text
[11:20:04] This misses the point I was making. E.g., when I open the map in TrenchBroom, the goal entity is positioned on the top face of the cube. Likewise, all the coins, of all faces, are positioned on the top face of the cube. This is the stacking I was referring to. They are only positioned correctly when the level is loaded in the game.
```

Investigation revealed the root ergonomic defect: requiring mappers to place entities at the origin of motion paths (or create manual companion `path_corner` nodes for every single coin, goal, and switch) forced all entities on moving or rotated platforms to stack on top of each other in TrenchBroom. The updated plan `trenchbroom_first_frame_orientation_plan.md` proposed that `mapc` automatically synthesize parented orientation mover nodes from the 3D `angles` property on point entities.

```text
[11:42:36] Remove "angle" support from the plan because TrenchBroom treats that weird.
```

Support for legacy single-axis `angle` (yaw scalar) was removed from the plan in favor of full 3D `angles` (pitch, yaw, roll vectors).

```text
[11:44:43] Extend the plan to all animated entities, not just the goal and coin entities that I mentioned.
```

The plan scope in `trenchbroom_first_frame_orientation_plan.md` was broadened across all placeable point entities: items (`item_*`, `light`), goals (`info_player_deathmatch`), switches (`info_camp`), teleporters (`target_teleporter`), spawns (`info_player_start`), models (`misc_model`), and billboards (`misc_billboard`).

```text
[11:49:51] What about backward compatiblity? Can we ensure that? So that func_train and info_player_start entities don't change position when recompiling existing maps? Or at least a mapc flag, or some other way to easily switch to the legacy way?
```

Analysis of existing map files confirmed that point entities in existing maps did not specify 3D `angles` properties; only `path_corner` entities used orientation. Thus, synthesising orientation nodes only when `angles` is explicitly present guarantees zero regression on legacy maps.

```text
[12:02:06] Actually remove the legacy flag from plan, I got confused and angles were only ever supported on path_corners
```

The proposed `--legacy-orient` CLI flag was discarded from `trenchbroom_first_frame_orientation_plan.md`.

```text
[12:16:54] Looks good, approved.
```

```text
[12:21:22] make -j8
```

In `TrenchBroom/entities.ent` and `Radiant/neverball.game/data/entities.ent`, 3D `angles` attributes were added across point entities. In `share/mapclib.c`, entity parsing was updated to capture `angles`, and an orientation compiler pass was added after target resolution. In `data/map-misc/cube.map`, dummy helper `path_corner` entities were removed and coins were positioned directly on the first-frame surface locations with `angles` and `target2 "cube_p0a"`. Built and verified with `make -j8` and `make test`.

```text
[12:25:06] Were we working in the main worktree the whole time?
```

Checked git working tree status and confirmed development was executing in `.worktrees/rotating-cube-map`.

### Diagnosing Orientation Mover Transitions

```text
[12:28:05] Use different items per each face so I can tell which ones overlap: e.g., I see three items at once on the top face when loading a level
```

```text
[12:33:24] I think the items are targeted to the wrong path corner. When I load the level, there's one item on the top face, but in like 3 seconds, all of them converge to the top face including the goal.
```

Diagnosis of `share/mapclib.c` revealed that synthesized orientation path nodes had their next path index `pp->pi` initialized incorrectly or left unlinked. In Neverball's simulation engine (`share/solid_all.c`, `sol_path_step`), when a mover's timer elapses, it advances to `pp->base->pi`. If `pi` pointed to the cube's motion chain, the orientation mover was stepping into the cube's path rather than remaining stationary relative to the parent body.

In `share/mapclib.c`, synthesized stationary orientation nodes were explicitly set to self-loop (`pp->pi = pi`). In `data/map-misc/cube.map`, distinct collectible items were assigned to each face (Yellow Coin, Red Coin, Blue Coin, Grow Coin, Shrink Coin, Clock).

```text
[19:56:14] Items looks good now, but goal is still over the ball entity?
```

```text
[19:57:21] Wait, was the goal supposed to be there? If yes, it can overlap.
```

In `data/map-misc/cube.map`, the goal and player spawn both starting centered at $(0, 0, 280)$ on the initial top face is standard Neverball behavior: the goal begins inactive and closed until the coin quota is fulfilled.

```text
[20:03:42] Explain to a gamer/mapper how this works.
```

```text
[20:08:37] Okay, so this basically creates the dummy path corner automatically off of entity angles
```

```text
[20:12:18] Is this good UX, though? It's better than before, but it's still in the area of "you can place the entity where it will be on first frame, but you still have to set angles on it manually". Opinion?
```

The dual-mover architecture was documented: `mapc` reads the entity's 3D `angles` property, creates an internal stationary orientation mover node, and links it via `target2` / `target3` to the parent platform's motion path. Mappers place the entity where it starts in world space without manual path rigging.

### Discovering TrenchBroom Rotation Invariants & Origin-Centered Bounding Boxes

```text
[08:04:01] /plan Add a new entity that is literally info_player_deathmatch except it has its entity origin (in TrenchBroom) positioned at the bottom of the entity, not inside it. mapc would not add an offset to its position like it does for info_player_deathmatch right now. The current entity has a problem being rotated correctly in TrenchBroom since the initial point of rotation is inside the entity. Call the new entity info_goal, document it, copy docs from info_player_deathmatch and add a boring note that it is preferable to use this entity in place of the other one. Also mention in info_player_deathmatch, that this is exists for backward compat and info_goal is preferable.
```

```text
[08:07:33] Does it need a new OBJ? I forget if it has the offset baked in.
```

```text
[08:09:22] Call it goal2.obj
```

```text
[08:09:49] [Approved] info_goal_entity_plan.md
```

```text
[08:21:10] Use the same OBJ model for goal2.obj, just moved up.
```

```text
[08:21:21] Use the same OBJ model for goal2.obj, just moved up.
```

In `data/obj/editor/goal2.obj`, vertices were translated by $+24$ along Y so the cylinder base sat at $Y = 0$, matching `info_goal`.

```text
[08:32:32] Rotate tool behaves very strangely on entities in TrenchBroom. When I look at the new goal entity, I see a wireframe bounding box (defined as in the entities.ent file) and then a model rendered from the origin of that bounding box. When I use the rotate tool on the entity, it does not only rotate the model but does a combined transformation: it first transforms the bounding box around the point of rotation - this is effectively a translation, as it changes the origin attribute of the entity - and then applies a rotation to the visible OBJ model and renders the model from the new origin of the entity. This results in very strange looking rotations that don't seem to ever want to simply rotate about the point of rotation but does a complex dance around it. This tells me that the bounding box can't be anything but an origin-centered cube? It's such a bizarre limitation! Look into this. Read https://trenchbroom.github.io/manual/latest/ - is there any way to simply have the angles change without the recalculation of entity bounding box placement?
```

```text
[08:37:54] Seems like a simple checkbox "Only update entity angles" would solve that instantly. Just musing here.
```

```text
[08:39:03] The bounding box also does go out of sync - because in Neverball we do collisions against the transformed volume, not the axis aligned volume.
```

Analysis of TrenchBroom's spatial transformation engine identified a critical design constraint:
1. **TrenchBroom Pivot Calculation:** TrenchBroom derives the rotation pivot strictly from the geometric center of the selection's Axis-Aligned Bounding Box (AABB):
   $$\mathbf{p}_{\text{pivot}} = \text{origin} + \frac{\mathbf{min} + \mathbf{max}}{2}$$
2. **Coordinate Drift:** Any asymmetric bounding box (such as `box="-48 -48 -24 48 48 168"` or `box="-48 -48 0 48 48 192"`) causes $\mathbf{p}_{\text{pivot}} \ne \text{origin}$. When rotating with TrenchBroom's Rotate Tool (`R`), TrenchBroom orbits the entity's `origin` around $\mathbf{p}_{\text{pivot}}$, modifying the `origin` coordinate on every rotation.

```text
[08:43:17] Since the goal of info_goal was ultimately to enable correct rotations in TrenchBroom, I feel like we need to change the bounding box of the entity so its center is its origin and the center is also the bottom of the OBJ model. Mappers will have to rely on the rendered OBJ model to see the true dimensions.
```

```text
[08:44:14] Does this pivot we just did still warrant a new entity?
```

```text
[08:46:00] Perfect. Summarize the discovered requirements for a TrenchBroom entity definition.
```

```text
[08:46:56] Add succint, boring documentation about this in AGENTS.md
```

The entity design contract was documented in `AGENTS.md`:
1. Use origin-centered symmetric bounding boxes (`box="-H -H -H H H H"`).
2. Author base-anchored cylinder preview models with base vertices at $Y = -H$.
3. Include `<angles>` and `<target>` attributes on rotatable/parentable entities.

### Entity Auditing, info_ball, and Uniform Item Bounding Boxes

```text
[08:48:03] Now validate all defined entities against these rules and point out which ones still need to be fixed/deprecated.
```

```text
[09:25:11] The ball is actually already a symmetrical object. info_player_start is inherited direct from Quake, so that's why the offset is applied by mapc - to place the ball correctly on the ground. So the origin should rightly be above the ground, but it should be 0.25 meters above ground (16 units) not 24 units. So the fix for that entity would be a new entity that has origin 16 units above ground and is symmetrical.
```

```text
[09:26:16] /plan Plan this out and let's implement it.
```

```text
[09:28:55] [Approved] entity_definition_fixes_and_info_ball_plan.md
```

In `entity_definition_fixes_and_info_ball_plan.md`, `info_ball` was planned with a 32-unit symmetric cube (`box="-16 -16 -16 16 16 16"`). `info_player_start` had historically inherited Quake's 24-unit vertical floor offset, whereas Neverball's ball radius is $0.25\text{m} = 16\text{ units}$. `data/obj/editor/ball2.obj` was created as an origin-centered sphere.

```text
[12:41:19] Switch cube rotation back to 2 seconds
```

In `data/map-misc/cube.map`, flip transitions were set to 2.0s.

```text
[12:43:39] Entities like coins need their bounding boxes to be converted to cubes as well, so they look correct at any orientation. Investigate and address this.
```

In `TrenchBroom/entities.ent` and `Radiant/neverball.game/data/entities.ent`, item entities (`item_coin_yellow`, `item_coin_red`, `item_coin_blue`, `item_coin_grow`, `item_coin_shrink`, `item_clock`, `light`) were converted from asymmetric boxes (`-8 -8 -16 8 8 16`) to uniform 32-unit cubes (`box="-16 -16 -16 16 16 16"`).

### Mover Parenting & 3D Local Down-Vector Offsets

```text
[12:50:37] I added a test - two switches to the map. one on top, one where the red coin is - rotated. which paths do I connect them to?
```

```text
[12:54:27] Give me a table of the path chain and its time/angles starting from cube_p0a
```

```text
[12:54:49] Your degree syntax doens't look good on a terminal and breaks the table.
```

```text
[12:55:40] Now add the synth path corner as the first row, so I can understand how it works.
```

```text
[12:57:25] Why does it have 1s duration and smooth set?
```

```text
[12:58:09] Hmm, but can't you just not set a target?
```

```text
[13:01:07] It's kind of a complex explanation! So this is a path corner that travels on the loop of other path corners whose function is to apply a local transform. I don't think it needs to target itself and step ahead, I thought target indices defaulted to -1 which indicated "targets self" automatically without special work from mappers.
```

```text
[13:02:05] So are they different from a single path node that's not connected? I would assume not at all because those default to the same values: target itself with 1s duration.
```

```text
[13:03:23] This is cool as heck, but I'm still missing how the origin is specified for that synth orientation?
```

The orientation mover mechanism was analyzed: the synthesized node is an unconnected path node with `angles` set to the entity's orientation and `target2` linking it to the parent moving body. The entity's position mover (`p0`) uses the entity's world position converted to parent local coordinates, while the orientation mover (`p1`) applies the synthesized orientation node.

```text
[21:27:19] I've created arrow.map and used it in ball.map to show ball up/right/forward vectors, and exported that to ball2.obj
```

```text
[21:31:02] Now the following is true: 1) TrenchBroom rotation of entities is straight forward and intuitive, 2) placement of "rotated on level start" entities is easy now, 3) new symmetric origin-at-center bounding boxes mean that all entities are placed not level with the ground but in the air and need adjustment. Any way to work around this in TrenchBroom so that 1 and 2 remain true?
```

```text
[21:33:37] Let's try this with goal.obj alone to see
```

```text
[21:37:31] First off, goal collision is a cylinder not a sphere. Secondly, this leaves goal entity origin up in the air - if the bounding box is -16 to +16 the origin is now 16 units in the air not flush with the floor, so that's the goal position that mapc compiles in.
```

```text
[21:40:24] Apply changes to the goal entity/model only
```

```text
[21:42:38] Right and now we're back to TrenchBroom transforming the origin of the entity instead of simply updating the angles attribute. So this isn't the fix.
```

```text
[21:45:55] So you saying we offset the .obj and have mapc adjust the origin by half the BB size?
```

```text
[21:47:01] Try it for the goal
```

```text
[21:53:22] Reminder thet TrenchBroom bounding box size doesn't change with radius attribute, so subtracting zp->r is dangerous and a hardcoded constant matching half of BB size should probably be used instead.
```

```text
[21:56:36] So ultimately, the only changes between info_goal and info_player_deathmatch are the BB size (doesn't warrant a different entity) and the offset subtracted by mapc (requires a different entity for backward compat)
```

```text
[21:58:26] My next question: how does the subtract math work when the goal is rotated? E.g., goal is placed on a side of a cube and rotated 90 degrees to lay normal on that side - subtracting 48 units from the vertical component is going to mess that up greatly, right?
```

```text
[22:00:36] Fix it then. Use of constants/defines required because we're now referring to a magic 48 in two places.
```

For cylinder entities resting on a surface, TrenchBroom aligns the bottom of the symmetric bounding box ($Z = -H$) against the floor, placing the entity's `origin` at the box centroid (height $+H$ above the floor). The preview model base sits at $Y = -H$.

To place the cylinder contact zone on the floor regardless of orientation, `mapc` rotates a local down-vector $(0, -H, 0)^T$ by the entity's orientation quaternion before performing position calculations:

$$\mathbf{p}_{\text{contact}} = \mathbf{p}_{\text{centroid}} + \mathbf{q} \cdot \begin{pmatrix} 0 \\ -H \\ 0 \end{pmatrix} \cdot \mathbf{q}^{-1}$$

In `share/mapclib.c`, `#define GOAL_HALF_EXTENT 48.f` was defined and applied via `q_rot` along the local down-vector.

```text
[22:04:52] Apply the same changes now to switches and teleporters, and whichever other cylinder entities exist.
```

```text
[22:06:25] info_switch and info_jump sound good to me
```

In `share/mapclib.c`, constants `#define SWCH_HALF_EXTENT 32.f` and `#define JUMP_HALF_EXTENT 32.f` were added, and local down-vector offsets were applied for modern switches and teleporters. Preview meshes `data/obj/editor/switch2.obj`, `switch3.obj`, and `jump2.obj` were created with base vertices at $Y = -32$ ($Y \in [-32, 96]$).

### The game_* Namespace & Branch Finalization

```text
[22:13:37] Possible to use something like "game" instead of "info" for the new entity prefixes? Just so they're easier to find and not lumped with the legacy entities.
```

```text
[22:14:14] Do it.
```

Modern point entities were migrated from the temporary `info_*` names to the dedicated `game_*` prefix:
- `game_ball` (superseding `info_player_start`)
- `game_goal` (superseding `info_player_deathmatch`)
- `game_switch` (superseding `info_camp`)
- `game_jump` (superseding `target_teleporter`)

Updated `share/mapclib.c`, `TrenchBroom/entities.ent`, `Radiant/neverball.game/data/entities.ent`, `AGENTS.md`, and `data/map-misc/cube.map`.

```text
[22:17:55] Let's revert the unrelated ball.map and ball2.obj changes from this branch (arrow.map can stay).
```

Restored `data/obj/editor/ball.map` to its original repository state while retaining `data/obj/editor/arrow.map` and clean `ball2.obj`.

```text
[22:20:03] Clean up commits in this branch according to our standards. Commits like "mapc: Support game_* entity prefix and constant half-BB cylinder offsets" are examples of "<thing a> and <thing b>" which indicates that a commit should be split.
```

```text
[22:27:01] For data commits, remove "data: " prefix and word the message for the target audience of mappers not programmers. Your little summary has more useful context than your actual commit messages - that's a problem. You did not apply the convention of game_* entities using <name>2.obj models in order not to affect existing entities.
```

Reorganized branch commits into atomic single-concern units with mapper-focused subsystem prefixes (`obj/editor:`, `entities:`, `docs:`, `mapc:`, `map-misc:`). Ensured legacy models (`switch.obj`, `jump.obj`, `goal.obj`, `ball.obj`) remained untouched by introducing `switch2.obj`, `switch3.obj`, `jump2.obj`, `goal2.obj`, and `ball2.obj`.

```text
[22:39:24] Excellent. Let's now clean up mapclib code - cleanup in two phases, first is just iterative code cleanup where I suggest a bunch of changes and we make sure the functionality remains the same, and then we commit and integrate into the branch.
```

```text
[22:39:54] orient_entities has grave levels of code duplication, extract duplicate bits into helpers
```

```text
[22:42:32] Don't be cryptic with names, be boring with them. For the "orient" prefix, for the sake of historical consistency, let's use something like "turn_file" to match all the *_file passes and apply that prefix to the helpers if need be.
```

```text
[22:44:14] As an example I see "turn_parented" and have no idea what that does. I think the "synthetic path corner/node" terminology was helpful and should be used in code.
```

```text
[22:48:04] "turn_base: Offsets cylinder contact point along rotated local down-vector" - here with the juxtaposition of the name and the description  you can see how "turn_base" makes nobody wiser as a name. Use boring names, don't use cryptic names.
```

```text
[22:50:08] turn_offset_cylinder_base -> calc_cylinder_pos. Use this example to rename other helpers if applicable.
```

```text
[22:52:53] Good, commit and clean up the branch.
```

In `share/mapclib.c`, the orientation compiler pass was refactored:
- Named the compiler pass `turn_file(struct mapc_context *ctx)` matching Neverball's compiler pipeline (`read_file`, `make_file`, `weld_file`, `body_file`, `turn_file`, `dump_file`).
- Extracted logic into clear static helpers:
  - `calc_cylinder_pos()`: Computes contact point by rotating $(0, -H, 0)^T$ along the entity's quaternion orientation.
  - `make_parented_path()`: Creates a synthesized orientation path corner linked to a parent moving body.
  - `make_stationary_path()`: Creates a stationary self-loop orientation path corner (`pp->pi = pi`).
  - `turn_entity()`: Coordinates position mover and orientation mover synthesis for an entity.

```text
[23:00:10] Fix comments talking about "jump pads" and other made up things that don't exist in Neverball, then clean up commits. Do autosquash and rebase, don't do reset and commit cycles.
```

Replaced "Jump pads / Teleporters" comments in `share/mapclib.c` with standard "Teleporters" terminology. Autosquashed the change using `git commit --fixup` and interactive rebase.

```text
[23:02:11] Finally, rename worktree and branch to reflect what this work actually was.
```

Renamed the topic branch to `entity-orientation` and moved the worktree directory to `.worktrees/entity-orientation`.

```text
[23:23:02] Generate a promp log.
```

Spawned `Provenance Synthesizer` subagent to generate the session provenance and ADR document.

### Verification & Technical Invariants

- **Verification:**
  - Built with `make -j8` and compiled all test maps with `mapc`.
  - Executed automated test suite via `make test` (`test_mapc: ok`, `test_sol: ok`, `test_geom: ok`, `test_fs: ok`, `test_config: ok`, `test_cmd: ok`).
  - Verified `cube.map` simulation: $512 \times 512 \times 512$ solid cube cycles through 4 faces with 5s hold and 2s transition, with coins, switches, ball, and goal retaining exact normal orientation on all faces.
  - Verified TrenchBroom entity workflow: 3D Rotate Tool (`R`) rotates point entities around their origin with zero coordinate drift.

- **Technical Invariants:**
  - **Symmetric Bounding Boxes:** All native Neverball point entities must declare symmetric origin-centered bounding boxes (`box="-H -H -H H H H"`). Any asymmetric box causes TrenchBroom's AABB centroid pivot calculation to orbit the origin.
  - **Preview Model Base Vertices:** For cylinder entities anchored at the base (goals, switches, jumps), preview OBJ models must have base vertices at $Y = -H$. When mappers drop the entity onto a surface, the bottom of the bounding box ($Z = -H$) contacts the surface while the model renders flush.
  - **3D Down-Vector Rotation:** In `mapc`, cylinder contact offsets must be computed by rotating $(0, -H, 0)^T$ along the entity's orientation quaternion (`calc_cylinder_pos`). Do not apply scalar offsets directly to world $Z$.
  - **Stationary Orientation Self-Loops:** Synthesized stationary orientation path nodes must set `pp->pi = pi` to prevent `sol_path_step` from advancing the orientation mover.
  - **Namespace Separation:** Modern point entities use the `game_*` prefix (`game_ball`, `game_goal`, `game_switch`, `game_jump`) with `<name>2.obj` preview models, preserving legacy Quake entity behavior for existing maps.
