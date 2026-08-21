---
name: agy-session-provenance
description: >-
  Extracts Antigravity CLI conversation transcripts, initial plan artifacts, and git commit
  histories to synthesize architectural decision records (ADRs), provenance documentation,
  and single-turn replay specifications with verbatim user prompts under prompts/YYYY_MM_DD_HHMMSS_<slug>.md.
  Use when asked to document the provenance of a feature/bugfix, distill a coding session,
  audit AI agent work, extract session prompts, or convert git commits and session logs into
  high-fidelity replay contracts.
---

# Session Provenance & Architectural Decision Record Skill

This skill allows Antigravity agents to retroactively inspect historical coding sessions, extract verbatim human prompts and plan artifacts, correlate them with landed git commits (single commits or commit ranges), and generate high-fidelity Architectural Decision Records (ADRs) and Single-Turn Replay Specifications stored in `./prompts/`.

---

## Why This Exists

During AI pair-programming, an initial `/plan` artifact captures initial intent, but frequently loses fidelity as the session evolves through iterative feedback, debugging, temporary experiments, and course corrections. Furthermore, features often land across multiple git commits.

This skill bridges the fidelity gap by combining two concrete anchors:
1. **Landed Git Commits & Diffs:** The hard technical truth of what was committed.
2. **Session Transcripts:** The raw conversational trajectory, verbatim human steering, discovered constraints, and rejected approaches.

---

## Directory & File Locations

- Helper script: [extract_session.py](./scripts/extract_session.py)
- Reference template: [template.md](./references/template.md)
- Reference example: [2026_08_16_212343_mtrl_filter_nearest.md](./examples/2026_08_16_212343_mtrl_filter_nearest.md)
- Output destination: `prompts/YYYY_MM_DD_HHMMSS_<slug>.md` (in the repository root)

---

## Subagent Delegation (Mandatory)

When asked to finalize a session or generate provenance, the primary agent **MUST** spawn an isolated subagent (`Provenance Synthesizer`) using `invoke_subagent` rather than performing synthesis in the main conversation context:

```json
{
  "TypeName": "self",
  "Role": "Provenance Synthesizer",
  "Prompt": "You are tasked with generating a high-fidelity Architectural Decision Record and Provenance document for commit <hash> (or range <base>..<head>) using the agy-session-provenance skill. Follow SKILL.md: generate the scaffold, inspect it with view_file, synthesize the document adhering strictly to Prompt-First rules, perform the Step 4 Quality Audit, and report the generated file path."
}
```

---

## CLI Reference & Flags (`extract_session.py`)

Do NOT run `python3 extract_session.py --help`. All flags and workflows are documented below:

| Flag | Description | Example |
|---|---|---|
| `--scaffold` | Generates starting markdown file in `prompts/` with embedded context comments | `python3 .agents/skills/agy-session-provenance/scripts/extract_session.py --scaffold --commit <hash>` |
| `--commit <hash>` | Targets a single commit hash or branch name | `--commit b38ec9ee` |
| `--range <base>..<head>` | Targets a commit range or feature branch | `--range 7f178e8d..d7879f4c` |
| `--session <cid>` | Explicitly binds to a specific Conversation ID UUID | `--session 247638d8-6fb3-46df-a685-441b1aec2349` |
| `--output <path>` | Sets custom output destination file path | `--output prompts/YYYY_MM_DD_HHMMSS_<slug>.md` |
| `--turn <spec>` | Deep inspection of specific turn(s) (`1`, `1..5`, `4-8`, `1,3,7`, `all`) | `--turn 1..5 --commit <hash>` |
| `--dossier` | Prints or exports full turn-by-turn markdown dossier | `--dossier --commit <hash> --output /tmp/dossier.md` |
| `--list` | Lists recent Antigravity sessions matching the workspace | `--list` |
| `--match` | Finds best matching session for a commit or range | `--match --commit <hash>` |
| `--extract` | Dumps raw structured session JSON model to stdout | `--extract --commit <hash>` |

