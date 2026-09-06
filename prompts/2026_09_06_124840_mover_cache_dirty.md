# Prompt Log Export 2026-09-06 12:48:40 UTC

- **Generator:** [git-prompt-log](https://github.com/parasti/git-prompt-log)
- **Export command:** `git prompt-log export --slug mover_cache_dirty`
- **Import command:** `git prompt-log import prompts/2026_09_06_124840_mover_cache_dirty.md`

---

- **Session:** `271352cd-c1ad-4372-96e7-92110530e76c`
- **Harness:** Antigravity CLI 1.1.27
- **Model:** Gemini 3.8 Flash (High)

## Commits

- `517ec6aa` tests: Add mover hierarchy and transform cache tests
- `ae0e40f8` share/solid_vary: Propagate dirty state to dependent movers

## Steering Prompts

#### [2026-09-03 22:32:22 UTC]

> /plan Community member MightyBurger reports that the mover transform cache/dirty setup breaks their map. See ../bug-20260904 and investigate.

#### [2026-09-03 22:43:24 UTC]

> Let me rephrase so you can tell me if I understand. Scene graph transforms are recalculated on demand - when something needs it, the transform is calculated and the dirty flags are cleared. So far, so good. What I don't understand is why would a parent moving invalidate a non-moving child - that is the real bug IMO. A parent changing transform should automatically move the child with it - the child's transform should apply within the local space of the parent. At least logically - perhaps in Neverball it is not supposed to work like that, I don't recall.

#### [2026-09-03 22:46:35 UTC]

> Okay, so yeah, the correct thing is - when a platform is marked dirty, all its children should be marked dirty.

#### [2026-09-03 22:48:23 UTC]

> But the code in the plan is more complex that we just discussed. Why?

#### [2026-09-03 22:49:37 UTC]

> "path nodes in a chain share a parent" - this ain't really true.

#### [2026-09-04 06:18:34 UTC]

> "path nodes in a chain share a parent" - this ain't really true.

#### [2026-09-04 06:20:34 UTC]

> Think we need to start with extensive red/green tests because this is just hypotheticals.

#### [2026-09-04 07:59:00 UTC]

> is_path_parent isn't even checking parents? It's just checking if the mover is acting on behalf of a path node.

#### [2026-09-04 08:01:36 UTC]

> I don't really view that as "parent". When a path node is attached via target2 or target3 to another node, yes, that other node is the "parent" node. But the movers aren't "parent movers", they're a helper structure that navigates a path chain.

#### [2026-09-04 08:03:56 UTC]

> Yes, build the tests.

Commits:
- `517ec6aa` tests: Add mover hierarchy and transform cache tests

#### [2026-09-04 08:09:43 UTC]

> On to the fix.

#### [2026-09-04 08:44:26 UTC]

> I don't understand how to read move_depends_on. It's using mi and mj names. Is it checking ancestors - in which order? The parameters come in backwards. It usually makes more sense to use better names for parameters when there are multiple.

#### [2026-09-04 08:51:19 UTC]

> is_move_dependent(fp, mi, mj)

#### [2026-09-04 08:55:06 UTC]

> Sorry, that actually has the same problem, due to the nature of English. move_drives makes more sense positionally.

#### [2026-09-04 09:03:54 UTC]

> is_move_driving to match up with is_move_dirty

Commits:
- `ae0e40f8` share/solid_vary: Propagate dirty state to dependent movers

<!-- git-prompt-log:metadata
{
  "version": 1,
  "exported_at": "2026-09-06 12:48:40 UTC",
  "export_command": "git prompt-log export --slug mover_cache_dirty",
  "import_command": "git prompt-log import prompts/2026_09_06_124840_mover_cache_dirty.md",
  "commits": [
    {
      "hash": "517ec6aa5d6b5622baf979deab0377a34b42c99e",
      "subject": "tests: Add mover hierarchy and transform cache tests",
      "note": "Assistant-Session: 271352cd-c1ad-4372-96e7-92110530e76c\nAssistant-Harness: Antigravity CLI 1.1.27\nAssistant-Model: Gemini 3.8 Flash (High)\nAssistant-Recorded: 2026-09-06 12:41:42 UTC\n\nAssistant-Prompts:\n  [2026-09-04 08:03:56 UTC] Yes, build the tests.\n  [2026-09-04 08:01:36 UTC] I don't really view that as \"parent\". When a path node is attached via target2 or target3 to another node, yes, that other node is the \"parent\" node. But the movers aren't \"parent movers\", they're a helper structure that navigates a path chain.\n  [2026-09-04 07:59:00 UTC] is_path_parent isn't even checking parents? It's just checking if the mover is acting on behalf of a path node.\n  [2026-09-04 06:20:34 UTC] Think we need to start with extensive red/green tests because this is just hypotheticals.\n  [2026-09-04 06:18:34 UTC] \"path nodes in a chain share a parent\" - this ain't really true.\n  [2026-09-03 22:49:37 UTC] \"path nodes in a chain share a parent\" - this ain't really true.\n  [2026-09-03 22:48:23 UTC] But the code in the plan is more complex that we just discussed. Why?\n  [2026-09-03 22:46:35 UTC] Okay, so yeah, the correct thing is - when a platform is marked dirty, all its children should be marked dirty.\n  [2026-09-03 22:43:24 UTC] Let me rephrase so you can tell me if I understand. Scene graph transforms are recalculated on demand - when something needs it, the transform is calculated and the dirty flags are cleared. So far, so good. What I don't understand is why would a parent moving invalidate a non-moving child - that is the real bug IMO. A parent changing transform should automatically move the child with it - the child's transform should apply within the local space of the parent. At least logically - perhaps in Neverball it is not supposed to work like that, I don't recall.\n  [2026-09-03 22:32:22 UTC] /plan Community member MightyBurger reports that the mover transform cache/dirty setup breaks their map. See ../bug-20260904 and investigate."
    },
    {
      "hash": "ae0e40f8abebb707930fd7a4584447248eb0e541",
      "subject": "share/solid_vary: Propagate dirty state to dependent movers",
      "note": "Assistant-Session: 271352cd-c1ad-4372-96e7-92110530e76c\nAssistant-Harness: Antigravity CLI 1.1.27\nAssistant-Model: Gemini 3.8 Flash (High)\nAssistant-Recorded: 2026-09-06 12:41:44 UTC\n\nAssistant-Prompts:\n  [2026-09-04 09:03:54 UTC] is_move_driving to match up with is_move_dirty\n  [2026-09-04 08:55:06 UTC] Sorry, that actually has the same problem, due to the nature of English. move_drives makes more sense positionally.\n  [2026-09-04 08:51:19 UTC] is_move_dependent(fp, mi, mj)\n  [2026-09-04 08:44:26 UTC] I don't understand how to read move_depends_on. It's using mi and mj names. Is it checking ancestors - in which order? The parameters come in backwards. It usually makes more sense to use better names for parameters when there are multiple.\n  [2026-09-04 08:09:43 UTC] On to the fix.\n  [2026-09-04 08:03:56 UTC] Yes, build the tests.\n  [2026-09-04 08:01:36 UTC] I don't really view that as \"parent\". When a path node is attached via target2 or target3 to another node, yes, that other node is the \"parent\" node. But the movers aren't \"parent movers\", they're a helper structure that navigates a path chain.\n  [2026-09-04 07:59:00 UTC] is_path_parent isn't even checking parents? It's just checking if the mover is acting on behalf of a path node.\n  [2026-09-04 06:20:34 UTC] Think we need to start with extensive red/green tests because this is just hypotheticals.\n  [2026-09-04 06:18:34 UTC] \"path nodes in a chain share a parent\" - this ain't really true.\n  [2026-09-03 22:49:37 UTC] \"path nodes in a chain share a parent\" - this ain't really true.\n  [2026-09-03 22:48:23 UTC] But the code in the plan is more complex that we just discussed. Why?\n  [2026-09-03 22:46:35 UTC] Okay, so yeah, the correct thing is - when a platform is marked dirty, all its children should be marked dirty.\n  [2026-09-03 22:43:24 UTC] Let me rephrase so you can tell me if I understand. Scene graph transforms are recalculated on demand - when something needs it, the transform is calculated and the dirty flags are cleared. So far, so good. What I don't understand is why would a parent moving invalidate a non-moving child - that is the real bug IMO. A parent changing transform should automatically move the child with it - the child's transform should apply within the local space of the parent. At least logically - perhaps in Neverball it is not supposed to work like that, I don't recall.\n  [2026-09-03 22:32:22 UTC] /plan Community member MightyBurger reports that the mover transform cache/dirty setup breaks their map. See ../bug-20260904 and investigate."
    }
  ]
}
-->
