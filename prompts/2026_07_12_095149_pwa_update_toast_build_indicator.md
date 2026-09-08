# Architectural Provenance: pwa: implement responsive two-button update toast and inline build indicator

- **Session Date:** `2026-07-12T09:51:49Z`
- **Agent:** Antigravity CLI 1.1.17 Gemini 3.5 Flash (High)
- **Base Commit:** [`7f178e8d`](https://github.com/Neverball/neverball/commit/7f178e8dbb7d36608cca3761a30a3b6e2ad6e3c7): `build: automate cache name generation and versioning in Makefile` (2026-07-15 07:52:17 UTC)
- **Commits:**
  - [`d7879f4c`](https://github.com/Neverball/neverball/commit/d7879f4c0f7fa856dab9a6af1471408d2b037003): `pwa: implement responsive two-button update toast and inline build indicator` (2026-07-15 07:52:17 UTC)
- **Files Modified:** `js/index.html`

---

## Annotated Prompt Log

### Initial Problem & Hypothesis

```text
[09:51:49] /plan I am a big fan of a PWA that never updates itself but is explicit "new version is available, refresh to update". Currently I have a really bad bug in the web version where I only get a new version with a force refresh and then on refresh that new version goes away, showing the old version again. Goal here would be to cover both of these, while keeping the code changes minimal and no external deps./plan
```

The initial planning phase investigated `js/service-worker.js` and `js/index.html` to diagnose two service worker caching defects:
1. **HTTP Cache Stale Resource Rollback:** During `installWorker()`, `cache.addAll(urls)` fetched assets using default browser HTTP caching rules. When a new service worker installed, cached responses from the browser's HTTP cache populated the Cache Storage, causing assets to revert on reload.
2. **Automatic Lifecycle Takeover:** The service worker called `self.skipWaiting()` immediately inside the `'install'` handler, activating and replacing the controlling worker without user interaction.

The implementation plan (`pwa_update_fix_plan.md`) targeted:
- Fetching pre-cached assets with `{ cache: 'reload' }` to bypass stale HTTP cache entries.
- Removing automatic `self.skipWaiting()` and handling explicit `SKIP_WAITING` messages.
- Adding an update toast banner and service worker lifecycle listeners (`updatefound`, `statechange`, and `controllerchange`) in `js/index.html`.

```text
[09:58:35] [Approved] pwa_update_fix_plan.md
```

In `js/service-worker.js`, `installWorker()` was rewritten to map `urls` with `fetch(url, { cache: 'reload' })` and `cache.put(url, response)`. The automatic `self.skipWaiting()` invocation was removed from the `'install'` listener and added into a `'message'` event handler for `type === 'SKIP_WAITING'`. In `js/index.html`, `.update-toast` styles, toast DOM markup, and registration hooks were added.

### Local Environment & Emscripten Compilation Setup

```text
[09:59:38] Run a local server so I can test.
```

A local Python HTTP server was started on `http://localhost:8080/` serving the `js/` directory.

```text
[10:03:35] Compile Emscripten build via docker
```

```text
[10:04:09] I have started Docker.
```

The `neverball-emscripten` Docker container was built from `.devcontainer/Dockerfile`, and Neverball was compiled using `make -f emscripten/ball.mk GL4ES_DIR=/opt/gl4es neverball`, generating `js/neverball.js`, `js/neverball.wasm`, and `js/neverball.data`.

### Automated Build Versioning & Cache Busting Discovery

```text
[10:31:02] I forgot I always need to bump cacheName!
```

```text
[10:34:02] Explain what I asked.
```

```text
[10:35:42] I feel like the cache name bump is "for when anything in the PWA other than service-worker.js changes" because service worker is already browser-handled. Won't this change just cause a cache to redownload on every single refresh?
```

Browser service worker update detection operates by checking `service-worker.js` for byte-level differences:
1. On a standard page reload without a new build, the browser fetches `service-worker.js`. If byte-identical, no installation executes and cached assets are served immediately.
2. Because `index.html` and game assets are served cache-first, changes to web shell code or game binaries are only detected if `service-worker.js` itself changes byte-for-byte, triggering the browser to install the new version in the background.

```text
[10:36:11] Okay, plan it out and implement.
```

```text
[10:37:15] Looks good, one small comment: suffix should be .in.js, so that it remains a .js file to all tools.
```

```text
[10:37:32] [Approved] cache_name_automation_plan.md
[Approved] pwa_update_fix_plan.md
[Approved] walkthrough.md
```

`js/service-worker.js` was renamed to `js/service-worker.in.js` with `const cacheName = 'neverball-@BUILD_VERSION@'`. In `emscripten/ball.mk`, a build rule was added to generate `js/service-worker.js` via `sed "s/@BUILD_VERSION@/$(VERSION)-$(shell date +%s)/g"` on each build, and `/js/service-worker.js` was added to `.gitignore`.

### Asset Bundling & Release Compilation

```text
[10:40:59] Wish I could test PWAs on localhost, but seems I can't! Can I?
```

Browsers treat `localhost` and `127.0.0.1` as secure origins, enabling full Service Worker and Cache Storage APIs without HTTPS.

```text
[10:44:15] Hmm, asset compilation is very partial. Check github workflow to see how its done.
```

```text
[10:45:32] I mean when I load the localhost version, it is missing title.sol that shows in the title screen, backgrounds - but fonts are not missing.
```

```text
[10:46:23] SOLs can be compiled natively - will be much faster, btw, with make -j8 sols
```

`.devcontainer/Dockerfile` was updated to install `libpng-dev`, `libjpeg-dev`, and `pkg-config`. Map source files were compiled into `.sol` level binaries natively using `make -j8 sols`, and the release build was compiled with `BUILD=release` in Docker, bundling all level assets into `js/neverball.data`.

### Toast Discovery, Testing & Responsive Layout

```text
[07:09:49] In your estimation, it is better to register service worker on page load or on button click, like it does now? Just answer, no edits.
```

Registering on page load allows background pre-caching of game assets before the user clicks "Play Now".

```text
[07:12:21] How do I test the toast now?
```

```text
[07:13:19] Regenerate the service worker.
```

`js/service-worker.js` was regenerated with a new timestamp suffix (`neverball-1.6.0-1783926813`) to test the update toast.

```text
[07:25:03] I like that you styled the toast to match the rest of the UI. Can you make it more responsive - mobile friendly?
```

In `js/index.html`, a `@media (max-width: 576px)` media query was added to `.update-toast`, setting `width: calc(100% - 3.2rem)`, `bottom: 1.6rem`, `padding: 1.2rem 1.6rem`, `flex-direction: column`, and `align-items: stretch`.

```text
[07:25:29] Does this require me to clear cache to see changes?
```

```text
[07:26:17] Rebuild service worker.
```

```text
[07:27:26] Regenerate again.
```

```text
[07:12:48] Summarize what's done in this session.
```

`js/service-worker.js` was rebuilt with timestamp suffixes `1783927584` and `1783927649` to verify responsive behavior under update transitions.

### Inline Badge Exploration & Specificity Debugging

```text
[07:13:57] In your opinion, is a toast better UX than an inline button to update?
```

```text
[07:14:48] Add an inline badge.
```

```text
[07:16:44] [Approved] inline_badge_plan.md
[Approved] cache_name_automation_plan.md
[Approved] pwa_update_fix_plan.md
[Approved] walkthrough.md
```

In `js/index.html`, `.update-badge` styles, `<button id="update-badge">`, and update event handlers were added.

```text
[07:18:28] Hmm, is the python server running?
```

```text
[07:18:52] Firefox can't load it.
```

The Python HTTP server was restarted explicitly bound to IPv4 loopback `127.0.0.1:8080`.

```text
[07:19:37] It shows the badge on first load, which ain't so good!
```

`.update-badge` specified `display: inline-flex;` after `.hidden { display: none; }` in the stylesheet, causing the badge to render on first load due to source order specificity. The rule `.update-badge.hidden { display: none; }` was added to fix initial visibility.

```text
[07:20:19] Recompile now, so I can see the badge.
```

```text
[07:21:48] Eh, I realize I don't need the badge since the toast can't be dismissed.
```

```text
[07:22:33] Recompile. PWA saved index.html, so I couldn't see the change iwth a simple reload.
```

In `js/index.html`, the `.update-badge` CSS rules, HTML button element, and JavaScript handlers were removed, and the service worker was rebuilt with timestamp suffix `1784100155`.

### Visual Styling Iterations & Theme Alignment

```text
[07:24:06] Now tweak the toast so it matches the rest of the UI. The web version shell is based on the in-game UI, so we can't do arbitrary stuff here.
```

```text
[07:25:12] Recompile.
```

```text
[07:25:29] Recompile.
```

In `js/index.html`, `.update-toast` was updated to use `.neverball-box` background styling (`rgba(25, 25, 25, 0.85)` with inset borders), and recompiled with timestamps `1784100314` and `1784100331`.

```text
[07:27:57] Revert the UI changes, neverball boxes don't appear to stack well.
```

```text
[07:28:33] Recompile
```

In `js/index.html`, `.update-toast` was reverted to glassmorphic styling (`rgba(15, 20, 30, 0.85)` with `12px` backdrop blur and `1.2rem` border-radius), and rebuilt with timestamp `1784100515`.

```text
[07:28:56] Change "Update" button text color to white.
```

```text
[07:29:15] Recompile
```

In `js/index.html`, the button text was updated to `color: #fff` with `text-shadow: 1px 1px 2px rgba(0, 0, 0, 0.8)`.

```text
[07:30:05] Yeah, now normalize the button style to neverball-button - just the simple button, with the same hover effect, but keep the background as if its already focused.
```

```text
[07:30:25] recompile
```

In `js/index.html`, the update button classes were normalized to `.neverball-button.update-button-focused`, applying `background: rgba(255, 178, 76, 0.73)` (Neverball focus highlight).

```text
[07:30:53] Don't change bg color on hover, just keep it as is.
```

```text
[07:31:11] recompile
```

In `js/index.html`, `.update-button-focused:hover` background overrides were removed to maintain constant background color while preserving native scale pulse animations.

### PWA Lifecycle & Caching Semantics

```text
[07:33:19] The recompile cycle is so interesting. It's so counterintuitive to me as a web developer - I expect instant change and I literally have to recompile and have the user "download the update" before they can even see it.
```

```text
[07:35:13] Surely devs do want some things to be instantly updated - provided the user is online? How is that handled? Via fetch?
```

Service workers operate as client-side network proxies. The static app shell (HTML, CSS, WASM binary, and level data) is served cache-first for offline capability and fast load times. Dynamic data or live endpoints bypass cache storage via `fetch` with Network-First or Network-Only caching strategies.

### Palette Tuning, Border Removal & Two-Button Toast

```text
[07:36:39] Good to know. I am still a little unhappy with the toast background. It's not a color that Neverball uses. Have a look at something like gui.h to see what colors are favored in the UI.
```

```text
[07:37:39] recompile
```

```text
[07:37:51] recompile
```

`share/gui.h` and `share/gui.c` were inspected for UI color definitions (`gui_gry` = `{ 0x55, 0x55, 0x55, 0xFF }`). In `js/index.html`, `.update-toast` background was set to `rgba(85, 85, 85, 0.95)`.

```text
[07:39:43] I removed the border in devtools and looks better without it - but the grey doesn't look appealing. I'm thinking something bright, happy? You know?
```

```text
[07:40:29] recompile
```

In `js/index.html`, `.update-toast` background was set to `rgba(64, 173, 71, 0.95)` (Neverball progress bar green) with `border: none`.

```text
[07:41:56] Add a second button there "not now". Apply focused style to "Update" button.
```

```text
[07:42:23] recompile
```

In `js/index.html`:
- Added `<button id="dismiss-button" class="neverball-button update-button-secondary">Not Now</button>` with `background: rgba(25, 25, 25, 0.6)`.
- Bound `dismissBtn.onclick = () => { toast.classList.remove('show'); };`.
- Wrapped buttons inside `.update-toast__buttons` with mobile `flex: 1` width distribution.

### Inline Build Indicator & MessageChannel Protocol

```text
[07:44:05] I'm loving this. But I need one more thing: a little text somewhere that indicates which version is currently installed - preferrably date based so players know how old their version is.
```

In `js/service-worker.in.js`, a `'message'` handler was added for `GET_VERSION` returning `event.ports[0].postMessage({ version: cacheName })`.

```text
[07:46:29] I actually think the location of it could be a sentence before "This version of Neverball is built with". "You are running Neverball version ... This version of Neverball is "
```

```text
[07:46:57] No opacity, normal text.
```

```text
[07:47:05] I approve, go.
```

In `js/index.html`, `<span id="version-indicator"></span>` was inserted into the footer paragraph. `Neverball.updateVersionText()` was implemented to send `GET_VERSION` to `navigator.serviceWorker.controller` using a `MessageChannel` and parse the Unix timestamp suffix from `cacheName`.

```text
[07:48:51] "Neverball version" -> "Neverball build", remove parens, and also make the date string internationally comprehensible.
```

In `js/index.html`, `Neverball.updateVersionText()` was updated to format the timestamp into ISO-style `YYYY-MM-DD HH:MM:SS`, rendering `"You are running Neverball build YYYY-MM-DD HH:MM:SS. "` without parentheses or opacity reduction.

### Cache Scope Audit & Commit Finalization

```text
[07:49:58] Is neverball-source.zip cached by the service worker?
```

In `js/service-worker.in.js`, `neverball-source.zip` is not listed in the `urls` pre-cache array, and the `fetch` listener passes uncached URLs directly to the network.

```text
[07:50:44] Summarize what we have done in this session.
```

```text
[07:51:22] Make commits, split when possible, but optional if difficult.
```

Changes were committed in three parts:
- `53d58ded` (`devcontainer: add native image and pkg-config dependencies`)
- `7f178e8d` (`build: automate cache name generation and versioning in Makefile`)
- `d7879f4c` (`pwa: implement responsive two-button update toast and inline build indicator`)

```text
[09:05:24] Log this session
```

Synchronized session prompt logs.

### Verification & Technical Invariants

- **Verification:**
  - Built and tested with `BUILD=release` inside `neverball-emscripten` Docker container.
  - Verified local HTTP server bound to `127.0.0.1:8080` in Chrome and Firefox.
  - Verified that updating `service-worker.js` timestamp triggers `updatefound` and displays `.update-toast`.
  - Tested clicking `#update-button` ("Update & Refresh"): sent `SKIP_WAITING` postMessage, triggered `controllerchange`, and initiated clean page reload with fresh assets.
  - Tested clicking `#dismiss-button` ("Not Now"): removed `.show` class, dismissing toast without reloading.
  - Tested `Neverball.updateVersionText()`: queried active service worker via `MessageChannel`, parsed timestamp, and formatted date string `"You are running Neverball build YYYY-MM-DD HH:MM:SS. "`.
- **Technical Invariants:**
  - **Service Worker Pre-Cache Reload:** `installWorker()` must fetch pre-cached assets using `{ cache: 'reload' }` to prevent stale HTTP cache entries from populating Cache Storage.
  - **Explicit SKIP_WAITING:** Service workers must not automatically invoke `self.skipWaiting()` during `install`; activation is gated behind user confirmation via `postMessage({ type: 'SKIP_WAITING' })`.
  - **Build Timestamp Suffix:** Cache invalidation relies on byte-level changes to `service-worker.js` generated via `sed "s/@BUILD_VERSION@/$(VERSION)-$(shell date +%s)/g"` in `emscripten/ball.mk`.
  - **MessageChannel Query:** The page queries the controlling service worker for `cacheName` using `MessageChannel` (`GET_VERSION`), decoupling client UI from build-time constants.
  - **Responsive Breakpoint:** Mobile screens (`max-width: 576px`) use column flex layout with full-width action buttons for touch targets.
