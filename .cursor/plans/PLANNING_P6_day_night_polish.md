# Plan: P6 Day/Night + Polish

**Created**: 2026-03  
**Status**: **Implemented**  
**GDD**: `.cursor/GDD.md` (Development Order: P6 | Day/Night + polish; Combat Rules: Day/Night cycle active, Night grants vampires bonus, day slight penalty)

---

## Implementation status (current codebase)

| Epic | Status | Main files / APIs |
|------|--------|-------------------|
| **Epic 1 – Day/Night Subsystem** | Done | `URTSDayNightSubsystem` (GameInstanceSubsystem): IsNight(), IsDay(), GetCurrentPhase(), GetSecondsUntilNextPhase(), PhaseDurationSeconds (default 300), bStartAtNight, OnDayNightPhaseChanged. Timer 1s advances phase. |
| **Epic 2 – Vampire Night Bonus** | Done | Damage ×1.1, MoveSpeed ×1.05 when Vampires + IsNight. Unit: GetDayNightDamageMultiplier(), GetDayNightMoveSpeedMultiplier(); combat uses in BuildGroupFromUnits (Attack *= multiplier); UpdateMoraleEffects uses move multiplier. |
| **Epic 3 – Vampire Day Penalty** | Done | Damage ×0.95, MoveSpeed ×0.95 when Vampires + IsDay. Same getters return 0.95. |
| **Epic 4 – UI & Polish** | Done | PlayerController: subscribes to OnDayNightPhaseChanged; on phase change shows "[Day/Night] Night falls" / "Dawn"; at start shows "[Day/Night] Day" or "Night". |

**Constants:** VampireNightDamageMultiplier=1.1, VampireDayDamageMultiplier=0.95, VampireNightMoveMultiplier=1.05, VampireDayMoveMultiplier=0.95 (RTSUnitCharacter.cpp).

---

## 1. Task Summary

Implement **P6**: **Day/Night cycle** (world state: day vs night; tunable duration; query IsNight/IsDay), **Vampire night bonus** (e.g. damage/speed/morale when IsNight), **Vampire day penalty** (slight debuff when IsDay), and **polish** (UI feedback for time of day, balance pass, MVP cleanup).

**GDD alignment:** COMBAT RULES – "Day/Night cycle active. Night grants vampires bonus; day slight penalty."

**Depends on:** P0–P4 done (Unit, Hero, FactionId, Combat, Morale). P5 (Hero respawn) optional; not required for Day/Night.

---

## 2. Execution Plan

### 2.1 Day/Night Cycle

- **Owner:** GameInstanceSubsystem (e.g. `URTSDayNightSubsystem`) or GameState. Single source of truth for "current phase" and time.
- **State:**
  - `bool IsNight()` (or `ETimeOfDay GetTimeOfDay()`: Day / Dusk / Night / Dawn).
  - Cycle: e.g. 5 min Day → 5 min Night (tunable). Or fixed phases: Day duration, Night duration.
- **Update:** Timer (e.g. every 1s) advance game time; when elapsed phase duration, flip phase and optionally broadcast delegate `OnDayNightChanged`.
- **Query:** Any system (Unit, UI, Region) can ask subsystem: `IsNight()`, `IsDay()`, or `GetTimeOfDay()`.

### 2.2 Vampire Night Bonus

- **When:** `FactionId == Vampires` and `IsNight()`.
- **Effect (choose one or combine, tunable):**
  - Damage multiplier (e.g. 1.1 or 1.15).
  - Move speed multiplier (e.g. 1.05).
  - Morale: small regen per tick or cap bonus (e.g. +2/5s when night, up to cap).
- **Where to apply:** Unit Tick or when dealing damage / moving: if Vampires and IsNight, apply multiplier. Prefer single place (e.g. `GetDamageMultiplier()`, `GetMoveSpeedMultiplier()` on Unit or in combat path) so balance is centralized.

### 2.3 Vampire Day Penalty

- **When:** `FactionId == Vampires` and `IsDay()`.
- **Effect (slight):** e.g. damage × 0.95, or move × 0.95, or morale drain +0.5/5s. One modifier is enough for "slight."
- **Where:** Same as bonus – single multiplier/getter that returns 1.0 for non-Vampires or when no day/night effect.

### 2.4 Polish (P6 scope)

- **Day/Night UI:** On-screen or HUD indicator (e.g. "Day" / "Night" or icon). Optional: debug message when phase changes.
- **Balance:** Tune night bonus / day penalty values in one place (constants or DataTable).
- **Cleanup:** Fix known MVP bugs; ensure Win/Lose, Secure, Recruit, Captain, Morale feedback are clear.
- **Optional:** Sky/lighting change with phase (Blueprint/Level Blueprint); not required for P6 if time is short.

### 2.5 Complexity

| Aspect       | Level  | Notes |
|-------------|--------|--------|
| Logic       | Low–Med| One subsystem, timer, two multipliers (Vampire night/day). |
| Integration | Low    | Unit or combat reads IsNight; applies multiplier. |
| Polish      | Low    | UI label, balance numbers. |

---

## 3. Epics & Tasks

