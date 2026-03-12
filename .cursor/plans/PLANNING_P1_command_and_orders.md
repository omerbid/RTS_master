# Plan: P1 Command Radius + Orders + Input

**Created**: 2026-03-07  
**Status**: Implemented (C++); use ARTSGameModeBase or set BP GameMode to RTSPlayerController + ARTSCameraPawn.  
**GDD**: `.cursor/GDD.md` (MVP Locked Values, Input, Command Range, Development Order P1)

---

## 1. Task Summary

Implement P1: **command radius enforcement**, **persistent orders** (Move; optional Attack), and **mouse-driven input** (LMB select, Shift+LMB add select, RMB context move/attack). Units outside Hero/Captain range ignore **new** orders; existing orders persist when leaving range. WASD/Arrows = map pan only.

**GDD alignment:** HERO-CENTRIC COMMAND DOCTRINE, Command Range (Hero 2500 UU, Captain 1200 UU), Input (MVP), Definition of Done (Move hero, issue orders).

**Post-combat behavior (per unit type):** When a combat order **ends** or the unit **wins** the fight, the unit follows its **PostCombatBehavior** from data: **Advance** (push forward), **Hold** (hold ground), or **Retreat** (fall back). Stored per unit type in `FUnitRow::PostCombatBehavior`; enum `ERTSPostCombatBehavior`.

**Depends on:** P0 done (ARTSUnitCharacter, ARTSHeroCharacter, URTSCommandAuthorityComponent, URTSDataRegistry, DataTables).

---

## 2. Execution Plan

### 2.1 Logic & Architecture

- **Command radius (already in place)**
  - `URTSCommandAuthorityComponent`: Hero default 2500 UU (set in C++). Captain 1200 UU when Captain is implemented (P3); for P1 only Hero issues orders.
  - `CanIssueOrderToUnit(TargetUnit)` already validates distance; add same-faction check if not already implied by design.

- **Orders**
  - **Order type:** At least **Move** (destination in world). Optional: **Attack** (target actor) for RMB on enemy.
  - **Persistence:** Units keep executing current order when leaving command range; only **new** order requests require authority check.
  - **Ownership:** Unit holds "current order" (e.g. Move destination or Attack target). Order execution can be: C++ (e.g. `URTSOrderComponent` or logic on Unit) or Blueprint event driven from C++ API.

- **Selection**
  - **Selection set:** Which units/heroes are selected. Owned by PlayerController or GameState or dedicated Subsystem (e.g. `URTSSelectionSubsystem` / state on `APlayerController`).
  - **LMB:** Select (single hit or box – MVP can start with single click).
  - **Shift+LMB:** Add to selection (multi-select).
  - **Authority for issuing:** When player issues order (RMB), determine **issuer** (e.g. selected Hero, or first selected Captain). Only units **in that issuer’s command radius** receive the new order; others are ignored (no new order).

- **Input (MVP)**
  - **Mouse:** LMB (Select), Shift+LMB (Add select), RMB (Context: Move or Attack).
  - **WASD / Arrows:** Map pan only (camera).
  - Prefer **Enhanced Input** (GDD + UNREAL_LINKS); fallback to legacy input if needed for speed.
  - Hit detection: cursor ray / line trace to world (ground = Move) and to actors (unit = select, enemy = Attack).

- **Post-combat behavior (per unit type)**
  - **When:** Only after combat ends—e.g. Attack order completes (target dead / order cancelled) or unit wins the fight. Not on general idle.
  - **Data:** Per unit type in `FUnitRow::PostCombatBehavior` (`ERTSPostCombatBehavior`: Advance, Hold, Retreat). Read from registry/DataTable by unit type.
  - **Effect:** Advance = move toward next threat/objective; Hold = stop and hold position; Retreat = move away from threat. Applied automatically when combat-end condition fires.

- **Flow**
  1. Player clicks LMB on unit → set selection to that unit (or add with Shift).
  2. Player clicks RMB on ground → get issuer (e.g. selected Hero); for each **selected unit** in **issuer’s command range**, issue **Move** to destination; others stay with current order.
  3. Player clicks RMB on enemy → same but issue **Attack** (if defined in P1 scope).
  4. When Attack/combat ends or unit wins → apply unit’s `PostCombatBehavior` from its unit type data.

### 2.2 Complexity Assessment

| Aspect       | Level  | Notes |
|-------------|--------|--------|
| Logic       | Medium | Selection + authority + order application; clear rules. |
| Integration | Medium | PlayerController, camera (pan), possibly HUD for feedback. |
| Input       | Medium | Enhanced Input + trace; one input config asset. |
| Testing     | Medium | PIE: select, move, leave range, issue again only in range. |

### 2.3 Performance

- No Tick-based selection or order checks; event-driven on click.
- Command radius: one distance check per selected unit when issuing (already O(n)).
- Ray/trace on click only.

### 2.4 Build Strategy

- **Step 1 – Selection:** Add selection storage (e.g. on PlayerController or Subsystem) and C++ API: AddToSelection, SetSelection, GetSelection, ClearSelection. Blueprint-callable if needed.
- **Step 2 – Input:** Add Enhanced Input mapping (LMB, Shift+LMB, RMB; WASD/Arrows for pan). In C++ or Blueprint: on LMB/RMB, run trace; update selection or issue order.
- **Step 3 – Orders:** Define order type (e.g. Move). On Unit: SetCurrentOrder(Move, Destination). Execution: move to destination (AI MoveTo or simple movement); persist until replaced or cancelled.
- **Step 4 – Issue path:** On RMB (ground): get issuer Hero from selection; for each selected unit, if `CanIssueOrderToUnit(unit)`, call SetCurrentOrder(Move, hit location). Optionally broadcast event for Blueprint (e.g. for VFX).
- **Step 5 – Post-combat:** On combat end (Attack order finished / unit won): read unit’s `PostCombatBehavior` from registry (FUnitRow); apply Advance / Hold / Retreat.
- **Step 6 – Captain (optional in P1):** If time, apply 1200 UU for Captain when issuer is Captain; else defer to P3.

