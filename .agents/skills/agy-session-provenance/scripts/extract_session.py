#!/usr/bin/env python3
"""
Antigravity Session Provenance Extractor & Dossier Library
Discovers local session transcripts, extracts turn-by-turn tool execution payloads,
correlates them with git commits/ranges, and synthesizes high-fidelity architectural provenance records.
"""

import argparse
import datetime
import json
import os
import re
import subprocess
import sys
from pathlib import Path
from typing import Dict, List, Optional, Tuple


def get_repo_root() -> Path:
    """Returns the root directory of the current git repository."""
    try:
        out = subprocess.check_output(
            ["git", "rev-parse", "--show-toplevel"], text=True, stderr=subprocess.DEVNULL
        ).strip()
        return Path(out).resolve()
    except subprocess.CalledProcessError:
        return Path.cwd().resolve()


def get_brain_dir() -> Path:
    """Locates the Antigravity / Gemini CLI brain directory."""
    candidates = [
        os.environ.get("ANTIGRAVITY_DATA_DIR", ""),
        os.environ.get("GEMINI_CLI_HOME", ""),
        os.path.expanduser("~/.gemini/antigravity-cli"),
        os.path.expanduser("~/.config/antigravity-cli"),
    ]
    for candidate in candidates:
        if candidate:
            b_path = Path(candidate) / "brain"
            if b_path.exists() and b_path.is_dir():
                return b_path
            if Path(candidate).name == "brain" and Path(candidate).is_dir():
                return Path(candidate)

    # Default fallback
    return Path(os.path.expanduser("~/.gemini/antigravity-cli/brain"))


def clean_user_prompt(content: str) -> str:
    """
    Extracts verbatim user prompt text, stripping only protocol envelope
    tags (<USER_REQUEST>, <ADDITIONAL_METADATA>, etc.) while preserving
    the user's exact words, casing, punctuation, and whitespace.
    """
    if not content:
        return ""

    req_match = re.search(r"<USER_REQUEST>\s*(.*?)\s*</USER_REQUEST>", content, re.DOTALL)
    if req_match:
        text = req_match.group(1)
    else:
        text = re.split(r"<ADDITIONAL_METADATA>", content, flags=re.DOTALL)[0]

    return text.strip()


def format_utc(dt_str: str) -> str:
    """Converts any ISO timestamp string to UTC formatted string (YYYY-MM-DD HH:MM:SS UTC)."""
    if not dt_str:
        return "Unknown UTC"
    try:
        dt = datetime.datetime.fromisoformat(dt_str.replace("Z", "+00:00")).astimezone(datetime.timezone.utc)
        return dt.strftime("%Y-%m-%d %H:%M:%S UTC")
    except Exception:
        return dt_str


def format_irc_time(dt_str: str) -> str:
    """Converts any ISO timestamp string to IRC formatted time (HH:MM:SS)."""
    if not dt_str:
        return "00:00:00"
    try:
        dt = datetime.datetime.fromisoformat(dt_str.replace("Z", "+00:00")).astimezone(datetime.timezone.utc)
        return dt.strftime("%H:%M:%S")
    except Exception:
        return dt_str


