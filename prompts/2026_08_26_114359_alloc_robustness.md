# Architectural Provenance: Memory Allocation Robustness, Container Hardening, and Unit Test Suite

- **Session Date:** `2026-08-26T11:43:59Z`
- **Agent:** Antigravity CLI 1.1.21 Gemini 3.7 Flash (High)
- **Base Commit:** [`28bbda36`](https://github.com/Neverball/neverball/commit/28bbda3627f659e95520b25b16591bd9c5f9b978): `Retroactively generate a bunch of prompt logs [skip-ci]` (2026-08-21 22:52:30 UTC)
- **Commits:**
  - [`86863d04`](https://github.com/Neverball/neverball/commit/86863d04dab19fb6f2755b7fb4658e47182fd17e): `doc: Add development and workflow conventions to AGENTS.md` (2026-08-26 22:37:19 UTC)
  - [`cc6b02df`](https://github.com/Neverball/neverball/commit/cc6b02dffdfabb1a2d18faa7cd17697ebc338409): `putt/course: Handle NULL pointers in course_load` (2026-08-26 22:37:19 UTC)
  - [`f6f790fc`](https://github.com/Neverball/neverball/commit/f6f790fcc5db663b69916db54e2ecaac3a93564b): `ball/set: Handle allocation failure in set_init` (2026-08-26 22:37:19 UTC)
  - [`8a8c699f`](https://github.com/Neverball/neverball/commit/8a8c699fc467f1b7ec74b6023cbeaf08ed917da3): `ball/set: Handle NULL pointers in set_load` (2026-08-26 22:37:19 UTC)
  - [`0b849a69`](https://github.com/Neverball/neverball/commit/0b849a69ac08bddde8f3b5011c3b2a0c7f90c8eb): `share/solid_draw: Handle loader failure in sol_load_full` (2026-08-26 22:37:19 UTC)
  - [`9339c437`](https://github.com/Neverball/neverball/commit/9339c437ab3d8250bba82ccbab1b520ae039f106): `share/solid_draw: Handle allocation failure in sol_load_draw` (2026-08-26 22:37:19 UTC)
  - [`6f345c41`](https://github.com/Neverball/neverball/commit/6f345c41fd0146cc51b08c70619a975ae911f124): `share/solid_draw: Handle NULL mesh in loader functions` (2026-08-26 22:37:19 UTC)
  - [`1e6b609e`](https://github.com/Neverball/neverball/commit/1e6b609eaa10b1481967181a50f6c4cc550a4e84): `share/solid_draw: Handle NULL material in render functions` (2026-08-26 22:37:19 UTC)
  - [`7698fcb4`](https://github.com/Neverball/neverball/commit/7698fcb4d222001333ab6ba7c160ddb0df0b5329): `share/solid_vary: Handle allocation failure in sol_load_lerp` (2026-08-26 22:37:19 UTC)
  - [`a53deb32`](https://github.com/Neverball/neverball/commit/a53deb32dc61db867d9f72182f6c0d349bee771f): `share/solid_vary: Handle allocation failure in sol_load_vary` (2026-08-26 22:37:19 UTC)
  - [`c79c1187`](https://github.com/Neverball/neverball/commit/c79c118700814adce7ae50cc74dcc035ec45d68c): `share/solid_base: Handle NULL pointers in loader functions` (2026-08-26 22:37:19 UTC)
  - [`94accb9c`](https://github.com/Neverball/neverball/commit/94accb9cfa7068bd05b61eb7d53a8a68f0178089): `share/solid_base: Handle allocation failure in sol_load_file` (2026-08-26 22:37:19 UTC)
  - [`e68fa744`](https://github.com/Neverball/neverball/commit/e68fa744bb3aeeb705b9eeeaa1ba52910c72bca0): `share/solid_base: Handle allocation failure in sol_load_geom` (2026-08-26 22:37:18 UTC)
  - [`399a516a`](https://github.com/Neverball/neverball/commit/399a516ab823c56dcd4be7930b7ac570f87acdf9): `share/fetch_emscripten: Handle allocation failure in create_and_link_fetch_info` (2026-08-26 22:37:18 UTC)
  - [`e9374800`](https://github.com/Neverball/neverball/commit/e93748002773759d11b4873e472b0e71af4d4252): `share/fetch_curl: Handle allocation failure in create_and_link_fetch_info` (2026-08-26 22:37:18 UTC)
  - [`eaa58d2a`](https://github.com/Neverball/neverball/commit/eaa58d2a80ffdab1bf61f7597c09761634c69252): `share/package: Handle allocation failure in mount_local_package` (2026-08-26 22:37:18 UTC)
  - [`f9308d60`](https://github.com/Neverball/neverball/commit/f9308d601b9403ce37a4f0ce11fe8546507eb5a1): `share/package: Handle allocation failure in package loader` (2026-08-26 22:37:18 UTC)
  - [`486fd069`](https://github.com/Neverball/neverball/commit/486fd069da68fccdf5c5c85712806787ffb189aa): `share/fs_common: Handle allocation failure in fs_load_cache` (2026-08-26 22:37:18 UTC)
  - [`9f98738a`](https://github.com/Neverball/neverball/commit/9f98738aa9a769e93d9aa04b78c45ff345859a8c): `share/fs_stdio: Handle allocation failure in path functions` (2026-08-26 22:37:18 UTC)
  - [`122b86f1`](https://github.com/Neverball/neverball/commit/122b86f1c40c7e91529fed4cd732b75446c4f203): `share/dir: Handle allocation failure in dir_list_files` (2026-08-26 22:37:18 UTC)
  - [`9cd74f35`](https://github.com/Neverball/neverball/commit/9cd74f3551cac3fa7c84e98c720554c6fcbccc07): `share/dir: Handle allocation failure in add_item` (2026-08-26 22:37:18 UTC)
  - [`f608942e`](https://github.com/Neverball/neverball/commit/f608942e344515f2e6361c7040e748b7ce583d9a): `share/dir: Handle empty array in del_item` (2026-08-26 22:37:18 UTC)
  - [`309e7460`](https://github.com/Neverball/neverball/commit/309e74608a39222751306d4f7e586d28ec2ea692): `share/lang: Handle NULL in gt_init` (2026-08-26 22:37:18 UTC)
  - [`ef9a66fb`](https://github.com/Neverball/neverball/commit/ef9a66fb725de728f06c4cf7c9e0cb549bc5b551): `share/config: Handle self-assignment in config_set_s` (2026-08-26 22:37:18 UTC)
  - [`b8025d2c`](https://github.com/Neverball/neverball/commit/b8025d2c3e8402b386abd75fd7f4eace6c82f540): `share/font: Handle allocation failure in font_load` (2026-08-26 22:37:18 UTC)
  - [`5355665d`](https://github.com/Neverball/neverball/commit/5355665dc26f63947665b3f806d1087862ec5660): `share/geom: Handle NULL material in back_init` (2026-08-26 22:37:18 UTC)
  - [`17639b25`](https://github.com/Neverball/neverball/commit/17639b25732972e42a7e5c7f7e457bbd96ae8690): `share/mtrl: Handle out-of-bounds index in mtrl_get` (2026-08-26 22:37:18 UTC)
  - [`0367e28a`](https://github.com/Neverball/neverball/commit/0367e28ae918d6a5ac1ce969a68ce4f8ca8b1a6a): `share/audio: Handle allocation failure in audio functions` (2026-08-26 22:37:18 UTC)
  - [`eedcb614`](https://github.com/Neverball/neverball/commit/eedcb614196b82eb62884722c34d89cf8a96bfa7): `share/audio: Handle initialization failure in voice_init` (2026-08-26 22:37:18 UTC)
  - [`c14f84d7`](https://github.com/Neverball/neverball/commit/c14f84d722906c952f9be9c17c4a901deced2704): `share/image: Handle allocation failure in load_surface` (2026-08-26 22:37:18 UTC)
  - [`95b219e6`](https://github.com/Neverball/neverball/commit/95b219e6b67d0cfaf4450c2debd639d3045b85ec): `share/image: Handle allocation failure in make_image_from_font` (2026-08-26 22:37:18 UTC)
  - [`e409d096`](https://github.com/Neverball/neverball/commit/e409d0963d4549558f01887ad81c288fba26814b): `share/gui: Handle allocation failure in gui_trunc_tail` (2026-08-26 22:37:18 UTC)
  - [`be49b6d1`](https://github.com/Neverball/neverball/commit/be49b6d163ef7f0a23c42dddf7fad6ef37cb9f08): `tests: Add tests for share/common` (2026-08-26 22:37:18 UTC)
  - [`a3049ac1`](https://github.com/Neverball/neverball/commit/a3049ac11a2e40a159c7680649affb6498a59aff): `tests: Add tests for share/queue` (2026-08-26 22:37:18 UTC)
  - [`e3cd5b46`](https://github.com/Neverball/neverball/commit/e3cd5b46de6aa143fb7a50fa488c356af3dbad26): `tests: Add tests for share/list` (2026-08-26 22:37:18 UTC)
  - [`e7e30cff`](https://github.com/Neverball/neverball/commit/e7e30cffccce45a97b794ff132ba1801c5c58193): `tests: Add tests for share/array` (2026-08-26 22:37:18 UTC)
  - [`bce8bfb6`](https://github.com/Neverball/neverball/commit/bce8bfb6020702e840ba55130cbce2745915e1d8): `tests: Add test harness` (2026-08-26 22:37:18 UTC)
  - [`c01e6fe1`](https://github.com/Neverball/neverball/commit/c01e6fe1002a14281abdbaf4fdf7045d90a3e034): `share/common: Handle NULL pointers in path_join` (2026-08-26 22:37:18 UTC)
  - [`ec540347`](https://github.com/Neverball/neverball/commit/ec54034764132ecd5ce1fff054e530baa13ead40): `share/common: Handle allocation failure in read_line` (2026-08-26 22:37:18 UTC)
  - [`3553e9e5`](https://github.com/Neverball/neverball/commit/3553e9e5872910815bb20ef506aab47da6466830): `share/queue: Handle NULL in queue functions` (2026-08-26 22:37:18 UTC)
  - [`f7dd021b`](https://github.com/Neverball/neverball/commit/f7dd021b4233e7f20f3e5d5b7c071e9bc0467b8e): `share/list: Add list_push function` (2026-08-26 22:37:18 UTC)
  - [`6467bb21`](https://github.com/Neverball/neverball/commit/6467bb21ad411d58bda3a02cc363845f85dad42e): `share/list: Handle NULL in list_rest` (2026-08-26 22:37:18 UTC)
  - [`1c4e2e9e`](https://github.com/Neverball/neverball/commit/1c4e2e9e62eba7c48724696d8e1b5cce099c2cc6): `share/array: Handle realloc failure in alloc_del` (2026-08-26 22:37:18 UTC)
  - [`cac1a759`](https://github.com/Neverball/neverball/commit/cac1a759cfa89d7275d364cac7f0582dcc0f924e): `share/array: Handle NULL in array functions` (2026-08-26 22:37:18 UTC)
  - [`4e7fa453`](https://github.com/Neverball/neverball/commit/4e7fa4533bfd88ddadd097aa7504166238c878bb): `share/array: Handle NULL in alloc functions` (2026-08-26 21:06:32 UTC)
- **Files Modified:** `.gitignore`, `AGENTS.md`, `Makefile`, `ball/set.c`, `putt/course.c`, `share/array.c`, `share/audio.c`, `share/common.c`, `share/config.c`, `share/dir.c`, `share/fetch_curl.c`, `share/fetch_emscripten.c`, `share/font.c`, `share/fs_common.c`, `share/fs_stdio.c`, `share/geom.c`, `share/gui.c`, `share/image.c`, `share/lang.c`, `share/list.c`, `share/list.h`, `share/mtrl.c`, `share/package.c`, `share/queue.c`, `share/solid_base.c`, `share/solid_draw.c`, `share/solid_vary.c`, `tests/test.h`, `tests/test_array.c`, `tests/test_common.c`, `tests/test_list.c`, `tests/test_main.c`, `tests/test_queue.c`

---

## Annotated Prompt Log

### Initial Problem & Hypothesis

```text
[11:43:59] /plan Let's do a deep dive into callers of allocation functions. Received feedback that aborting on allocation failure is more robust and less complex than "doing the right thing". I don't share that outlook largely because we target Emscripten where OOM does not mean anything other than hitting an Emscripten/browser limit from which recovery is possible and also because code that uses abort or makes assumptions about its callers is impossible to test and reuse outside its context. So let's actually do the work and find the bugs that are revealed by checking allocation failures - an example of this is e.g. alloc_add failing in setup_mover and not signalling this anywhere just creating a partially damaged structure. Here is a report by a community member uau: As for bug reports... the code in solid_vary.c setup_mover() looks like obviously bullshit for example. What happens if "if ((move = alloc_add(alloc)))" doesn't run? I really doubt it is anything sane.
set.c has array_add() calls and then uses the result without further checking, which is worse than aborting immediately in array_add.
same for dir.c
uau
 — 
8/24/26, 12:19 AM
mtrl.c mtrl_cache() can return -1 if array_add there fails; I think that could lead to a misleading assertion failure later (or undefined behavior if assertions are disabled) if default_mtrl gets set to -1 for example and then mtrl_get(default_mtrl) runs.
uau
 — 
8/24/26, 12:27 AM
In fact I think the only "correct" cases of alloc_add() or array_add() error recovery are in package.c, and even that has the "Backward compatibility" case using the return value without checking.
uau
 — 
8/24/26, 12:37 AM
Though I'm not sure if the package.c behavior can be called correct either. Is it really better to silently skip some packages than abort? That won't even be in the middle of a game where aborting could particularly annoy a user; would needing to restart the game be worse than some packages suddenly missing, possibly creating mysterious problems the user would have very little chance of diagnosing?
```

An audit of dynamic memory allocation and container usage across `share/array.c`, `share/solid_vary.c`, `share/solid_base.c`, `share/solid_draw.c`, `share/mtrl.c`, `share/dir.c`, `share/package.c`, and `ball/set.c` confirmed several critical failure modes:
1. `share/solid_vary.c` `setup_mover()` invoked `alloc_add()` without validating the return pointer, populating corrupt keyframes on allocation failure.
2. `ball/set.c` `set_load()` and `set_init()` used `array_add()` results unchecked, leaving partially initialized set arrays.
3. `share/dir.c` `add_item()` ignored `array_add()` failure, leading to NULL dereferences during sorting and lookup.
4. `share/mtrl.c` `mtrl_cache()` returned `-1` on `array_add()` failure, resulting in out-of-bounds array reads in `mtrl_get(-1)`.
5. `share/package.c` `load_installed_packages()` and `load_available_packages()` had unhandled allocation paths and silent failures.

An initial audit and remediation plan artifact was generated covering error recovery strategies, container invariants, and rollback semantics.

### Constraint Discovery & Course Corrections

```text
[12:00:23] Emscripten build instructions are very wrong. Build happens via Docker, see Github actions for examples.
```

The build instructions in the remediation plan were updated to document the Docker-based Emscripten environment (`emscripten/emsdk:3.1.65`, `.devcontainer/install-gl4es.sh`, and `make -f emscripten/ball.mk BUILD=release`) matching the project's GitHub Actions workflow.

```text
[12:02:52] A couple of historic notes: array.[ch] module was written explicitly to delegate pointer checks to the caller. In retrospect, I consider that a mistake. Secondly, concering set_load - if that function does not check the pointer passed into it, it is wrong and needs to be fixed. That doesn't mean that alloc_add/array_add should not be checked, but a proper fix looks deeper than surface level if checks.
```

A two-tiered error handling architecture was established:
1. **Foundation Container Layer (`share/array.c`):** All `alloc_*` and `array_*` functions must guard against NULL container pointers up front and return safe sentinel values (`NULL` for pointers, `0` for counts, `-1` for indices) without crashing.
2. **Domain Loader Layer (`ball/set.c`, `share/solid_*`, `putt/course.c`):** High-level asset loaders must validate input pointers (`if (!s || !filename || !*filename) return 0;`), verify every container mutation, and perform transactional rollback upon allocation failure.

```text
[12:07:34] And to frame this correctly: aborting the game is worse than skipping a package due to allocation failure. We should not be treating a user-hostile abort as acceptable UX.
```

The core runtime recovery principle was locked in: subsystem failures must degrade gracefully and emit diagnostic warnings rather than executing user-hostile aborts.

```text
[12:11:18] Would it be possible to do red/green testing? Something minimal that regardless can demonstrate an issue and the fact that the issue was subsequently fixed.
```

A Red/Green testing strategy was added to the plan, requiring reproducible test cases that trigger failures against unhardened code and pass once defensive checks and rollback handling are applied.

```text
[12:16:02] You will work in a worktree under .worktrees (create with git worktree add). You will save your work with atomic commits, keeping them small and human reviewable. Do a full pass of the plan. If it looks good, proceed.
```

Created the `.worktrees/alloc-robustness` git worktree on branch `alloc-robustness`. Implemented an initial unit test suite in `tests/test_alloc.c` and began applying defensive checks and rollback handling across `share/array.c`, `ball/set.c`, `share/mtrl.c`, `share/geom.c`, `share/dir.c`, `share/solid_vary.c`, `share/solid_base.c`, `share/solid_draw.c`, `share/package.c`, `share/common.c`, `share/gui.c`, `share/image.c`, `share/audio.c`, `share/queue.c`, and `share/list.c`.

```text
[12:37:34] I don't think it is yet time to generate a prompt log.
```

In `putt/course.c`, added input validation checks (`if (!crs || !filename || !*filename) return 0;`) to `course_load()`.

```text
[12:41:42] Split commits like 4d58081a into one per module or function, it is covering too much ground at once.
```

Multi-module commits were re-split into atomic, single-module commits.

```text
[12:42:58] I will point out that "share:" prefix is unconventional, it makes more sense to prefix with module, e.g., "share/audio:"
```

Commit message subject prefixes were standardized to specify the full module path (e.g. `share/audio:`, `share/array:`, `ball/set:`, `putt/course:`).

```text
[12:44:55] I will point out that commit message in the form of "<do thing> and <do another thing>" ultimately indicates a conflation of jobs. Usually these are the likeliest candidate commits to be split.
```

Compound commits were decomposed into single-concern commits:
- In `share/array.c`, primitive allocation guards (`alloc_*`) were separated from container array functions (`array_*`).
- In `share/solid_base.c`, geometric reallocation handling (`sol_load_geom`) was separated from file header loading (`sol_load_file`).
- In `share/dir.c`, deletion handling (`del_item`) was separated from addition allocation checks (`add_item`).

```text
[12:50:37] I'm worried to see changes like queue_free changing fundamentally from requiring the queue to be empty to freeing its cells without regard for what those cells refer to. It's a programmer mistake to use queue_free on a non-empty queue. The assert should have stayed in place and no code should have been added to free the contents of the queue.
```

In `share/queue.c`, the element-draining loop in `queue_free()` was reverted, restoring `assert(queue_empty(q))`. Queues do not own their item pointers, and calling `queue_free` on a non-empty queue violates caller lifecycle invariants rather than representing an allocation failure. A unit test verifying the assertion behavior was added.

```text
[13:03:08] Learn to use git rebase with a custom editor. reset + recommit is very costly.
```

Transitioned from `git reset` and manual re-committing to scripted interactive rebasing (`git rebase -i` with custom sequence and message editor scripts).

```text
[13:06:58] Clean up the commits, making their commit messages as boring as possible. You are a boring programmer who never writes two ways for this task.
```

Standardized all commit subjects on `alloc-robustness` to the single boring formula: `<module>: Handle <condition> in <function>` (e.g., `share/array: Handle NULL in alloc functions`, `ball/set: Handle NULL pointers in set_load`).

### Subagent Audits & Codebase Hardening

```text
[13:09:30] Have a subagent review your work.
```

Invoked a research subagent to audit the 24 commits on `alloc-robustness`. The review verified the correctness of pointer guards and identified edge cases in path parsing and list linking.

```text
[13:13:47] Have another subagent go over the source code to see if you missed call sites/allocation sites
```

Invoked a subagent to scan the entire codebase for missed allocation sites. The audit identified unhandled allocations across `share/lang.c`, `share/common.c`, `share/config.c`, `share/font.c`, `share/audio.c`, `share/image.c`, `share/fs_stdio.c`, `share/fs_common.c`, `share/package.c`, `share/fetch_curl.c`, and `share/fetch_emscripten.c`.

```text
[13:25:54] Yes.
```

```text
[13:26:17] Stop. Start with red-green tests.
```

Added Red/Green test cases to `tests/test_alloc.c` for `path_join`, `gt_init`, `alloc_del`, `config_set_s`, and `list_cons` safe linking. Applied targeted fixes:
- `share/common.c`: Guarded `path_join()` against NULL head and tail inputs.
- `share/array.c`: Updated `alloc_del()` to use a temporary pointer during `realloc` shrinkage so that reallocation failure does not lose the existing memory block.
- `share/config.c`: Protected `config_set_s()` against self-assignment by duplicating the new string before freeing the old value.
- `share/font.c`: Added SDL_RWops and font load failure checks to `font_load()`.
- `share/audio.c`: Handled allocation failure in `voice_init()`, and guarded against NULL voice/music pointers in `audio_play()`.
- `share/image.c`: Handled surface creation failure in `load_surface()`.
- `share/solid_draw.c`: Added allocation rollback to `sol_load_draw()` and checked return values in `sol_load_full()`.
- `share/dir.c`: Cleaned up allocated list and string resources on failure in `dir_list_files()`.
- `share/fs_stdio.c`: Handled allocation failures in `zip_list_files()` and `insert_strings_into_list()`.
- `share/fs_common.c`: Validated cache list insertion in `fs_load_cache()`.
- `share/package.c`: Checked list insertion in `mount_local_package()`.
- `share/fetch_curl.c`: Handled allocation failure in `create_and_link_fetch_info()`.
- `share/lang.c`: Guarded `getenv()` result in `gt_init()` before calling `strdup()`.

### Container Evolution: The `list_push` Primitive

```text
[13:37:46] In your estimation, what change would have the biggest architectural gain after having done these dozens of edits?
```

Evaluated the architectural trade-offs of linear arena allocation for mesh geometry (`struct s_base`) versus container-level mutation primitives for linked lists.

```text
[13:43:57] /plan Implement an arena structure in the code and use it in the most impactful case.
```

Drafted an implementation plan for an arena allocator in `share/solid_base.c`.

```text
[13:47:46] I don't see the benefits. You started by claiming that goto fail would become unnecessary and yet it's there in the plan. Wouldn't it make more sense to have arena_new accept the sum of all s_base counters?
```

Updated the arena plan to pre-size the arena to the exact byte sum of all `s_base` counters (`sol_base_size(fp)`).

```text
[13:50:36] [Rejected] arena_allocator_implementation_plan.md
```

Rejected the arena allocator approach in favor of introducing a safe list prepending primitive (`list_push`) to simplify list mutations across the codebase. Drafted `list_push_refactoring_plan.md`.

```text
[20:57:39] [Approved] list_push_refactoring_plan.md
```

Implemented `list_push` in `share/list.h` and `share/list.c`:

```c
int list_push(List *listp, void *val)
{
    List node;

    if (!listp)
        return 0;

    if (!(node = list_cons(val, *listp)))
        return 0;

    *listp = node;
    return 1;
}
```

Refactored list insertion call sites across `share/dir.c`, `share/fs_stdio.c`, `share/fs_common.c`, `share/package.c`, `share/fetch_curl.c`, and `share/fetch_emscripten.c` to use `list_push()`.

```text
[21:05:35] Clean up the commits in this branch such that list_push is introduced early on and a change in approach is never needed - the fixes use list_push to begin with.
```

Reordered branch commits via interactive rebase to introduce `list_push` early in the history (`share/list: Add list_push function`), ensuring all subsequent subsystem commits used `list_push` directly without intermediate churn.

```text
[21:14:02] What happened in sol_load_lerp? I notice sol_lerp_copy appears there twice.
```

Diagnosed a duplicate `sol_lerp_copy(fp)` call in `sol_load_lerp()` (`share/solid_vary.c`). The redundant call was removed and squashed into `share/solid_vary: Handle allocation failure in sol_load_lerp`.

### TSA Review & Gameplay Fault Injection

```text
[21:38:44] Now that tree-sitter-analyzer is installed, what might we do differently using those tools for the scope of this branch?
```

Evaluated Tree-Sitter Analyzer (TSA) workflows for AST symbol navigation, caller trees, and edit impact analysis across the codebase.

```text
[21:42:00] Have a subagent review this branch using TSA tools.
```

Invoked a subagent to perform an AST and call-graph audit across all modified functions on `alloc-robustness`.

```text
[21:51:41] Add a small hack that makes an allocation function fail about once every 10 seconds, just so I can manually test it during gameplay
```

Added a periodic fault injection hook to `voice_init()` in `share/audio.c` to simulate runtime memory allocation failure during gameplay.

```text
[21:54:41] That works very well. Do another one.
```

Added a second fault injection hook to `image_next2()` in `share/base_image.c` to simulate texture allocation failure.

```text
[21:57:42] Good, remove the injections.
```

Reverted all temporary fault injection hooks from `share/audio.c` and `share/base_image.c`.

### Modular Unit Test Framework & AGENTS.md Conventions

```text
[22:00:21] Clean up the unit tests, compile properly with something like `make test`, split into single-concern files, the usual.
```

Decomposed monolithic `tests/test_alloc.c` into modular test suites:
- `tests/test.h`: Shared test macros (`TEST_RUN`, `TEST_ASSERT`, `TEST_ASSERT_FORK`).
- `tests/test_array.c`: Array and alloc primitive tests.
- `tests/test_list.c`: List and `list_push` tests.
- `tests/test_queue.c`: Queue lifecycle and empty assertion tests.
- `tests/test_common.c`: `path_join` and `read_line` tests.
- `Makefile`: Added `make test` target compiling with `-UNDEBUG`.

```text
[22:05:45] Wait a second, is that standard convention to have separate binaries for each unit test? Just wondering.
```

```text
[22:06:28] I di prefer a single runner.
```

Unified the test framework into a single executable runner (`tests/test_main.c` linking all test suites). Updated `Makefile` and `.gitignore` accordingly.

```text
[22:09:29] Clean up test output. It is kind of unreadable.
```

Fixed stdio buffer inheritance issues by adding `fflush(stdout)` and `fflush(stderr)` to `TEST_ASSERT_FORK` before subprocess exit.

```text
[22:18:57] Rewrite test harness strings to be boring.
```

Removed decorative banners and formatted headers from `tests/test.h` and suite runners, standardizing output to plain `PASS: <name>` lines.

```text
[22:20:41] What stylistic advice have you consistently received from me during this session? Make a concise list.
```

```text
[22:21:51] Add this to AGENTS.md while at the same time applying these conventions while in the process of adding it.
```

```text
[22:23:42] Review and tweak it so the rules are more universal and not only applicable to this session. E.g., relevant: work in a worktree; irrelevant: shared primitives and abstractions.
```

Added `## Development and workflow conventions` to `AGENTS.md`:
1. Work in dedicated git worktrees (`.worktrees/<branch>`).
2. Atomic, single-concern commits.
3. Boring commit message discipline (`<module>: Handle <condition> in <function>`).
4. Red/Green test methodology.
5. In-place history editing via interactive git rebase.

```text
[22:28:26] Do a final review of the changes in this branch.
```

```text
[22:29:57] Do a final cleanup of commits.
```

Audited all 45 commits on `alloc-robustness` (`28bbda36..HEAD`), split remaining compound commits in `share/solid_draw.c`, standardized commit subjects, verified `Assisted-by` attribution tags, and verified `make test` and full engine compilation.

```text
[22:42:32] Generate and commit the prompt log (provenance) for this session.
```

### Verification & Technical Invariants

- **Verification:**
  - `make test` executes the unified test runner `build/tests/test`, running 26 unit tests across `share/array`, `share/list`, `share/queue`, and `share/common` (`PASS: 26, FAIL: 0`, exit code `0`).
  - Full native build (`make all`) builds `neverball`, `neverputt`, and `mapc` binaries with zero warnings or errors.
  - Interactive fault injection verified that runtime allocation failures in audio streams and texture loading degrade gracefully without aborting the game process.
- **Technical Invariants:**
  - **Two-Tiered Error Handling:** Foundation container primitives (`array_*`, `alloc_*`, `list_*`, `queue_*`) must validate pointers defensively and never dereference NULL. Domain loaders (`set_load`, `course_load`, `sol_load_*`) must validate parameters up front and gracefully roll back partial state upon allocation failure.
  - **List Mutation via `list_push`:** Dynamic list prepending must use `list_push(&list, item)`, which updates the list head pointer only on successful allocation and returns `1` on success / `0` on failure without leaking or corrupting existing elements.
  - **Realloc Shrinkage Safety:** `alloc_del` must use temporary pointers during `realloc` shrinkage to prevent clobbering the base pointer if reallocation fails.
  - **Assert-Guarded Queue Deallocation:** `queue_free(q)` requires `assert(queue_empty(q))`; queues do not own their item contents and must not blindly free elements.
  - **Test Isolation via Subprocess Execution:** Unit tests that verify assertion aborts or memory termination must execute within forked subprocesses with explicit `fflush()` to prevent stdio buffer duplication.
  - **Boring Commit Discipline:** Commit messages follow `<module>: Handle <condition> in <function>`.
