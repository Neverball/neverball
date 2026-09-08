# Architectural Provenance: strbuf: Update call sites to use STRBUF and CSTR

- **Session Date:** `2026-07-17T08:32:44Z`
- **Agent:** Antigravity CLI 1.1.17 Gemini 3.7 Flash (High)
- **Base Commit:** [`da5ead45`](https://github.com/Neverball/neverball/commit/da5ead4584e7fceb6e7b5525c373c3ab5081c188): `strbuf: Add STRBUF typedef and enforce compile-time lvalue safety with CSTR` (2026-07-28 21:26:37 UTC)
- **Commits:**
  - [`08c2db90`](https://github.com/Neverball/neverball/commit/08c2db90ad4dc5c29e56116c2dc367df31f803d5): `strbuf: Update call sites to use STRBUF and CSTR` (2026-07-28 21:26:37 UTC)
- **Files Modified:** `ball/main.c`, `share/audio_emscripten.c`, `share/mapclib.c`

---

## Annotated Prompt Log

### Initial Problem & Hypothesis

```text
[08:32:44] Investigate https://github.com/Neverball/neverball/issues/445
```

Issue #445 reported that `mapc` failed when compiling map files with long path names. Investigation of `share/strbuf/strbuf.h` identified that `struct strbuf`'s internal buffer was hardcoded to `char buf[64]`. When map compiler path lengths exceeded 63 characters (plus null terminator), paths were truncated by `strbuf` helpers, causing filesystem calls (`fs_add_path`, `fs_set_write_dir`) to fail on truncated directories.

```text
[08:38:03] Do make -j8 instead to compile all of the project.
```

In `share/strbuf/strbuf.h`, the buffer size of `struct strbuf` was increased from 64 to `MAXSTR` (256 bytes, defined in `share/common.h`). A parallel clean build via `make -j8` verified compilation across all project targets.

### Stack Lifetime Analysis & Footgun Discovery

```text
[08:39:47] Did you explore any potential for stack overflows? Did you understand the function of strbuf (stack-based strings that can be returned from functions)?
```

Analysis of the `strbuf` subsystem identified its core architecture: `strbuf` helpers return `struct strbuf` by value so that caller expressions receive isolated stack copies rather than sharing a single `static char` buffer (avoiding multi-argument clobbering in functions like `printf` or `joinstr`). However, returning temporary structs by value introduces a C lifetime caveat: under ISO C, the lifetime of a temporary struct returned by value ends at the end of the enclosing full-expression (the semicolon). Assigning the `.buf` pointer of a temporary directly to a `const char *` variable (e.g., `const char *p = JOINSTR(...);`) creates a dangling pointer pointing to an expired stack frame.

Regarding stack consumption, increasing `buf[64]` to `buf[MAXSTR]` (256 bytes) in shallow, non-recursive helper chains (`mapc_opts`, `fs_add_path`, audio loaders) remains well within standard thread stack limits (512 KB to 8 MB) while `MIN(strlen(output), sizeof(sb.buf) - 1u)` and explicit null-termination prevent buffer overruns.

```text
[08:45:27] Very interesting - I did not know about the lifetime caveat. So strictly speaking that pattern is susceptible to future brekage as code is modified! Can you tell me how often I have used that pattern?
```

Codebase inspection identified 6 call sites exhibiting the dangling temporary pointer pattern across three files:
1. `share/audio_emscripten.c` (2 occurrences in `audio_play` and `audio_music_fade_to`): `const char *mp3 = JOINSTR(SUBSTR(filename, 0, len - 3), "mp3");`
2. `ball/main.c` (4 occurrences):
   - Link prefix and map parsing: `const char *set_part = SUBSTR(...)`, `const char *map_part = SUBSTR(...)`, `const char *set_file = JOINSTR(...)`
   - Level matching: `const char *sol_basename = JOINSTR(map_part, ".sol")`
   - Package set handling: `const char *file = JOINSTR(package_id, ".txt")`

```text
[08:51:34] This return-struct-by-value approach was a test to see how far I can push it. It worked well enough, but gets bogged down juggling structs and pointers and having to come up with names for all the variants of functions/macros. Wondering if a small preallocated memory pool would have been a better approach?
```

```text
[08:51:59] This return-struct-by-value approach was a test to see how far I can push it. It worked well enough, but gets bogged down juggling structs and pointers and having to come up with names for all the variants of functions/macros. Wondering if a small preallocated memory pool would have been a better approach?
```

Evaluated architectural trade-offs between struct-by-value temporaries and preallocated circular ring buffer / arena memory pools. Ring buffers simplify API ergonomics by returning `const char *` without struct wrapping and avoid semicolon lifetime expiration, but introduce potential silent wrap-around clobbering in dense loops and require thread-local storage or explicit context parameter passing. Arena allocators offer explicit scoping (`temp_arena_start` / `temp_arena_end`) at the cost of passing allocator handles through call chains.

### Reproducing Temporary Lifetime Corruption

```text
[09:06:34] Write a small standalone C file that demonstrates the lifetime caveat.
```

Created a standalone demonstration file `lifetime_caveat.c` featuring `get_dangling_ptr()` returning `make_str(...).buf` and a subsequent `clobber_stack()` call that allocated local stack arrays to overwrite the popped stack memory, reliably demonstrating string corruption under `-O0`.

```text
[09:08:00] Put the file in neverball folder
```

```text
[09:08:36] Add more logging so I can see what it should be vs what it is.
```

Moved `lifetime_caveat.c` to the project root and added comparison logging contrasting the expected string against the corrupted actual value of the dangling pointer after `clobber_stack()`.

```text
[09:09:20] That is crazy, a literal footgun waiting to happen...
```

```text
[09:09:48] Recompile with that
```

Recompiled `lifetime_caveat.c` with Clang using `-fsanitize=address -g -O0` and executed with `ASAN_OPTIONS=detect_stack_use_after_return=1`. AddressSanitizer intercepted the illegal read in `main` on the popped stack frame of `get_dangling_ptr`, confirming `stack-use-after-return`.

### API Redesign: Compile-Time Lvalue Safety & Refactoring

```text
[09:22:03] /plan Plan an extension/modification of the strbuf API such that this issue becomes impossible. E.g., preferring to work exclusively with strbuf structs and requiring explicit assignment of a result to a strbuf struct before being able to access its .buf member - albeit I do imagine that if any convenience macro is introduced to access .buf, it may result in this situation again.../plan
```

Generated `strbuf_api_refactoring.md`. The design prevents temporary struct lifetime bugs statically by:
1. Enforcing lvalue semantics via an address-of operator in the buffer accessor: `#define CSTR(sb) ((void)&(sb), (sb).buf)`. Because ISO C forbids taking the address of an rvalue / function return temporary, any attempt to write `CSTR(substr(...))` immediately triggers a compile-time error (`cannot take the address of an rvalue of type 'struct strbuf'`).
2. Eliminating unsafe convenience macros (`JOINSTR`, `SUBSTR`, `DIR_NAME`, `BASE_NAME`) that concealed rvalue returns behind raw pointer access.
3. Requiring callers to store results into explicit local `struct strbuf` variables.

```text
[09:26:10] I approve. I also recommend renaming STR to CSTR. I also recommend tweaking the variable names such that the _buf suffix isn't required - it's noisy.
```

Executed the approved refactoring across `share/strbuf/strbuf.h`, `share/strbuf/base_name.h`, `share/strbuf/dir_name.h`, `share/strbuf/joinstr.h`, `share/strbuf/substr.h`, `ball/main.c`, `share/audio_emscripten.c`, and `share/mapclib.c`:
- Defined `#define CSTR(sb) ((void)&(sb), (sb).buf)` in `share/strbuf/strbuf.h`.
- Removed `BASE_NAME`, `DIR_NAME`, `JOINSTR`, and `SUBSTR` macros.
- Refactored `ball/main.c` (`link_handle`, `handle_installed_action`), `share/audio_emscripten.c` (`audio_play`, `audio_music_fade_to`), and `share/mapclib.c` (`mapc_opts`, `mapc_dump`, `mapc_compile_internal`) to use local `struct strbuf` stack variables and access strings via `CSTR(...)`.
- Verified clean build via `make clean && make -j8`.

```text
[09:29:39] Add a comment to CSTR macro definition to explain why it's defined that way.
```

```text
[09:30:39] Add a typedef of struct strbuf to STRBUF
```

Added explanatory documentation above `CSTR` in `share/strbuf/strbuf.h` detailing how `(void)&(sb)` forces a compile error when an rvalue temporary is passed. Added `typedef struct strbuf STRBUF;` in `share/strbuf/strbuf.h`.

```text
[09:32:16] Replace all usage of struct strbuf with the new type
```

Replaced all occurrences of `struct strbuf` with `STRBUF` across `share/strbuf/strbuf.h`, `share/strbuf/joinstr.h`, `share/strbuf/substr.h`, `ball/main.c`, `share/audio_emscripten.c`, and `share/mapclib.c`.

### Final Verification & Commit Decomposition

```text
[21:23:30] resume
```

Updated `lifetime_caveat.c` to use `STRBUF` and `CSTR`, and ran a full clean build (`make clean && make -j8`), confirming zero compiler warnings or errors.

```text
[21:24:43] Commit, splitting into multiple logical commits as necessary.
```

Removed `lifetime_caveat.c` and split the changes into logical commits:
1. `strbuf: Increase buffer capacity to MAXSTR` (`f9632a12`): Increases `struct strbuf` buffer size from 64 to `MAXSTR` (256 bytes) to fix #445.
2. `strbuf: Add STRBUF typedef and enforce compile-time lvalue safety with CSTR` (`da5ead45`): Adds `STRBUF` typedef, defines lvalue-safe `CSTR`, and removes dangling-pointer macros (`JOINSTR`, `SUBSTR`, `DIR_NAME`, `BASE_NAME`).
3. `strbuf: Update call sites to use STRBUF and CSTR` (`08c2db90`): Updates call sites in `ball/main.c`, `share/audio_emscripten.c`, and `share/mapclib.c` to use `STRBUF` variables and `CSTR()`.

```text
[22:10:38] Could the CAM_MODE enums/logic be integrated into the CAM_1/CAM_2/CAM_3 system? So the extra cameras (original 1.4.0 and new hybrid) become extra cameras to select?
```

Explored camera mode enumeration logic in `share/game_common.h` and `share/game_common.c` regarding integrating camera modes into the `CAM_1`/`CAM_2`/`CAM_3` selection system without making codebase changes in this scope.

### Verification & Technical Invariants

- **Verification:**
  - Parallel build verification: Full clean build (`make clean && make -j8`) completed with zero warnings and errors.
  - Compile-time safety validation: Verified that passing temporary rvalue struct returns directly to `CSTR()` (e.g. `CSTR(substr(...))`) triggers compilation failure (`error: cannot take the address of an rvalue of type 'struct strbuf'` / `error: lvalue required as unary '&' operand`).
  - Runtime sanitizer verification: Verified `lifetime_caveat.c` under AddressSanitizer (`-fsanitize=address` with `ASAN_OPTIONS=detect_stack_use_after_return=1`), confirming ASan intercepts `stack-use-after-return` violations on temporary structs.
- **Technical Invariants:**
  - **Compile-Time Lvalue Safety:** Access to internal `strbuf` character buffers must use `CSTR(sb)`, which evaluates `(void)&(sb)` to trigger a compile-time error if an rvalue temporary is passed.
  - **Explicit Stack Variable Assignment:** Functions returning strings by value (`substr`, `joinstr`, `base_name_strbuf`, `dir_name_strbuf`) must be assigned to local `STRBUF` stack variables before accessing their buffer via `CSTR()`.
  - **Removal of Dangling Convenience Macros:** Convenience macros that wrap function returns and yield raw `char *` pointers (`JOINSTR`, `SUBSTR`, `DIR_NAME`, `BASE_NAME`) must not be reintroduced.
  - **Buffer Capacity Bounds:** `STRBUF` buffer capacity is set to `MAXSTR` (256 bytes) with bounded string copies (`MIN(strlen(output), sizeof(sb.buf) - 1u)`) and guaranteed null termination.
