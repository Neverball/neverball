# Architectural Provenance: share/fetch_curl: Multi-handle thread synchronization and graceful error handling

- **Session Date:** `2026-08-27T07:48:44Z`
- **Agent:** Antigravity CLI 1.1.22 Gemini 3.7 Flash (High)
- **Base Commit:** [`0b95a1f6`](https://github.com/Neverball/neverball/commit/0b95a1f640beb9f1e2fb4cc2512ca96e6a4c2d23): `Merge branch 'alloc-fixes'` (2026-08-26 23:09:04 UTC)
- **Commits:**
  - [`8e1e1684`](https://github.com/Neverball/neverball/commit/8e1e16845e90b54d7aeec90e4ef6c701bea204f2): `share/fetch_curl: Document mutex synchronization and clean up code style` (2026-08-27 08:02:24 UTC)
  - [`2f97ca7d`](https://github.com/Neverball/neverball/commit/2f97ca7d01e822257b07e7d9ec274930ed5941d4): `share/fetch_curl: Handle initialization and multi-add errors gracefully` (2026-08-27 08:00:29 UTC)
  - [`712b1e43`](https://github.com/Neverball/neverball/commit/712b1e4378482375e662354b3240149f0af40d5d): `share/fetch_curl: Fix multi handle race condition between threads` (2026-08-27 07:59:37 UTC)
- **Files Modified:** `share/fetch_curl.c`

---

## Annotated Prompt Log

### Initial Problem & Hypothesis

```text
[07:48:44] /plan I have applied a patch I received from community member uau to fix a race condition in our use of libcurl from two threads. Review it exhaustively, checking against libcurl documentation and Neverball code, to ensure it does address the issue. Commit it unmodified, crediting uau, then apply whatever changes you deem necessary and this change: remove use of abort() which is a user-hostile UX pattern for an optional download feature that bares no effect on the rest of the game's functioning.
```

In `share/fetch_curl.c`, background downloading operates via a worker thread running `fetch_thread_main()` that polls and steps active transfers using libcurl's multi interface (`curl_multi_poll` and `curl_multi_perform`). Concurrently, when the game initiates a download via `fetch_file()`, the main thread adds easy handles to the shared `multi_handle` via `curl_multi_add_handle()`. Because libcurl multi handles are not thread-safe, concurrent access between `curl_multi_poll()` and `curl_multi_add_handle()` produces data races and memory corruption.

Inspection of the proposed patch from Uoti Urpala [uau] showed that mutual exclusion was resolved using a two-mutex synchronization protocol (`fetch_sync_mutex` and `fetch_curl_mutex`). An initial review and implementation plan was prepared covering:
1. Verification of the two-mutex handshake against libcurl concurrency semantics.
2. Committing the synchronization patch unmodified with contributor attribution.
3. Replacing `abort()` calls across `fetch_init()` and `fetch_file()` with graceful error handling and resource cleanup.
4. Documenting the mutex roles and aligning brace formatting with project conventions.

### Constraint Discovery & Design Discussion

```text
[07:51:14] I will prompt you to create the provenance record, remove all mention of it from the plan.
```

The implementation plan was updated to remove references to the provenance generation step until explicitly requested.

```text
[07:54:19] Before we proceed, it's breaking my brain to lock a mutex to lock another mutex. Can you think of an simpler pattern (if possible)?
```

Analysis of the synchronization architecture compared the two-mutex handshake against alternative concurrency models:
- **Two-Mutex Interlocking Handshake:** When the worker thread polls, holding a single lock across `curl_multi_poll()` would block the main thread for up to the full poll timeout (33ms). Releasing the lock during polling creates a race where `curl_multi_wakeup()` wakes the worker, but the worker may finish its iteration and re-enter `curl_multi_poll()` before the main thread can acquire the mutex and add its handle. The patch resolves this by using `fetch_sync_mutex` as an entry gate: the worker acquires and immediately releases `fetch_sync_mutex` while locking `fetch_curl_mutex` for the duration of the poll/step cycle. When the main thread calls `fetch_lock_mutex()`, it locks `fetch_sync_mutex`, wakes the multi handle via `curl_multi_wakeup()`, and locks `fetch_curl_mutex`. Holding `fetch_sync_mutex` on the main thread prevents the worker thread from starting another poll iteration until the main thread finishes adding its transfer.
- **Single-Mutex Request Queue Alternative:** A producer-consumer model where `fetch_file()` pushes a transfer descriptor onto a thread-safe pending queue protected by a single mutex and pings `curl_multi_wakeup()`. The worker thread drains the queue and exclusively owns all `curl_multi_*` operations, eliminating concurrent access to `multi_handle` entirely.

```text
[07:59:28] Let's stick to uau's solution, but it bares a boring comment for each mutex and the places where they are used together.
```

Implementation proceeded across three atomic commits in `share/fetch_curl.c`:

1. **Commit 1 (`712b1e43`):** `share/fetch_curl: Fix multi handle race condition between threads`
   - Committed `uau`'s patch unmodified, introducing `fetch_sync_mutex` and `fetch_curl_mutex` to coordinate `multi_handle` between the main thread and the worker thread, with contributor attribution to Uoti Urpala [uau].

2. **Commit 2 (`2f97ca7d`):** `share/fetch_curl: Handle initialization and multi-add errors gracefully`
   - Removed all `abort()` calls from `share/fetch_curl.c`.
   - In `fetch_init()`, handled `curl_multi_init()` failure by calling `curl_global_cleanup()`, setting `fetch_enabled = 0`, and returning cleanly.
   - In `fetch_thread_init()`, changed return type from `void` to `int` and added error handling for `SDL_CreateMutex()` and `SDL_CreateThread()`, freeing partially allocated mutexes and resetting `fetch_thread_running` on failure.
   - In `fetch_file()`, checked the return value of `curl_multi_add_handle()`. On failure (`res != CURLM_OK`), logged the error via `curl_multi_strerror(res)`, freed the allocated transfer via `unlink_and_free_fetch_info(fi)`, and returned `0`.

3. **Commit 3 (`8e1e1684`):** `share/fetch_curl: Document mutex synchronization and clean up code style`
   - Added explanatory comments detailing the individual roles of `fetch_curl_mutex` and `fetch_sync_mutex`, as well as their lock ordering in `fetch_thread_main()`, `fetch_lock_mutex()`, and `fetch_unlock_mutex()`.
   - Standardized control statement braces to the Allman convention.
   - Added `curl_multi_wakeup(multi_handle)` with a `NULL` guard in `fetch_thread_quit()` to trigger immediate termination of `curl_multi_poll()` during shutdown.

### Provenance Finalization

```text
[08:07:03] Generate provenance
```

Delegated the generation of the Architectural Decision Record and Provenance specification to the `Provenance Synthesizer` subagent.

### Verification & Technical Invariants

- **Verification:**
  - Build verified clean via `make -j4`.
  - Test suite executed via `make test` with all suites reporting `ok` (`test_common`, `test_dir`, `test_fs`, `test_lang`, `test_map`, `test_mtrl`, `test_set`, `test_sol`, `test_strbuf`).
- **Technical Invariants:**
  - **Multi-Handle Concurrency:** libcurl `CURLM *` handles are not thread-safe. Access to `multi_handle` across threads must be serialized using `fetch_lock_mutex()` and `fetch_unlock_mutex()`.
  - **Lock Ordering & Handshake Protocol:** The main thread must acquire `fetch_sync_mutex`, wake `multi_handle` via `curl_multi_wakeup()`, and acquire `fetch_curl_mutex` before performing multi handle modifications. Mutexes must be unlocked in reverse order (`fetch_curl_mutex` then `fetch_sync_mutex`). The worker thread must pass through `fetch_sync_mutex` before acquiring `fetch_curl_mutex` to poll and step.
  - **Graceful Error Handling:** Subsystem failures (libcurl initialization, thread/mutex allocation, or transfer addition) must not crash the game via `abort()`. Failures must clean up allocated resources, log diagnostic messages, disable the subsystem (`fetch_enabled = 0`), and return failure status `0`.