def parse_transcript(transcript_path: Path) -> Optional[Dict]:
    """
    Parses transcript.jsonl / transcript_full.jsonl into a rich structured session model
    with turn-by-turn tool execution payloads, model diagnosis summaries, and plan artifacts.
    """
    if not transcript_path.exists():
        return None

    # Prefer transcript_full.jsonl if present in the same directory for untruncated model thoughts
    full_transcript = transcript_path.parent / "transcript_full.jsonl"
    target_path = full_transcript if full_transcript.exists() else transcript_path

    steps = []
    with open(target_path, "r", encoding="utf-8", errors="replace") as f:
        for line in f:
            line = line.strip()
            if line:
                try:
                    steps.append(json.loads(line))
                except json.JSONDecodeError:
                    continue

    if not steps:
        return None

    user_step_indices = []
    for idx, s in enumerate(steps):
        if s.get("type") == "USER_INPUT" or s.get("source") == "USER_EXPLICIT":
            user_step_indices.append(idx)

    turns = []
    files_touched_global = set()
    plan_artifacts_dict = {}
    detected_model = None

    for turn_idx, start_idx in enumerate(user_step_indices, 1):
        end_idx = user_step_indices[turn_idx] if turn_idx < len(user_step_indices) else len(steps)
        prompt_step = steps[start_idx]
        created_at = prompt_step.get("created_at") or ""
        raw_content = prompt_step.get("content", "")
        cleaned_prompt = clean_user_prompt(raw_content)

        turn_tool_actions = []
        turn_files_modified = set()
        turn_model_texts = []

        for s_idx in range(start_idx + 1, end_idx):
            step = steps[s_idx]
            content = step.get("content", "")

            # Check model settings change
            if content and "Model Selection" in content:
                m = re.search(
                    r"Model Selection[`\x27]\s*from\s+.*?\s+to\s+([A-Za-z0-9\.\-_ \(\)]+?)(?:\.\s*No need|\.\n|\.$|<)",
                    content,
                )
                if m:
                    detected_model = m.group(1).strip()

            if step.get("type") == "PLANNER_RESPONSE":
                tcalls = step.get("tool_calls", [])
                if not tcalls and content:
                    clean_content = content.strip()
                    # Strip pre-flight checks from model narrative text
                    clean_content = re.sub(r"`\[PRE-FLIGHT CHECK\].*?`\[END CHECK\]\s*", "", clean_content, flags=re.DOTALL)
                    if clean_content:
                        turn_model_texts.append(clean_content)

                for tc in tcalls:
                    fname = tc.get("name", "")
                    args = tc.get("args") or tc.get("arguments") or {}

                    target_file = (
                        args.get("TargetFile")
                        or args.get("AbsolutePath")
                        or args.get("SearchPath")
                        or args.get("DirectoryPath")
                        or ""
                    )
                    if target_file:
                        bname = os.path.basename(target_file.strip('\"\''))
                        files_touched_global.add(bname)

                    desc = (
                        args.get("Instruction")
                        or args.get("Description")
                        or args.get("toolSummary")
                        or args.get("toolAction")
                        or args.get("CommandLine")
                        or ""
                    )

                    is_mod = fname in ("write_to_file", "replace_file_content", "multi_replace_file_content", "run_command")
                    if is_mod and target_file:
                        bname = os.path.basename(target_file.strip('\"\''))
                        turn_files_modified.add(bname)

                    turn_tool_actions.append({
                        "name": fname,
                        "target": os.path.basename(target_file.strip('\"\'')) if target_file else "",
                        "target_full": target_file,
                        "description": str(desc).strip('\"\'') if desc else "",
                        "is_modification": is_mod,
                        "raw_args": args,
                    })

                    # Check for plan/walkthrough artifact creation
                    if ("plan" in target_file.lower() or "walkthrough" in target_file.lower()) and fname == "write_to_file":
                        code_content = args.get("CodeContent", "")
                        if code_content:
                            bname = os.path.basename(target_file).strip('\"\'')
                            plan_artifacts_dict[bname] = {
                                "path": target_file,
                                "name": bname,
                                "content": code_content,
                            }

        turns.append({
            "index": turn_idx,
            "start_step": start_idx,
            "end_step": end_idx,
            "created_at": created_at,
            "irc_time": format_irc_time(created_at),
            "prompt": cleaned_prompt,
            "tool_actions": turn_tool_actions,
            "files_modified": list(turn_files_modified),
            "model_diagnosis": "\n\n".join(turn_model_texts).strip(),
        })

    # Also inspect brain directory for standalone .md artifacts
    cid_dir = transcript_path.parent.parent.parent
    if cid_dir.exists():
        for md_file in cid_dir.glob("*.md"):
            if not md_file.name.endswith(".metadata.json"):
                try:
                    content = md_file.read_text(encoding="utf-8", errors="replace")
                    bname = md_file.name.strip('\"\'')
                    plan_artifacts_dict[bname] = {
                        "path": str(md_file),
                        "name": bname,
                        "content": content,
                    }
                except Exception:
                    pass

    first_time = steps[0].get("created_at") if steps else ""
    last_time = steps[-1].get("created_at") if steps else ""

    return {
        "conversation_id": cid_dir.name,
        "start_time": first_time,
        "end_time": last_time,
        "steps_count": len(steps),
        "user_prompts": [{"step_index": t["start_step"], "created_at": t["created_at"], "prompt": t["prompt"]} for t in turns],
        "turns": turns,
        "files_touched": sorted(list(files_touched_global)),
        "plan_artifacts": list(plan_artifacts_dict.values()),
        "detected_model": detected_model,
    }


