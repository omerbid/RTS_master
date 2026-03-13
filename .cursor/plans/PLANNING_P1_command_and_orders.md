# Plan: P1 Command Radius + Orders + Input

**Created**: 2026-03-07  
**Updated**: 2026-03-08 (P1 all phases aligned with docs; implementation complete)  
**Status**: ✅ Implemented – Selection, Orders, Camera, Promote, Visual feedback, Command range indicator, Post-combat behavior.  
**GDD**: `.cursor/GDD.md` (MVP Locked Values, Input, Command Range, Development Order P1)

**Related docs:** `P0_P1_P2_REMAINING.md` (מה נשאר) | `P0_P1_QUICK_REFERENCE.md` (Attack, DataTables) | `RTS_TESTS_P0_P1.md` (P1.1–P1.5) | `RTS_ARCHITECTURE.md` | `PROJECT_MAP.md`

---

## P1 All Phases – Overview

| Phase | Name | Status | Notes |
|-------|------|--------|--------|
| **1** | Selection | ✅ Done | ARTSPlayerController: SetSelection, AddToSelection, ClearSelection, GetSelection; LMB/Shift+LMB; **box select** (LMB drag); SetSelected (ring, custom depth). |
| **2** | Input (Mouse + Pan + Promote) | ✅ Done | LMB/RMB, Shift+LMB, Shift+RMB (Promote); WASD/Arrows pan; scroll zoom; middle mouse pitch; BindKey (legacy). |
| **3** | Orders (Move, Attack) + Post-combat | ✅ Done | URTSOrderComponent, FRTSOrderPayload, Move/Attack; PostCombatBehavior (Advance/Hold/Retreat) from FUnitRow. |
| **4** | Issue Orders with Authority | ✅ Done | GetOrderIssuer(), GetHitUnderCursor; CanIssueOrderToUnit; only units in range get new order; persistence when leaving range. |
| **5** | Camera | ✅ Done | ARTSCameraPawn: pan (WASD/Arrows), zoom (scroll), pitch (middle drag); ZoomMin/Max, PitchMin/Max. |
| **6** | P1 Polish | ✅ Done | Selection ring + custom depth; command range indicator (DrawDebugCircle when Hero/Captain selected). |
| — | **Remaining** | Optional / Validation | Run P1 tests (RTS_TESTS_P0_P1.md); optional box select; optional C++ comments / Blueprint checklist. |

---

## Implementation status (aligned with current code)

| Area | Classes / files | Notes |
|------|------------------|--------|
| **Selection** | `ARTSPlayerController` | SelectedActors, SetSelection, AddToSelection, RemoveFromSelection, ClearSelection, GetSelection; LMB trace → SetSelection/AddToSelection; ground click → ClearSelection. |
| **Hit / Issuer** | `ARTSPlayerController` | GetHitUnderCursor (ECC_Visibility + ECC_Pawn); GetOrderIssuer() (first Hero or Captain in selection). |
| **Orders** | `URTSOrderComponent` (on Unit) | FRTSOrderPayload, ERTSOrderType (None, Move, Attack); SetCurrentOrder; MoveTo; Attack target; ApplyPostCombatBehavior (Advance/Hold/Retreat from FUnitRow::PostCombatBehavior). |
| **Authority** | `URTSCommandAuthorityComponent` (on Hero/Captain) | CommandRadius (Hero 2500, Captain 1200); CanIssueOrderToUnit(distance + same faction). |
| **Input** | `ARTSPlayerController` | BindKey: LMB, Shift+LMB, RMB, Shift+RMB (Promote); ARTSCameraPawn: WASD, Arrows, scroll, middle mouse. |
| **Camera** | `ARTSCameraPawn` | Pan, zoom (500–4000), pitch (-85° to -30°). |
| **Visual** | Selection ring (SelectionRingMesh), CustomDepth; command range DrawDebugCircle when Hero/Captain selected. |

**Remaining (per P0_P1_P2_REMAINING):** Run P1 tests from `RTS_TESTS_P0_P1.md` (P1.1–P1.5); optionally add box select (post-MVP).

---

## 1. Task Summary

Implement P1: **command radius enforcement**, **persistent orders** (Move, Attack), **mouse-driven input** (LMB select, Shift+LMB add select, RMB context move/attack), **Promote to Captain** (Shift+RMB), **visual feedback** (selection ring, custom depth), **command range indicator**, and **camera** (WASD pan, zoom, pitch). Units outside Hero/Captain range ignore **new** orders; existing orders persist when leaving range.

**GDD alignment:** HERO-CENTRIC COMMAND DOCTRINE, Command Range (Hero 2500 UU, Captain 1200 UU), Input (MVP), Definition of Done (Move hero, Promote captain, issue orders).

