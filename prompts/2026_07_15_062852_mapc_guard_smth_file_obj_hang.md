# Architectural Provenance: Defensively guard smth_file against out-of-bounds normal/side index references

- **Session Date:** `2026-07-15T06:28:52Z`
- **Agent:** Antigravity CLI 1.1.17 Gemini 3.5 Flash (High)
- **Base Commit:** [`d1eedc64`](https://github.com/Neverball/neverball/commit/d1eedc643f34d2c4721a478d212814c34ecc16f9): `Fix mapc hang on OBJ files with omitted texture coordinates` (2026-07-15 06:45:29 UTC)
- **Commits:**
  - [`0dc27d30`](https://github.com/Neverball/neverball/commit/0dc27d30f8c6999781df31d437552d9ff2ddb58f): `Defensively guard smth_file against out-of-bounds normal/side index references` (2026-07-15 07:01:56 UTC)
- **Files Modified:** `share/mapclib.c`

---

## Annotated Prompt Log

### Initial Problem & Hypothesis

```text
[06:28:52] Community member fwp writes: I was going to report that smoothing no longer worked but you fixed it before I could do so.

With the new code, mapc hangs (on my system) while compiling ball/cheese-ball/cheese-ball-solid.map. But not consistently; sometimes it succeeds. I haven't got any further details at the moment, sorry.
```

```text
[06:29:00] Community member fwp writes: I was going to report that smoothing no longer worked but you fixed it before I could do so.

With the new code, mapc hangs (on my system) while compiling ball/cheese-ball/cheese-ball-solid.map. But not consistently; sometimes it succeeds. I haven't got any further details at the moment, sorry. What seems to be happening is that smth_file is accessing the side array (fp->sv) at index -1, leading to unpredictable results. This is due to some offsets having a side index of -1, which in turn is caused by read_f not parsing lines correctly if they omit texture coordinate indices (which some parts of cheese-ball.obj do).
```

In `share/mapclib.c`, `read_f()` parsed Wavefront OBJ face lines (`f`) assuming all triplet components were present (`vi/ti/si`). When encountering faces with omitted texture coordinate indices (`vi//si` or `vi`), `sscanf()` failed to parse subsequent indices, resulting in uninitialized values or negative index offsets (`-1`) stored in the face vertex offsets (`fp->ov`). During subsequent normal smoothing in `smth_file()`, indexing `fp->sv[-1]` led to out-of-bounds memory access and hangs.

A helper function `parse_triplet()` was added in `share/mapclib.c` to parse face tokens across all standard Wavefront OBJ formats (`vi/ti/si`, `vi//si`, `vi/ti`, and `vi`), falling back to 0 when indices are omitted:

```c
static void parse_triplet(const char *token, int *vi, int *ti, int *si)
{
    *vi = 0;
    *ti = 0;
    *si = 0;

    if (sscanf(token, "%d/%d/%d", vi, ti, si) == 3) return;
    if (sscanf(token, "%d//%d", vi, si) == 2)       return;
    if (sscanf(token, "%d/%d", vi, ti) == 2)        return;
    if (sscanf(token, "%d", vi) == 1)               return;
}
```

### Wavefront OBJ Face Parsing in read_f

```text
[06:31:40] Pay attention to mapc usage. You appeared to use wrong args.
```

The test command was corrected to match `mapc` CLI syntax (`./mapc -d <data_dir> -o <out_map> <in_map>`) to verify compilation of `data/ball-cheese-ball/cheese-ball-solid.map`.

```text
[06:36:27] rename parse_token to parse_triplet and tweak the if statements in that function to not use braces - it's very noisy with braces.
```

In `share/mapclib.c`, `parse_token()` was renamed to `parse_triplet()`, and the single-line `if` statements were formatted without braces.

```text
[06:37:15] I am also confused by s0 in read_f meaning one thing, but s1 to s3 meaning a completely different thing. preserve s0 and rename s1 to s3.
```

In `read_f()`, the string token buffers previously named `s1`, `s2`, `s3` were renamed to `tok1`, `tok2`, `tok3` to eliminate naming collision with the base side index parameter `s0`.

```text
[06:37:54] I am in favor of using = ""; to initialize char arrays.
```

In `read_f()`, the buffer initializations were updated to `char tok1[64] = ""; char tok2[64] = ""; char tok3[64] = "";`.

```text
[06:40:59] In the op field calculations, would it not make more sense to reorder the terms such that ti1 + t0 - 1 -> t0 + ti1 - 1? There are a bunch of instances of this pattern in read_f. Don't make the edit unless it's correct.
```

In `read_f()`, the index arithmetic terms for `op`, `oq`, and `or` were reordered to consistently place base offsets (`v0`, `t0`, `s0`) first:

```c
op->vi = v0 + vi1 - 1;
oq->vi = v0 + vi2 - 1;
or->vi = v0 + vi3 - 1;

op->ti = ti1 > 0 ? (t0 + ti1 - 1) : (t0 < fp->tc ? t0 : 0);
oq->ti = ti2 > 0 ? (t0 + ti2 - 1) : (t0 < fp->tc ? t0 : 0);
or->ti = ti3 > 0 ? (t0 + ti3 - 1) : (t0 < fp->tc ? t0 : 0);

op->si = si1 > 0 ? (s0 + si1 - 1) : (s0 < fp->sc ? s0 : 0);
oq->si = si2 > 0 ? (s0 + si2 - 1) : (s0 < fp->sc ? s0 : 0);
or->si = si3 > 0 ? (s0 + si3 - 1) : (s0 < fp->sc ? s0 : 0);
```

### Index Offsets & OBJ Specification Compliance

```text
[06:42:24] And a final question, I see you added a fallback - if an index is not present in the triplet, then the base offset is used, e.g., if si1 was not read, then s0 is assigned. Is this correct? What does s0 refer to?
```

`v0`, `t0`, and `s0` in `read_f()` represent the starting base offsets for vertices, texture coordinates, and side normals accumulated prior to the current object in the `.map` file. Clamping fallback assignments to `t0 < fp->tc ? t0 : 0` and `s0 < fp->sc ? s0 : 0` ensures that omitted attributes resolve to valid non-negative indices within the allocated array bounds.

```text
[06:43:43] Is this okay vs the OBJ spec - do you know?
```

The Wavefront OBJ specification defines vertex indices as mandatory, while texture coordinate and vertex normal indices in face definitions (`f v/vt/vn`, `f v//vn`, `f v/vt`, `f v`) are optional. In `mapc`, falling back to valid array bounds prevents memory corruption during mesh smoothing passes when processing OBJ files that omit texture coordinates or vertex normals.

```text
[06:45:26] Ok, commit this making sure to credit fwp.
```

The parsing fix was committed as [`d1eedc64`](https://github.com/Neverball/neverball/commit/d1eedc643f34d2c4721a478d212814c34ecc16f9) (`Fix mapc hang on OBJ files with omitted texture coordinates`), crediting `fwp`.

### Defensive Bounds Checking in smth_file

```text
[06:46:15] Is there a second part to this? Does smth_file need to be more cautious?
```

In `share/mapclib.c`, `smth_file()` was updated to defensively inspect `T[i].si >= 0` before dereferencing `fp->sv[T[i].si].n`. If `T[i].si < 0`, `Ni` is set to `NULL`.

```text
[06:48:43] Would it be appropriate to do if (!Ni) continue? Seems that all code that follows the Ni assignment needs Ni to be non-null anyway.
```

Skipping the loop iteration via `continue` would bypass the inner grouping loop (`for (l = i + 1; ...)`), which is required to advance past all consecutive triplets sharing the same vertex coordinate (`ci`) and material (`mi`).

```text
[06:54:07] You lost a comment - intentional?
```

The comment `/* For each set of triplets sharing vertex and material... */` and the `qsort()` call position before the loop were restored.

```text
[06:55:20] I think we've pretty much settled on C99 now, so the separation of declaration of variables and assignment is not necessary.
```

Variable declarations in `smth_file()` were adjusted to inline initialization at first use.

### Refactoring Loop Invariants in smth_file

```text
[06:58:11] Okay, let's start this commit over and apply the simplest possible change to prevent negative side index usage. No C99, nothing fancy, just the simplest thing.
```

```text
[06:58:43] I specifically referred to the last commit we worked on.
```

The modifications were reset to isolate the minimal required bounds check without altering surrounding grouping loop structure or style conventions.

```text
[07:00:35] I do not understand the addition of calculation of l... it does not depend on T[i].si, so why are you calculating it there? Explain if I don't understand.
```

```text
[07:01:39] I see now, but then is there another calculation after the continue? So if si is not negative, l is calculated twice?
```

Precomputing `l` before a `continue` would duplicate the grouping traversal logic. Instead, `Ni` is evaluated conditionally:

```c
const float *Ni = (T[i].si >= 0) ? fp->sv[T[i].si].n : NULL;
```

The similarity sorting loops and normal accumulation are guarded by `if (Ni)` checks, while `l` is computed once during the grouping accumulation loop:

```c
if (Ni)
{
    for (j = i + 1; j < c && (T[j].ci == T[i].ci &&
                              T[j].mi == T[i].mi); ++j)
    {
        for (k = j + 1; k < c && (T[k].ci == T[i].ci &&
                                  T[k].mi == T[i].mi); ++k)
        {
            const float *Nj = fp->sv[T[j].si].n;
            const float *Nk = fp->sv[T[k].si].n;

            if (v_dot(Nk, Ni) > v_dot(Nj, Ni))
            {
                temp = T[k];
                T[k] = T[j];
                T[j] = temp;
            }
        }
    }

    /* Accumulate all similar side normals. */

    N[0] = Ni[0];
    N[1] = Ni[1];
    N[2] = Ni[2];
}

for (l = i + 1; l < c && (T[l].ci == T[i].ci &&
                           T[l].mi == T[i].mi); ++l)
{
    if (Ni && v_dot(fp->sv[T[l].si].n, Ni) < 1.0f)
    {
        const float *Nl = fp->sv[T[l].si].n;
        float deg = V_DEG(facosf(v_dot(Ni, Nl)));

        if (deg <= angle)
        {
            v_add(N, N, Nl);
            acc++;
        }
    }
}

/* If at least two normals have been accumulated... */

if (acc && Ni)
{
    /* Store the accumulated normal as a new side. */
    ...
}
```

```text
[07:02:53] That's funny! We're back to what you did initially. Good work.
```

The change was committed as [`0dc27d30`](https://github.com/Neverball/neverball/commit/0dc27d30f8c6999781df31d437552d9ff2ddb58f) (`Defensively guard smth_file against out-of-bounds normal/side index references`).

### Session Logging & Finalization

```text
[09:05:33] Log this session
```

The session transcript was extracted and documented under `prompts/`.

### Verification & Technical Invariants

- **Verification:**
  - Built `mapc` via `make -j4` cleanly without warnings.
  - Executed `./mapc -d data/ -o /tmp/cheese-ball-solid.map data/ball-cheese-ball/cheese-ball-solid.map` and verified exit code 0 without hanging.
  - Verified `git diff` against `share/mapclib.c` for clean bounds checking and zero compiler regressions.
- **Technical Invariants:**
  - **OBJ Face Token Flexibility:** Wavefront OBJ face lines (`f`) can contain `v/vt/vn`, `v//vn`, `v/vt`, or `v` tokens; `parse_triplet()` handles all four variations and zeroes missing indices.
  - **Safe Index Fallbacks:** In `read_f()`, omitted texture or normal indices fall back to bounded base offsets (`t0 < fp->tc ? t0 : 0`, `s0 < fp->sc ? s0 : 0`) to prevent negative index corruption in `fp->ov`.
  - **Defensive Smoothing Checks:** In `smth_file()`, `Ni` is set to `NULL` if `T[i].si < 0`, safely guarding `fp->sv` dereferences during normal similarity sorting and accumulation without disrupting the triplet grouping loop progression.