def find_all_sessions(brain_dir: Path, repo_root: Path) -> List[Dict]:
    """Discovers all local sessions related to the target repository."""
    sessions = []
    if not brain_dir.exists():
        return sessions

    repo_str = str(repo_root).lower()
    repo_name = repo_root.name.lower()

    for cid_dir in brain_dir.iterdir():
        if not cid_dir.is_dir() or cid_dir.name.startswith("."):
            continue
        transcript_path = cid_dir / ".system_generated" / "logs" / "transcript.jsonl"
        if not transcript_path.exists():
            continue

        try:
            with open(transcript_path, "r", encoding="utf-8", errors="replace") as f:
                header_chunk = "".join([f.readline() for _ in range(10)])

            if repo_str in header_chunk.lower() or repo_name in header_chunk.lower():
                parsed = parse_transcript(transcript_path)
                if parsed and parsed.get("turns"):
                    sessions.append(parsed)
        except Exception:
            continue

    sessions.sort(key=lambda x: x["start_time"] or "1970-01-01T00:00:00Z", reverse=True)
    return sessions


def parse_commit_spec(commit_spec: str) -> Optional[Dict]:
    """
    Parses a commit spec (single hash, range like base..head, or branch name)
    and returns list of commit hashes, timestamps, messages, files touched, base commit, and diff.
    """
    try:
        is_range = ".." in commit_spec
        base_commit = None

        if is_range:
            base_ref = commit_spec.split("..")[0]
            if not base_ref:
                base_ref = "HEAD~1"
            base_out = subprocess.check_output(
                ["git", "show", "-s", "--format=%H|%cI|%s", base_ref], text=True
            ).strip()
            if base_out:
                parts = base_out.split("|", 2)
                base_commit = {
                    "hash": parts[0],
                    "short_hash": parts[0][:8],
                    "date": format_utc(parts[1]),
                    "message": parts[2],
                }

            log_out = subprocess.check_output(
                ["git", "log", "--format=%H|%cI|%s", commit_spec], text=True
            ).strip()
            diff_out = subprocess.check_output(
                ["git", "diff", commit_spec], text=True
            ).strip()
            files_out = subprocess.check_output(
                ["git", "diff", "--name-only", commit_spec], text=True
            ).strip()
        else:
            try:
                base_out = subprocess.check_output(
                    ["git", "show", "-s", "--format=%H|%cI|%s", f"{commit_spec}~1"],
                    text=True,
                    stderr=subprocess.DEVNULL,
                ).strip()
                if base_out:
                    parts = base_out.split("|", 2)
                    base_commit = {
                        "hash": parts[0],
                        "short_hash": parts[0][:8],
                        "date": format_utc(parts[1]),
                        "message": parts[2],
                    }
            except subprocess.CalledProcessError:
                base_commit = None

            log_out = subprocess.check_output(
                ["git", "show", "-s", "--format=%H|%cI|%s", commit_spec], text=True
            ).strip()
            diff_out = subprocess.check_output(
                ["git", "show", "--format=", commit_spec], text=True
            ).strip()
            files_out = subprocess.check_output(
                ["git", "diff-tree", "--no-commit-id", "--name-only", "-r", commit_spec],
                text=True,
            ).strip()

        commits = []
        if log_out:
            for line in log_out.split("\n"):
                parts = line.split("|", 2)
                if len(parts) == 3:
                    commits.append({
                        "hash": parts[0],
                        "short_hash": parts[0][:8],
                        "date": format_utc(parts[1]),
                        "message": parts[2],
                    })

        files = [f.strip() for f in files_out.split("\n") if f.strip()]

        return {
            "spec": commit_spec,
            "is_range": is_range,
            "base_commit": base_commit,
            "commits": commits,
            "files": files,
            "diff": diff_out,
        }
    except subprocess.CalledProcessError as e:
        sys.stderr.write(f"Error reading git spec '{commit_spec}': {e}\n")
        return None


