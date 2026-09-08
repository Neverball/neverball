# Architectural Provenance: mtrl: implement filter-nearest flag for exact color transitions

- **Session Date:** `2026-08-16T17:58:37Z`
- **Agent:** Antigravity CLI 1.1.17 Gemini 3.7 Flash (High)
- **Base Commit:** [`91242eb6`](https://github.com/Neverball/neverball/commit/91242eb60236a313fade0daf0eb284f95ef48993): `Plug memory leaks in sol_free_vary` (2026-08-16 18:23:33 UTC)
- **Commits:**
  - [`b38ec9ee`](https://github.com/Neverball/neverball/commit/b38ec9ee82f3d4b1e5651c9ceaff6458e4639275): `mtrl: implement filter-nearest flag for exact color transitions` (2026-08-16 18:23:43 UTC)
- **Files Modified:** `share/mtrl.c`, `share/solid_base.c`, `share/solid_base.h`

---

## Annotated Prompt Log

### Initial Problem & Hypothesis

```text
[17:58:37] /plan A material flag that sets GL filtering to exact values so white/black can be next to each other without smudging.
```

In Neverball's material system (`share/mtrl.c`), all textures were previously loaded with `IF_MIPMAP` and sampled with default linear filtering (`GL_LINEAR` / `GL_LINEAR_MIPMAP_LINEAR`). The initial planning phase investigated `share/solid_base.c` and `share/mtrl.c` to map out material flag allocation and OpenGL texture parameter bindings.

### Constraint Discovery: Naming Discipline & Decoupling

```text
[18:04:19] Two points: in mtrl_flags, only use the exact name, never add aliases; secondly, let's rename M_NEAREST in a way that can (in the future potentially) select other filtering types, and separate min and mag filtering - but don't add those, just inform yourself for the naming of M_NEAREST.
```

In `share/solid_base.h`, the macro was defined as `M_FILTER_NEAREST (1 << 12)`, and in `share/solid_base.c`, only the single keyword `"filter-nearest"` was registered in `mtrl_flags[]`.

```text
[18:05:11] It's good.
```

Implementation commenced across `share/solid_base.h`, `share/solid_base.c`, and `share/mtrl.c`.

### Verification & Iterative Debugging

```text
[18:07:49] As a test, replace all materials in easy.map with a checker texture
```

In `data/map-easy/easy.map`, materials were temporarily swapped to reference the test checker texture.

```text
[18:09:57] Rebuild the png to have just black/white, it seems to have the filtering baked in
```

The test PNG was re-generated with hard 1-bit black/white pixels to eliminate pre-blurred source texels.

```text
[18:12:20] Is the png bundled in any downloaded addon? in ~/.neverball-dev/
```

```text
[18:13:05] Remove it and remove the entry from installed packages.
```

Cached addon packages in `~/.neverball-dev/` were removed to ensure Neverball loaded the modified local texture asset.

```text
[18:15:51] Revert changes to easy.map and instead add a little hack in level texture loader to replace all materials with that material.
```

In `share/mtrl.c`, `find_mtrl()` was temporarily modified to redirect all texture requests to the test material, while `data/map-easy/easy.map` was restored cleanly.

```text
[18:19:30] Revert the hack, it's all good now.
```

The temporary loader hook in `share/mtrl.c` was removed.

### Verification & Technical Invariants

```text
[18:20:01] Reason with me - mipmap/aniso disabling mandatory for this?
```

In OpenGL, mipmap downsampling averages adjacent texels into box-filtered lower levels of detail, causing distant or oblique surfaces to blur unless mipmap generation is disabled by passing `0` instead of `IF_MIPMAP` to `make_image_from_file()`.

```text
[18:20:42] So we done here?
```

- **Verification:** Built with `make -j4` and tested against test map `Radiant/bumper.map` and test texture `data/textures/mtrl/check-black-white`. Verified sharp, non-aliased pixel borders under OpenGL runtime rendering.
- **Technical Invariants:**
  - **Exact keyword only:** Only `"filter-nearest"` is recognized in `mtrl_flags[]`; do not add aliases (`nearest`, `point`).
  - **Disable mipmaps:** Textures with `M_FILTER_NEAREST` must pass `0` (not `IF_MIPMAP`) to `make_image_from_file()`.
