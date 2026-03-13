# Plan: P0 Core Setup – Aligned with All New Docs

**Created**: 2026-03-13  
**Updated**: 2026-03-13  
**Status**: Active  
**Parent**: `PLANNING_P0_core_setup.md`

**Design docs (authoritative):**

| Doc | Path | Role for P0 |
|-----|------|-------------|
| Master GDD MVP | `Docs/MASTER_GDD_MVP.md` | Core concept, pillars, map structure, region variables, hero actions, roadmap |
| Core Game Mechanics | `Docs/CORE_GAME_MECHANICS.md` | Philosophy, economy, CarryingCapacity, region properties, hero actions |
| Campaign Layer Spec | `Docs/campaign_layer_spec.md` | Region fields, hero actions, region events, campaign loop |
| Economy System | `Docs/economy_system.md` | Population, FoodCapacity, settlement types, core variables |
| Architecture Guide | `Docs/AI_Knowledge/ARCHITECTURE_GUIDE.md` | Layers, key classes, C++/Blueprint/data rules |
| Region System | `Docs/AI_Knowledge/REGION_SYSTEM.md` | Region variables, pulse, simulation |
| Implementation Phases | `Docs/AI_Knowledge/IMPLEMENTATION_PHASES.md` | Phase 1–7 (Region → Recolonization) |
| Project Brain | `Docs/AI_Knowledge/PROJECT_BRAIN.md` | World layers, region/settlement/population/wildlife, technical architecture |
| Project Overview | `Docs/AI_Knowledge/PROJECT_OVERVIEW.md` | Pillars, Cursor rule: extend don’t replace |
| Coding Rules | `Docs/AI_Knowledge/CODING_RULES.md` | No remove, extend, data-driven, no heavy Tick, subsystems |
| README for Cursor | `Docs/README_FOR_CURSOR.md` | Doc layout, rule: don’t burn existing code |

---

## 1. Purpose

- P0 is the **foundation** that matches the new docs: data, units, heroes, squads, regions, command, morale, orders, victory.
- **Nothing already built is removed or rewritten** (per README_FOR_CURSOR, PROJECT_OVERVIEW, CODING_RULES).
- P0 adds **only** what the docs require for the “Region Layer” and “Game Layer” so that later phases (Implementation Phases 1–7) can build on it.

---

## 2. What Already Exists (Do Not Remove or Replace)

| Area | Existing | Doc alignment |
|------|----------|----------------|
| **Data** | `URTSDataRegistry`, `RTSDataTypes.h`, DT_Units_MVP, DT_Heroes_MVP, DT_HeroXP_Prototype, DT_MoraleThresholds | Data Layer; balance data in DataTables (ARCHITECTURE_GUIDE). |
| **Units** | `ARTSUnitCharacter` – FactionId, UnitId, CachedUnitData, Overrides, MoraleComponent, OrderComponent, Squad, Rank, Captain, Detached | Core entities (MASTER_GDD); Actor Layer. |
| **Heroes** | `ARTSHeroCharacter` – HeroId, CommandAuthority 2500, SecureRegion, TryStartSecureRegion, IssueOrderToUnitsInRange | Hero-centric command (MASTER_GDD, CORE_GAME_MECHANICS); SecureRegion from campaign_layer_spec. |
| **Morale** | `URTSMoraleComponent` | Combat/morale (GDD). |
| **Command** | `URTSCommandAuthorityComponent` – CanIssueOrderToUnit | Hero-centric command. |
| **Squad** | `URTSSquadState`, `URTSSquadManagerSubsystem` | Grouping for orders/morale. |
| **Region** | `ARTSRegionVolume` – ControlLevel 0–5 per faction, Population, Stability, DominantFaction, IsPointInRegion, IsContested, SetControlLevelForFaction, overlap, control gain 0→4, Secure→5 | Region Layer (PROJECT_BRAIN, REGION_SYSTEM); ControlLevel 0–5 (MASTER_GDD, campaign_layer_spec). |
| **Secure** | `URTSSecureRegionComponent` – 15s channel | Hero action SecureRegion (MASTER_GDD, campaign_layer_spec). |
| **Victory** | `URTSVictorySubsystem` – win on 5, lose on no hero + no region ≥3 | Win/lose (P2). |
| **Orders** | `URTSOrderComponent` – Move, Attack | Tactical layer. |
| **Input** | `ARTSPlayerController` – selection, RMB Move/Attack, UnitInfoWidget | Game Layer. |
| **Game mode** | `ARTSGameModeBase`, `ARTSCameraPawn` | Game Layer. |

**Rule:** Do not delete or replace these. Only add or extend.

---

## 3. Mapping: New Docs → Current Code (P0 Alignment)

### 3.1 MASTER_GDD_MVP / Campaign Layer / Economy / Region System

| Doc term | Current implementation | Gap (P0 only) |
|----------|------------------------|---------------|
| Factions: Humans, Vampires, Werewolves | `EFactionId`, FactionId on Unit/Hero | None |
| Region: ControlLevel (0–5) | ControlLevelHumans/Vampires/Werewolves, SetControlLevelForFaction | None |
| Region: Population | `ARTSRegionVolume::Population` | None |
| Region: FoodCapacity | — | Add as placeholder (economy_system, CORE_GAME_MECHANICS) |
| Region: WildlifeCount | — | Add as placeholder (MASTER_GDD, REGION_SYSTEM) |
| Region: ThreatLevel | — | Add as placeholder (campaign_layer_spec, economy_system) |
| Region: GarrisonPower | — | Add as placeholder (MASTER_GDD, campaign_layer_spec) |
| Region: SettlementState | — | Add as placeholder (MASTER_GDD, economy_system) |
| Region: OwnerFaction | DominantFaction (derived from control) | Optional: explicit OwnerFaction if needed later |
| Hero: SecureRegion | `URTSSecureRegionComponent`, TryStartSecureRegion | None |
| Hero: RaiseMilitia, FortifySettlement, … | — | Post-P0 (Implementation Phase 2+) |
| Region events: Raid, RefugeeWave, … | — | Post-P0 |
| Map 4km×4km, Region types (Capital, City, …) | Single region MVP; no type enum yet | Optional: RegionType enum placeholder |

