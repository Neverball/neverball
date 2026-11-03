# Prompt Log Export 2026-11-03 14:39:34 UTC

- **Generator:** [git-prompt-log](https://github.com/parasti/git-prompt-log)
- **Export command:** `git prompt-log export --range origin/master..HEAD --commit --slug parented_path_relative_orientation`
- **Import command:** `git prompt-log import prompts/2026_11_03_143934_parented_path_relative_orientation.md`

---

- **Session:** `1e5f59b3-e451-42b5-9fad-f1a5235b9687`
- **Harness:** Antigravity CLI 1.4.12
- **Model:** Gemini 3.8 Flash (High)

## Commits

- `2f4aaad0` tests/test_solid: Add test for parented mover relative orientation
- `8818760a` share/mapclib: Compute parented path orientation relative to basis path

## Steering Prompts

#### [2026-11-03 13:57:33 UTC]

> I am editing a section of easy.map off-grid (as a test) - using linked groups does wonders, I can edit the grid-aligned version that's marked not-for-export while rotating the off-grid version how I need. And generally mapc and Neverball can handle that except for one thing I just discovered: when I have an item_coin in the linked groups that's attached to a path moving back and forth in a simple manner, it appears in the wrong location in the off-grid version. When the off-grid linked instance is rotated item_coin obtains angles and path_corner obtains angles, and this combination somehow breaks mapc logic for what should be a simple linear motion. Investigate and report back.

#### [2026-11-03 14:22:14 UTC]

> There is an error in your analysis which is otherwise sound: make_parented_path has nothing to do with entities resting on platforms. An entity is not "parented to a platform". A "parent" in this case refers to a path_corner that moves another path_corner.

#### [2026-11-03 14:26:33 UTC]

> I feel like this could be resolved by mapc not assuming basis angles of 0 0 0 but instead taking the basis angles of the path_corner - if I understand the issue correctly.

#### [2026-11-03 14:27:49 UTC]

> Let me elaborate: if path_corner and item_coin both have the same angles, their relative orientation is identity. This is what mapc should be working with.

#### [2026-11-03 14:31:55 UTC]

> Implement this on master. Don't commit easy.map.

Commits:
- `2f4aaad0` tests/test_solid: Add test for parented mover relative orientation
- `8818760a` share/mapclib: Compute parented path orientation relative to basis path

<!-- git-prompt-log:metadata
{
  "version": 1,
  "exported_at": "2026-11-03 14:39:34 UTC",
  "export_command": "git prompt-log export --range origin/master..HEAD --commit --slug parented_path_relative_orientation",
  "import_command": "git prompt-log import prompts/2026_11_03_143934_parented_path_relative_orientation.md",
  "commits": [
    {
      "hash": "2f4aaad0ab4a98db85c516dbef6a93b45a47a316",
      "subject": "tests/test_solid: Add test for parented mover relative orientation",
      "note": "Assistant-Session: 1e5f59b3-e451-42b5-9fad-f1a5235b9687\nAssistant-Harness: Antigravity CLI 1.4.12\nAssistant-Model: Gemini 3.8 Flash (High)\nAssistant-Recorded: 2026-11-03 14:37:02 UTC\n\nAssistant-Prompts:\n  [2026-11-03 14:31:55 UTC] Implement this on master. Don't commit easy.map.\n  [2026-11-03 14:27:49 UTC] Let me elaborate: if path_corner and item_coin both have the same angles, their relative orientation is identity. This is what mapc should be working with.\n  [2026-11-03 14:26:33 UTC] I feel like this could be resolved by mapc not assuming basis angles of 0 0 0 but instead taking the basis angles of the path_corner - if I understand the issue correctly.\n  [2026-11-03 14:22:14 UTC] There is an error in your analysis which is otherwise sound: make_parented_path has nothing to do with entities resting on platforms. An entity is not \"parented to a platform\". A \"parent\" in this case refers to a path_corner that moves another path_corner.\n  [2026-11-03 13:57:33 UTC] I am editing a section of easy.map off-grid (as a test) - using linked groups does wonders, I can edit the grid-aligned version that's marked not-for-export while rotating the off-grid version how I need. And generally mapc and Neverball can handle that except for one thing I just discovered: when I have an item_coin in the linked groups that's attached to a path moving back and forth in a simple manner, it appears in the wrong location in the off-grid version. When the off-grid linked instance is rotated item_coin obtains angles and path_corner obtains angles, and this combination somehow breaks mapc logic for what should be a simple linear motion. Investigate and report back."
    },
    {
      "hash": "8818760a60baab93354fe307ae11a89e0d96aaf5",
      "subject": "share/mapclib: Compute parented path orientation relative to basis path",
      "note": "Assistant-Session: 1e5f59b3-e451-42b5-9fad-f1a5235b9687\nAssistant-Harness: Antigravity CLI 1.4.12\nAssistant-Model: Gemini 3.8 Flash (High)\nAssistant-Recorded: 2026-11-03 14:37:15 UTC\n\nAssistant-Prompts:\n  [2026-11-03 14:31:55 UTC] Implement this on master. Don't commit easy.map.\n  [2026-11-03 14:27:49 UTC] Let me elaborate: if path_corner and item_coin both have the same angles, their relative orientation is identity. This is what mapc should be working with.\n  [2026-11-03 14:26:33 UTC] I feel like this could be resolved by mapc not assuming basis angles of 0 0 0 but instead taking the basis angles of the path_corner - if I understand the issue correctly.\n  [2026-11-03 14:22:14 UTC] There is an error in your analysis which is otherwise sound: make_parented_path has nothing to do with entities resting on platforms. An entity is not \"parented to a platform\". A \"parent\" in this case refers to a path_corner that moves another path_corner.\n  [2026-11-03 13:57:33 UTC] I am editing a section of easy.map off-grid (as a test) - using linked groups does wonders, I can edit the grid-aligned version that's marked not-for-export while rotating the off-grid version how I need. And generally mapc and Neverball can handle that except for one thing I just discovered: when I have an item_coin in the linked groups that's attached to a path moving back and forth in a simple manner, it appears in the wrong location in the off-grid version. When the off-grid linked instance is rotated item_coin obtains angles and path_corner obtains angles, and this combination somehow breaks mapc logic for what should be a simple linear motion. Investigate and report back."
    }
  ]
}
-->