def score_session_match(session: Dict, commit_info: Dict) -> Tuple[int, List[str]]:
    """Scores how closely an Antigravity session corresponds to a commit or commit range."""
    score = 0
    reasons = []

    if not commit_info or not commit_info.get("commits"):
        return 0, reasons

    commit_dates = []
    for c in commit_info["commits"]:
        try:
            dt = datetime.datetime.strptime(c["date"], "%Y-%m-%d %H:%M:%S UTC").replace(tzinfo=datetime.timezone.utc)
            commit_dates.append(dt)
        except Exception:
            pass

    if commit_dates and session.get("end_time"):
        try:
            s_end = datetime.datetime.fromisoformat(session["end_time"].replace("Z", "+00:00"))
            min_diff_sec = min(abs((cd - s_end).total_seconds()) for cd in commit_dates)
            if min_diff_sec < 7200:
                score += 50
                reasons.append(f"Time match: within {min_diff_sec/60:.1f}m")
            elif min_diff_sec < 86400:
                score += 20
                reasons.append(f"Time match: within {min_diff_sec/3600:.1f}h")
        except Exception:
            pass

    commit_files = set(os.path.basename(f) for f in commit_info.get("files", []))
    session_files = set(session.get("files_touched", []))
    matched_files = commit_files.intersection(session_files)
    if matched_files:
        score += len(matched_files) * 20
        reasons.append(f"Files matched: {', '.join(sorted(matched_files))}")

    all_prompt_text = " ".join(t["prompt"] for t in session.get("turns", []))
    for c in commit_info["commits"]:
        words = re.findall(r"\b\w{4,}\b", c["message"].lower())
        matched_words = [w for w in words if w in all_prompt_text.lower()]
        if matched_words:
            score += len(matched_words) * 5
            reasons.append(f"Keywords in prompts: {', '.join(set(matched_words))}")

    return score, reasons


