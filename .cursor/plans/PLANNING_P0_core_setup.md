# Plan: P0 Core Setup (Hero, Unit, SquadState, Region, MoraleComponent, CommandAuthority, DataTables)

**Created**: 2026-03-05
**Status**: Draft

---

## 1. Task Summary

We will establish the C++ core layer for the RTS MVP: base Hero and Unit characters, Region actor, Squad state representation, Morale and Command Authority components, and integration with existing data tables (`RTSDataTypes.h` + CSV). The goal is to have data-driven, faction-agnostic core entities that match the GDD (three factions, command range, morale, region control) and can be extended via Blueprints.

GDD alignment: CORE ENTITIES, HERO-CENTRIC COMMAND DOCTRINE, REGION CONTROL SYSTEM, COMBAT RULES, MVP LOCKED VALUES (P0–P3).

---

## 2. Execution Plan

### 2.1 Logic & Architecture

- **Core classes (C++ Base)**
  - `ARTSUnitCharacter` (inherits `ACharacter`)
    - Holds references to: faction (EFactionId), unit row (FUnitRow), Morale component, Squad state reference, Command authority state (read-only).
    - Uses `FUnitRow` from DataTable to initialize stats (HP, Damage, MoveSpeed, MoraleBase, PowerScale, SpecialTag).
  - `ARTSHeroCharacter` (inherits `ARTSUnitCharacter`)
    - Adds: command range config (Hero range from GDD), hero XP component hook, morale aura value, hero-specific DataTable row (`FHeroRow`).
    - Implements `IssueOrderToUnitsInRange()` using CommandAuthority component / helper.
  - `ARTSRegionVolume` (inherits `AVolume` or `AActor` with volume component)
    - Tracks control levels per faction (0–5), stability, population, dominant faction.
    - Provides API for control gain over time when uncontested (used in later phases but fields exist now).
- **Squad State Representation**
  - Runtime: `USquadState` (UObject) or component-like UObject owned by GameState or a dedicated manager.
    - Holds: list of unit references, average morale, cached faction, optional captain reference.
    - Exposes `RecalcMorale()` that pulls per-unit morale from components, computes average and applies "strong lift weak / weak drag strong" logic.
  - GDD name `ARTSQuadActor` is treated as the conceptual squad; actual implementation is `USquadState` (no separate Actor unless later needed).
- **Components**
  - `UMoraleComponent` (ActorComponent on Unit/Hero)
    - Stores `CurrentMorale` (0–100), base morale, and handles modifiers from events (damage taken, hero presence, detached state).
    - Applies thresholds from Morale MVP rules (e.g. <30: -15% speed/order responsiveness, <15: auto-retreat chance) via interfaces/events to movement/AI.
  - `UCommandAuthorityComponent` (ActorComponent on Hero/Captain)
    - Stores command range radius (Hero: 2500 UU, Captain: 1200 UU from GDD).
    - Provides queries: `CanIssueOrderToUnit(AActor* Issuer, ARTSUnitCharacter* TargetUnit)` using distance & faction checks.
    - Not responsible for order logic itself; only authority validation.
- **Data Integration**
  - Use `RTSDataTypes.h` (`EFactionId`, `FUnitRow`, `FHeroRow`, `FHeroXPRow`, `FMoraleThresholdRow`, etc.).
  - Import CSVs as Unreal DataTables:
    - `DT_Units_MVP.csv` → `FUnitRow`
    - `DT_Heroes_MVP.csv` → `FHeroRow`
    - `DT_HeroXP_Prototype.csv` → `FHeroXPRow`
    - `DT_MoraleThresholds.csv` → `FMoraleThresholdRow`
  - Create a lightweight C++ data access layer (e.g. `URTSDataRegistry` or GameInstanceSubsystem) to fetch rows by IDs and cache pointers.
- **Flow of Data & Orders**
  - On spawn, Unit/Hero asks data registry for its row (by UnitId/HeroId) and initializes stats.
  - Hero uses CommandAuthority to validate targets before issuing orders (orders themselves will be implemented in later phases but authority checks are designed now).
  - SquadState subscribes to unit life/membership changes and recalculates morale when needed.

### 2.2 Complexity Assessment

| Aspect       | Level  | Notes |
|-------------|--------|-------|
| Logic       | Medium | Multiple systems (data, morale, region, authority) but each is simple and focused. |
| Integration | High   | Must align with existing Blueprints, DataTables, and future MVP phases (P1–P3). |
| Testing     | Medium | Needs in-editor spawning, DataTable validation, and simple authority/morale checks. |
| Maintenance | Medium | Data-driven approach reduces code changes but requires clear ownership (registry, subsystems). |

### 2.3 Performance Considerations

- Avoid heavy logic in `Tick` on Units/Heroes; prefer timers, events, and aggregated updates (e.g. SquadState recalculates morale on membership/morale change, not every frame).
- Command radius checks should be done when issuing orders, not continuously.
- Region control logic will be lightweight timers / time-accumulation, not per-frame scanning.
- Data lookups should be cached (e.g. DataRegistry initializes on start and gives const pointers to rows).
- Blueprint work (later) should be thin wrappers over C++ logic to avoid expensive graph work.

### 2.4 Build Strategy

- **Step 1 – Data & Registry**
  - Verify `RTSDataTypes.h` compiles in the UE project (add module include if needed).
  - Import MVP CSVs as DataTables and reference their assets in a central registry (C++ or config).
