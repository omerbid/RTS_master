# Plan: P3 Squad + Morale (Container, 0–100, Detached Behavior)

**Created**: 2026-03  
**Status**: **Implemented**  
**GDD**: `.cursor/GDD.md` (Combat Rules, Morale 0–100, Detached units, Captain, MVP Locked Values P3)

---

## Implementation status (current codebase)

| Epic | Status | Main files / APIs |
|------|--------|-------------------|
| **Epic 1 – Squad container** | Done | `RTSSquadState` (CaptainUnit, SetCaptain, AddMember/RemoveMember, ApplyMoraleDeltaToAll); `RTSSquadManagerSubsystem` (GetOrCreateSquadForFaction, AddUnitToFactionSquad); `RTSUnitCharacter` (SquadReference, SetSquad). Unit `BeginPlay` → AddUnitToFactionSquad; `Destroyed` → Captain -10, RemoveMember, casualty -5. |
| **Epic 2 – Morale thresholds** | Done | `RTSUnitCharacter`: UpdateMoraleEffects (1s) – MaxWalkSpeed × 0.85 when morale &lt; 30; GetOrderResponsivenessMultiplier; `RTSOrderComponent` uses it in TickMove. Morale &lt; 15: 20% every 5s → TickLowMoraleAutoRetreat (in UpdateDetachedAndDrain). |
| **Epic 3 – Morale sources** | Done | Casualty: -5 on member death (ApplyMoraleDeltaToAll). Detached drain: -1/5s in UpdateDetachedAndDrain. Hero presence buff: **optional**, not implemented. |
| **Epic 4 – Detached behavior** | Done | UpdateDetachedAndDrain (5s): no Hero 2500 / Captain 1200 in range → bIsDetached; drain -1/5s. SetCurrentOrder(Attack) rejected when bIsDetached. |
| **Epic 5 – Captain** | Done | bIsCaptain, Rank ≥3, TryPromoteToCaptain (adds CommandAuthority 1200, SetCaptain on squad). **Input:** Shift+RMB on selected unit (with Hero as issuer) → TryPromoteToCaptain. Captain death → ApplyMoraleDeltaToAll(-10). |

**Constants in code:** CaptainDeathMoralePenalty = -10, CasualtyMoralePenalty = -5, MoraleThresholdLow = 30, MoraleThresholdCritical = 15, MoraleSpeedMultiplierLow = 0.85, DetachedMoraleDrainPer5s = -1, HeroCommandRadius = 2500, CaptainCommandRadius = 1200.

### Squad morale rating (weighted by Rank / Level)

הסקוואד מחשב **רייטינג מורל משותף** (לא ממוצע פשוט): כל לוחם תורם לפי Rank (ובהמשך Level).  
**נוסחה:** `rating = sum(morale_i × weight_i) / sum(weight_i)` (ממוצע משוקלל).

- **למה לחלק ב־sum(weight_i) ולא במספר הלוחמים:** כך לוחם וותיק (משקל גבוה) "שווה" יותר לרייטינג – סקוואד עם הרבה טירונים ו־וותיק אחד משקף יותר את המורל של הוותיק.
- **משקלות (Level + Rank):** Level 1 R1–R5 → 0.75–1.0; Level 2 R1–R5 → 0.85–1.15; Level 3+ R1–R5 → 0.95–1.3. `FUnitRow::Level` + `ARTSUnitCharacter::Level` (מהדאטה); ברירת מחדל 1.
- **באף/דיבאף:** שאר האפקטים (נפגעים, קפטן, מנותק, אופציונלי גיבור) ממשיכים לחול על **מורל היחיד** (CurrentMorale); רייטינג הסקוואד (AverageMorale) משמש ל־UI וללוגיקה ברמת סקוואד.

**מימוש:** `RTSSquadState::RecalcMorale()` + `GetMoraleContributionWeight(Level, Rank)`; Unit מחזיק `Level` (מ־FUnitRow, ברירת מחדל 1).

---

## 1. Task Summary

Implement P3: **squad as container** (units belong to squads; squad has members, captain reference, `RecalcMorale()`), **morale 0–100** with GDD thresholds (<30: -15% move/order responsiveness; <15: 20% every 5s auto-retreat), **morale sources** (casualties, Hero presence, detached state), **detached behavior** (no Hero/Captain in range → defensive AI only, morale drain -1/5s), and **Captain** (unit with `bIsCaptain`, Rank ≥3 eligible, 1200 UU command range, death = squad morale penalty).

**GDD alignment:** COMBAT RULES (Morale affects speed, order execution, auto-retreat), HERO-CENTRIC COMMAND (Captain local range), Squad (Members, SquadMorale, Captain, RecalcMorale), Detached units (defensive AI, morale drain).

