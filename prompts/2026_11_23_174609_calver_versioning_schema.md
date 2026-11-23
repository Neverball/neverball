# Prompt Log Export 2026-11-23 17:46:09 UTC

- **Generator:** [git-prompt-log](https://github.com/parasti/git-prompt-log)
- **Export command:** `git prompt-log export --commit --slug calver_versioning_schema`
- **Import command:** `git prompt-log import prompts/2026_11_23_174609_calver_versioning_schema.md`

---

- **Session:** `dd213163-5152-45d4-ba66-e4ab5df9beb5`
- **Harness:** Antigravity CLI 1.5.15
- **Model:** Gemini 4 Hydrogen (Medium)

## Commits

- `6c0459ba` scripts/version: Match only neverball-* tags in git describe
- `d3a0bdd9` .github/workflows/release: Validate neverball-YYYY.<increment> tag format
- `20cbca60` doc/alpha: Update example tag to YYYY.<increment> schema

## Steering Prompts

#### [2026-11-23 17:37:07 UTC]

> /plan Move to YYYY.<increment> tagging/versioning schema.

#### [2026-11-23 17:38:51 UTC]

> [tool:ask_question] neverball-YYYY.<increment> (e.g. neverball-2026.1) (Recommended)
> 1-based (2026.1, 2026.2, ...) (Recommended)
> Keep -alpha/-beta/-rc suffixes when needed (e.g. 2026.1-rc.1) (Recommended)
> Leave neverball-version.txt as-is (rely on git describe and release.yml stamping)

#### [2026-11-23 17:42:39 UTC]

> Should we do that?

Commits:
- `6c0459ba` scripts/version: Match only neverball-* tags in git describe
- `d3a0bdd9` .github/workflows/release: Validate neverball-YYYY.<increment> tag format
- `20cbca60` doc/alpha: Update example tag to YYYY.<increment> schema

<!-- git-prompt-log:metadata
{
  "version": 1,
  "exported_at": "2026-11-23 17:46:09 UTC",
  "export_command": "git prompt-log export --commit --slug calver_versioning_schema",
  "import_command": "git prompt-log import prompts/2026_11_23_174609_calver_versioning_schema.md",
  "commits": [
    {
      "hash": "6c0459ba65898de0554483050378911c4428a1b8",
      "subject": "scripts/version: Match only neverball-* tags in git describe",
      "note": "Assistant-Session: dd213163-5152-45d4-ba66-e4ab5df9beb5\nAssistant-Harness: Antigravity CLI 1.5.15\nAssistant-Model: Gemini 4 Hydrogen (Medium)\nAssistant-Recorded: 2026-11-23 17:44:52 UTC\n\nAssistant-Prompts:\n  [2026-11-23 17:42:39 UTC] Should we do that?\n  [2026-11-23 17:38:51 UTC] [tool:ask_question] neverball-YYYY.<increment> (e.g. neverball-2026.1) (Recommended)\n    1-based (2026.1, 2026.2, ...) (Recommended)\n    Keep -alpha/-beta/-rc suffixes when needed (e.g. 2026.1-rc.1) (Recommended)\n    Leave neverball-version.txt as-is (rely on git describe and release.yml stamping)\n  [2026-11-23 17:37:07 UTC] /plan Move to YYYY.<increment> tagging/versioning schema."
    },
    {
      "hash": "d3a0bdd92885300f36dcf53d3f7c6c0460bb0893",
      "subject": ".github/workflows/release: Validate neverball-YYYY.<increment> tag format",
      "note": "Assistant-Session: dd213163-5152-45d4-ba66-e4ab5df9beb5\nAssistant-Harness: Antigravity CLI 1.5.15\nAssistant-Model: Gemini 4 Hydrogen (Medium)\nAssistant-Recorded: 2026-11-23 17:44:53 UTC\n\nAssistant-Prompts:\n  [2026-11-23 17:42:39 UTC] Should we do that?\n  [2026-11-23 17:38:51 UTC] [tool:ask_question] neverball-YYYY.<increment> (e.g. neverball-2026.1) (Recommended)\n    1-based (2026.1, 2026.2, ...) (Recommended)\n    Keep -alpha/-beta/-rc suffixes when needed (e.g. 2026.1-rc.1) (Recommended)\n    Leave neverball-version.txt as-is (rely on git describe and release.yml stamping)\n  [2026-11-23 17:37:07 UTC] /plan Move to YYYY.<increment> tagging/versioning schema."
    },
    {
      "hash": "20cbca60bb5821b5ced2c5b268100cb05a20df0f",
      "subject": "doc/alpha: Update example tag to YYYY.<increment> schema",
      "note": "Assistant-Session: dd213163-5152-45d4-ba66-e4ab5df9beb5\nAssistant-Harness: Antigravity CLI 1.5.15\nAssistant-Model: Gemini 4 Hydrogen (Medium)\nAssistant-Recorded: 2026-11-23 17:44:54 UTC\n\nAssistant-Prompts:\n  [2026-11-23 17:42:39 UTC] Should we do that?\n  [2026-11-23 17:38:51 UTC] [tool:ask_question] neverball-YYYY.<increment> (e.g. neverball-2026.1) (Recommended)\n    1-based (2026.1, 2026.2, ...) (Recommended)\n    Keep -alpha/-beta/-rc suffixes when needed (e.g. 2026.1-rc.1) (Recommended)\n    Leave neverball-version.txt as-is (rely on git describe and release.yml stamping)\n  [2026-11-23 17:37:07 UTC] /plan Move to YYYY.<increment> tagging/versioning schema."
    }
  ]
}
-->