GDD: Command Range table, Input (MVP), persistent orders.

---

## 3. Epics & Tasks

### Epic 1: Selection

- [x] Define selection holder (`ARTSPlayerController` with TArray of selected actors).
- [x] API: SetSelection(Unit), AddToSelection(Unit), RemoveFromSelection(Unit), ClearSelection(), GetSelection().
- [x] LMB: trace under cursor; if unit (ARTSUnitCharacter or subclass), SetSelection(hit) or AddToSelection(hit) when Shift.
- [ ] Optional: box select (can be post-MVP or simple rectangle).

### Epic 2: Input (Mouse + Pan)

- [ ] Add Enhanced Input asset / mapping: Left Mouse Button, Shift+Left Mouse Button, Right Mouse Button; WASD + Arrows for map pan only.
- [ ] Wire LMB / Shift+LMB to selection logic (trace → update selection).
- [ ] Wire RMB to “context order”: trace → if ground, prepare Move; if enemy actor, prepare Attack (if in scope).
- [ ] Ensure WASD/Arrows only drive camera pan (no unit control).

### Epic 3: Orders (Move; optional Attack) + Post-combat (per unit type)

- [ ] Define order payload (e.g. Move: FVector destination; Attack: AActor* target). Struct or enum + payload.
- [ ] On Unit: SetCurrentOrder(OrderType, Payload). Stored on unit; execution (e.g. MoveTo) in C++ or Blueprint.
- [ ] Move execution: use AI MoveTo or Character movement toward destination; clear or replace on new order.
- [ ] Optional: Attack order (set target; unit moves toward and attacks when in range).
- [ ] **Post-combat behavior (per unit type):** When combat order ends or unit wins fight, apply unit’s `PostCombatBehavior` from `FUnitRow` (Advance / Hold / Retreat). Implement: Advance = move toward next threat/objective; Hold = stop and hold position; Retreat = move away from threat. Data: `ERTSPostCombatBehavior` in `RTSDataTypes.h`, `FUnitRow::PostCombatBehavior`; add column to DT_Units CSV/DataTable.

### Epic 4: Issue Orders with Authority

- [ ] On RMB (ground): from selection, pick issuer (Hero preferred; else Captain if any). Get hit location as move destination.
- [ ] For each selected unit: if issuer’s `CanIssueOrderToUnit(unit)` and same faction, call unit->SetCurrentOrder(Move, destination).
- [ ] Units not in range: do not change their order (persistent order behavior).
- [ ] Confirm Hero CommandRadius = 2500, Captain = 1200 (from data or C++ default).

### Epic 5: Camera Pan

- [x] WASD / Arrows drive camera movement only (ARTSCameraPawn).
- [ ] Top-down camera; zoom in/out if not already present (optional).

---

## 4. Tests & Validation

- [ ] LMB on unit: selection updates (single).
- [ ] Shift+LMB: unit added to selection.
- [ ] RMB on ground: selected units in Hero range receive Move; units outside range do not get new order.
- [ ] Unit with Move order: moves toward destination; order persists when walking out of range.
- [ ] No keyboard control of units; WASD/Arrows only pan camera.
- [ ] When combat/Attack order ends or unit wins: unit follows its PostCombatBehavior (Advance/Hold/Retreat) from unit type data.

---

## 5. Documentation

- [ ] Update plan status when epics complete.
- [ ] Short comment in C++: selection ownership, order persistence rule (GDD).
- [ ] Blueprint checklist if new PlayerController or HUD Blueprints are created (parent class, input binding).

---

## 6. Dependencies & Risks

**Dependencies:** P0 complete. Input mapping (Enhanced Input) and camera pan (existing or new).

**Risks:**

| Risk | Mitigation |
|------|-------------|
| Enhanced Input vs legacy | Prefer Enhanced Input; if blocking, use legacy for P1 and migrate later. |
| Issuer selection (multiple Heroes) | P1: one Hero per faction; issuer = selected Hero. Later: prefer “main” Hero or first Hero in selection. |
| Order execution (MoveTo) | Use existing AI/Movement (e.g. SimpleMoveToLocation) or Character movement; keep order payload in C++. |

---

## 7. Acceptance Criteria

- [ ] Player can select unit(s) with LMB / Shift+LMB.
- [ ] Player can issue Move order with RMB on ground; only units in Hero command radius (2500 UU) receive the new order.
- [ ] Orders persist when unit leaves range (no automatic cancel).
- [ ] WASD/Arrows pan camera only.
- [ ] No new order for units outside range (they keep current behavior).
- [ ] When combat ends or unit wins: unit follows its unit-type PostCombatBehavior (Advance / Hold / Retreat).

**Definition of Done (P1):** Command radius enforced for new orders; persistent Move order; mouse-driven select and context order; camera pan only on keyboard; post-combat behavior per unit type.

---

## 8. Rollback / Fallback

- If Enhanced Input delays: use legacy input for LMB/RMB and pan; migrate to Enhanced later.
- If order execution is complex: minimal “Move” (set destination; unit uses existing movement/AI) and defer Attack to next phase.
- Selection can live on PlayerController first; extract to Subsystem later if needed.