**Post-combat behavior (per unit type):** When a combat order **ends** or the unit **wins** the fight, the unit follows its **PostCombatBehavior** from data: **Advance** (push forward), **Hold** (hold ground), or **Retreat** (fall back). Stored per unit type in `FUnitRow::PostCombatBehavior`; enum `ERTSPostCombatBehavior`. Implemented in `URTSOrderComponent::ApplyPostCombatBehavior()`.

**Depends on:** P0 done (ARTSUnitCharacter, ARTSHeroCharacter, URTSCommandAuthorityComponent, URTSDataRegistry, DataTables).  
**Feeds into:** P3 (Captain, Squad), P4 (Economy – selection for Recruit).

---

## 2. Execution Plan

### 2.1 Logic & Architecture

- **Command radius (in place)**
  - `URTSCommandAuthorityComponent`: Hero 2500 UU, Captain 1200 UU (P3). `GetOrderIssuer()` returns first Hero or Captain in selection.
  - `CanIssueOrderToUnit(TargetUnit)` validates distance and same-faction.

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
  - **Mouse:** LMB (Select), Shift+LMB (Add select), RMB (Context: Move or Attack), **Shift+RMB** (Promote to Captain on selected unit).
  - **WASD / Arrows:** Map pan only (camera).
  - **Scroll wheel:** Zoom in/out. **Middle mouse + drag:** Camera pitch.
  - Implemented via legacy `BindKey` (InputComponent). Hit detection: `GetHitUnderCursor` – ECC_Visibility + ECC_Pawn fallback.

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
- **Step 6 – Captain:** GetOrderIssuer returns Captain when no Hero; Captain has CommandAuthority 1200 (P3). Promote input: Shift+RMB (P1).

GDD: Command Range table, Input (MVP), persistent orders.

---

## 3. Epics & Tasks

### Epic 1: Selection

- [x] Define selection holder (`ARTSPlayerController` with TArray of selected actors).
- [x] API: SetSelection(Unit), AddToSelection(Unit), RemoveFromSelection(Unit), ClearSelection(), GetSelection().
- [x] LMB: trace under cursor; if unit (ARTSUnitCharacter or subclass), SetSelection(hit) or AddToSelection(hit) when Shift.
- [x] SetSelected(bSelected) on selection change – visual feedback (ring, custom depth).
- [x] Optional: box select (LMB drag → units in screen rect; Shift+drag = add to selection).

### Epic 2: Input (Mouse + Pan + Promote)

- [x] LMB: trace → selection; Shift+LMB: add to selection.
- [x] RMB: context order (Move on ground, Attack on enemy, Move on same-faction unit).
- [x] **Shift+RMB:** Promote to Captain on selected unit (same faction, Rank ≥3, not Hero).
- [x] WASD / Arrows: camera pan only (no unit control).
- [x] Scroll wheel: zoom in/out. Middle mouse + drag: camera pitch.
- [x] Input via BindKey (legacy); Enhanced Input optional for future.

### Epic 3: Orders (Move, Attack) + Post-combat (per unit type)

- [x] Order payload: `FRTSOrderPayload` (MoveDestination, AttackTarget); `ERTSOrderType` (None, Move, Attack).
- [x] Unit: SetCurrentOrder(OrderType, Payload). Stored on `URTSOrderComponent`.
- [x] Move execution: MoveTo in OrderComponent; Attack order supported.
- [x] **Post-combat behavior (per unit type):** When combat order ends or unit wins fight, apply unit’s `PostCombatBehavior` from `FUnitRow` (Advance / Hold / Retreat). Implement: Advance = move toward next threat/objective; Hold = stop and hold position; Retreat = move away from threat. Data: `ERTSPostCombatBehavior` in `RTSDataTypes.h`, `FUnitRow::PostCombatBehavior`; add column to DT_Units CSV/DataTable.

### Epic 4: Issue Orders with Authority

- [x] On RMB: GetOrderIssuer() (Hero preferred; else Captain). Get hit location from GetHitUnderCursor.
- [x] For each selected unit: if issuer’s `CanIssueOrderToUnit(unit)` and same faction, call unit->SetCurrentOrder(Move, destination).
- [x] Units not in range: no new order (persistent order behavior).
- [x] Hero CommandRadius = 2500, Captain = 1200 (from C++ / P3).

### Epic 5: Camera

- [x] WASD / Arrows: camera pan only (ARTSCameraPawn).
- [x] Zoom in/out (scroll wheel; ZoomMin 500, ZoomMax 4000).
- [x] Camera pitch (middle mouse + drag; PitchMin -85°, PitchMax -30°).

### Epic 6: P1 Polish – Visual Feedback & Command Range

