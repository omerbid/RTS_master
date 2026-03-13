# Plan: P0 + Phases 1–7 (Aligned with New Docs)

**Created**: 2026-03-13  
**Status**: Active  
**Design refs**: `Docs/MASTER_GDD_MVP.md`, `Docs/CORE_GAME_MECHANICS.md`, `Docs/campaign_layer_spec.md`, `Docs/economy_system.md`, `Docs/AI_Knowledge/IMPLEMENTATION_PHASES.md`, `Docs/AI_Knowledge/PROJECT_BRAIN.md`, `Docs/AI_Knowledge/WORLD_SIMULATION.md`

---

## Overview

| Stage | Name | Doc source | Depends on |
|-------|------|------------|------------|
| **P0** | Core setup | MASTER_GDD, ARCHITECTURE_GUIDE | — |
| **Phase 1** | Region simulation | IMPLEMENTATION_PHASES, WORLD_SIMULATION, REGION_SYSTEM | P0 |
| **Phase 2** | Settlement system | IMPLEMENTATION_PHASES, PROJECT_BRAIN, economy_system | P0, Phase 1 |
| **Phase 3** | Human NPC population | IMPLEMENTATION_PHASES, PROJECT_BRAIN, CORE_GAME_MECHANICS | P0, Phase 1–2 |
| **Phase 4** | Wildlife ecosystem | IMPLEMENTATION_PHASES, PROJECT_BRAIN, MASTER_GDD | P0, Phase 1 |
| **Phase 5** | Refugee migration | IMPLEMENTATION_PHASES, PROJECT_BRAIN, campaign_layer_spec | P0, Phase 1–3 |
| **Phase 6** | Caravan trade routes | IMPLEMENTATION_PHASES, PROJECT_BRAIN | P0, Phase 1–2 |
| **Phase 7** | Settlement collapse and recolonization | IMPLEMENTATION_PHASES, PROJECT_BRAIN, CORE_GAME_MECHANICS | P0, Phase 1–2 |

**Rule (all phases):** Do not remove or replace existing systems. Extend only. See `Docs/README_FOR_CURSOR.md`, `Docs/AI_Knowledge/CODING_RULES.md`.

---

## P0 – Core Setup

**Full plan:** `.cursor/plans/PLANNING_P0_revised_with_new_docs.md`

**Summary:** Data (Registry, DataTables), Units, Heroes, Squads, Morale, Command Authority, Regions (Control 0–5, placeholders: FoodCapacity, WildlifeCount, ThreatLevel, GarrisonPower, SettlementState), Secure Region, Victory/Lose, Orders, Input, Game Mode. All already built; Region placeholders added per new docs.

**Done:** §2 and §4.1 of revised P0 plan.  
**Do not break:** Any class listed in PLANNING_P0_revised_with_new_docs.md §2.

---

## Phase 1 – Region Simulation

**Docs:** `IMPLEMENTATION_PHASES.md`, `WORLD_SIMULATION.md`, `REGION_SYSTEM.md`, `MASTER_GDD_MVP.md` (Development Roadmap Phase 1).

**Goal:** World simulation pulse that updates region state (population growth, threat decay, etc.) so regions “simulate even when unloaded” (PROJECT_BRAIN).

**Depends on:** P0 (ARTSRegionVolume with all fields, including placeholders).

**Key deliverables:**

| Task | Description | Doc |
|------|-------------|-----|
| [ ] | **URTSWorldSubsystem** (GameInstanceSubsystem or WorldSubsystem) | PROJECT_BRAIN, WORLD_SIMULATION |
| [ ] | Simulation pulse every **120–300 seconds** (configurable) | WORLD_SIMULATION, REGION_SYSTEM |
| [ ] | Pulse logic: **ThreatDecay**, **PopulationGrowth** (formula from CORE_GAME_MECHANICS: BaseGrowth * (FoodCapacity / Population) where applicable), optional **WildlifeGrowth** stub | WORLD_SIMULATION, CORE_GAME_MECHANICS |
| [ ] | Update **ARTSRegionVolume** state (Population, ThreatLevel, FoodCapacity, etc.) from pulse; no removal of existing control 0–5 / IsContested / Secure logic | REGION_SYSTEM |
| [ ] | Regions store state; actors spawn when player enters (or keep current overlap-based logic); far regions can run abstract simulation only | PROJECT_BRAIN (Performance) |

**Do not break:** Existing Region control gain timer, Secure, Victory, overlap tracking, GetHeroesInRegion, IsContested.

---

## Phase 2 – Settlement System

**Docs:** `IMPLEMENTATION_PHASES.md`, `PROJECT_BRAIN.md` (Settlement Layer), `economy_system.md`, `MASTER_GDD_MVP.md` (Map Structure, Region Types).