def generate_dossier(session: Dict, commit_info: Optional[Dict] = None) -> str:
    """
    Renders a comprehensive, turn-by-turn Markdown dossier of the session,
    providing complete visibility into prompts, harness edits, and model diagnoses.
    """
    lines = []
    lines.append(f"# Session Dossier: `{session.get('conversation_id', 'unknown')}`\n")
    lines.append(f"- **Start Time:** `{session.get('start_time', '')}`")
    lines.append(f"- **End Time:** `{session.get('end_time', '')}`")
    lines.append(f"- **Detected Model:** `{session.get('detected_model') or 'Gemini 3.6 Flash (High)'}`")
    lines.append(f"- **Total Turns:** {len(session.get('turns', []))}")
    lines.append(f"- **Files Touched:** {', '.join(f'`{f}`' for f in session.get('files_touched', [])) or '_None_'}\n")

    if commit_info:
        lines.append("## Correlated Git Commits\n")
        if commit_info.get("base_commit"):
            bc = commit_info["base_commit"]
            lines.append(f"- **Base Commit:** `{bc['short_hash']}`: {bc['message']} ({bc['date']})")
        for c in commit_info.get("commits", []):
            lines.append(f"- **Commit:** `{c['short_hash']}`: {c['message']} ({c['date']})")
        lines.append(f"- **Commit Files:** {', '.join(f'`{f}`' for f in commit_info.get('files', []))}\n")

    if session.get("plan_artifacts"):
        lines.append("## Plan & Architecture Artifacts\n")
        for pa in session["plan_artifacts"]:
            lines.append(f"### Artifact: `{pa['name']}`\n")
            lines.append("```markdown")
            lines.append(pa["content"].strip())
            lines.append("```\n")

    lines.append("## Turn-by-Turn Trajectory\n")
    for t in session.get("turns", []):
        lines.append(f"### Turn {t['index']} | `[{t['irc_time']}]` (Steps {t['start_step']}–{t['end_step']})\n")
        lines.append(f"```text\n[{t['irc_time']}] {t['prompt']}\n```\n")

        mod_actions = [a for a in t.get("tool_actions", []) if a["is_modification"]]
        read_actions = [a for a in t.get("tool_actions", []) if not a["is_modification"]]

        if mod_actions:
            lines.append("**Harness Modifications:**")
            for a in mod_actions:
                if a["name"] == "run_command":
                    lines.append(f"- `run_command`: `{a['description']}`")
                elif a["target"]:
                    lines.append(f"- `{a['name']}` `{a['target']}`: {a['description']}")
                else:
                    lines.append(f"- `{a['name']}`: {a['description']}")
            lines.append("")

        if read_actions:
            read_summary = ", ".join(f"`{a['name']}` {a['target']}" for a in read_actions[:5])
            if len(read_actions) > 5:
                read_summary += f" (+{len(read_actions)-5} more)"
            lines.append(f"*Read/Inspection tools:* {read_summary}\n")

        if t.get("model_diagnosis"):
            lines.append("**Model Diagnosis / Narrative Output:**")
            lines.append(f"> {t['model_diagnosis'].replace(chr(10), chr(10) + '> ')}\n")

        lines.append("---\n")

    return "\n".join(lines)


def parse_turn_spec(turn_spec: str, total_turns: int) -> List[int]:
    """Parses turn spec string like '1', '1..5', '1-5', '1,3,7', or 'all' into a list of 1-based turn indices."""
    if not turn_spec or turn_spec.lower() == "all":
        return list(range(1, total_turns + 1))

    indices = []
    for part in re.split(r"[,;]\s*", turn_spec.strip()):
        if ".." in part:
            s, e = part.split("..", 1)
            start = int(s) if s else 1
            end = int(e) if e else total_turns
            indices.extend(range(max(1, start), min(total_turns, end) + 1))
        elif "-" in part and not part.startswith("-"):
            s, e = part.split("-", 1)
            start = int(s) if s else 1
            end = int(e) if e else total_turns
            indices.extend(range(max(1, start), min(total_turns, end) + 1))
        else:
            try:
                idx = int(part)
                if 1 <= idx <= total_turns:
                    indices.append(idx)
            except ValueError:
                pass

    # Deduplicate while preserving order
    seen = set()
    result = []
    for i in indices:
        if i not in seen:
            seen.add(i)
            result.append(i)
    return result