---

## Step-by-Step Workflow

### Step 1: Identify Target Commits & Session

Determine the feature scope requested by the user:
- **Single commit:** e.g., `b38ec9ee`
- **Commit range / Feature branch:** e.g., `HEAD~3..HEAD` or `origin/master..feature-branch`
- **Specific Conversation ID:** e.g., `247638d8-6fb3-46df-a685-441b1aec2349`
- **Recent session:** If unspecified, list recent sessions to identify the target:
  ```bash
  python3 .agents/skills/agy-session-provenance/scripts/extract_session.py --list
  ```

### Step 2: Generate Scaffold & Inspect Session Dossier

Generate the self-contained markdown scaffold in `prompts/` and inspect session trajectory using the built-in library tools (**DO NOT write ad-hoc Python scripts or bash/sed slicing pipelines**):

```bash
# 1. Generate self-contained markdown scaffold (pre-populated with turn actions & diagnosis comments):
python3 .agents/skills/agy-session-provenance/scripts/extract_session.py --scaffold --commit <commit_hash>

# 2. (Optional) Inspect specific turn or range of turns in depth (e.g. 1..5, 4-8, 1,3,7, or all):
python3 .agents/skills/agy-session-provenance/scripts/extract_session.py --turn 1..5 --commit <commit_hash>

# 3. (Optional) Save or view complete Markdown dossier:
python3 .agents/skills/agy-session-provenance/scripts/extract_session.py --dossier --commit <commit_hash>
python3 .agents/skills/agy-session-provenance/scripts/extract_session.py --dossier --commit <commit_hash> --output /tmp/dossier.md

# 4. (Optional) Extract full structured JSON payload:
python3 .agents/skills/agy-session-provenance/scripts/extract_session.py --extract --commit <commit_hash>
```

### Step 3: Synthesize Literate Provenance & Replay Spec

Open the generated scaffold in `prompts/YYYY_MM_DD_HHMMSS_<slug>.md` using `view_file`. The scaffold contains embedded context comments (`<!-- TURN N CONTEXT ... -->`) detailing the harness actions and model diagnosis for each prompt. Edit and finalize the document following the Prompt-First structure in [references/template.md](./references/template.md):

1. **Header & Metadata:**
   - Session Date (UTC format: `YYYY-MM-DDTHH:MM:SSZ`).
   - Agent harness, version, and verbatim model (`Agent: <harness> <version> <verbatim model>`).
   - Base Commit (parent commit hash, message, and UTC timestamp).
   - List of commit hashes, messages, and UTC dates (`YYYY-MM-DD HH:MM:SS UTC`).
   - List of modified files.
   - (Do NOT include machine-local conversation IDs or local file paths).

2. **Annotated Prompt Log (`## Annotated Prompt Log`):**
   - Present all user prompts in strictly ascending chronological timestamp order with IRC prefixes (`[HH:MM:SS] <text>`).
   - **Prompt-First Narrative (No Pre-Prompt Echoes):**
     - **The Log Begins with Prompt 1:** The Annotated Prompt Log begins immediately with the first verbatim user prompt. Do NOT write an introductory paragraph or baseline summary before the first prompt; there is no conversational context prior to the user's opening prompt. All codebase baseline context and initial architectural exploration belong strictly in the narrative *following* the opening prompt.
     - **Zero Pre-Prompt Echoes:** Do NOT write an introductory sentence before any prompt that summarizes what the prompt is about to say.
     - **Zero Speculative Rationales & Backstories:** Do NOT invent, hallucinate, or reverse-engineer unstated user motivations, workflow justifications, or fictional backstories. If a user asks to "add a flag to neverball to generate a checker image" without explaining why, do NOT invent reasons why they wanted it. Document strictly observable technical facts: files touched, architecture planned, functions added, and real constraints encountered.
     - **Prose strictly follows the prompt:** Place technical prose *after* the prompt (or after a group of closely related diagnostic turns).
     - **What the post-prompt prose documents:**
       1. **Underlying Engine / Runtime Mechanics:** Explain why a bug occurred, how an API works, or what architectural constraints apply (e.g. `EMCCFLAGS` `-s EXIT_RUNTIME=1`, transient user activation tokens, WebAudio lifecycle states, capturing vs bubbling event phases).
       2. **Harness Code Modifications:** Document the exact file touchpoints, functions, and landed utility classes/macros executed in response (grounded in the harness tool calls and git diff).
       3. **Investigation & Failed Hypotheses:** Explain what was diagnosed during iterative debugging and why intermediate approaches were replaced.
     - **Transitions:** Routine build commands or approvals (`"[Approved]"`, `"Build"`, `"Commit"`) can be grouped or followed directly by the single concrete action taken.
     - **Concluding Technical Invariants & Verification:** Conclude the log with a dedicated subsection (`### Verification & Technical Invariants`) capturing exact verification test runs, exit status codes, scanline/data assertions, and genuine architectural constraints discovered or locked in during the session.