### Epic 1: Day/Night Subsystem

- [x] Add **URTSDayNightSubsystem** (GameInstanceSubsystem).
- [x] State: `bool bIsNight`; `float PhaseDurationSeconds` (default 300); `float ElapsedInPhase`.
- [x] Timer: every 1s advance ElapsedInPhase; when ≥ PhaseDurationSeconds, toggle bIsNight, reset ElapsedInPhase, broadcast OnDayNightPhaseChanged.
- [x] API: IsNight(), IsDay(), GetCurrentPhase(), GetSecondsUntilNextPhase(); FOnRTSDayNightPhaseChanged delegate.
- [x] Initialize: bStartAtNight option; start timer when World valid.

### Epic 2: Vampire Night Bonus

- [x] When Vampires and IsNight(): Damage ×1.1, MoveSpeed ×1.05.
- [x] Unit: GetDayNightDamageMultiplier(), GetDayNightMoveSpeedMultiplier(); combat: BuildGroupFromUnits multiplies Snapshot.Attack; UpdateMoraleEffects multiplies MaxWalkSpeed.
- [x] Constants: VampireNightDamageMultiplier=1.1f, VampireNightMoveMultiplier=1.05f.

### Epic 3: Vampire Day Penalty

- [x] When Vampires and IsDay(): Damage ×0.95, MoveSpeed ×0.95.
- [x] Same getters return VampireDayDamageMultiplier / VampireDayMoveMultiplier (0.95f).

### Epic 4: Day/Night UI & Polish

- [x] **UI:** At start show "[Day/Night] Day" or "Night" (AddOnScreenDebugMessage). Phase change: "[Day/Night] Night falls" / "Dawn".
- [x] **Phase change:** PlayerController subscribes to OnDayNightPhaseChanged; OnDayNightPhaseChanged() shows message.
- [x] **Balance:** Multipliers in RTSUnitCharacter.cpp constants; tunable.
- [ ] **Polish checklist:** Optional: review Win/Lose, Secure, Recruit, Captain, Morale; optional FX.

---

## 4. Technical Notes

### Where to apply multipliers

- **Damage:** In the place where damage is applied to a target (e.g. `TakeDamage` on victim, or attacker’s damage × multiplier before apply). If Vampires are always the **attacker**, apply multiplier to attacker’s damage when Vampires + night/day. Prefer: attacker-side, so `FinalDamage = BaseDamage * Attacker->GetDayNightDamageMultiplier()` (Unit returns 1.0 for non-Vampires, 1.1 night, 0.95 day for Vampires).
- **Move speed:** In Unit’s movement update: base speed from data × morale × day/night. E.g. in `UpdateMoraleEffects` or in a single `GetFinalMoveSpeed()` that includes morale and day/night for Vampires.

### Subsystem vs GameState

- **Subsystem:** No dependency on GameState; works in PIE and standalone. Prefer **URTSDayNightSubsystem** (GameInstanceSubsystem) so any world can query it.
- **GameState:** If you already have RTSGameState with replicated state, you could add time there; then only use subsystem for non-replicated/single-player.

### Performance

- One timer per game; one query per Unit per tick (or per damage application) – cheap. No per-frame iteration over all units for day/night.

---

## 5. Tests & Validation

- [ ] Cycle: start game, wait PhaseDuration seconds – phase flips Day↔Night; IsNight/IsDay correct.
- [ ] Vampire at night: deal damage or move – bonus applied (e.g. 10% more damage or 5% more speed).
- [ ] Vampire by day: slight penalty (e.g. 5% less damage).
- [ ] Non-Vampire units: no day/night effect (multiplier 1.0).
- [ ] UI shows Day/Night (or debug message when phase changes).

---

## 6. Dependencies & Risks

| Dependency | Status |
|------------|--------|
| P0 (Unit, FactionId) | Done |
| P1–P4 (Combat, Morale, Economy) | Done |
| P5 (Hero respawn) | Optional; not required for P6 |

**Risks:**

| Risk | Mitigation |
|------|------------|
| Where damage is applied unclear | Grep for TakeDamage / ApplyDamage; add multiplier at single point. |
| Too many multipliers (night + day + morale) | Keep day/night in one helper; morale stays in existing system. |

---

## 7. Acceptance Criteria

- [x] Day/Night cycle runs (PhaseDurationSeconds 300 default); IsNight/IsDay queryable.
- [x] Vampires get night bonus (damage 10%, move 5%).
- [x] Vampires get day penalty (damage and move 5%).
- [x] Non-Vampire factions unaffected (multiplier 1.0).
- [x] UI: start phase + phase change messages on screen.
- [ ] Polish: optional balance and MVP cleanup.

**Definition of Done (P6):** Day/Night subsystem live; Vampire bonus/penalty applied; UI/feedback for phase. **Status: Implemented.**

---

## 8. Optional / Later

- Dusk/Dawn as separate short phases (4 phases instead of 2).
- Sky/lighting/atmosphere change with phase (Blueprint).
- DataTable for phase duration and multiplier values.
- Werewolves day/night (GDD mentions Vampires; Werewolves can be added similarly if desired).
