---
name: execute
description: Executes implementation according to an existing plan. Use when the user writes "Execute" or asks to implement, build, or run the plan. Reads from .cursor/plans/, performs tasks in order, and updates progress. Implementation must align with the GDD.
---

# Execute Skill

When the user says **"Execute"** (or requests implementation), run the plan from `.cursor/plans/` and implement the tasks.

## GDD – Source of Truth (Critical)

The **GDD (Game Design Document)** is the project's bible. Every implementation must align with it.

- **Location**: `.cursor/GDD.md` (or path specified by user)
- **Before executing**: Ensure the plan references GDD. When in doubt, consult GDD for mechanics, values, terminology.
- **During implementation**: Match GDD specs. Do not deviate without user approval.
- **If GDD contradicts plan**: Prefer GDD. Flag the conflict to the user.

## Workflow

1. **Load the plan** – Read from `.cursor/plans/PLANNING.md` or `PLANNING_[task_slug].md` (ask if multiple exist).
2. **Load GDD** – Read `.cursor/GDD.md` for reference. Use when task details are ambiguous.
3. **Verify readiness** – Check dependencies, prerequisites. If blocked, report and stop.
4. **Execute by epics** – Work through epics in order. Complete all tasks in an epic before moving to the next.
5. **Update progress** – Mark tasks `[x]` as done in the plan file after each completed task.
6. **Report** – Summarize what was done and what remains.

## Execution Rules

### Order
- Follow **Epic order** (Epic 1 → 2 → 3...)
- Follow **Build Strategy** from section 2.4
- Respect **Dependencies** from section 6

### Per Task
1. Read the task description
2. Implement (create/edit files, blueprints, etc.)
3. Mark `[x]` in the plan
4. If task fails: document why, consider Rollback from section 8, ask user

### Batch Size
- Complete **one epic per execution round** unless the user asks for more
- After each epic: update plan, run validation from section 4 if applicable
- Report progress before stopping

## Plan File Updates

After completing tasks, update the plan:

```markdown
- [x] Task 1.1   ← completed
- [ ] Task 1.2   ← next
```

Update **Status** when appropriate:
- `Draft` → `In Progress` (when first task starts)
- `In Progress` → `Completed` (when all epics done and acceptance criteria met)

## If No Plan Exists

1. Check `.cursor/plans/` for any `PLANNING*.md`
2. If none: tell the user "No plan found. Create one with 'Plan' first."
3. Do not guess or invent a plan

## If Plan Is Unclear

- Ambiguous task → ask user for clarification before implementing
- Missing details → use "Build Strategy" and "Logic & Architecture" to infer, document assumptions

## RTS_Monsters Execution Notes

- **Blueprints**: Create/edit in Content/RTS/ per structure (Blueprints/, Data/, Core/)
- **Data assets**: Use existing archetypes (DA_FactionArchetype, etc.) as parents
- **Core types**: Add enums/structs in Core/ when needed
- **Testing**: Play-in-editor, verify blueprints compile, check for errors in Output Log

## Blueprint Creation Checklist (Required Output)

**I cannot edit .uasset files** – they are binary. When tasks require Blueprints, output a **Blueprint Creation Checklist** for the user to create in Unreal Editor.

### Format

At the end of each Epic (or when Blueprints are needed), output:

```markdown
## Blueprint Creation Checklist – [Epic/Task Name]

Create these in Unreal Editor (Content/RTS/...):

### 1. BP_[ClassName]
- **Path**: Content/RTS/Blueprints/BP_[ClassName].uasset
- **Parent**: [Actor/Pawn/Character/... or existing BP]
- **Variables**:
  - VariableName (Type) – Description
- **Functions/Events**:
  - EventName – [Logic in pseudocode or step-by-step]
- **Notes**: [Any connections, delegates, etc.]

### 2. DA_[DataAssetName]
- **Path**: Content/RTS/Data/.../DA_[Name].uasset
- **Parent**: DA_[Archetype] (e.g. DA_FactionArchetype)
- **Properties to set**: [List with values if known]

...
```

### Rules

- **One checklist per Epic** that involves Blueprints
- Be **specific**: exact paths, parent classes, variable types
- For logic: describe in **pseudocode** or numbered steps the user can implement as nodes
- User creates these manually in the editor; I handle config files, C++, and text-based assets

## Completion Checklist

Before marking plan as Completed:
- [ ] All epics' tasks done
- [ ] Section 4 (Tests & Validation) verified
- [ ] Section 5 (Documentation) done
- [ ] Section 7 (Acceptance Criteria) met