**Depends on:** P0 done (URTSSquadState, URTSMoraleComponent, ARTSUnitCharacter, ARTSHeroCharacter). P1 recommended (selection, orders). P2 optional (region; not required for P3).

---

## 2. Execution Plan

### 2.1 Logic & Architecture

- **Squad container (URTSSquadState – existing)**
  - Already: SquadId, FactionId, AverageMorale, Members (weak), AddMember, RemoveMember, RecalcMorale, GetMembers().
  - **Add / wire:**
    - **Captain reference:** Optional `TWeakObjectPtr<ARTSUnitCharacter>` CaptainUnit; set when a unit in the squad is promoted to Captain. Expose getter for Blueprint if needed.
    - **Squad owner:** Ensure a manager (GameState or subsystem) creates squads and assigns units. Units need a **SquadReference** (e.g. `TWeakObjectPtr<URTSSquadState>` or URTSSquadState*) on ARTSUnitCharacter so they know their squad. When a unit is added to a squad, set unit’s SquadReference; when removed (death or leave), clear and call squad->RemoveMember.
    - **RecalcMorale:** Call when: member added/removed, or any member’s morale component changes (optional: event from MoraleComponent or periodic poll). AverageMorale = sum(CurrentMorale) / count; optionally apply “strong lift weak / weak drag strong” per GDD if defined.

- **Unit ↔ Squad**
  - **ARTSUnitCharacter:** Add `SquadReference` (URTSSquadState* or TWeakObjectPtr). BlueprintReadOnly. Set by squad manager when unit joins; cleared when unit leaves or is destroyed.
  - **On unit death:** In Destroyed or TakeDamage path: if unit has SquadReference, call squad->RemoveMember(this) and optionally apply morale delta to remaining members (e.g. -5 per casualty from data or constant).
  - **Squad assignment:** At spawn or via “assign to squad” action: manager adds unit to a squad (create squad if needed); unit’s SquadReference = that squad; squad->AddMember(unit); squad->RecalcMorale().

- **Morale thresholds (0–100)**
  - **Data:** Use DT_MoraleThresholds (FMoraleThresholdRow: Name, Min, Max, Effect) or hardcode for P3: <30 → “Low”, <15 → “Critical”.
  - **Effects (GDD):**
    - **&lt; 30:** -15% Move Speed, -15% Order responsiveness (e.g. delay before executing new order, or slower attack/move).
    - **&lt; 15:** 20% chance every 5 seconds to **Auto-Retreat** (unit cancels order and moves toward friendly Hero/region or fallback).
  - **Implementation:** On Unit (or a small “morale applier” that runs periodically / on morale change): read CurrentMorale from MoraleComponent; if &lt; 30 apply 0.85f to move speed and an “order responsiveness” multiplier; if &lt; 15 register a 5s timer that with 20% probability triggers Auto-Retreat. Movement: either scale CharacterMovementComponent max speed by morale factor, or a multiplier on the Unit that movement reads. Order responsiveness: when issuing or ticking order, apply delay or speed factor from morale.

- **Morale sources**
  - **Casualties:** When a squad member dies, apply morale delta to each remaining member (e.g. ApplyMoraleDelta(-5)); value can come from DataTable or constant.
  - **Hero presence:** When a Hero (same faction) is within command range of the unit, optional morale buff (e.g. +0.5 per second up to cap, or one-time bump when Hero enters range). Use CommandAuthorityComponent or distance check to Hero.
  - **Detached state:** When no Hero and no Captain in command range, apply **morale drain -1 every 5 seconds** (timer on unit or subsystem). “In range” = any same-faction Hero/Captain with CanIssueOrderToUnit(unit) or distance &lt; command radius.

- **Detached behavior**
  - **Definition:** Unit is **detached** if there is no same-faction Hero or Captain within that authority’s command range.
  - **Rules:** Defensive AI only (do not advance to attack; hold position or retreat toward friendly). Morale drain -1/5s (above). Low morale thresholds still apply (speed, auto-retreat).
  - **Implementation:** Query periodically or on tick: “is any Hero/Captain of my faction within 2500 (Hero) or 1200 (Captain)?” If no → set bIsDetached = true; apply drain; and set behavior to “defensive” (e.g. cancel Attack orders, or AI only accepts Hold/Retreat). If yes → bIsDetached = false; stop drain; normal orders allowed.