def format_turns(session: Dict, turn_indices: List[int]) -> str:
    """Formats deep inspection details for a list of turn indices."""
    turns = session.get("turns", [])
    if not turns:
        return "Error: No turns found in session."

    outputs = []
    for turn_num in turn_indices:
        if turn_num < 1 or turn_num > len(turns):
            continue
        t = turns[turn_num - 1]
        lines = []
        lines.append(f"# Deep Inspection: Turn {t['index']} | `[{t['irc_time']}]`")
        lines.append(f"- **Step Range:** {t['start_step']} to {t['end_step']}")
        lines.append(f"- **Timestamp:** `{t['created_at']}`\n")
        lines.append(f"## Verbatim Human Prompt\n```text\n[{t['irc_time']}] {t['prompt']}\n```\n")

        lines.append("## Tool Calls Executed")
        mod_actions = [a for a in t.get("tool_actions", []) if a["is_modification"]]
        read_actions = [a for a in t.get("tool_actions", []) if not a["is_modification"]]

        if mod_actions:
            lines.append("### Modifications:")
            for a in mod_actions:
                tgt_str = f" -> `{a['target']}`" if a["target"] else ""
                lines.append(f"- `{a['name']}`{tgt_str}: {a['description']}")
                raw_args = a.get("raw_args", {})
                if "ReplacementContent" in raw_args:
                    snippet = str(raw_args["ReplacementContent"]).strip()
                    if snippet:
                        lines.append(f"  *Diff Snippet:*\n  ```\n  {snippet[:250]}\n  ```")
                elif "CommandLine" in raw_args:
                    lines.append(f"  *Command:* `{raw_args['CommandLine']}`")
            lines.append("")

        if read_actions:
            read_summary = ", ".join(f"`{a['name']}` {a['target']}" for a in read_actions[:6])
            lines.append(f"*Read/Inspection calls:* {read_summary}\n")

        if t.get("model_diagnosis"):
            lines.append("## Model Diagnosis & Output")
            lines.append(f"> {t['model_diagnosis'].replace(chr(10), chr(10) + '> ')}\n")

        outputs.append("\n".join(lines))

    return "\n\n" + ("=" * 80) + "\n\n".join(outputs)


def generate_scaffold(
    session: Dict, commit_info: Optional[Dict], title: str = "", model_name: str = "Gemini 3.7 Flash (High)"
) -> str:
    """Generates the starting Prompt-First markdown scaffold populated directly from session records."""
    if not title:
        if commit_info and commit_info.get("commits"):
            title = commit_info["commits"][0]["message"]
        elif session.get("turns"):
            first_p = session["turns"][0]["prompt"]
            title = first_p.split("\n")[0].replace("/plan", "").strip()
        else:
            title = "Feature Provenance"

    timestamp_str = session.get("start_time") or datetime.datetime.now(datetime.timezone.utc).isoformat()
    if timestamp_str and not timestamp_str.endswith("Z"):
        timestamp_str = format_utc(timestamp_str)

    agent_str = "Antigravity CLI"
    try:
        ver_out = subprocess.check_output(["agy", "--version"], text=True, stderr=subprocess.DEVNULL).strip()
        if ver_out:
            agent_str = f"Antigravity CLI {ver_out}"
    except Exception:
        try:
            ver_out = subprocess.check_output(["antigravity", "--version"], text=True, stderr=subprocess.DEVNULL).strip()
            if ver_out:
                agent_str = f"Antigravity CLI {ver_out}"
        except Exception:
            pass

    resolved_model = session.get("detected_model") or model_name or "Gemini 3.7 Flash (High)"
    agent_md = f"- **Agent:** {agent_str} {resolved_model}\n"

    base_commit_md = ""
    if commit_info and commit_info.get("base_commit"):
        bc = commit_info["base_commit"]
        base_commit_md = f"- **Base Commit:** [`{bc['short_hash']}`](https://github.com/Neverball/neverball/commit/{bc['hash']}): `{bc['message']}` ({bc['date']})\n"

    commits_md = ""
    if commit_info and commit_info.get("commits"):
        commits_md = "\n".join([
            f"  - [`{c['short_hash']}`](https://github.com/Neverball/neverball/commit/{c['hash']}): `{c['message']}` ({c['date']})"
            for c in commit_info["commits"]
        ])
    else:
        commits_md = "  - Commit: (uncommitted or pending)"

    files_touched_md = ""
    if commit_info and commit_info.get("files"):
        files_touched_md = ", ".join([f"`{f}`" for f in commit_info["files"]])
    elif session.get("files_touched"):
        files_touched_md = ", ".join([f"`{f}`" for f in session["files_touched"]])
    else:
        files_touched_md = "_None recorded_"

    prompts_narrative_blocks = []
    turns = session.get("turns", [])

    if turns:
        t0 = turns[0]
        prompts_narrative_blocks.append("### Initial Problem & Hypothesis\n")
        prompts_narrative_blocks.append(f"```text\n[{t0['irc_time']}] {t0['prompt']}\n```\n")

        mod_actions0 = [a for a in t0.get("tool_actions", []) if a["is_modification"]]
        diag0 = t0.get("model_diagnosis", "").strip()
        context_lines = []
        if mod_actions0:
            context_lines.append("Harness actions:")
            for a in mod_actions0:
                context_lines.append(f"- {a['name']} {a['target']}: {a['description']}")
        if diag0:
            context_lines.append(f"Diagnosis / Plan note:\n{diag0[:300]}")
        if context_lines:
            joined_ctx = "\n".join(context_lines)
            prompts_narrative_blocks.append(f"<!-- TURN {t0['index']} CONTEXT:\n{joined_ctx}\n-->\n")

        prompts_narrative_blocks.append("*(Explain observable plan scope, files touched, and technical implementation. Do NOT invent unstated user motivations or workflow backstories)*\n")

        if len(turns) > 1:
            prompts_narrative_blocks.append("### Constraint Discovery & Course Corrections\n")
            for t in turns[1:]:
                prompts_narrative_blocks.append(f"```text\n[{t['irc_time']}] {t['prompt']}\n```\n")
                mods = [a for a in t.get("tool_actions", []) if a["is_modification"]]
                diag = t.get("model_diagnosis", "").strip()
                ctx_lines = []
                if mods:
                    ctx_lines.append("Harness actions:")
                    for a in mods:
                        ctx_lines.append(f"- {a['name']} {a['target']}: {a['description']}")
                if diag:
                    ctx_lines.append(f"Diagnosis note:\n{diag[:300]}")
                if ctx_lines:
                    joined = "\n".join(ctx_lines)
                    prompts_narrative_blocks.append(f"<!-- TURN {t['index']} CONTEXT:\n{joined}\n-->\n")

                prompts_narrative_blocks.append("*(Document diagnosis, underlying engine/runtime mechanics, and code changes)*\n")

    narrative_section = "\n".join(prompts_narrative_blocks)

    doc = f"""# Architectural Provenance: {title}

- **Session Date:** `{timestamp_str}`
{agent_md}{base_commit_md}- **Commits:**
{commits_md}
- **Files Modified:** {files_touched_md}

---

## Annotated Prompt Log

{narrative_section}
### Verification & Technical Invariants
- *(Document verification test runs, exit codes, and genuine subsystem constraints/invariants)*
"""
    return doc


