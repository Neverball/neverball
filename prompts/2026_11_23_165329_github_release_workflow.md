# Prompt Log Export 2026-11-23 16:53:29 UTC

- **Generator:** [git-prompt-log](https://github.com/parasti/git-prompt-log)
- **Export command:** `git prompt-log export --commit --slug github_release_workflow`
- **Import command:** `git prompt-log import prompts/2026_11_23_165329_github_release_workflow.md`

---

- **Session:** `a102d8ea-5eb3-4901-9d04-1619944dbe98`
- **Harness:** Antigravity CLI 1.5.15
- **Model:** Gemini 4 Hydrogen (Medium)

## Commits

- `1fe6fcdc` .github/workflows: enable workflow_call on macos and windows
- `f65bf278` .github/workflows/release: add tag-triggered draft release workflow
- `ad585ed7` scripts/package-macos: use macos-apple and macos-intel DMG suffixes
- `1bcf9917` .github/workflows/release: make doc/release-notes.md optional
- `320f2165` .github/workflows/release: verify tag matches neverball-version.txt

## Steering Prompts

#### [2026-11-20 22:39:17 UTC]

> What's the standard approach for having a github bot craft a release from a tag and its built artifacts?

#### [2026-11-20 22:41:56 UTC]

> We'll use actions we already have for building artifacts, right?

#### [2026-11-20 22:44:45 UTC]

> /plan plan this out. we can't really generate notes because most of the development never used PRs and pushed direct on master. i'm actually unsure how much effort should be put into the release notes or should i focus on a blog post on the website (or do both at the same time). previous work on this is in a branch called "release", but I am interested in a fresh take more than reuse.

#### [2026-11-20 22:52:48 UTC]

> 1) draft, 2) yes, read from doc/release-notes.md and archive previous notes, 3) yes, attach source as a .tar.gz, 4) yes, -alpha, -beta and -rc set --prerelease

Commits:
- `1fe6fcdc` .github/workflows: enable workflow_call on macos and windows
- `f65bf278` .github/workflows/release: add tag-triggered draft release workflow

#### [2026-11-20 23:27:54 UTC]

> I dropped the latest commit, don't need a template. But I will admit that writing release notes feels like a slog to me, with almost no benefit. How do you summarize 12 years of intermittent development in a way that's actually useful for players to read.

#### [2026-11-21 09:54:53 UTC]

> Why is parseInclude called twice? Should be at the top of the file where the arg parsing is.

#### [2026-11-21 09:57:46 UTC]

> This is bad style: "if (isBase ? !includeBase : !includeAddon)" - fix to readable code.

#### [2026-11-21 09:58:40 UTC]

> I dropped the latest commit, don't need a template. But I will admit that writing release notes feels like a slog to me, with almost no benefit. How do you summarize 12 years of intermittent development in a way that's actually useful for players to read.

#### [2026-11-23 14:49:09 UTC]

> Look at another session I just had in the neverball-website project, where we discussed publishing of releases/prereleases on the website. This session remains priority, but pick up the discussion from there.

#### [2026-11-23 14:55:55 UTC]

> I guess the macOS builds need suffixes changed to -macos-apple.dmg and -macos-intel.dmg. I am a fan of -apple and -intel because those align with Apple marketing terminology ("Apple Silicon" and all that). Architectures would be more correct, but harder to distinguish for regular people I think (maybe not). Do add the fallback for release notes, I'm still figuring out how to best go about that.

Commits:
- `ad585ed7` scripts/package-macos: use macos-apple and macos-intel DMG suffixes
- `1bcf9917` .github/workflows/release: make doc/release-notes.md optional

#### [2026-11-23 16:17:07 UTC]

> Wondering how maintainers manage this juggling of release notes and tags. Like, what the process is.

#### [2026-11-23 16:19:23 UTC]

> I like pattern 2. So I would actually write a post first, then link to it from the draft UI.

#### [2026-11-23 16:44:58 UTC]

