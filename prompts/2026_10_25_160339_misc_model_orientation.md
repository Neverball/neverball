# Prompt Log Export 2026-10-25 16:03:39 UTC

- **Generator:** [git-prompt-log](https://github.com/parasti/git-prompt-log)
- **Export command:** `git prompt-log export --commit --slug misc_model_orientation`
- **Import command:** `git prompt-log import prompts/2026_10_25_160339_misc_model_orientation.md`

---

- **Session:** `fd1bc80c-2e9f-4d81-a4e3-685442d85f59`
- **Harness:** Antigravity CLI 1.4.7
- **Model:** Gemini 3.8 Flash (High)

## Commits

- `cbe8f679` share/mapclib: Bake static misc_model geometry and support orientation
- `5f8206fc` tests/test_solid: Add test for parented mover orientation

## Steering Prompts

#### [2026-10-24 20:53:35 UTC]

> /plan When an entity has angles set on it in the map editor and is not linked to any path, a dummy path node is inserted at entity origin to piggy back on Neverball entity scene graph and orient the entity correctly in-game. This generally works very well, except in one case: misc_models (and more generally, bodies in Neverball). a misc_model does not have an origin once it is processed by mapc, in other words, it's origin is (0, 0, 0) and its OBJ vertices are transformed to be absolute. so each instance of the same OBJ is actually a different set of vertices. And turn_entity that creates or adjusts the linked path node for an entity, is simply not called for bodies. In simple terms, rotating a misc_model or another entity that ends up a body in Neverball, has no effect in-game. Let's fix that and make the behavior match other entities.

#### [2026-10-24 21:05:44 UTC]

> Honestly, we should not make misc_model a body. It's a point entity, not a brush container.

#### [2026-10-24 21:22:16 UTC]

> Curious about this plan. You're saying we can keep encoding misc_models as bodies no problem?

#### [2026-10-24 21:31:15 UTC]

> Does this cleaner arch you describe actually address the problems you listed?

#### [2026-10-24 21:33:34 UTC]

> Good, makes sense. Integrate this into the plan.

#### [2026-10-24 21:35:27 UTC]

> [Approved] misc_model_orientation_plan.md

Commits:
- `cbe8f679` share/mapclib: Bake static misc_model geometry and support orientation
- `5f8206fc` tests/test_solid: Add test for parented mover orientation

<!-- git-prompt-log:metadata
{
  "version": 1,
  "exported_at": "2026-10-25 16:03:39 UTC",
  "export_command": "git prompt-log export --commit --slug misc_model_orientation",
  "import_command": "git prompt-log import prompts/2026_10_25_160339_misc_model_orientation.md",
  "commits": [
    {
      "hash": "cbe8f679475da8ddf95c44d0ae4711a23d40b001",
      "subject": "share/mapclib: Bake static misc_model geometry and support orientation",
      "note": "Assistant-Session: fd1bc80c-2e9f-4d81-a4e3-685442d85f59\nAssistant-Harness: Antigravity CLI 1.4.7\nAssistant-Model: Gemini 3.8 Flash (High)\nAssistant-Recorded: 2026-10-25 12:09:59 UTC\n\nAssistant-Prompts:\n  [2026-10-24 21:35:27 UTC] [Approved] misc_model_orientation_plan.md\n  [2026-10-24 21:33:34 UTC] Good, makes sense. Integrate this into the plan.\n  [2026-10-24 21:31:15 UTC] Does this cleaner arch you describe actually address the problems you listed?\n  [2026-10-24 21:22:16 UTC] Curious about this plan. You're saying we can keep encoding misc_models as bodies no problem?\n  [2026-10-24 21:05:44 UTC] Honestly, we should not make misc_model a body. It's a point entity, not a brush container.\n  [2026-10-24 20:53:35 UTC] /plan When an entity has angles set on it in the map editor and is not linked to any path, a dummy path node is inserted at entity origin to piggy back on Neverball entity scene graph and orient the entity correctly in-game. This generally works very well, except in one case: misc_models (and more generally, bodies in Neverball). a misc_model does not have an origin once it is processed by mapc, in other words, it's origin is (0, 0, 0) and its OBJ vertices are transformed to be absolute. so each instance of the same OBJ is actually a different set of vertices. And turn_entity that creates or adjusts the linked path node for an entity, is simply not called for bodies. In simple terms, rotating a misc_model or another entity that ends up a body in Neverball, has no effect in-game. Let's fix that and make the behavior match other entities."
    },
    {
      "hash": "5f8206fc2aef1a6935a2f2b884c79b58014925c2",
      "subject": "tests/test_solid: Add test for parented mover orientation",
      "note": "Assistant-Session: fd1bc80c-2e9f-4d81-a4e3-685442d85f59\nAssistant-Harness: Antigravity CLI 1.4.7\nAssistant-Model: Gemini 3.8 Flash (High)\nAssistant-Recorded: 2026-10-25 12:10:02 UTC\n\nAssistant-Prompts:\n  [2026-10-24 21:35:27 UTC] [Approved] misc_model_orientation_plan.md\n  [2026-10-24 21:33:34 UTC] Good, makes sense. Integrate this into the plan.\n  [2026-10-24 21:31:15 UTC] Does this cleaner arch you describe actually address the problems you listed?\n  [2026-10-24 21:22:16 UTC] Curious about this plan. You're saying we can keep encoding misc_models as bodies no problem?\n  [2026-10-24 21:05:44 UTC] Honestly, we should not make misc_model a body. It's a point entity, not a brush container.\n  [2026-10-24 20:53:35 UTC] /plan When an entity has angles set on it in the map editor and is not linked to any path, a dummy path node is inserted at entity origin to piggy back on Neverball entity scene graph and orient the entity correctly in-game. This generally works very well, except in one case: misc_models (and more generally, bodies in Neverball). a misc_model does not have an origin once it is processed by mapc, in other words, it's origin is (0, 0, 0) and its OBJ vertices are transformed to be absolute. so each instance of the same OBJ is actually a different set of vertices. And turn_entity that creates or adjusts the linked path node for an entity, is simply not called for bodies. In simple terms, rotating a misc_model or another entity that ends up a body in Neverball, has no effect in-game. Let's fix that and make the behavior match other entities."
    }
  ]
}
-->
