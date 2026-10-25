# Prompt Log Export 2026-10-25 23:23:29 UTC

- **Generator:** [git-prompt-log](https://github.com/parasti/git-prompt-log)
- **Export command:** `git prompt-log export --commit --slug trenchbroom_obj_coordinates`
- **Import command:** `git prompt-log import prompts/2026_10_25_232329_trenchbroom_obj_coordinates.md`

---

- **Session:** `06fe9f9a-ec70-42e7-a5f3-bc7d76c42fe7`
- **Harness:** Antigravity CLI 1.4.7
- **Model:** Gemini 3.8 Flash (High)

## Commits

- `e85803cf` scripts: Add OBJ conversion script for TrenchBroom map coordinates
- `b61abc4c` share/mapclib: Remap OBJ coordinates and normals from TrenchBroom space
- `0e176a84` data: Convert OBJ models to TrenchBroom map coordinates
- `3864aff5` TrenchBroom: Update entity model scale and documentation

## Steering Prompts

#### [2026-10-25 15:22:37 UTC]

> /plan Our OBJs have a small problem: they use a different coordinate system (the standard OpenGL coordinate system) than TrenchBroom (uses a Quake-style coordinate sytem of XY being the ground dimensions and Z being the height dimension). So our OBJs in TrenchBroom 3D view look wrong. From what I've seen there is no way around this: TrenchBroom uses assimp internally and does not provide a way to remap the loaded coordinates/normals. So I'm at a point where I'm considering to provide new OBJs specifically TrenchBroom-compatible and rewriting mapclib to do the remapping. I hate being dictated by a single third party tool, but I see no other way to enable good experience for mappers.

#### [2026-10-25 15:46:13 UTC]

> What's a standard way to indicate a coordinate system? E.g., in OpenGL or in 3D libraries?

#### [2026-10-25 15:56:27 UTC]

> Just from your context, is it realistic to patch TB instead to allow configurable remapping?

#### [2026-10-25 22:04:13 UTC]

> Let's do it old style: convert OBJs, update mapc, no backward compatibility shit. If maps break somewhere, those OBJs need to be updated.

#### [2026-10-25 22:16:48 UTC]

> Can mapc apply a heuristic here to detect one vs the other at compile time.

#### [2026-10-25 22:24:58 UTC]

> Okay, so no fallback, it's too finicky. Make sure to update OBJs in ../neverball-packages as well.

#### [2026-10-25 22:28:31 UTC]

> Exclude invisible_face2.obj, that's a direct Tb OBJ export. Rest is fine.

Commits:
- `e85803cf` scripts: Add OBJ conversion script for TrenchBroom map coordinates
- `b61abc4c` share/mapclib: Remap OBJ coordinates and normals from TrenchBroom space
- `0e176a84` data: Convert OBJ models to TrenchBroom map coordinates

#### [2026-10-25 22:42:29 UTC]

> Too many commits and I think GameConfig scale change is missing.

Commits:
- `3864aff5` TrenchBroom: Update entity model scale and documentation

<!-- git-prompt-log:metadata
{
  "version": 1,
  "exported_at": "2026-10-25 23:23:29 UTC",
  "export_command": "git prompt-log export --commit --slug trenchbroom_obj_coordinates",
  "import_command": "git prompt-log import prompts/2026_10_25_232329_trenchbroom_obj_coordinates.md",
  "commits": [
    {
      "hash": "e85803cf217c25bf020a9b13bc72654c0f8f48aa",
      "subject": "scripts: Add OBJ conversion script for TrenchBroom map coordinates",
      "note": "Assistant-Session: 06fe9f9a-ec70-42e7-a5f3-bc7d76c42fe7\nAssistant-Harness: Antigravity CLI 1.4.7\nAssistant-Model: Gemini 3.8 Flash (High)\nAssistant-Recorded: 2026-10-25 22:32:08 UTC\n\nAssistant-Prompts:\n  [2026-10-25 22:28:31 UTC] Exclude invisible_face2.obj, that's a direct Tb OBJ export. Rest is fine.\n  [2026-10-25 22:24:58 UTC] Okay, so no fallback, it's too finicky. Make sure to update OBJs in ../neverball-packages as well.\n  [2026-10-25 22:16:48 UTC] Can mapc apply a heuristic here to detect one vs the other at compile time.\n  [2026-10-25 22:04:13 UTC] Let's do it old style: convert OBJs, update mapc, no backward compatibility shit. If maps break somewhere, those OBJs need to be updated.\n  [2026-10-25 15:56:27 UTC] Just from your context, is it realistic to patch TB instead to allow configurable remapping?\n  [2026-10-25 15:46:13 UTC] What's a standard way to indicate a coordinate system? E.g., in OpenGL or in 3D libraries?\n  [2026-10-25 15:22:37 UTC] /plan Our OBJs have a small problem: they use a different coordinate system (the standard OpenGL coordinate system) than TrenchBroom (uses a Quake-style coordinate sytem of XY being the ground dimensions and Z being the height dimension). So our OBJs in TrenchBroom 3D view look wrong. From what I've seen there is no way around this: TrenchBroom uses assimp internally and does not provide a way to remap the loaded coordinates/normals. So I'm at a point where I'm considering to provide new OBJs specifically TrenchBroom-compatible and rewriting mapclib to do the remapping. I hate being dictated by a single third party tool, but I see no other way to enable good experience for mappers."
    },
    {
      "hash": "b61abc4caeb82b7b6049576cd9583ee0a9e3cdf1",
      "subject": "share/mapclib: Remap OBJ coordinates and normals from TrenchBroom space",
      "note": "Assistant-Session: 06fe9f9a-ec70-42e7-a5f3-bc7d76c42fe7\nAssistant-Harness: Antigravity CLI 1.4.7\nAssistant-Model: Gemini 3.8 Flash (High)\nAssistant-Recorded: 2026-10-25 22:32:33 UTC\n\nAssistant-Prompts:\n  [2026-10-25 22:28:31 UTC] Exclude invisible_face2.obj, that's a direct Tb OBJ export. Rest is fine.\n  [2026-10-25 22:24:58 UTC] Okay, so no fallback, it's too finicky. Make sure to update OBJs in ../neverball-packages as well.\n  [2026-10-25 22:16:48 UTC] Can mapc apply a heuristic here to detect one vs the other at compile time.\n  [2026-10-25 22:04:13 UTC] Let's do it old style: convert OBJs, update mapc, no backward compatibility shit. If maps break somewhere, those OBJs need to be updated.\n  [2026-10-25 15:56:27 UTC] Just from your context, is it realistic to patch TB instead to allow configurable remapping?\n  [2026-10-25 15:46:13 UTC] What's a standard way to indicate a coordinate system? E.g., in OpenGL or in 3D libraries?\n  [2026-10-25 15:22:37 UTC] /plan Our OBJs have a small problem: they use a different coordinate system (the standard OpenGL coordinate system) than TrenchBroom (uses a Quake-style coordinate sytem of XY being the ground dimensions and Z being the height dimension). So our OBJs in TrenchBroom 3D view look wrong. From what I've seen there is no way around this: TrenchBroom uses assimp internally and does not provide a way to remap the loaded coordinates/normals. So I'm at a point where I'm considering to provide new OBJs specifically TrenchBroom-compatible and rewriting mapclib to do the remapping. I hate being dictated by a single third party tool, but I see no other way to enable good experience for mappers."
    },
    {
      "hash": "0e176a84256eb3ab94ef463a343be2c4e4630b49",
      "subject": "data: Convert OBJ models to TrenchBroom map coordinates",
      "note": "Assistant-Session: 06fe9f9a-ec70-42e7-a5f3-bc7d76c42fe7\nAssistant-Harness: Antigravity CLI 1.4.7\nAssistant-Model: Gemini 3.8 Flash (High)\nAssistant-Recorded: 2026-10-25 22:33:20 UTC\n\nAssistant-Prompts:\n  [2026-10-25 22:28:31 UTC] Exclude invisible_face2.obj, that's a direct Tb OBJ export. Rest is fine.\n  [2026-10-25 22:24:58 UTC] Okay, so no fallback, it's too finicky. Make sure to update OBJs in ../neverball-packages as well.\n  [2026-10-25 22:16:48 UTC] Can mapc apply a heuristic here to detect one vs the other at compile time.\n  [2026-10-25 22:04:13 UTC] Let's do it old style: convert OBJs, update mapc, no backward compatibility shit. If maps break somewhere, those OBJs need to be updated.\n  [2026-10-25 15:56:27 UTC] Just from your context, is it realistic to patch TB instead to allow configurable remapping?\n  [2026-10-25 15:46:13 UTC] What's a standard way to indicate a coordinate system? E.g., in OpenGL or in 3D libraries?\n  [2026-10-25 15:22:37 UTC] /plan Our OBJs have a small problem: they use a different coordinate system (the standard OpenGL coordinate system) than TrenchBroom (uses a Quake-style coordinate sytem of XY being the ground dimensions and Z being the height dimension). So our OBJs in TrenchBroom 3D view look wrong. From what I've seen there is no way around this: TrenchBroom uses assimp internally and does not provide a way to remap the loaded coordinates/normals. So I'm at a point where I'm considering to provide new OBJs specifically TrenchBroom-compatible and rewriting mapclib to do the remapping. I hate being dictated by a single third party tool, but I see no other way to enable good experience for mappers."
    },
    {
      "hash": "3864aff5f79961ba06db08b6df45099ece3d4dee",
      "subject": "TrenchBroom: Update entity model scale and documentation",
      "note": "Assistant-Session: 06fe9f9a-ec70-42e7-a5f3-bc7d76c42fe7\nAssistant-Harness: Antigravity CLI 1.4.7\nAssistant-Model: Gemini 3.8 Flash (High)\nAssistant-Recorded: 2026-10-25 22:46:18 UTC\n\nAssistant-Prompts:\n  [2026-10-25 22:42:29 UTC] Too many commits and I think GameConfig scale change is missing.\n  [2026-10-25 22:28:31 UTC] Exclude invisible_face2.obj, that's a direct Tb OBJ export. Rest is fine.\n  [2026-10-25 22:24:58 UTC] Okay, so no fallback, it's too finicky. Make sure to update OBJs in ../neverball-packages as well.\n  [2026-10-25 22:16:48 UTC] Can mapc apply a heuristic here to detect one vs the other at compile time.\n  [2026-10-25 22:04:13 UTC] Let's do it old style: convert OBJs, update mapc, no backward compatibility shit. If maps break somewhere, those OBJs need to be updated.\n  [2026-10-25 15:56:27 UTC] Just from your context, is it realistic to patch TB instead to allow configurable remapping?\n  [2026-10-25 15:46:13 UTC] What's a standard way to indicate a coordinate system? E.g., in OpenGL or in 3D libraries?\n  [2026-10-25 15:22:37 UTC] /plan Our OBJs have a small problem: they use a different coordinate system (the standard OpenGL coordinate system) than TrenchBroom (uses a Quake-style coordinate sytem of XY being the ground dimensions and Z being the height dimension). So our OBJs in TrenchBroom 3D view look wrong. From what I've seen there is no way around this: TrenchBroom uses assimp internally and does not provide a way to remap the loaded coordinates/normals. So I'm at a point where I'm considering to provide new OBJs specifically TrenchBroom-compatible and rewriting mapclib to do the remapping. I hate being dictated by a single third party tool, but I see no other way to enable good experience for mappers."
    }
  ]
}
-->