**Goal:** Settlements as actors; types Capital, City, Town, Village, Hamlet; population growth and food production per settlement (CORE_GAME_MECHANICS, economy_system).

**Depends on:** P0, Phase 1 (region simulation so regions can reference or own settlement state).

**Key deliverables:**

| Task | Description | Doc |
|------|-------------|-----|
| [ ] | **ARTSSettlement** (or **BP_Settlement** from C++ base) | PROJECT_BRAIN |
| [ ] | Settlement **types**: Capital, City, Town, Village, Hamlet (enum or data-driven) | MASTER_GDD, economy_system |
| [ ] | **Population growth** and **food production** per settlement; link to Region (Population, FoodCapacity) or own fields | economy_system, CORE_GAME_MECHANICS |
| [ ] | Optional: **SettlementState** on Region or on Settlement (Normal, Collapsed, etc.); collapse/recovery logic can be Phase 7 | campaign_layer_spec, economy_system |
| [ ] | Data: DT_SettlementTypes or similar if from “תוכן נוסף ועדכון”; do not replace DT_Units_MVP / DT_Heroes_MVP | ARCHITECTURE_GUIDE |

**Do not break:** ARTSRegionVolume existing API; Phase 1 pulse can write to Region + Settlement.

---

## Phase 3 – Human NPC Population

**Docs:** `IMPLEMENTATION_PHASES.md`, `PROJECT_BRAIN.md` (Population Layer), `CORE_GAME_MECHANICS.md` (population as physical NPCs).

**Goal:** Humans as physical NPCs in the world; states Idle, Working, Fleeing, Captured; used by monsters/armies (capture, convert). CORE_GAME_MECHANICS: “בני אדם הם NPC פיזיים”.

**Depends on:** P0, Phase 1–2 (regions/settlements so population has places to be and flee to).

**Key deliverables:**

| Task | Description | Doc |
|------|-------------|-----|
| [ ] | **ARTSHumanNPC** actor | PROJECT_BRAIN |
| [ ] | States: **Idle**, **Working**, **Fleeing**, **Captured** | PROJECT_BRAIN |
| [ ] | Spawn from settlements (Phase 2) or from region population count; link to Region Population / FoodCapacity | economy_system |
| [ ] | **CaptureHuman()** / convert to unit (monster or army); hook into existing faction/unit system without replacing it | CORE_GAME_MECHANICS |
| [ ] | ThreatLevel → villagers flee (CORE_GAME_MECHANICS §4); destination Town/City/Capital | CORE_GAME_MECHANICS |

**Do not break:** ARTSUnitCharacter, ARTSHeroCharacter, selection, orders, squads, morale.

---

## Phase 4 – Wildlife Ecosystem

**Docs:** `IMPLEMENTATION_PHASES.md`, `PROJECT_BRAIN.md` (Wildlife Layer), `MASTER_GDD_MVP.md` (Wildlife System).

**Goal:** Animals (Deer, Boar, Wolf, Bear); food for werewolves; ecosystem pressure; wildlife regenerates via regional simulation.

**Depends on:** P0, Phase 1 (region simulation for WildlifeCount / wildlife growth).

**Key deliverables:**

| Task | Description | Doc |
|------|-------------|-----|
| [ ] | **ARTSAnimal** actor (or base + Blueprint variants) | PROJECT_BRAIN |
| [ ] | Types: **Deer**, **Boar**, **Wolf**, **Bear** (data or enum) | MASTER_GDD, PROJECT_BRAIN |
| [ ] | **WildlifeCount** on Region updated by pulse (Phase 1) and/or spawn/despawn of ARTSAnimal | REGION_SYSTEM |
| [ ] | Werewolf faction can consume wildlife (food); hook into existing systems without replacing | MASTER_GDD, CORE_GAME_MECHANICS |
| [ ] | Optional: predator–prey, frontier pressure; can start minimal | PROJECT_BRAIN |

**Do not break:** Region overlap, control, Secure; Unit/Hero combat and orders.

---

## Phase 5 – Refugee Migration

**Docs:** `IMPLEMENTATION_PHASES.md`, `PROJECT_BRAIN.md` (Migration Layer), `campaign_layer_spec.md` (Region events: RefugeeWave).

**Goal:** Refugees spawn when settlements are attacked; travel to safer regions (Town, City, Capital); population redistribution.

**Depends on:** P0, Phase 1–3 (regions, settlements, human NPCs).

**Key deliverables:**