- [x] **Selection visual:** SelectionRingMesh (flat cylinder at feet) + SetRenderCustomDepth on mesh when selected.
- [x] **Command range indicator:** DrawDebugCircle when Hero/Captain selected (PlayerTick).

---

## 4. Tests & Validation

- [x] LMB on unit: selection updates (single).
- [x] Shift+LMB: unit added to selection.
- [x] RMB on ground: selected units in Hero range receive Move; units outside range do not get new order.
- [x] Unit with Move order: moves toward destination; order persists when walking out of range.
- [x] No keyboard control of units; WASD/Arrows only pan camera.
- [x] When combat/Attack order ends or unit wins: unit follows its PostCombatBehavior (Advance/Hold/Retreat) from unit type data.
- [ ] **Run full P1 test matrix:** Execute P1.1–P1.5 from `RTS_TESTS_P0_P1.md` (manual or Automation) and mark pass/fail.

---

## 5. Documentation

- [x] Update plan status when epics complete.
- [x] (Optional) Short comment in C++: selection ownership, order persistence rule (GDD).
- [x] (Optional) Blueprint checklist: see P1_BLUEPRINT_CHECKLIST below.

---

## 6. Dependencies & Risks

**Dependencies:** P0 complete (Unit, Hero, CommandAuthority, DataRegistry, DataTables). Camera and input implemented in P1 (ARTSCameraPawn, ARTSPlayerController BindKey). **P2:** Secure Region input can call `GetOrderIssuer()->TryStartSecureRegion()` from same controller.

**Risks:**

| Risk | Mitigation |
|------|-------------|
| Enhanced Input vs legacy | Prefer Enhanced Input; if blocking, use legacy for P1 and migrate later. |
| Issuer selection (multiple Heroes) | P1: one Hero per faction; issuer = selected Hero. Later: prefer “main” Hero or first Hero in selection. |
| Order execution (MoveTo) | Use existing AI/Movement (e.g. SimpleMoveToLocation) or Character movement; keep order payload in C++. |

---

## 7. Acceptance Criteria

- [x] Player can select unit(s) with LMB / Shift+LMB.
- [x] Player can issue Move/Attack order with RMB; only units in Hero/Captain command radius receive the new order.
- [x] Orders persist when unit leaves range (no automatic cancel).
- [x] WASD/Arrows pan camera only; zoom (scroll), pitch (middle mouse).
- [x] No new order for units outside range (they keep current behavior).
- [x] When combat ends or unit wins: unit follows its unit-type PostCombatBehavior (Advance / Hold / Retreat).
- [x] Shift+RMB: Promote to Captain (Rank ≥3). Selection visual (ring, custom depth). Command range indicator.

**Definition of Done (P1):** ✅ Command radius enforced; persistent Move/Attack; mouse-driven select and context order; Promote to Captain; camera pan/zoom/pitch; visual feedback; post-combat behavior.

---

## 8. Rollback / Fallback

- If Enhanced Input delays: use legacy input for LMB/RMB and pan; migrate to Enhanced later.
- If order execution is complex: minimal “Move” (set destination; unit uses existing movement/AI) and defer Attack to next phase.
- Selection can live on PlayerController first; extract to Subsystem later if needed.

---

## P1 Blueprint checklist (optional)

When creating or editing Blueprints that use P1 systems:

| Blueprint | Parent class | Notes |
|-----------|--------------|--------|
| **PlayerController** | `RTSPlayerController` (C++) | Input (LMB/RMB, S for Secure) and selection are in C++; override only if adding UI or extra bindings. |
| **Unit / Hero** | `RTSUnitCharacter` / `RTSHeroCharacter` | Set **Unit Id** / **Hero Id** per instance in level (Details); do not rely on Class Defaults for different types. |
| **HUD / Unit info** | As needed | Use `Get Selection` from PlayerController; refresh on selection change. |

---

## 9. Related Documents

- `P0_P1_P2_REMAINING.md` – P1 core complete; remaining: run P1 tests (RTS_TESTS_P0_P1).
- `P0_P1_QUICK_REFERENCE.md` – DataTables paths; how to trigger Attack (RMB on enemy); Unit/Hero overrides.
- `RTS_TESTS_P0_P1.md` – P1.1 Selection, P1.2 Move, P1.3 Attack, P1.4 Input, P1.5 Authority; run and mark ✓.
- `P3_DEPENDENCIES_STATUS.md` – Captain, Squad, Morale (P3); Promote input bound in P1.
- `PLANNING_P4_economy.md` – Economy depends on P1 selection for Recruit flow.
- `PLANNING_P2_region_control.md` – P2 Secure Region: bind input to `GetOrderIssuer()->TryStartSecureRegion()`.