> I'm thinking the version should be driven by the release script not individually by version.sh invocations. version.sh is designed to pick a version from neverball-version.txt when outside of Git (in a source archive) but use git describe otherwise. And as I write this, I'm thinking these macos/windows builds should be done inside the release workflow, not as separate actions that compile assets separately - just seems wildly wasteful to run all the checkout and SOL builds twice? Maybe I'm wrong, what's your estimation?

#### [2026-11-23 16:48:17 UTC]

> Yeah, do the first one. Can you also make release.yml fail if tagged version does not match neverball-version.txt? I made that mistake with the first three alphas, but rightly should have updated them to match the tag.

Commits:
- `320f2165` .github/workflows/release: verify tag matches neverball-version.txt

<!-- git-prompt-log:metadata
{
  "version": 1,
  "exported_at": "2026-11-23 16:53:29 UTC",
  "export_command": "git prompt-log export --commit --slug github_release_workflow",
  "import_command": "git prompt-log import prompts/2026_11_23_165329_github_release_workflow.md",
  "commits": [
    {
      "hash": "1fe6fcdca28cb5dc3241e7f8dcbec4611f9d7599",
      "subject": ".github/workflows: enable workflow_call on macos and windows",
      "note": "Assistant-Session: a102d8ea-5eb3-4901-9d04-1619944dbe98\nAssistant-Harness: Antigravity CLI 1.5.13\nAssistant-Model: Gemini 4 Hydrogen (Medium)\nAssistant-Recorded: 2026-11-20 22:53:52 UTC\n\nAssistant-Prompts:\n  [2026-11-20 22:52:48 UTC] 1) draft, 2) yes, read from doc/release-notes.md and archive previous notes, 3) yes, attach source as a .tar.gz, 4) yes, -alpha, -beta and -rc set --prerelease\n  [2026-11-20 22:44:45 UTC] /plan plan this out. we can't really generate notes because most of the development never used PRs and pushed direct on master. i'm actually unsure how much effort should be put into the release notes or should i focus on a blog post on the website (or do both at the same time). previous work on this is in a branch called \"release\", but I am interested in a fresh take more than reuse.\n  [2026-11-20 22:41:56 UTC] We'll use actions we already have for building artifacts, right?\n  [2026-11-20 22:39:17 UTC] What's the standard approach for having a github bot craft a release from a tag and its built artifacts?"
    },
    {
      "hash": "f65bf2789d900cd0bbba0c003d7c3fbe975b9c12",
      "subject": ".github/workflows/release: add tag-triggered draft release workflow",
      "note": "Assistant-Session: a102d8ea-5eb3-4901-9d04-1619944dbe98\nAssistant-Harness: Antigravity CLI 1.5.13\nAssistant-Model: Gemini 4 Hydrogen (Medium)\nAssistant-Recorded: 2026-11-20 22:53:53 UTC\n\nAssistant-Prompts:\n  [2026-11-20 22:52:48 UTC] 1) draft, 2) yes, read from doc/release-notes.md and archive previous notes, 3) yes, attach source as a .tar.gz, 4) yes, -alpha, -beta and -rc set --prerelease\n  [2026-11-20 22:44:45 UTC] /plan plan this out. we can't really generate notes because most of the development never used PRs and pushed direct on master. i'm actually unsure how much effort should be put into the release notes or should i focus on a blog post on the website (or do both at the same time). previous work on this is in a branch called \"release\", but I am interested in a fresh take more than reuse.\n  [2026-11-20 22:41:56 UTC] We'll use actions we already have for building artifacts, right?\n  [2026-11-20 22:39:17 UTC] What's the standard approach for having a github bot craft a release from a tag and its built artifacts?"
    },
    {
      "hash": "ad585ed7bc82a8c472b016bbecba4f5556354e02",
      "subject": "scripts/package-macos: use macos-apple and macos-intel DMG suffixes",
      "note": "Assistant-Session: a102d8ea-5eb3-4901-9d04-1619944dbe98\nAssistant-Harness: Antigravity CLI 1.5.15\nAssistant-Model: Gemini 4 Hydrogen (Medium)\nAssistant-Recorded: 2026-11-23 14:56:14 UTC\n\nAssistant-Prompts:\n  [2026-11-23 14:55:55 UTC] I guess the macOS builds need suffixes changed to -macos-apple.dmg and -macos-intel.dmg. I am a fan of -apple and -intel because those align with Apple marketing terminology (\"Apple Silicon\" and all that). Architectures would be more correct, but harder to distinguish for regular people I think (maybe not). Do add the fallback for release notes, I'm still figuring out how to best go about that.\n  [2026-11-23 14:49:09 UTC] Look at another session I just had in the neverball-website project, where we discussed publishing of releases/prereleases on the website. This session remains priority, but pick up the discussion from there.\n  [2026-11-21 09:58:40 UTC] I dropped the latest commit, don't need a template. But I will admit that writing release notes feels like a slog to me, with almost no benefit. How do you summarize 12 years of intermittent development in a way that's actually useful for players to read.\n  [2026-11-21 09:57:46 UTC] This is bad style: \"if (isBase ? !includeBase : !includeAddon)\" - fix to readable code.\n  [2026-11-21 09:54:53 UTC] Why is parseInclude called twice? Should be at the top of the file where the arg parsing is.\n  [2026-11-20 23:27:54 UTC] I dropped the latest commit, don't need a template. But I will admit that writing release notes feels like a slog to me, with almost no benefit. How do you summarize 12 years of intermittent development in a way that's actually useful for players to read.\n  [2026-11-20 22:52:48 UTC] 1) draft, 2) yes, read from doc/release-notes.md and archive previous notes, 3) yes, attach source as a .tar.gz, 4) yes, -alpha, -beta and -rc set --prerelease\n  [2026-11-20 22:44:45 UTC] /plan plan this out. we can't really generate notes because most of the development never used PRs and pushed direct on master. i'm actually unsure how much effort should be put into the release notes or should i focus on a blog post on the website (or do both at the same time). previous work on this is in a branch called \"release\", but I am interested in a fresh take more than reuse.\n  [2026-11-20 22:41:56 UTC] We'll use actions we already have for building artifacts, right?\n  [2026-11-20 22:39:17 UTC] What's the standard approach for having a github bot craft a release from a tag and its built artifacts?"
    },
    {
      "hash": "1bcf99175ac7b85d98d8b260266356d892df17fb",
      "subject": ".github/workflows/release: make doc/release-notes.md optional",
      "note": "Assistant-Session: a102d8ea-5eb3-4901-9d04-1619944dbe98\nAssistant-Harness: Antigravity CLI 1.5.15\nAssistant-Model: Gemini 4 Hydrogen (Medium)\nAssistant-Recorded: 2026-11-23 14:56:14 UTC\n\nAssistant-Prompts:\n  [2026-11-23 14:55:55 UTC] I guess the macOS builds need suffixes changed to -macos-apple.dmg and -macos-intel.dmg. I am a fan of -apple and -intel because those align with Apple marketing terminology (\"Apple Silicon\" and all that). Architectures would be more correct, but harder to distinguish for regular people I think (maybe not). Do add the fallback for release notes, I'm still figuring out how to best go about that.\n  [2026-11-23 14:49:09 UTC] Look at another session I just had in the neverball-website project, where we discussed publishing of releases/prereleases on the website. This session remains priority, but pick up the discussion from there.\n  [2026-11-21 09:58:40 UTC] I dropped the latest commit, don't need a template. But I will admit that writing release notes feels like a slog to me, with almost no benefit. How do you summarize 12 years of intermittent development in a way that's actually useful for players to read.\n  [2026-11-21 09:57:46 UTC] This is bad style: \"if (isBase ? !includeBase : !includeAddon)\" - fix to readable code.\n  [2026-11-21 09:54:53 UTC] Why is parseInclude called twice? Should be at the top of the file where the arg parsing is.\n  [2026-11-20 23:27:54 UTC] I dropped the latest commit, don't need a template. But I will admit that writing release notes feels like a slog to me, with almost no benefit. How do you summarize 12 years of intermittent development in a way that's actually useful for players to read.\n  [2026-11-20 22:52:48 UTC] 1) draft, 2) yes, read from doc/release-notes.md and archive previous notes, 3) yes, attach source as a .tar.gz, 4) yes, -alpha, -beta and -rc set --prerelease\n  [2026-11-20 22:44:45 UTC] /plan plan this out. we can't really generate notes because most of the development never used PRs and pushed direct on master. i'm actually unsure how much effort should be put into the release notes or should i focus on a blog post on the website (or do both at the same time). previous work on this is in a branch called \"release\", but I am interested in a fresh take more than reuse.\n  [2026-11-20 22:41:56 UTC] We'll use actions we already have for building artifacts, right?\n  [2026-11-20 22:39:17 UTC] What's the standard approach for having a github bot craft a release from a tag and its built artifacts?"
    },
    {
      "hash": "320f216511eac592892cfa4a51b24b98a194316b",
      "subject": ".github/workflows/release: verify tag matches neverball-version.txt",
      "note": "Assistant-Session: a102d8ea-5eb3-4901-9d04-1619944dbe98\nAssistant-Harness: Antigravity CLI 1.5.15\nAssistant-Model: Gemini 4 Hydrogen (Medium)\nAssistant-Recorded: 2026-11-23 16:48:36 UTC\n\nAssistant-Prompts:\n  [2026-11-23 16:48:17 UTC] Yeah, do the first one. Can you also make release.yml fail if tagged version does not match neverball-version.txt? I made that mistake with the first three alphas, but rightly should have updated them to match the tag.\n  [2026-11-23 16:44:58 UTC] I'm thinking the version should be driven by the release script not individually by version.sh invocations. version.sh is designed to pick a version from neverball-version.txt when outside of Git (in a source archive) but use git describe otherwise. And as I write this, I'm thinking these macos/windows builds should be done inside the release workflow, not as separate actions that compile assets separately - just seems wildly wasteful to run all the checkout and SOL builds twice? Maybe I'm wrong, what's your estimation?\n  [2026-11-23 16:19:23 UTC] I like pattern 2. So I would actually write a post first, then link to it from the draft UI.\n  [2026-11-23 16:17:07 UTC] Wondering how maintainers manage this juggling of release notes and tags. Like, what the process is.\n  [2026-11-23 14:55:55 UTC] I guess the macOS builds need suffixes changed to -macos-apple.dmg and -macos-intel.dmg. I am a fan of -apple and -intel because those align with Apple marketing terminology (\"Apple Silicon\" and all that). Architectures would be more correct, but harder to distinguish for regular people I think (maybe not). Do add the fallback for release notes, I'm still figuring out how to best go about that.\n  [2026-11-23 14:49:09 UTC] Look at another session I just had in the neverball-website project, where we discussed publishing of releases/prereleases on the website. This session remains priority, but pick up the discussion from there.\n  [2026-11-21 09:58:40 UTC] I dropped the latest commit, don't need a template. But I will admit that writing release notes feels like a slog to me, with almost no benefit. How do you summarize 12 years of intermittent development in a way that's actually useful for players to read.\n  [2026-11-21 09:57:46 UTC] This is bad style: \"if (isBase ? !includeBase : !includeAddon)\" - fix to readable code.\n  [2026-11-21 09:54:53 UTC] Why is parseInclude called twice? Should be at the top of the file where the arg parsing is.\n  [2026-11-20 23:27:54 UTC] I dropped the latest commit, don't need a template. But I will admit that writing release notes feels like a slog to me, with almost no benefit. How do you summarize 12 years of intermittent development in a way that's actually useful for players to read.\n  [2026-11-20 22:52:48 UTC] 1) draft, 2) yes, read from doc/release-notes.md and archive previous notes, 3) yes, attach source as a .tar.gz, 4) yes, -alpha, -beta and -rc set --prerelease\n  [2026-11-20 22:44:45 UTC] /plan plan this out. we can't really generate notes because most of the development never used PRs and pushed direct on master. i'm actually unsure how much effort should be put into the release notes or should i focus on a blog post on the website (or do both at the same time). previous work on this is in a branch called \"release\", but I am interested in a fresh take more than reuse.\n  [2026-11-20 22:41:56 UTC] We'll use actions we already have for building artifacts, right?\n  [2026-11-20 22:39:17 UTC] What's the standard approach for having a github bot craft a release from a tag and its built artifacts?"
    }
  ]
}
-->
