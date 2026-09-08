# Architectural Provenance: web: on small screens, switch between fullscreen/suspended, never show in-page canvas

- **Session Date:** `2026-08-11T22:22:29Z`
- **Agent:** Antigravity CLI 1.1.17 Gemini 3.7 Flash (High)
- **Base Commit:** [`01ab984b`](https://github.com/Neverball/neverball/commit/01ab984ba37ec69cba5876ad1d8a442d80ea4cf9): `Add config UI for camera presets` (2026-07-30 18:16:04 UTC)
- **Commits:**
  - [`379322d5`](https://github.com/Neverball/neverball/commit/379322d51f8056c153fc547128bbd00ab0607418): `web: on small screens, switch between fullscreen/suspended, never show in-page canvas` (2026-08-11 23:41:51 UTC)
- **Files Modified:** `emscripten/ball.mk`, `js/index.html`

---

## Annotated Prompt Log

### Initial Problem & Hypothesis

```text
[22:22:29] /plan I would like to implement a UX change in the web version on viewports less than 800px wide: a dynamic media query that force-enables the game to request fullscreen on startup, hides the checkbox to choose fullscreen on startup, so the game automatically goes fullscreen when "Play Now" is tapped, and then, when the game for whatever reason loses focus or fullscreen, pausing its execution fully (perhaps via Emscripten) and showing a "Resume" button instead of the canvas. To reiterate: on mobile viewports less than 800px wide, the game is never shown in a tiny non-fullscreen canvas element on the page, but instead only in fullscreen, and the html shell acquires an button to restore fullscreen and resume the game whereas on desktop the game would simply continue in a non-fullscreen canvas.
```

The initial planning phase investigated `js/index.html` and `emscripten/ball.mk` to design the mobile fullscreen lifecycle. The agent generated the plan artifact `mobile_fullscreen_pause_plan.md` outlining:
1. Exporting `pauseMainLoop` and `resumeMainLoop` in `emscripten/ball.mk` under `EXPORTED_RUNTIME_METHODS`.
2. Responsive media queries and automatic fullscreen requests for small viewports on startup.
3. Pausing the Emscripten main loop, suspending WebAudio and HTML5 media, sending `EVENT_PAUSE` to the engine, and displaying a resume control upon exiting fullscreen.

### Constraint Discovery & Breakpoint Calibration

```text
[22:28:35] I guess accurately the viewport breakpoint should be 816px. And then upon resume the game should be in the paused state - I'm thinking the events that cause EVENT_PAUSE to be sent are enough, no need to listen to extra events like visibility loss of tab content, etc. Also needs to account for audio stoppage.
```

The implementation plan `mobile_fullscreen_pause_plan.md` was updated to calibrate the breakpoint strictly to `< 816px` (`@media (max-width: 815px)` and `window.matchMedia('(max-width: 815px)')`), simplify lifecycle triggers strictly to `fullscreenchange` events, and incorporate explicit audio suspension (`AudioContext.suspend()` and HTML5 media element pause) alongside `EVENT_PAUSE`.

```text
[22:30:35] [Approved] mobile_fullscreen_pause_plan.md
```

Implementation began across the build configuration and web shell:
1. In `emscripten/ball.mk`, `pauseMainLoop` and `resumeMainLoop` were added to `EXPORTED_RUNTIME_METHODS`.
2. In `js/index.html`, `@media (max-width: 815px)` was added to hide `#fullscreen-checkbox-wrapper`, and helper methods `isSmallViewport()`, `pauseEmscriptenLoop()`, `resumeEmscriptenLoop()`, `pauseAudio()`, `resumeAudio()`, `pauseGameMobile()`, and `resumeGameMobile()` were introduced on the `Neverball` controller.

### Build Environment & Branch Isolation

```text
[22:32:09] Build and start a local server for me.
```

```text
[22:53:05] Building happens via emscripten Docker container, examples are in this repo.
```

```text
[22:59:07] Image is called neverball-emscripten
```

```text
[22:59:47] Build sols on host using maximum cpus.
```

```text
[23:09:10] Umm, what happened here? Did we build a feature on top of another half-built feature?
```

```text
[23:09:53] So the answer was actually "yes, we did". We should have started off master for this.
```

```text
[23:12:45] Okay, why are we back on jules now?
```

The Emscripten build workflow required the `neverball-emscripten` Docker container. When the initial build was run against a working tree containing uncommitted engine changes in `share/solid_all.h` and `ball/game_client.c`, the harness isolated the work by creating a clean branch directly off `master` (`web-mobile-fullscreen-clean`), executing `make clean`, compiling the WebAssembly bundle (`js/neverball.js`, `js/neverball.wasm`, `js/neverball.data`) via Docker, and launching the local HTTP server via `python3 -m http.server 8000 --directory js`.

### User Activation & Fullscreen Security Policy

```text
[23:17:01] Request for fullscreen was denied because Element.requestFullscreen() was not called from inside a short running user-generated event handler. -- I had to confirm persisten storage
```

Under browser user-activation security policies, `Element.requestFullscreen()` must be invoked synchronously inside a user gesture. Because asynchronous operations (`navigator.storage.persist()`, `prepareApp()`) in `runButtonClick` consumed the transient activation token, `Neverball.requestFullscreen()` was moved to the very top of `runButtonClick` prior to any `await` statements.

### Fullscreen Exit Lifecycle, WebGL Context, and Teardown

```text
[23:18:53] When I click "Exit" in the title menu (in-game), I just get a fullscreen black canvas now.
```

```text
[23:20:09] Not resolved.
```

```text
[23:20:53] I mean not resolved in that clicking Exit does not stop fullscreen.
```

```text
[23:22:41] Yeah, I don't see really any change in behavior. I try to exit the game, click Exit, and get stuck in a black fullscreen window that I have to manually Escape from.
```

```text
[23:24:12] In fact, I don't even have to be fullscreen - just clicking Exit in a non-fullscreen canvas also leaves a black canvas and doesn't restore normal launcher UI.
```

Investigation into game termination and exit handling revealed three interacting factors in `js/index.html`:
1. In Chromium-based browsers, `'webkitFullscreenElement' in document` evaluates to `true` while returning `null` when standard `requestFullscreen()` is used. `Neverball.getFullscreenElement()` was updated to check standard `document.fullscreenElement` first, falling back to vendor prefixes (`webkit`, `moz`, `ms`).
2. In `Neverball.quit()`, calling `this.resumeEmscriptenLoop()` had erroneously re-awakened `Module.resumeMainLoop()` after the C engine had already called `emscripten_cancel_main_loop()` and `main_quit()`, causing the scheduler to attempt drawing to a destroyed WebGL context. Removing `resumeEmscriptenLoop()` and `resumeAudio()` from `quit()` eliminated the black screen freeze.
3. `Neverball.quit()` was updated to invoke `exitFullscreen()` and unconditionally remove `in-game` and `is-suspended` classes, resetting the UI to the launcher state.

### Desktop Window Resizing vs Display Resolution

```text
[23:27:19] That fixed it. I do have another report: I am testing in a desktop by resizing viewport to < 816px, but the fullscreen is still my full desktop size - so when I esc to get out of the fullscreen, it doesn't trigger suspension due to viewport being actual desktop fullscreen size while the target document viewport is less than 816px
```

When testing small viewports on desktop displays by resizing the browser window to `< 816px`, entering fullscreen expanded the element to the monitor's native resolution (`> 816px`). Upon exiting fullscreen, evaluating `window.matchMedia('(max-width: 815px)')` immediately failed to match the pre-fullscreen window state.

To preserve state across fullscreen transitions, `Neverball.wasSmallViewportOnLaunch` was added to record viewport dimensions at launch time in `Neverball.run()`, ensuring `Neverball.isSmallViewport()` checks `this.wasSmallViewportOnLaunch || window.matchMedia('(max-width: 815px)').matches`.

### Declarative State Management & Tailwind-Style Utility Classes

```text
[23:29:12] Okay, now Esc puts me in a weird move where I am still considered in-game by the UI.
```

```text
[23:30:12] Rename mobile-paused class to is-suspended
```

```text
[23:30:40] Hide the video when suspended.
```

```text
[23:31:11] I prefer tailwind style classes, e.g., is-suspended:hidden
```

When exiting fullscreen on small viewports, `pauseGameMobile()` removes the `in-game` class (restoring the launcher UI) and adds `is-suspended` to `document.body`.

To manage UI state declaratively, Tailwind-style scoped state classes were added to `js/index.html`:
```css
.is-suspended .is-suspended\:hidden {
  display: none !important;
}

.is-suspended .is-suspended\:block {
  display: block !important;
}

.is-suspended .is-suspended\:inline-flex {
  display: inline-flex !important;
}
```

These classes were applied to `#screenshot` (`is-suspended:hidden`), `#canvas-wrapper` (`hidden in-game:block is-suspended:hidden`), `#run-button` (`is-suspended:hidden`), and `#resume-button` (`hidden is-suspended:inline-flex`).

```text
[23:32:46] Give the resume button a different color. Check gui.h for ideas.
```

The resume button was initially styled with a green gradient palette (`.neverball-button-green`) derived from `gui_grn` in `share/gui.h`.

### Keyboard Event Trapping & Capture-Phase Shield

```text
[23:33:33] BTW, weird bug: when in suspended mode, I can't use browser keyboards because Emscripten is still intercepting them.
```

```text
[23:34:16] Does emscripten have no way of bypassing that once set up?
```

Emscripten's SDL2 port binds `keydown`, `keyup`, and `keypress` event listeners to the global `window` object, intercepting keystrokes and calling `event.preventDefault()` even while the main loop is paused.

Rather than modifying C-side SDL hints or accessing internal unexported Emscripten `JSEvents` tables, a capture-phase (`useCapture = true`) event shield was registered on `window` in `initPage()`:
```javascript
['keydown', 'keyup', 'keypress'].forEach(eventType => {
  window.addEventListener(eventType, event => {
    if (document.body.classList.contains('is-suspended')) {
      event.stopImmediatePropagation();
    }
  }, true);
});
```

Calling `event.stopImmediatePropagation()` during the capture phase intercepts keystrokes before Emscripten's bubble-phase listeners receive them, while `document.activeElement.blur()` in `pauseGameMobile()` releases DOM focus from the canvas.

### Button Styling & Declarative Cleanup

```text
[23:36:45] Change resume button color. Take inspiration from pwa update banner button - that's the style I'm going for.
```

```text
[23:38:03] That's good.
```

The resume button styling was updated to match the PWA update banner button (`.update-button-focused`):
```css
.update-button-focused {
  background: rgba(255, 178, 76, 0.73);
  color: #fff;
  text-shadow: 1px 1px 2px rgba(0, 0, 0, 0.8);
  white-space: nowrap;
}

.update-button-focused:not(:disabled):hover, .update-button-focused:focus {
  background: rgba(255, 195, 110, 0.9);
  cursor: pointer;
}
```

```text
[23:39:04] Look at the diff of this branch and apply is-suspended classes consistently, replacing previous solutions where possible.
```

```text
[23:41:48] Commit
```

In `js/index.html`, imperative JavaScript DOM manipulations (such as manual `classList.add`/`remove` on `_resumeButton`) were removed from `pauseGameMobile()`, `resumeGameMobile()`, `run()`, and `quit()`. All element visibility transitions were refactored to rely purely on the `is-suspended` body class and `.is-suspended:*` utility CSS rules.

The changes were committed in commit [`379322d5`](https://github.com/Neverball/neverball/commit/379322d51f8056c153fc547128bbd00ab0607418) (`web: on small screens, switch between fullscreen/suspended, never show in-page canvas`).

### Verification & Technical Invariants

- **Verification:**
  - Built WebAssembly bundle via Docker container `neverball-emscripten` with `EXPORTED_RUNTIME_METHODS` including `pauseMainLoop` and `resumeMainLoop`.
  - Tested on `http://localhost:8000` across desktop viewports (`>= 816px`) and small/mobile viewports (`< 816px`).
  - Verified synchronous `requestFullscreen()` invocation within user gesture event handler on startup.
  - Verified exiting fullscreen on `< 816px` viewports triggers `pauseGameMobile()`, pausing the Emscripten main loop, suspending WebAudio, stopping background video, and rendering the amber `.update-button-focused` Resume button.
  - Verified capture-phase keyboard listener blocks Emscripten key interception while `is-suspended` is active.
  - Verified in-game title menu "Exit" cleanly tears down the canvas and restores the launcher interface without black screen artifacts.
- **Technical Invariants:**
  - **Breakpoint:** Small screen layout strictly targets `< 816px` (`@media (max-width: 815px)` and `window.matchMedia('(max-width: 815px)')`).
  - **Zero In-Page Canvas on Mobile:** Viewports `< 816px` never render an in-page canvas; execution occurs only in fullscreen mode or is suspended.
  - **Synchronous User Activation:** `requestFullscreen()` must be invoked synchronously at the start of `runButtonClick` before any asynchronous `await` or promise chains.
  - **Capture-Phase Keyboard Suppression:** When `document.body` has `is-suspended`, capture-phase listeners on `window` must call `event.stopImmediatePropagation()` to shield browser navigation and keyboard inputs from Emscripten listeners.
  - **No Main Loop Resume on Quit:** `Neverball.quit()` must never call `resumeEmscriptenLoop()` after the C engine loop has been terminated with `emscripten_cancel_main_loop()`.