def main():
    parser = argparse.ArgumentParser(
        description="Antigravity Session Provenance Discovery & Dossier CLI Suite"
    )
    parser.add_argument("--list", action="store_true", help="List recent sessions for current repository")
    parser.add_argument("--commit", type=str, help="Single git commit hash (e.g. b38ec9ee)")
    parser.add_argument("--range", type=str, help="Git commit range (e.g. HEAD~3..HEAD)")
    parser.add_argument("--session", type=str, help="Specific Conversation ID or short prefix")
    parser.add_argument("--match", action="store_true", help="Find best matching session for commit/range")
    parser.add_argument("--dossier", action="store_true", help="Generate comprehensive Markdown session dossier")
    parser.add_argument("--turn", type=str, help="Inspect turn number or range in depth (e.g. 1, 1..5, 4-8, 1,3,7, or all)")
    parser.add_argument("--extract", action="store_true", help="Extract session data and output JSON")
    parser.add_argument("--scaffold", action="store_true", help="Generate provenance markdown scaffold")
    parser.add_argument("--title", type=str, default="", help="Custom title for scaffold")
    parser.add_argument("--model", type=str, default="Gemini 3.7 Flash (High)", help="Model name used during session")
    parser.add_argument("--output", type=str, help="Output file path for scaffold or dossier")

    args = parser.parse_args()
    repo_root = get_repo_root()
    brain_dir = get_brain_dir()

    if args.list:
        sessions = find_all_sessions(brain_dir, repo_root)
        print(f"Found {len(sessions)} Antigravity session(s) for repository: {repo_root.name}\n")
        print(f"{'TIMESTAMP':<22} | {'SESSION ID':<36} | {'TURNS':<5} | {'FIRST PROMPT'}")
        print("-" * 110)
        for s in sessions[:20]:
            first_p = (
                s["turns"][0]["prompt"].replace("\n", " ")[:45]
                if s.get("turns")
                else ""
            )
            print(f"{s['start_time']:<22} | {s['conversation_id']:<36} | {len(s.get('turns', [])):<5} | {first_p}")
        return

    commit_spec = args.range or args.commit
    commit_info = parse_commit_spec(commit_spec) if commit_spec else None

    target_session = None
    if args.session:
        candidates = list(brain_dir.glob(f"{args.session}*"))
        if candidates and (candidates[0] / ".system_generated" / "logs" / "transcript.jsonl").exists():
            target_session = parse_transcript(candidates[0] / ".system_generated" / "logs" / "transcript.jsonl")
        else:
            sys.stderr.write(f"Error: Session {args.session} not found in {brain_dir}\n")
            sys.exit(1)
    elif commit_info:
        sessions = find_all_sessions(brain_dir, repo_root)
        scored = []
        for s in sessions:
            score, reasons = score_session_match(s, commit_info)
            if score > 0:
                scored.append((score, reasons, s))
        scored.sort(key=lambda x: x[0], reverse=True)

        if args.match:
            print(f"Top matching sessions for commit spec '{commit_spec}':\n")
            for score, reasons, s in scored[:5]:
                print(f"Score: {score} | Session: {s['conversation_id']} | Start: {s['start_time']}")
                print(f"  Reasons: {', '.join(reasons)}")
                first_p = s["turns"][0]["prompt"] if s.get("turns") else ""
                print(f"  First Prompt: {first_p[:80]}...\n")
            return

        if scored:
            target_session = scored[0][2]
        else:
            sessions = find_all_sessions(brain_dir, repo_root)
            if sessions:
                target_session = sessions[0]

    if not target_session:
        sessions = find_all_sessions(brain_dir, repo_root)
        if sessions:
            target_session = sessions[0]
        else:
            sys.stderr.write("Error: No Antigravity sessions found for this repository.\n")
            sys.exit(1)

    if args.turn:
        turn_indices = parse_turn_spec(args.turn, len(target_session.get("turns", [])))
        print(format_turns(target_session, turn_indices))
        return

    if args.dossier:
        dossier_text = generate_dossier(target_session, commit_info)
        if args.output:
            out_path = Path(args.output)
            out_path.parent.mkdir(parents=True, exist_ok=True)
            out_path.write_text(dossier_text, encoding="utf-8")
            print(f"Dossier written to {out_path}")
        else:
            print(dossier_text)
        return

    if args.extract:
        output_payload = {
            "session": target_session,
            "git": commit_info,
        }
        print(json.dumps(output_payload, indent=2))
        return

    if args.scaffold or not (args.list or args.match or args.extract or args.dossier or args.turn):
        scaffold_content = generate_scaffold(target_session, commit_info, args.title, model_name=args.model)

        if args.output:
            out_path = Path(args.output)
        else:
            ts = target_session.get("start_time") or datetime.datetime.now(datetime.timezone.utc).isoformat()
            dt = datetime.datetime.fromisoformat(ts.replace("Z", "+00:00"))
            date_prefix = dt.strftime("%Y_%m_%d_%H%M%S")

            slug_raw = args.title or (commit_info["commits"][0]["message"] if commit_info and commit_info.get("commits") else "provenance")
            slug = re.sub(r"[^a-zA-Z0-9_]+", "_", slug_raw.lower()).strip("_")[:40]

            prompts_dir = repo_root / "prompts"
            prompts_dir.mkdir(parents=True, exist_ok=True)
            out_path = prompts_dir / f"{date_prefix}_{slug}.md"

        out_path.parent.mkdir(parents=True, exist_ok=True)
        out_path.write_text(scaffold_content, encoding="utf-8")
        print(f"Scaffold generated successfully: {out_path}")


if __name__ == "__main__":
    main()
