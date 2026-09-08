# Architectural Provenance: ball/game_common: Encapsulate struct game_base and decouple client and server

- **Session Date:** `2026-08-30T20:39:26Z`
- **Agent:** Antigravity CLI 1.1.22 Gemini 3.7 Flash (High)
- **Base Commit:** [`b7815302`](https://github.com/Neverball/neverball/commit/b78153022528a2ab269138ec2eaa0890edb93387): `Merge branch 'sol-path-bounds'` (2026-08-30 10:47:51 UTC)
- **Commits:**
  - [`7f012922`](https://github.com/Neverball/neverball/commit/7f0129228505af17f94d34ffb39097cf6a3fb0ea): `ball/game_common: Encapsulate struct game_base and decouple client and server` (2026-08-31 07:49:41 UTC)
- **Files Modified:** `ball/game_client.c`, `ball/game_common.c`, `ball/game_common.h`, `ball/game_server.c`

---

## Annotated Prompt Log

### Synchronous Crash Path & Shared Singleton Diagnosis

```text
[20:39:26] I don't fully understand the path of the crash. I've already deployed the fix, but wondering how can null + 0 even happen at that point - all the accesses seem to be happening inside for loops that loop over the counters.
```

Investigation analyzed the relationship between mover counters and path counters across `share/solid_vary.c` and `share/solid_all.c`. While callers loop over item (`vary->hc`) or mover (`vary->mc`) counts, mover evaluation dereferences the base path array (`vary->base->pv`) using path index `vary->mv[mi].pi`. The mover array in `struct s_vary` and the path array in `struct s_base` are distinct allocations governed by separate counters.

```text
[20:42:35] How can the counters not be in sync? vary is initialized off of base and shouldn't vary be deallocated at the point where the crash happens?
```

```text
[20:44:57] Strictly speaking, mover fix made the other fixes moot, but was a good choice to do anyway?
```

```text
[20:47:02] This explanation doesn't make sense because all these operations are synchronous - they don't happen in parallel. You can't get in a place where something is freed while something else renders it.
```

Tracing the synchronous call tree revealed the root cause: Neverball operates synchronously in a single thread, but during gameplay exit transitions (`st_level.c:goto_exit()`), `game_server_free(NULL)` was called before `game_client_free(NULL)`. Because `game_server_free()` called `game_base_free(NULL)`, the shared global `game_base` singleton in `ball/game_common.c` had its geometry data deallocated (`sol_free_base(&game_base)`) and `game_base.pv` set to `NULL`. The client state transition (`goto_state_fade()`) immediately triggered a repaint pass (`fail_paint()` $\rightarrow$ `game_client_draw()`), which evaluated mover transforms against `gd.vary.base` (pointing directly to the now-freed `game_base`).

```text
[20:50:58] Strictly speaking, mover fix made the other fixes moot, but was a good choice to do anyway?
```

```text
[20:53:17] Right, but isn't vary freedy during game_client_free? It somehow still has movers/
```

```text
[20:53:46] resume
```

```text
[20:55:44] But cube.sol did not have 0 paths. It literally had path_corner entities at the time of crash.
```

```text
[20:58:46] resume
```

```text
[21:00:02] coin.map does not have 0 paths.
```

```text
[22:35:31] What the heck? So we're freeing game base while client still uses it?
```

```text
[22:36:34] You didn't eveb answer.
```

The trace confirmed that `game_base` was a shared mutable global singleton in `ball/game_common.c` without reference counting or coordination between subsystems. When `game_server` deallocated `game_base`, `game_client` continued to hold a dangling reference in `gd.vary.base`, dereferencing `base->pv` during subsequent paint passes.

### Architecture Planning & Heap Memory Profiling

```text
[22:45:06] /plan Fix the base SOL being deallocated while it is still being referenced. Evaluate whether sharing the baes SOL actually has any benefit.
```

An architectural evaluation was conducted to assess whether sharing `struct s_base` between client and server provided meaningful memory savings, or whether full decoupling was preferable.

```text
[07:15:55] "duplicate allocations of multi-megabyte BSP and vertex buffers is desirable" - this is not based in code, just some waxing poetic.
```

A memory analysis script scanned all 435 `.sol` files in the `data/` directory to measure real in-memory heap consumption:
- Average `s_base` heap allocation: **399.8 KB**
- Median `s_base` heap allocation: **~250 KB**
- Single largest map (`bankaround.sol`): **3.53 MB**
- Average disk load time: **1–5 ms**

```text
[07:17:50] Measure loaded s_base size of bankaround.sol
```

Direct measurement of `sol_load_base()` heap allocations for `map-vidski/bankaround.sol` confirmed a total in-memory size of 3,705,916 bytes (3.534 MB), predominantly consisting of offsets (`ov`: 849.4 KB), geometries (`gv`: 661.8 KB), indices (`iv`: 513.6 KB), and sides (`sv`: 430.4 KB).

```text
[07:20:47] I don't even understand the reference counted proposal. Say you need a different base model loaded but the old one still has references?
```

Reference counting a single shared global `game_base` slot was discarded because it failed during overlapping level transitions—if `game_server` begins loading a new level while `game_client` is still completing a fade-out transition of the previous level, a single shared slot cannot hold two distinct level geometries simultaneously. Full subsystem decoupling was chosen.

### Encapsulating `struct game_base` & Subsystem Decoupling

```text
[07:32:24] Yeah, do it.
```

Initial decoupling started by declaring static `struct s_base` and path variables in both `ball/game_server.c` and `ball/game_client.c`.

```text
[07:33:43] Redo the plan so the code is not duplicated but takes a pointer, maybe game_base is a struct now that holds a s_full and a char *?
```

The architecture was refined to prevent duplicating loading and caching logic across server and client. In `ball/game_common.h`, `struct game_base` was introduced to encapsulate `struct s_base base` and `char *path`:

```c
struct game_base
{
    struct s_base base;
    char         *path;
};

int  game_base_load(struct game_base *, const char *);
void game_base_free(struct game_base *, const char *);
```

`ball/game_common.c` implemented `game_base_load()` and `game_base_free()` as reusable functions operating on a caller-supplied `struct game_base *gb`.

```text
[07:35:05] [Approved] plan_base_sol_lifecycle.md
```

The refactoring landed across four files:
1. `ball/game_common.h`: Replaced `extern struct s_base game_base;` declaration with `struct game_base` and pointer-based function prototypes.
2. `ball/game_common.c`: Removed `struct s_base game_base;` and `static char *base_path;`. Replaced with `game_base_load(struct game_base *gb, const char *path)` and `game_base_free(struct game_base *gb, const char *next)`.
3. `ball/game_server.c`: Declared `static struct game_base server_base;`, passing `&server_base` to `game_base_load()` and `game_base_free()`, and passing `&server_base.base` to `sol_load_vary()`.
4. `ball/game_client.c`: Declared `static struct game_base client_base;`, passing `&client_base` to `game_base_load()` and `game_base_free()`, and passing `&client_base.base` to `sol_load_vary()`.

```text
[07:46:51] Add NULL checks parameters of these modified functions.
```

Defensive parameter checks were added to `ball/game_common.c`:
- `game_base_load()`: Returns `0` if `!gb || !path`.
- `game_base_free()`: Returns immediately if `!gb`.

```text
[07:51:12] I rebased branch on master. Since I had a deployment in the middle of a session, I've already generated a prompt log once in this session.  Possible to amend the existing prompt log or better to do a new one? I think a new one would be better.
```

The branch `sol-path-bounds` was rebased on top of `master` (`b7815302`), isolating the `struct game_base` encapsulation and decoupling into a clean atomic commit [`7f012922`](https://github.com/Neverball/neverball/commit/7f0129228505af17f94d34ffb39097cf6a3fb0ea), and synthesizing this second-phase provenance record.

### Verification & Technical Invariants

- **Verification:**
  - Build verified with `make -j8` compiling `neverball`, `neverputt`, and `mapc`.
  - Test runner verified via `make test` with all 31 unit tests passing across all modular test suites (`test_common`, `test_dir`, `test_fs`, `test_lang`, `test_map`, `test_mtrl`, `test_set`, `test_sol`, `test_strbuf`, `test_solid`).
- **Technical Invariants:**
  - **Subsystem Decoupling:** `game_server` and `game_client` must each maintain their own independent `struct game_base` instances (`server_base` and `client_base`). Neither subsystem may access or deallocate the other's base geometry.
  - **Re-entrant Independence:** `game_server` can load or deallocate a level while `game_client` is still active or fading out another level without corrupting vertex, mover, or BSP node pointers.
  - **Pointer-Based Interface:** `game_base_load(struct game_base *gb, const char *path)` and `game_base_free(struct game_base *gb, const char *next)` must validate `gb` (and `path` for load) prior to dereferencing.
  - **Path Cache Invalidation:** `game_base_load()` must verify whether `gb->path` matches `path` before reloading; if different, it must call `sol_free_base(&gb->base)`, free `gb->path`, and set `gb->path = NULL` before invoking `sol_load_base()`.