- **Captain**
  - **Eligibility:** Unit with Rank ≥ 3 (from data or runtime rank) can be promoted. GDD: “Player manually promotes unit.”
  - **Data:** Add or use `bIsCaptain` on ARTSUnitCharacter; add `URTSCommandAuthorityComponent` to Unit when promoted (or use same component with 1200 UU when bIsCaptain).
  - **Promotion:** Input: e.g. right-click selected unit → “Promote to Captain” or key + click. When applied: set bIsCaptain = true; ensure unit has CommandAuthorityComponent with CommandRadius = 1200; optionally set as squad’s Captain (squad->CaptainUnit = this).
  - **Death:** When Captain dies: apply morale penalty to squad (e.g. squad->ApplyMoraleDeltaToAll(-10) or per-member -10). Then RemoveMember(Captain); RecalcMorale.

### 2.2 Complexity Assessment

| Aspect       | Level  | Notes |
|-------------|--------|--------|
| Logic       | Medium | Squad membership, morale thresholds, detached check, Captain lifecycle. |
| Integration | Medium | Unit, Hero, CommandAuthority, MoraleComponent, GameState/subsystem. |
| Testing     | Medium | PIE: squad add/remove, morale thresholds (speed, auto-retreat), detached drain, Captain promote/death. |

### 2.3 Performance

- Detached check: not every frame for every unit; use timer (e.g. every 2–5s) or event when “authority” enters/leaves range.
- RecalcMorale: on membership/morale change, not every tick.
- Auto-retreat 5s timer: one timer per unit when morale <15, or shared subsystem timer that iterates low-morale units.

### 2.4 Build Strategy

- **Step 1 – Squad ↔ Unit:** Add SquadReference to ARTSUnitCharacter. Ensure squad manager (or GameState) creates squads and assigns units; AddMember/RemoveMember set/clear SquadReference. On unit death, RemoveMember and optional casualty morale delta.
- **Step 2 – Morale thresholds:** Implement threshold reader (from DT or constants). Apply <30: move speed and order responsiveness multiplier. Implement <15: 5s timer, 20% auto-retreat.
- **Step 3 – Morale sources:** Casualty delta on squad member death; Hero presence buff (optional); detached drain -1/5s when no Hero/Captain in range.
- **Step 4 – Detached behavior:** Compute bIsDetached (no authority in range); when detached: apply drain, switch to defensive-only behavior (no new Attack; hold or retreat).
- **Step 5 – Captain:** bIsCaptain, Rank eligibility, CommandAuthority 1200 on Unit, promotion input, Captain ref on squad, Captain death = squad morale penalty.

---

## 3. Epics & Tasks

### Epic 1: Squad Container & Unit Reference

- [x] Add `SquadReference` (e.g. `TWeakObjectPtr<URTSSquadState>`) to ARTSUnitCharacter; BlueprintReadOnly getter.
- [x] Ensure a squad manager (GameState or subsystem) creates URTSSquadState instances and assigns units (AddMember). On AddMember: set unit->SquadReference = this squad. → **RTSSquadManagerSubsystem**; Unit BeginPlay calls AddUnitToFactionSquad.
- [x] On RemoveMember: clear unit->SquadReference (if unit still valid).
- [x] When a unit is destroyed (or OnDeath): if SquadReference valid, call RemoveMember(this) and apply casualty morale delta to remaining members (e.g. -5 each or from data). → Destroyed(): Captain -10, RemoveMember, then ApplyMoraleDeltaToAll(-5).
- [x] Add optional **CaptainUnit** (TWeakObjectPtr<ARTSUnitCharacter>) to URTSSquadState; set when a member is promoted to Captain.

### Epic 2: Morale Thresholds (Speed, Responsiveness, Auto-Retreat)

- [x] Define or read thresholds: &lt;30 (Low), &lt;15 (Critical). Use DT_MoraleThresholds or constants.
- [x] **&lt; 30:** Apply -15% move speed (e.g. scale CharacterMovementComponent MaxWalkSpeed by 0.85 when CurrentMorale &lt; 30). Apply -15% order responsiveness (e.g. delay before executing new order, or multiplier on order tick).
- [x] **&lt; 15:** Every 5 seconds, with 20% probability, trigger **Auto-Retreat** (cancel current order; issue Move toward friendly Hero or fallback position; or set “retreating” state).
- [x] RecalcMorale called when members or their morale change; AverageMorale used for UI or squad-level logic if needed.

### Epic 3: Morale Sources (Casualties, Hero, Detached)

- [x] **Casualties:** On squad member death, apply morale delta to each remaining member (ApplyMoraleDelta(-5) or from data).
- [ ] **Hero presence:** Optional (not implemented; P3.1): when same-faction Hero in command range, apply small morale recovery or buff (e.g. +0.5/5s up to cap).
- [x] **Detached drain:** When unit has no same-faction Hero and no Captain within their command range, apply ApplyMoraleDelta(-1) every 5 seconds. Use timer or subsystem that checks “authority in range” per unit.

