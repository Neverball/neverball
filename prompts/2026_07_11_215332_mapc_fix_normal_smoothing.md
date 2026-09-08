# Architectural Provenance: mapc: fix normal smoothing

- **Session Date:** `2026-07-11T21:53:32Z`
- **Agent:** Antigravity CLI 1.1.17 Gemini 3.7 Flash (High)
- **Base Commit:** [`616f7d24`](https://github.com/Neverball/neverball/commit/616f7d24a281fd7d7d9df947a578aee7ad3573bc): `mapc: handle brushes in double-precision; track plane/vert connectivity` (2026-05-18 21:13:22 UTC)
- **Commits:**
  - [`5311bd9d`](https://github.com/Neverball/neverball/commit/5311bd9d5d6e18076cc3522607aab2c721a2b209): `mapc: fix normal smoothing` (2026-07-11 22:41:27 UTC)
- **Files Modified:** `.gitignore`, `share/mapclib.c`

---

## Annotated Prompt Log

### Initial Problem & Hypothesis

```text
[21:53:32] User reports that the "angle" material flag is broken since the latest mapc rewrite - smooth normals are not generated for surfaces with angle materials applied. Plan out a fix.
```

The initial planning phase inspected git history for `share/mapclib.c`, comparing against the earlier map compiler overhaul (commit `28382048`).

```text
[21:56:30] I was referring to double-precision rewrite.
```

Investigation shifted to commit `616f7d24` (`mapc: handle brushes in double-precision; track plane/vert connectivity`). Diff analysis revealed that commit `616f7d24` had removed `uniq_side(ctx)` from `uniq_file()` to prevent plane deduplication, causing side indices (`si`) to remain distinct across all brush faces. The initial fix proposal drafted in `angle_fix_plan.md` hypothesized restoring `uniq_side(ctx)` prior to `smth_file()`.

### Constraint Discovery: 1:1 Map Plane Fidelity & Canonical Vertices

```text
[22:01:16] "side" processing was specifically altered at several points to match 1:1 with input data: the planes are the "source of truth" for the rest of the derived data. if a .map file has 64 planes, the .sol file must have those same 64 planes as sides. this arises from the requirement to match 1:1 vertex/edge/face wise to what map editors generate: they do not do any such dedupe pass.  a deduplication pass destroys the source of truth.
```

Side processing in `mapc` is required to preserve a 1:1 mapping between `.map` plane definitions and `.sol` side structures to maintain structural equivalence with map editor geometry. Global plane deduplication was intentionally eliminated.

This established two distinct failure points in `smth_file()`:
1. **Cross-brush vertex grouping:** Vertices from separate brush lumps are not deduplicated globally, so sorting triplets (`struct b_trip`) by raw vertex index `vi` in `comp_trip()` failed to group coincident vertices across adjacent brushes.
2. **Coplanar face discrimination:** `smth_file()` previously checked `T[l].si != T[i].si` to distinguish different planes meeting at an edge from coplanar triangles on the same plane. Without side deduplication, coplanar triangles across different brushes have unique side indices, breaking this check.

The plan was updated to confine all changes entirely within `smth_file()`:
- Construct a temporary position-based canonical vertex equivalence map (`SMALL_VERT = 0.001f` epsilon).
- Store canonical vertex indices `ci` in `struct b_trip` and sort/group by `ci` instead of `vi`.
- Compare actual face normal vectors using `v_dot(...)` rather than comparing side indices (`si`).

```text
[22:12:37] [Approved] angle_fix_plan.md
```

```text
[22:13:27] implement
```

```text
[22:14:20] Implement plan
```

In `share/mapclib.c`, `struct b_trip` was extended with `int ci`, `comp_trip()` was updated to compare `tp->ci` against `tq->ci`, and normal dot product comparisons (`v_dot(fp->sv[T[l].si].n, Ni) < 1.0f` and `v_dot(Nk, Ni) > v_dot(Nj, Ni)`) replaced side index checks in `smth_file()`.

### Diagnosis & Equivalence Loop Correction

```text
[22:28:16] I visually inspected Medium 08 and didn't really see a smooth surface.
```

```text
[22:29:18] Medium 08 is cross.sol
```

Inspection of `cross.map` (Medium 08) revealed that smoothing was still not applying correctly across adjacent brushes.

Diagnosis of the vertex equivalence loop in `smth_file()` revealed an indexing defect: the initial implementation iterated `jdx` over `0 .. nc - 1` (where `nc` was a running counter of unique classes) but indexed `fp->vv[jdx]`. Because `fp->vv` is the raw array of vertices rather than a table of class representatives, comparing `fp->vv[idx]` against `fp->vv[0 .. nc - 1]` compared against the first `nc` vertices in the file rather than the canonical representative of each class.

The loop was corrected in `share/mapclib.c` to iterate `jdx` from `0` to `idx - 1`:
```c
for (jdx = 0; jdx < idx; jdx++)
    if (fabsf(fp->vv[idx].p[0] - fp->vv[jdx].p[0]) < SMALL_VERT &&
        fabsf(fp->vv[idx].p[1] - fp->vv[jdx].p[1]) < SMALL_VERT &&
        fabsf(fp->vv[idx].p[2] - fp->vv[jdx].p[2]) < SMALL_VERT)
        break;

canonical_verts[idx] = jdx;
```
For any vertex `idx`, `canonical_verts[idx]` is assigned the index of the first coincident vertex `jdx` in `fp->vv`, establishing `jdx` as the canonical representative.

### Storage Cleanliness & Memory Management

```text
[22:34:13] I observed the plan decided to use vert_swaps because it's unused storage. But this makes the code unreadable. The correct approach would have been to add new, appropriately named storage and potentially remove the unused storage.
```

In `share/mapclib.c`:
1. The unused member `int vert_swaps[MAXV]` was removed from `struct mapc_context`.
2. Inside `smth_file()`, `canonical_verts` was dynamically allocated via `malloc(fp->vc * sizeof (int))` with an allocation failure guard calling `overflow(ctx, "canonical vertices")`.
3. `canonical_verts` is freed before `smth_file()` returns.

```text
[09:01:34] Log this session
```

Session distillation and prompt logging executed.

### Verification & Technical Invariants

- **Verification:** Built `mapc` via `make mapc` (compiling `share/mapclib.c` and `share/mapc.c` with zero warnings). Tested compilation of `data/map-medium/cross.map` into `cross.sol` and verified smooth normal generation on angle material surfaces across brush boundaries.
- **Technical Invariants:**
  - **1:1 Plane Source of Truth:** `uniq_side()` must not be called during general map parsing or before smoothing. Side definitions in `.sol` files must maintain a 1:1 correspondence with the input `.map` file plane definitions.
  - **Position-Based Canonical Vertices:** Normal smoothing across brush lump boundaries requires position-based equivalence mapping (`SMALL_VERT = 0.001f` epsilon) localized to `smth_file()`.
  - **Vector Normal Comparisons:** Coplanar and edge boundary checks during normal smoothing must operate on surface normal dot products (`v_dot()`), never by asserting side index inequality (`si != si`).
