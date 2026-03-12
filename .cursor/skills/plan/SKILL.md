---
name: plan
description: Creates a structured planning document for tasks. Use when the user writes "Plan" or asks to plan a task, feature, or implementation. Writes the plan to .cursor/plans/ for later reference and tracking. All planning is based on the GDD.
---

# Plan Skill

When the user says **"Plan"** (or requests planning), create a structured planning document and save it to `.cursor/plans/`.

## GDD – Source of Truth (Critical)

The **GDD (Game Design Document)** is the project's bible. Every plan must be derived from and aligned with it.

- **Location**: `.cursor/GDD.md` (or path specified by user)
- **Before planning**: Read the GDD. Extract relevant sections for the task.
- **During planning**: Reference GDD for mechanics, systems, balance, terminology. Do not contradict it.
- **If GDD missing**: Ask user to provide it before planning.

## Workflow

1. **Load GDD** – Read `.cursor/GDD.md` (or user-specified path). If missing, ask for it.
2. **Clarify the task** – If the task is unclear, ask for specifics before planning.
3. **Create the plan** – Based on GDD. Follow the template below.
4. **Save to file** – Write to `.cursor/plans/PLANNING_[task_name].md` (use short slug for task_name, e.g. `unit_movement`, `resource_system`).
5. **Show the user** – Display the plan content in the response.

## Plan File Location

- **Path**: `.cursor/plans/PLANNING_[task_slug].md`
- **Current task**: Use `PLANNING.md` when no specific name is given.
- **Reference**: Read this file when resuming work to check progress and compare against implementation.

## Plan Template

Use this structure for every plan:

```markdown
# Plan: [Task Name]

**Created**: [Date]
**Status**: Draft | In Progress | Completed

---

## 1. Task Summary

[2-3 sentences describing what we're building and why]

---

## 2. Execution Plan

### 2.1 Logic & Architecture
- Core logic flow
- Key components and their responsibilities
- Data flow and dependencies
- Integration points with existing systems

### 2.2 Complexity Assessment
| Aspect | Level (Low/Medium/High) | Notes |
|--------|-------------------------|-------|
| Logic | | |
| Integration | | |
| Testing | | |
| Maintenance | | |

### 2.3 Performance Considerations
- Expected impact on frame rate / load times
- Bottlenecks to avoid
- Optimization opportunities
- UE-specific: Blueprint vs C++, tick usage, event-driven vs polling

### 2.4 Build Strategy
- Order of implementation (what depends on what)
- How it fits with existing project structure (RTS_Monsters: Blueprints, Core, Data)
- Reuse of existing archetypes, enums, structs
- **GDD alignment**: Cite which GDD sections this implements

---

## 3. Epics & Tasks

Break work into epics. Each epic = trackable milestone.

### Epic 1: [Name]
- [ ] Task 1.1
- [ ] Task 1.2
- [ ] Task 1.3

### Epic 2: [Name]
- [ ] Task 2.1
- [ ] Task 2.2

### Epic 3: [Name]
- [ ] Task 3.1

---

## 4. Tests & Validation

- [ ] Unit/component tests (if applicable)
- [ ] Integration checks
- [ ] Manual test scenarios
- [ ] Edge cases to verify
- [ ] UE: Play-in-editor verification, blueprint validation

---

## 5. Documentation

- [ ] Inline comments for complex logic
- [ ] Blueprint/node documentation
- [ ] README or design doc updates (if needed)
- [ ] Data asset descriptions

---

## 6. Dependencies & Risks

### Dependencies
- External systems / assets
- Prerequisites (other epics that must complete first)

### Risks
| Risk | Mitigation |
|------|------------|
| | |

---

## 7. Acceptance Criteria

- [ ] Criterion 1
- [ ] Criterion 2
- [ ] Definition of "Done"

---

## 8. Rollback / Fallback

- What to revert if implementation fails
- Simpler alternative if scope must be reduced
```

## When Resuming Work

1. Read the plan file from `.cursor/plans/`
2. Compare current implementation against the plan
3. Update checkboxes and status
4. Adjust plan if requirements changed

## RTS_Monsters Context

When planning for this project, consider:
- **Blueprint-only** – No C++ unless explicitly added
- **Structure**: Blueprints, Core (enums/structs), Data (Archetypes, Heroes, Settlements, Rules)
- **Existing systems**: Factions, Heroes, Resources, Morale/Fear, Units, Settlements
- **Engine**: UE 5.4, Top Down template, Enhanced Input
