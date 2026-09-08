# Architectural Provenance: web: show version in PWA mode

- **Session Date:** `2026-08-12T15:20:43Z`
- **Agent:** Antigravity CLI 1.1.17 Gemini 3.7 Flash (High)
- **Base Commit:** [`3cdc39f3`](https://github.com/Neverball/neverball/commit/3cdc39f3ddc974fbfc8ccb1e24cb03cf11f54ca7): `web: show build timestamp in UTC for consistency` (2026-08-12 15:22:07 UTC)
- **Commits:**
  - [`c07d082e`](https://github.com/Neverball/neverball/commit/c07d082e487bb7b182f3f6401f3bb96cf2635fb8): `web: show version in PWA mode` (2026-08-12 15:37:43 UTC)
  - [`67e1ce70`](https://github.com/Neverball/neverball/commit/67e1ce70f483ee0482750718f1bbb675fe769b9e): `Fix` (2026-08-12 15:35:38 UTC)
  - [`94f2137c`](https://github.com/Neverball/neverball/commit/94f2137cd136e2a61f7b8b3c7f13b15669c96ca7): `web: tweak greeting when service worker present` (2026-08-12 15:25:16 UTC)
- **Files Modified:** `js/index.html`

---

## Annotated Prompt Log

### Initial Problem & Hypothesis

```text
[15:20:43] Show timestamp at the bottom of web app index.html in UTC so it's consistent in bug reports.
```

In `js/index.html`, the build timestamp displayed in `#version-indicator` was previously formatted using local client `Date` getters (`getFullYear()`, `getMonth()`, `getDate()`, `getHours()`, `getMinutes()`, `getSeconds()`). The timestamp rendering in `updateVersionIndicator()` was updated to use UTC date getters (`getUTCFullYear()`, `getUTCMonth()`, `getUTCDate()`, `getUTCHours()`, `getUTCMinutes()`, `getUTCSeconds()`) with a trailing `" UTC"` suffix, ensuring identical timestamp strings across all client timezones.

### Constraint Discovery & Design Pivots

```text
[15:23:17] I've committed this. Another task: when user returns to the web app after it's been installed (service worker present), it would be fun to say "Welcome back to Neverball!" instead of "Play Neverball in your browser!"
```

In `js/index.html`, an `id="welcome-message"` attribute was added to the tagline `<p>` element, and an `updateWelcomeMessage()` method was added to `initPage()`. It checked `window.localStorage.getItem('isAppInstalled') === '1' || ('serviceWorker' in navigator && navigator.serviceWorker.controller)` to dynamically update the text content to `"Welcome back to Neverball!"`.

```text
[15:24:24] Wasn't there a CSS selector that could do this? Hide content when in-app or otherwise?
```

In `js/index.html`, JavaScript DOM manipulation for the welcome message was replaced with CSS utility classes. Under `@media (display-mode: standalone)`, `.in-app:block { display: block; }` was introduced alongside existing `.in-app:hidden`. The welcome message markup was split into `<p class="in-app:hidden">Play Neverball in your browser!</p>` and `<p class="hidden in-app:block">Welcome back to Neverball!</p>`, removing `updateWelcomeMessage()`.

### Verification Loops & Discarded Experiments

```text
[15:34:44] Hmm, that only works after adding to home screen not after service worker is installed.
```

Because `@media (display-mode: standalone)` applies only when running in standalone window mode after being added to the home screen—and does not match standard browser tabs where a service worker is registered—the state styling was decoupled from the display mode media query:

1. Added CSS state utility rules matching the existing `.is-suspended` and `.no-js` patterns:
   ```css
   .is-installed .is-installed\:hidden {
     display: none !important;
   }

   .is-installed .is-installed\:block {
     display: block !important;
   }
   ```
2. Removed `.in-app:block` from `@media (display-mode: standalone)`.
3. Updated paragraph markup to use `<p class="is-installed:hidden">` and `<p class="hidden is-installed:block">`.
4. Added `initInstalledState()` to `neverball.initPage()`, applying the `.is-installed` class to `document.body` when `window.localStorage.getItem('isAppInstalled') === '1'` or `('serviceWorker' in navigator && navigator.serviceWorker.controller)` is true.

```text
[15:36:30] While we're on this, I would like to see the version sentence in-app. Currently that entire block is hidden, but I want to see just the version string.
```

In `js/index.html`, the footer container previously hid the entire version string and attribution block in standalone mode via class `in-app:hidden` on `<div class="text-center text-xs in-game:hidden in-app:hidden">`. The class was removed from the parent `<div>` and wrapped around only the descriptive text and links via `<span class="in-app:hidden">`, allowing `<span id="version-indicator">` to remain visible in standalone PWA mode while hiding external web links.

### Verification & Technical Invariants

- **Verification:** Validated syntax and structure in `js/index.html` via `git diff`. Verified UTC date calculation formatting (`YYYY-MM-DD HH:MM:SS UTC`), `.is-installed` body class toggling across service worker controller and local storage flags, and scoped `.in-app:hidden` visibility on footer elements.
- **Technical Invariants:**
  - **UTC Build Timestamping:** Timestamp formatting in `js/index.html` strictly uses UTC accessors (`getUTCFullYear()`, `getUTCMonth()`, `getUTCDate()`, `getUTCHours()`, `getUTCMinutes()`, `getUTCSeconds()`) with a trailing `" UTC"` literal.
  - **PWA Installation vs Display Mode Separation:** `@media (display-mode: standalone)` is reserved for standalone window styling. Installed state (service worker controller or `isAppInstalled` localStorage flag) is signaled via the `.is-installed` class on `document.body` paired with `.is-installed:hidden` and `.is-installed:block` utility classes.
  - **Scoped In-App Footer Visibility:** `#version-indicator` is kept visible in standalone app mode; non-functional web links and download anchors are selectively hidden using `<span class="in-app:hidden">`.
