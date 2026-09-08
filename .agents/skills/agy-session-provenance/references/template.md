# Architecture Decision Record & Provenance Template (Literate Format)

Use this template when synthesizing a provenance document in `prompts/YYYY_MM_DD_HHMMSS_<slug>.md`.

```markdown
# Architectural Provenance: [Feature / Task Title]

- **Session Date:** `YYYY-MM-DDTHH:MM:SSZ`
- **Agent:** [Harness] [Version] [Verbatim Model String]
- **Base Commit:** [`[BASE_SHORT_HASH]`](https://github.com/Neverball/neverball/commit/[BASE_FULL_HASH]): `[Base commit message]` (YYYY-MM-DD HH:MM:SS UTC)
- **Commits:**
  - [`[SHORT_HASH]`](https://github.com/Neverball/neverball/commit/[FULL_HASH]): `[Commit message]` (YYYY-MM-DD HH:MM:SS UTC)
- **Files Modified:** `path/to/file1.c`, `path/to/file2.h`

*(Or for Multi-Repository Sessions spanning multiple projects:)*
<!--
- **Session Date:** `YYYY-MM-DDTHH:MM:SSZ`
- **Agent:** [Harness] [Version] [Verbatim Model String]
- **Repositories & Commits:**
  - `repo-a`: Base [`[BASE_HASH]`](url): `[message]` (UTC), Commits: [`[HASH]`](url)
  - `repo-b`: Base [`[BASE_HASH]`](url): `[message]` (UTC), Commits: [`[HASH]`](url)
- **Files Modified:** `repo-a/path/to/file1.c`, `repo-b/path/to/file2.php`
-->

---

## Annotated Prompt Log

[Weave the narrative strictly in ascending chronological order. Let prompts speak for themselves without preceding introductory summaries. Document strictly observable technical facts: files touched, APIs used, engine mechanics, and harness code modifications strictly AFTER each prompt (or group of diagnostic prompts). NEVER speculate or invent unstated user motivations, backstories, or workflow rationales.]

### Initial Problem & Hypothesis

```text
[HH:MM:SS] [VERBATIM INITIAL USER PROMPT / PLAN INITIATION]
```

The initial planning phase investigated [files/components] and generated [plan_artifact.md] to [observable technical implementation]. *(Do not invent unstated user rationales or workflow backstories)*.

### Constraint Discovery & Design Pivots

```text
[HH:MM:SS] [VERBATIM USER STEERING PROMPT]
```

In [file], [function/class] was modified to [action]. This works by [underlying engine/runtime mechanism].

### Verification Loops & Discarded Experiments

```text
[HH:MM:SS] [VERBATIM EXPERIMENTAL / TEST PROMPT]
```

Diagnosis revealed [underlying root cause]. In [file], [clean fix] was applied, and [temporary diagnostic hooks] were reverted.

### Verification & Technical Invariants

```text
[HH:MM:SS] [VERBATIM VERIFICATION / FINAL ALIGNMENT PROMPT]
```

- **Verification:** [Document exact build and test runs, commands executed, exit codes, and test assertions verified].
- **Technical Invariants:** [List genuine subsystem constraints, performance rules, or lifecycle requirements discovered or locked in during the session].
```