- **Step 2 – Core Characters**
  - Implement `ARTSUnitCharacter` and `ARTSHeroCharacter` with basic stat initialization from DataTables.
  - Expose IDs/Faction as BlueprintReadOnly properties so existing/future Blueprints can derive.
- **Step 3 – Components**
  - Implement `UMoraleComponent` and `UCommandAuthorityComponent`, attach to Unit/Hero base classes.
- **Step 4 – SquadState**
  - Implement `USquadState` and minimal manager (e.g. a subsystem or GameState-owned array) to own squad instances.
- **Step 5 – Region Actor**
  - Implement `ARTSRegionVolume` with fields for control, stability, population; basic API for future control updates.

GDD alignment: CORE ENTITIES (Hero/Unit/Squad/Region), HERO-CENTRIC COMMAND DOCTRINE (authority component), COMBAT RULES (morale effects), MVP LOCKED VALUES (P0–P3), Architecture docs (data-driven, no hardcoded factions).

---

## 3. Epics & Tasks

### Epic 1: Data Backbone & Registry

- [ ] Ensure `RTSDataTypes.h` is part of a UE module and compiles.
- [ ] Create UE DataTables for Units, Heroes, HeroXP, Morale thresholds using provided CSVs.
- [ ] Implement a simple `URTSDataRegistry` (e.g. GameInstanceSubsystem) for row lookup by IDs.
- [ ] Add basic validation (log errors if IDs missing or duplicate).

### Epic 2: Core Characters & Components

- [ ] Implement `ARTSUnitCharacter` with stat initialization from `FUnitRow`.
- [ ] Implement `ARTSHeroCharacter` inheriting from `ARTSUnitCharacter` with hero-specific data from `FHeroRow`.
- [ ] Implement `UMoraleComponent` and attach to Unit/Hero.
- [ ] Implement `UCommandAuthorityComponent` and attach to Hero (and later Captain units).

### Epic 3: SquadState & Region Skeleton

- [ ] Implement `USquadState` (UObject) with member list and `RecalcMorale()` API.
- [ ] Implement a manager/owner for squads (e.g. GameState or subsystem) and basic membership functions.
- [ ] Implement `ARTSRegionVolume` with control/stability/population fields and faction-aware control values.
- [ ] Expose minimal Blueprint-accessible functions (get control level, get dominant faction) for future UI.

---

## 4. Tests & Validation

- [ ] DataTables load successfully; no missing row structs or CSV import errors.
- [ ] Spawning a Unit/Hero in PIE initializes stats from DataTables (HP, Damage, MoveSpeed, MoraleBase, PowerScale).
- [ ] MoraleComponent correctly initializes morale and can be adjusted via a simple debug function.
- [ ] CommandAuthority returns true/false correctly for units inside/outside range (using debug spheres or log output).
- [ ] SquadState recalculates average morale correctly when units join/leave or morale changes.
- [ ] RegionVolume instances can be placed in a map and hold faction control values without errors.

---

## 5. Documentation

- [ ] Short comments on each class/Component describing intent and how it maps to the GDD.
- [ ] Update `.cursor/GDD.md` or a separate note with actual C++ class names (ARTSUnitCharacter, ARTSHeroCharacter, ARTSRegionVolume, USquadState, UMoraleComponent, UCommandAuthorityComponent).
- [ ] High-level diagram (optional) linking DataTables → DataRegistry → Core classes → Components.

---

## 6. Dependencies & Risks

### Dependencies

- Unreal C++ toolchain working (project compiles after adding classes/components).
- DataTables imported from the external RTS pack (CSV) and accessible in the project.
- GDD and Companion docs considered source of truth for ranges, morale thresholds, and XP behavior.

### Risks

| Risk | Mitigation |
|------|------------|
| Overcoupling of components and characters | Keep Morale and CommandAuthority as generic, re-usable ActorComponents; avoid direct assumptions about specific factions/units. |
| Data lookup errors at runtime | Centralize access via DataRegistry and log/validate at startup. |
| Squad implementation needing Actor later | Start with `USquadState`; if position-based behavior is required, we can wrap it with an Actor that owns a state object. |
| Region logic growing complex | Keep P0 region logic minimal (data-only); add behavior in later phases (P2). |

---

## 7. Acceptance Criteria

- [ ] All Epics 1–3 tasks are implemented and compile.
- [ ] Unit/Hero classes and components exist and are visible in Unreal Editor for Blueprint extension.
- [ ] DataTables are wired and used for stat initialization.
- [ ] CommandAuthority and Morale components function in simple in-editor tests.
- [ ] Region volumes can be placed and configured with control/stability per faction.

Definition of "Done": C++ core layer for Hero, Unit, SquadState, Region, Morale, and CommandAuthority exists, compiles, and is data-driven, ready for P1 (command radius + orders) without requiring refactors.

---

## 8. Rollback / Fallback

- If integration with RTSDataTypes proves too rigid, fall back to a minimal internal struct for P0 and plan a migration step later.
- If `USquadState` as UObject is insufficient, we can introduce `ARTSQuadActor` that wraps the existing state without changing data structures.
- If DataRegistry subsystem complicates startup, temporarily use direct references to DataTables on GameMode/GameInstance and refactor later.