### Multi-Repository & Parent Workspace Sessions

When Antigravity runs from a parent folder containing multiple repositories (e.g. `~/Development/` spanning `neverball`, `neverball-website`, `neverball-addon-service`):

1. **Session Discovery:** Session logs are indexed globally by Conversation ID in `~/.gemini/antigravity-cli/brain/`. You can pass `--session <cid>` directly to `extract_session.py` regardless of the current working directory.
2. **Output Placement:**
   - **Parent / Workspace Level:** If a central `prompts/` directory exists in the workspace root, output directly there: `--output /path/to/workspace/prompts/YYYY_MM_DD_HHMMSS_<slug>.md`.
   - **Primary Coordinating Repo:** Alternatively, place the record in the main repository where primary integration logic landed.
3. **Multi-Repo Metadata Header:**
   Group commits by repository rather than assuming a single repository:
   ```markdown
   - **Session Date:** `2026-08-06T20:47:01Z`
   - **Agent:** Antigravity CLI 1.1.15 Gemini 3.7 Flash (High)
   - **Repositories & Commits:**
     - `neverball-website`: Base [`a1b2c3d4`](https://github.com/.../commit/a1b2c3d4): `msg` (UTC), Commits: [`e5f6g7h8`](https://github.com/.../commit/e5f6g7h8)
     - `neverball-addon-service`: Base [`11223344`](https://github.com/.../commit/11223344): `msg` (UTC), Commits: [`55667788`](https://github.com/.../commit/55667788)
   - **Files Modified:** `neverball-website/deploy.sh`, `neverball-addon-service/index.php`
   ```
4. **Subdirectory Path Prefixing:** In the prompt narrative, prefix file paths with their respective repository folder name (`neverball-website/...`, `neverball-addon-service/...`).

### Step 4: Quality & Grounding Audit Checklist

Verify that the generated record fulfills the standard:
- **Zero Pre-Prompt Echo Check:** Verify that prompts are NOT preceded by redundant summary sentences.
- **Zero Speculative Rationale Check:** Verify that the narrative contains NO invented user motivations, backstories, or reverse-engineered workflow justifications that were not stated in the prompts.
- **Git Diff & Tool Call Grounding Check:** Cross-check every identifier, class name, function name, and build target mentioned in the prose against the actual tool calls and landed git diff.
- **Privacy Check:** All timestamps are normalized to UTC (`YYYY-MM-DDTHH:MM:SSZ` or `YYYY-MM-DD HH:MM:SS UTC`). No local conversation IDs or user paths.
- **Strict Chronology Check:** All prompt timestamps appear in ascending chronological order.

### Step 5: Validate Document Integrity

Review the generated file in `prompts/` to ensure:
- Filename matches `prompts/YYYY_MM_DD_HHMMSS_<slug>.md`.
- Verbatim prompts are intact.
- Document is lean, scannable, and free of redundant diff restatements.
