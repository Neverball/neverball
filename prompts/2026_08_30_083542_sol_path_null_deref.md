# Architectural Provenance: Solid mover evaluation NULL guards and client state lifecycle cleanup

- **Session Date:** `2026-08-30T08:35:42Z`
- **Agent:** Antigravity CLI 1.1.22 Gemini 3.7 Flash (High)
- **Base Commit:** [`85fa4c61`](https://github.com/Neverball/neverball/commit/85fa4c619e967fdbd3f66a48743d2bf01c37eab1): `Merge branch 'level-title-display'` (2026-08-28 22:01:46 UTC)
- **Commits:**
  - [`876bd598`](https://github.com/Neverball/neverball/commit/876bd598481eb82ca8bcb2df25808f73ce046a4e): `share/solid_all: Handle NULL base and path pointers in mover evaluation` (2026-08-30 10:06:22 UTC)
  - [`829f4f26`](https://github.com/Neverball/neverball/commit/829f4f26a32b002b1a3e6e69ede202a6aabb622c): `share/solid_vary: Handle out-of-bounds path index in setup_mover` (2026-08-30 10:06:26 UTC)
  - [`7dc63a1c`](https://github.com/Neverball/neverball/commit/7dc63a1ca0649b5aa0d5331ab9cc32f3f2acbfe0): `tests: Add unit tests for solid mover evaluation with NULL base` (2026-08-30 10:07:02 UTC)
  - [`c4dee307`](https://github.com/Neverball/neverball/commit/c4dee3078af1760120a75d82ff84566af4a5ff53): `ball/game_client: Guard game_client_draw and reset state before freeing` (2026-08-30 10:31:39 UTC)
- **Files Modified:** `Makefile`, `ball/game_client.c`, `ball/game_server.c`, `share/solid_all.c`, `share/solid_vary.c`, `tests/test.h`, `tests/test_main.c`, `tests/test_solid.c`

---

## Annotated Prompt Log

### Initial Problem & Hypothesis

```text
[08:35:42] Game crash dump in @[crash.txt]
```

The crash log (`crash.txt`) reported an `EXC_BAD_ACCESS (SIGSEGV)` with `KERN_INVALID_ADDRESS at 0x0000000000000024` on macOS ARM64 occurring during level teardown and rendering.

Call stack at the faulting thread:
`main()` $\rightarrow$ `st_paint()` $\rightarrow$ `fail_paint()` $\rightarrow$ `game_draw()` $\rightarrow$ `game_draw_fore()` $\rightarrow$ `sol_entity_p()` $\rightarrow$ `get_move_transform()` $\rightarrow$ `get_move_pos()`.

Disassembly and register state inspection identified the faulting instruction as `ldr w21, [x8, #0x24]`, where `x8` was `0x0` (`NULL`), corresponding to accessing `vary->base->pv[curr_pi].pi`. In `struct b_path` (`share/solid_base.h`), `int pi` is at byte offset `36` (`0x24`), resulting in a memory read translation fault at address `0x0000000000000024`.

The initial exploration across `share/solid_all.c` and `share/solid_vary.c` added broad pointer guards, boundary checks, path cycle walk protections, and `MAX_MOVE_DEPTH` recursion limits, alongside a unit test suite in `tests/test_solid.c`.

### Build Verification & Worktree Isolation

```text
[09:46:14] make -j8
```

The build was compiled with `make -j8` and verified with `make test`.

```text
[09:47:04] Put this work in a separate worktree
```

A dedicated git worktree was created on branch `sol-path-bounds` (`git worktree add -b sol-path-bounds .worktrees/sol-path-bounds master`). The changes were transferred, verified with `make test`, and committed, restoring the original worktree cleanly.

### Root Cause Analysis & Lifecycle Diagnosis

```text
[09:51:51] What caused the crash?
```

Root cause analysis traced the fault through four interconnected engine states:
1. **Level Load Failure:** When an invalid or uncompiled map was selected, `game_client_init()` invoked `game_client_free()`, resetting `game_base` and leaving `vary->base->pv = NULL` and `base->pc = 0`.
2. **State Transition to `st_fail`:** `game_base_load()` failed, switching state to `st_fail`.
3. **Paint Pass Execution:** The event loop dispatched `fail_paint()` $\rightarrow$ `game_draw()` $\rightarrow$ `game_draw_fore()` $\rightarrow$ `sol_entity_p()`.
4. **Unchecked Path Array Access:** In `share/solid_all.c`, `get_move_pos()` evaluated `vary->base->pv[curr_pi].pi` without checking if `vary->base` or `vary->base->pv` were `NULL`.

```text
[09:53:45] The run was done with `./neverball -l cube.sol`. Exit happened with Cmd+W during gameplay.
```

Analysis of the exit sequence during active gameplay or `st_fail`:
When Cmd+W triggered `main_quit()`, `game_client_free()` and `game_base_free()` deallocated geometry data, setting `base->pv` to `NULL`. If an OS window repaint event arrived before window destruction completed, `st_paint` executed `fail_paint()` and `game_draw()`, attempting to render entities whose underlying geometry buffers were already freed.

### Minimal Patch Refinement & Module Prefixing

```text
[09:56:37] Changes too invasive - I realize they fix it all, but I'm a fan of minimal patches.
```

The branch `sol-path-bounds` was reset to `master` and replaced with a focused 6-line patch:
- In `share/solid_all.c`: Added `!vary->base || !vary->base->pv` guards to `get_move_pos()`, `get_move_rot()`, `get_path_pos()`, `get_path_rot()`, and `sol_body_w()`.
- In `share/solid_vary.c`: Added `(fp->base && pi >= fp->base->pc)` to `setup_mover()` to prevent instantiating movers on nonexistent paths.
- In `tests/test_solid.c`: Created targeted unit tests covering `NULL` base evaluation (`test_sol_body_p_null_base`, `test_sol_body_e_null_base`, `test_sol_body_w_null_base`, and `test_sol_load_vary_oob_path_ref`).
- Integrated `tests/test_solid.c` into `tests/test.h`, `tests/test_main.c`, and `Makefile`.

```text
[10:05:27] "sol" isn't a module
```

The commits on `sol-path-bounds` were split and recommitted using standard repository module path prefixes:
1. [`876bd598`](https://github.com/Neverball/neverball/commit/876bd598481eb82ca8bcb2df25808f73ce046a4e): `share/solid_all: Handle NULL base and path pointers in mover evaluation`
2. [`829f4f26`](https://github.com/Neverball/neverball/commit/829f4f26a32b002b1a3e6e69ede202a6aabb622c): `share/solid_vary: Handle out-of-bounds path index in setup_mover`
3. [`7dc63a1c`](https://github.com/Neverball/neverball/commit/7dc63a1ca0649b5aa0d5331ab9cc32f3f2acbfe0): `tests: Add unit tests for solid mover evaluation with NULL base`

### Direct Access Audit & Fallback Semantics

```text
[10:27:19] Are these checks looking at data the function itself is accessing? Not checking on behalf of children?
```

Audit of each guarded site confirmed all checks guard memory directly dereferenced by the function itself:
- `get_move_pos`: Dereferences `vary->base->pv[curr_pi].pi` (line 60) and `vary->base->pv + curr_pi` (line 65).
- `get_move_rot`: Dereferences `vary->base->pv[curr_pi].pi` (line 98) and `vary->base->pv + curr_pi` (line 103).
- `get_path_pos`: Dereferences `vary->base->pv + pi` for `pp->p` (lines 129, 140, 144).
- `get_path_rot`: Dereferences `vary->base->pv + pi` for `pp->e` (lines 155, 163).
- `sol_body_w`: Dereferences `vary->base->pv + mp->pi` (line 315) and `vary->base->pv + pp->pi` (line 316).
- `setup_mover`: Checks `pi >= fp->base->pc` before allocating a mover.

```text
[10:27:58] So what's the effect? Client still renders, but doesn't crash?
```

When `vary->base` or `vary->base->pv` is `NULL`:
- `get_move_pos` and `get_path_pos` return `POS_IDENTITY` (`{0, 0, 0}`).
- `get_move_rot` and `get_path_rot` return `ROT_IDENTITY` (`{1, 0, 0, 0}`).
- `sol_body_w` returns `0`.
Entities and bodies evaluate to identity transforms without faulting, allowing rendering and teardown passes to complete safely.

### Client Lifecycle State Management

```text
[10:29:29] game_client_free was added recently. Perhaps that change did not account for the game client still rendering. The fix you did is good, but we need to address the original shortcoming.
```

Inspection of commit `38534698` (`web: fix WASM exit graphics corruption and memory leak`) identified two lifecycle state shortcomings:
1. `game_client_draw()` did not check `gd.state`, allowing OpenGL rendering passes (`game_lerp_apply`, `game_draw`) to execute against freed memory during asynchronous repaint events.
2. `game_client_free()` and `game_server_free()` cleared `gd.state = 0` / `server_state = 0` only after freeing buffers (`gl`, `gd.draw`, `gd.vary`, `game_base`), leaving state flags active while deallocation was underway.

In commit [`c4dee307`](https://github.com/Neverball/neverball/commit/c4dee3078af1760120a75d82ff84566af4a5ff53) (`ball/game_client: Guard game_client_draw and reset state before freeing`):
- In `ball/game_client.c`: Added `if (gd.state)` guard around `game_client_draw()` and moved `gd.state = 0` to the beginning of `game_client_free()`.
- In `ball/game_server.c`: Moved `server_state = 0` to the beginning of `game_server_free()`.

### Provenance Finalization

```text
[10:36:25] Write the prompt log.
```

Spawned `Provenance Synthesizer` subagent to generate the Architectural Decision Record and Provenance document under `prompts/`.

### Verification & Technical Invariants

- **Verification:**
  - Build verified with `make -j8`.
  - Test runner verified via `make test` with all test suites passing (`test_common`, `test_dir`, `test_fs`, `test_lang`, `test_map`, `test_mtrl`, `test_set`, `test_sol`, `test_strbuf`, `test_solid`).
  - Unit tests in `tests/test_solid.c` explicitly verify that invoking `sol_body_p()`, `sol_body_e()`, `sol_body_w()`, and `sol_entity_p()` on a `struct s_vary` with `vary.base->pv == NULL` or out-of-bounds path references returns default identity vectors and angular velocity 0 without segmentation faults.
- **Technical Invariants:**
  - **Direct Dereference Guards in `share/solid_all.c`:** Functions accessing `vary->base->pv` (`get_move_pos`, `get_move_rot`, `get_path_pos`, `get_path_rot`, `sol_body_w`) must check `!vary->base || !vary->base->pv` before indexing into path arrays.
  - **Mover Initialization Bounds:** `setup_mover()` in `share/solid_vary.c` must ensure `pi < fp->base->pc` when `fp->base` is non-NULL to prevent instantiating movers referencing nonexistent paths.
  - **Client Lifecycle State Invariants:** `game_client_draw()` in `ball/game_client.c` must only execute when `gd.state` is active (`if (gd.state)`).
  - **Early State Flag Invalidation:** Deallocation functions (`game_client_free` in `ball/game_client.c` and `game_server_free` in `ball/game_server.c`) must reset their state flags (`gd.state = 0`, `server_state = 0`) immediately at the start of the deallocation block before freeing downstream buffers (`gl`, `gd.draw`, `gd.vary`, `game_base`).