### Epic 4: Detached Behavior

- [x] Define **bIsDetached**: true when no Hero (2500 UU) and no Captain (1200 UU) of same faction in range.
- [x] Update bIsDetached periodically (timer 2–5s) or when authorities move; when true, apply detached morale drain (Epic 3).
- [x] **Defensive AI only:** When detached, unit does not accept new Attack orders. → SetCurrentOrder(Attack) returns early when bIsDetached.

### Epic 5: Captain

- [x] Add **bIsCaptain** to ARTSUnitCharacter (BlueprintReadWrite); default false.
- [x] Add **Rank** (1–3) to Unit (from data or runtime); Rank ≥ 3 = eligible for Captain.
- [x] Give Captain units **URTSCommandAuthorityComponent** with CommandRadius = 1200 (or add component on promote). P1 already uses 2500 for Hero.
- [x] **Promotion input:** Shift+RMB on selected unit (Hero as issuer) → TryPromoteToCaptain. Add “Promote to Captain” (e.g. key + click on selected unit, or context menu). Validate: unit is same faction, Rank ≥ 3, not already Captain. Set bIsCaptain = true; set squad->CaptainUnit = this if unit has SquadReference.
- [x] **Captain death:** On Captain unit death, before RemoveMember: apply squad morale penalty (e.g. ApplyMoraleDelta(-10) to each member or single squad-wide call). Then RemoveMember(Captain); RecalcMorale.

---

## 4. Tests & Validation

- [ ] Unit added to squad: SquadReference set; RecalcMorale updates AverageMorale.
- [ ] Unit removed / death: SquadReference cleared; remaining members get casualty morale delta; RecalcMorale updated.
- [ ] Morale &lt; 30: unit move speed reduced; order responsiveness reduced.
- [ ] Morale &lt; 15: within ~25s (5s × 5) at least one auto-retreat likely (20% per 5s); unit moves toward friendly or holds.
- [ ] Detached: no Hero/Captain in range; morale drains -1/5s; unit does not accept/cannot execute Attack (defensive only).
- [ ] Hero in range: detached drain stops; optional morale buff if implemented.
- [ ] Captain: promote Rank 3 unit → bIsCaptain, 1200 range; Captain death → squad morale penalty.

---

## 5. Documentation

- [ ] Comment morale thresholds and sources (GDD: &lt;30, &lt;15, casualties, Hero, detached).
- [ ] Comment Captain eligibility (Rank ≥ 3), 1200 UU, death penalty.
- [ ] Update plan status when epics complete.

---

## 6. Dependencies & Risks

### Dependencies

- P0: URTSSquadState, URTSMoraleComponent, ARTSUnitCharacter, ARTSHeroCharacter, URTSCommandAuthorityComponent.
- P1 (recommended): selection and orders so player can move units and promote Captain.
- Squad manager (GameState or subsystem) that owns squad instances.

### Risks

| Risk | Mitigation |
|------|------------|
| RecalcMorale frequency | Call on add/remove and when morale changes; avoid per-frame. |
| Detached check cost | Timer-based or event when Hero/Captain enters/leaves range; cache “in range” per unit. |
| Auto-retreat vs orders | Clear spec: auto-retreat cancels current order and sets retreat destination; defensive AI overrides new Attack when detached. |

---

## 7. Acceptance Criteria

- [x] Units can be assigned to squads; SquadReference set; squad has Members and optional Captain; RecalcMorale works.
- [x] Morale 0–100: &lt;30 gives -15% move and -15% order responsiveness; &lt;15 gives 20% every 5s auto-retreat.
- [x] Morale sources: casualties reduce squad morale; detached drain -1/5s; optional Hero buff (not implemented).
- [x] Detached units: defensive only, morale drain; no new Attack when detached.
- [x] Captain: Rank ≥3 promotable, 1200 UU, death causes squad morale penalty.

**Definition of Done (P3):** Squad container with membership and Captain; morale thresholds and sources applied; detached behavior and drain; Captain promote and death penalty. **Status: Implemented.**

### Optional P3.1 (later)
- Hero presence morale buff (when Hero in command range).
- Move morale constants (e.g. -5, -10) to DataTable if desired.
- Dedicated key binding for "Promote to Captain" (optional; Shift+RMB already works).

---

## 8. Rollback / Fallback

- If Captain is complex: ship P3 without Captain first (squad + morale + detached); add Captain in P3.1.
- If DT_MoraleThresholds is missing: use hardcoded 30 and 15 thresholds and document.
- Auto-retreat: minimal version = force Hold when morale <15; add random retreat in follow-up.
