# Architectural Provenance: ball: Display level title on play start

- **Session Date:** `2026-08-28T20:44:21Z`
- **Agent:** Antigravity CLI 1.1.22 Gemini 3.7 Flash (High)
- **Base Commit:** [`2ea06e6f`](https://github.com/Neverball/neverball/commit/2ea06e6f8fddb381a6919d5b685bf08eda62ae73): `Merge branch 'gameplay-goal-state-option'` (2026-08-28 20:35:01 UTC)
- **Commits:**
  - [`273850fd`](https://github.com/Neverball/neverball/commit/273850fdda062a7f9d010435600c65f11ab4b1e9): `ball: Add level_title and level_desc message helpers` (2026-08-28 21:55:32 UTC)
  - [`59a2b96d`](https://github.com/Neverball/neverball/commit/59a2b96de1fcd2df2adfae853bd6f39695c415d4): `ball: Display level title on play start` (2026-08-28 21:55:32 UTC)
- **Files Modified:** `Makefile`, `ball/level.c`, `ball/level.h`, `ball/st_level.c`, `tests/test.h`, `tests/test_level.c`, `tests/test_main.c`

---

## Annotated Prompt Log

### Initial Problem & Hypothesis

```text
[20:44:21] /plan When a level message starts with "=Some Text=" on a line of its own, that's the level title. When a level has a title, show that title on play start instead of "Level 01". To not lose information, level sequence number can be shown after the already-displayed level set name e.g. "Neverball Easy #<seq>" (unless it's a roman numeral in which case no hash precedes it). And goes without saying that the first line is not displayed in the level description box.
```

In Neverball, level metadata loaded from `.map` files includes an optional `"message"` string displayed in the pre-game level card (`ball/st_level.c`). Level authors in Nevermania and community map sets commonly adopted the convention of enclosing a level title in equals signs (`=Title=`) on the first line of the message.

The initial planning phase investigated `ball/level.h`, `ball/level.c`, and `ball/st_level.c` and generated `level_title_display_plan.md` to map out the implementation:
1. Pure helper functions in `ball/level.c` to parse the level title and extract the remaining description text.
2. GUI layout adjustments in `ball/st_level.c` to display the title in place of `"Level %s"` / `"Bonus Level %s"`, relocate the level sequence number to the set header line (`"<Set> #<num>"` or `"<Set> <num>"`), and omit the title from the multi-line description box (`gui_multi`).
3. Dedicated unit tests in `tests/test_level.c` integrated into the test runner.

### Constraint Discovery & Course Corrections

```text
[20:48:26] I think str_is_roman is excessive. You can do an inverse check and simply check the first digit for '0' to '9' which don't appear in roman numerals.
```

```text
[20:49:11] Not isdigit, literally check if first character falls between character constants '0' to '9'
```

To distinguish Roman numeral level identifiers (e.g., `"Easy I"`, `"Hard IV"`) from Arabic decimal numbered levels (e.g., `"01"`, `"12"`), an inverse check was selected rather than parsing full Roman numeral grammar. Because Roman numerals never start with an Arabic digit, checking whether the initial character falls within `'0'` through `'9'` determines whether a `'#'` prefix should be added.

To avoid locale-dependent behavior from `<ctype.h>` `isdigit()`, the plan was updated to use a literal character constant comparison:
```c
const char *hp = (ln && *ln >= '0' && *ln <= '9') ? "#" : "";
```

```text
[20:50:35] Evaluate STRBUF usage for level_parse_msg
```

```text
[20:54:29] Hmm, option 1 is idiomatic but uses sb.buf directly without a sane wrapper which I am not a fan of.
```

```text
[20:55:52] That's better. It does have the non-obvious "strip_spaces(CSTR(title));" - I would never have figured out what this is doing, but I assume it works?
```

```text
[20:58:25] Yes
```

Evaluated `STRBUF` (`share/strbuf/strbuf.h`) patterns for level message parsing. `STRBUF` encapsulates a 256-byte stack-allocated buffer (`struct strbuf`) returned by value. Direct access to the internal `.buf` member was avoided in favor of `substr()` (`share/strbuf/substr.h`), eliminating the need for in-place string mutation or separate whitespace trimming helpers.

The API was decoupled into two distinct helpers:
- `STRBUF level_title(const struct level *level)` returning the parsed title by value (or empty `STRBUF` if untitled).
- `const char *level_desc(const struct level *level)` returning a pointer into `level_msg(level)` advanced past the title line and line delimiter characters (`\\`, `\r`, `\n`).

### Implementation & Atomic Commits

```text
[20:59:56] [Approved] level_title_display_plan.md
```

Development proceeded in an isolated worktree on branch `level-title-display`:

1. **Commit 1 ([`273850fd`](https://github.com/Neverball/neverball/commit/273850fdda062a7f9d010435600c65f11ab4b1e9)):** Added `level_title()` and `level_desc()` to `ball/level.h` and `ball/level.c`. Added null-safety checking to `level_msg()`. Created unit tests in `tests/test_level.c` with test stubs for unresolved linker symbols, registered `test_level_all()` in `tests/test.h` and `tests/test_main.c`, and updated `Makefile` `TEST_SRCS`.
2. **Commit 2 ([`59a2b96d`](https://github.com/Neverball/neverball/commit/59a2b96de1fcd2df2adfae853bd6f39695c415d4)):** Updated `ball/st_level.c:level_gui()` to retrieve `level_title()` and `level_desc()`. When `*t` is non-empty, rendered `lvlattr` with the title and formatted `setattr` with the level number and mode (`MODE_CHALLENGE`, `MODE_STANDALONE`, or standard set). Passed `desc` to `gui_multi()`.

### Viewport Bounds & Dynamic Font Downscaling

```text
[21:09:02] What's the longest title from ~/Development/neverball-packages maps?
```

```text
[21:11:04] These long ones are wreaking havoc on the layout. GUI_LRG can show at most 18 characters on a 9/16 screen.
```

```text
[21:12:34] Let's try it - don't commit
```

A scan across all map files in `~/Development/neverball-packages` identified level titles up to 34 characters (e.g., `"Assault Course (KodeBreaker Remix)"` in `set-kb5`, `"Everything You Always Wanted to..."` in `set-typhoon`). At `GUI_LRG` (font point size $s / 7$ where $s = \min(\text{device\_w}, \text{device\_h})$), titles exceeding 18 characters overflow horizontal boundaries on narrow screens.

Dynamic font downscaling was introduced in `ball/st_level.c`:
```c
int font_size = (!*t && b) ? GUI_MED : GUI_LRG;
const int max_w = video.device_w;

if (gui_measure(lvlattr, font_size).w > max_w)
{
    if (font_size == GUI_LRG && gui_measure(lvlattr, GUI_MED).w <= max_w)
        font_size = GUI_MED;
    else
        font_size = GUI_SML;
}
```

```text
[21:14:19] "Parkour Course" feels like it should fit, yet picks a GUI_MED?
```

```text
[21:15:48] Wait a second, I think I meant 16/9 screen
```

```text
[21:16:19] In fact, Neverball GUI works off of the smallest of width/height to always fit inside the viewport
```

```text
[21:17:52] I realize I'm looking at a bonus level. Do they get the dynamic calculation as well or just hardcoded to GUI_MED?
```

```text
[21:18:55] Let's try it.
```

```text
[21:20:10] Don't do the 0.85 scaling
```

A standalone measurement tool using SDL_ttf (`measure.c`) diagnosed why `"Parkour Course"` (14 chars) was downscaled on a bonus level. Bonus levels previously initialized `font_size` directly to `GUI_MED` (`b ? GUI_MED : GUI_LRG`). Because `"Parkour Course"` was on a bonus level, it started at `GUI_MED` and never attempted `GUI_LRG`.

The font size initialization was changed to `(!*t && b) ? GUI_MED : GUI_LRG`, allowing titled bonus levels to start at `GUI_LRG` and downscale only when measured width exceeds `max_w`. A temporary `0.85` scale factor was discarded in favor of matching full `video.device_w`.

```text
[21:21:58] Commit and clean up (do not merge)
```

```text
[21:26:19] Hmm, did the min get lost?
```

Updated the commit on `level-title-display` and verified tests.

### Layout Decoupling Experiment & Reversion

```text
[21:31:53] /plan A very long title pushes the width of the message box and pushes the Start/Back buttons to extreme right/left. Wish they were more independent and Start/Back never ended up either extremely close or extremely far apart.
```

```text
[21:35:29] Let's try it - do not add the comments, though.
```

```text
[21:38:00] Nah, revert this. Also, remove the MIN we re-added, it does break things.
```

In `ball/st_level.c`, an experimental layout modification wrapped the description `gui_multi()` inside an independent `gui_hstack` with outer `gui_filler()` widgets to decouple description box width from the bottom `Start` / `Back` button bar.

Testing revealed that this decoupling introduced visual inconsistencies across differing viewport aspect ratios. The layout changes were reverted cleanly back to direct vertical stack (`gui_vstack`) nesting, and `max_w` was restored to `video.device_w`.

### Extended Title Formats & Delimiter Parsing

```text
[21:43:38] Evaluate handling chmod's level titles that don't just stick to = indicators. Can we support those or just too much trouble?
```

```text
[21:48:33] Show this to me in C99, I can't parse these strncasecmp
```

```text
[21:48:58] Show this to me in C99, I can't parse these strncasecmp and isalnum things.
```

```text
[21:50:49] Looks good.
```

A scan across all 25 levels in `set-chmod` and other package sets identified alternative title formats used in community maps:
1. **Explicit Prefixes:** `Name: Losing Ground`, `Level: Split`.
2. **Repeated Delimiters:** `==Liars==`, `===Ski(ll) Ball Done Right!===`, `~~~ Sines ~~~`, `!! Sinking !!`, `:: Powers of Two ::`, `>>> Running Amok >>>`.
3. **Bracket & Chevron Enclosures:** `{[(Endurance)]}`, `(( Time Puzzle ))`, `<< Tunnel Size Puzzle >>`, `> Overused Theme <`.

The parser in `ball/level.c` was expanded with character classification helpers `is_bracket_open()`, `is_bracket_close()`, and `is_wrapper()`:
- Matches `Name:` and `Level:` prefixes.
- Matches symmetric bracket pairs (`(`, `[`, `{`, `<`, `>`).
- Matches identical opening and closing punctuation wrappers (`=`, `~`, `!`, `:`, `>`).
- Extracts inner content with `substr()` while stripping outer delimiters and padding spaces.

Extended test coverage was added to `tests/test_level.c:test_level_title_extended()`.

```text
[21:54:29] Add a boring short comment to the code that this is originating from conventions established by Nevermania and grew because of chmod's levels.
```

```text
[21:55:54] Generate a prompt log.
```

Added origin comment to `ball/level.c` and squashed all modifications cleanly into the two atomic commits on branch `level-title-display`.

---

### Verification & Technical Invariants

- **Verification:**
  - Ran `make -j8 && make test` in the `level-title-display` worktree. All 27 unit test suites passed with exit status 0 (`test: ok`).
  - Unit tests in `tests/test_level.c` verify:
    - Standard `=Title=` parsing (`test_level_title_standard`).
    - Title-only message parsing without description (`test_level_title_only`).
    - Newline delimiter variations `\\`, `\n`, `\r\n` (`test_level_title_newline`, `test_level_desc_newline`).
    - Untitled message pass-through (`test_level_title_untitled`, `test_level_desc_untitled`).
    - Malformed and unclosed delimiter rejection (`test_level_title_malformed`).
    - Extended title formats: `Name:`, `Level:`, multi-equals, `~`, `!`, `:`, brackets `{[(...)]}`, and chevrons `<<...>>` (`test_level_title_extended`).
    - Empty and null level pointer safety (`test_level_desc_empty_or_null`).
- **Technical Invariants:**
  - **Stack Allocation & Immutability:** `level_title()` returns a 256-byte `STRBUF` value. `level_desc()` returns a direct `const char *` pointer into `level_msg()` without heap allocation.
  - **First-Line Scoping:** Title extraction is confined strictly to the first line of `level->message` delimited by `\r`, `\n`, or `\\`.
  - **Delimiter Symmetry:** Opening punctuation and bracket characters must match their corresponding closing characters. Unbalanced wrappers fall back to treating the entire text as a standard description.
  - **Digit-Based Sequencing:** Level sequence numbers receive a `'#'` prefix if and only if the level name begins with an ASCII digit `'0'`..`'9'`.
  - **Dynamic Viewport Downscaling:** Titles starting at `GUI_LRG` (or `GUI_MED` for untitled bonus levels) downscale to `GUI_MED` and `GUI_SML` if `gui_measure(lvlattr, font_size).w > video.device_w`.