| Task | Description | Doc |
|------|-------------|-----|
| [ ] | **ARTSRefugeeGroup** actor | PROJECT_BRAIN |
| [ ] | Spawn on **Raid** / settlement attack (event from campaign_layer_spec); link to Phase 2–3 | campaign_layer_spec |
| [ ] | Movement toward **Town / City / Capital** (safer regions); update Region Population on arrival | PROJECT_BRAIN, CORE_GAME_MECHANICS |
| [ ] | **RefugeeWave** as region event (campaign_layer_spec); can trigger migration or spawn refugee groups | campaign_layer_spec |

**Do not break:** Region control, Victory, Human NPC states, Phase 1 pulse.

---

## Phase 6 – Caravan Trade Routes

**Docs:** `IMPLEMENTATION_PHASES.md`, `PROJECT_BRAIN.md` (Trade Layer), `MASTER_GDD_MVP.md`.

**Goal:** Caravans move along trade routes between settlements; contain Traders, Guards, Wealth; can be ambushed by monsters.

**Depends on:** P0, Phase 1–2 (regions, settlements for routes and endpoints).

**Key deliverables:**

| Task | Description | Doc |
|------|-------------|-----|
| [ ] | **ARTSCaravan** actor | PROJECT_BRAIN |
| [ ] | Moves along **trade routes** between settlements (define routes in data or from Region/Settlement links) | PROJECT_BRAIN |
| [ ] | Contains: **Traders**, **Guards**, **Wealth** (stats or components) | PROJECT_BRAIN |
| [ ] | Ambush by monsters (Vampires “prefer cities and caravans” – PROJECT_BRAIN); hook into combat/orders without replacing | PROJECT_BRAIN |

**Do not break:** Orders, factions, Region control, Phase 1–2.

---

## Phase 7 – Settlement Collapse and Recolonization

**Docs:** `IMPLEMENTATION_PHASES.md`, `PROJECT_BRAIN.md` (World Simulation: settlement collapse, recolonization), `CORE_GAME_MECHANICS.md` (§7 מחזור חיי ישוב), `campaign_layer_spec.md` (SettlementCollapse, Recolonization).

**Goal:** Settlements can collapse (e.g. Village → Declining → Ruins); recolonization restores them. Loop: Village → Raided → Declining → Ruins → Recolonization → Village.

**Depends on:** P0, Phase 1–2 (region simulation, settlements).

**Key deliverables:**

| Task | Description | Doc |
|------|-------------|-----|
| [ ] | **SettlementCollapse** logic: update SettlementState / Region state when conditions met (e.g. threat, no population) | WORLD_SIMULATION, campaign_layer_spec |
| [ ] | **Recolonization** logic: restore settlement/region from Ruins to Village (or similar); tie to Region/Settlement state | WORLD_SIMULATION, CORE_GAME_MECHANICS |
| [ ] | Lifecycle: Village → Raided → Declining → Ruins → Recolonization → Village (CORE_GAME_MECHANICS §7) | CORE_GAME_MECHANICS |
| [ ] | Integrate with Phase 1 pulse (SettlementCollapse, Recolonization in WORLD_SIMULATION) | WORLD_SIMULATION |

**Do not break:** ARTSRegionVolume control 0–5, Secure, Victory; Phase 1–2 region/settlement data.

---

## Cross-Phase: Hero Actions (campaign_layer_spec)

**SecureRegion** – already in P0.  
**RaiseMilitia, FortifySettlement, SuppressRevolt, HuntMonsters, ClaimRuins, EscortCaravan** – implement when the supporting systems exist (e.g. RaiseMilitia with Phase 2–3, EscortCaravan with Phase 6). No need to change P0; add as new abilities/inputs in the phase that introduces the related content.

---

## Summary Table

| Phase | Main class(es) | Pulse / events |
|-------|----------------|----------------|
| P0 | (existing) | — |
| 1 | URTSWorldSubsystem | Pulse 120–300s: ThreatDecay, PopulationGrowth, WildlifeGrowth |
| 2 | ARTSSettlement | Population/food per settlement; link to Region |
| 3 | ARTSHumanNPC | States; capture/convert; flee on ThreatLevel |
| 4 | ARTSAnimal | Types; WildlifeCount; werewolf food |
| 5 | ARTSRefugeeGroup | RefugeeWave; move to safe regions |
| 6 | ARTSCaravan | Trade routes; ambush |
| 7 | (Phase 1 + 2) | SettlementCollapse, Recolonization in pulse |

**Refs:** `PLANNING_P0_revised_with_new_docs.md`, `Docs/MASTER_GDD_MVP.md`, `Docs/AI_Knowledge/IMPLEMENTATION_PHASES.md`, `Docs/AI_Knowledge/PROJECT_BRAIN.md`.