### 3.2 CORE_GAME_MECHANICS

| Concept | P0 status |
|---------|-----------|
| Continuous campaign RTS, same map | Supported (one map, regions on it). |
| Population as physical NPCs | Post-P0 (Phase 3). |
| CarryingCapacity (FoodCapacity) | Add FoodCapacity on Region as placeholder. |
| ThreatLevel → villagers flee | Add ThreatLevel placeholder; logic post-P0. |
| Hero regional actions | SecureRegion done; others post-P0. |

### 3.3 ARCHITECTURE_GUIDE / CODING_RULES

| Rule | P0 compliance |
|------|----------------|
| Heavy simulation → C++, Actors/visuals → Blueprint, Data → DataTables | Current code follows this. |
| Never remove existing; extend only | This plan adds fields only. |
| Subsystems for global simulation | DataRegistry, VictorySubsystem, SquadManager; WorldSubsystem post-P0. |

### 3.4 IMPLEMENTATION_PHASES (Docs)

- **Phase 1 – Region simulation:** P0 provides Region with all doc fields (including placeholders); simulation logic (pulse 120–300s) is Phase 1 content, not P0.
- **Phase 2 – Settlements:** ARTSSettlement post-P0; P0 only Region placeholders.
- **Phases 3–7:** Human NPCs, Wildlife, Refugee, Caravan, Recolonization – all post-P0.

---

## 4. P0 Tasks (Additions Only)

### 4.1 ARTSRegionVolume – add fields from docs

Add the following as **placeholders** (no behavior change; used by later phases).

| Task | Field | Type | Default | Doc source |
|------|--------|------|--------|------------|
| [x] | FoodCapacity | float | 0 | MASTER_GDD, economy_system, CORE_GAME_MECHANICS (CarryingCapacity) |
| [x] | WildlifeCount | int32 | 0 | MASTER_GDD, REGION_SYSTEM, campaign_layer_spec |
| [x] | ThreatLevel | float | 0 | MASTER_GDD, campaign_layer_spec, economy_system |
| [x] | GarrisonPower | float | 0 | MASTER_GDD, campaign_layer_spec |
| [x] | SettlementState | int32 | 0 | MASTER_GDD, economy_system (0=None, 1=Normal, 2=Collapsed) |

- Do **not** change existing logic (control 0–5, IsContested, Secure, overlap, control gain timer).
- [x] **RegionType** (ERegionType: Unknown, Capital, City, Town, Village, Hamlet, WildForest, Ruins) added to RTSDataTypes.h and ARTSRegionVolume (Region|Docs).

### 4.2 Documentation

- [x] §3 in this plan is the single **P0 alignment** reference; `Docs/P0_ALIGNMENT.md` points to it.
- [x] `Docs/README_FOR_CURSOR.md` states: do not burn existing code; new docs reflected by additions only.

### 4.3 DataTables

- Keep existing: DT_Units_MVP, DT_Heroes_MVP, DT_HeroXP_Prototype, DT_MoraleThresholds.
- New tables from “תוכן נוסף ועדכון” (e.g. DT_HumanUnits, DT_WildlifeTypes, DT_SettlementTypes): **optional for P0**; add only if product decides to load them in P0; do not replace current tables.

---

## 5. What P0 Does Not Do (Reserved for Later Phases)

- **URTSWorldSubsystem** – world simulation pulse (120–300 s) – Phase 1.
- **ARTSSettlement**, **ARTSHumanNPC**, **ARTSAnimal**, **ARTSRefugeeGroup**, **ARTSCaravan** – Phases 2–7.
- Population/wildlife simulation, migration, caravans, recolonization – Phases 2–7.
- Hero actions beyond SecureRegion (RaiseMilitia, FortifySettlement, etc.) – Phase 2+.
- Region events (Raid, RefugeeWave, SettlementCollapse, etc.) – Phase 2+.

---

## 6. Acceptance Criteria

- [x] All existing systems in §2 remain and work (verified; no removals).
- [x] ARTSRegionVolume has FoodCapacity, WildlifeCount, ThreatLevel, GarrisonPower, SettlementState, RegionType as non-breaking placeholders (Region|Docs category).
- [x] Mapping (§3) is the single place for “doc term → current code”; Docs/P0_ALIGNMENT.md references it.
- [x] No existing P0/P1/P2 code removed or rewritten because of the new docs.

**Definition of Done (P0 per new docs):** Foundation is preserved; Region is aligned with all doc region variables via placeholders; P0 is the base for Implementation Phases 1–7 without burning current build.

---

## 7. For Other Agents

- Read this file and §2 before changing code. Do not delete or replace listed systems.
- When adding from docs: add new fields or new classes; do not replace existing behavior. Document in §3 or here.
- Refs: `PLANNING_P0_core_setup.md`, `Docs/README_FOR_CURSOR.md`, `Docs/MASTER_GDD_MVP.md`, `Docs/CORE_GAME_MECHANICS.md`, `Docs/AI_Knowledge/PROJECT_BRAIN.md`, `Docs/AI_Knowledge/IMPLEMENTATION_PHASES.md`.
