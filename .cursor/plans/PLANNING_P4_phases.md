# Plan: P4 Economy – Phases 1–7

**Created**: 2026-02-21  
**Updated**: 2026-02-21 (Implementation complete)  
**Status**: ✅ Complete  
**GDD**: `.cursor/GDD.md` (Economy MVP, Recruit units)  
**Parent**: `PLANNING_P4_economy.md`

---

## Overview

P4 Economy – **7 phases, all done**. Humans use Money; Vampires/Werewolves convert Population or **capture ARTSHumanNPC** (Phase 3). Flow: Hero in region → key R → recruit first unit. Physical capture: ARTSHumanNPC within 600 UU → capture, spawn Thrall, destroy NPC.

| Phase | Focus | Status |
|-------|--------|--------|
| 1 | Data – RecruitCostMoney, ConvertPopulationCost | ✅ Done |
| 2 | Economy Config (rates) | ✅ Done |
| 3 | Economy Subsystem | ✅ Done |
| 4 | Money Gain (Humans) | ✅ Done |
| 5 | Recruitment Logic | ✅ Done |
| 6 | Recruit Input & Flow | ✅ Done |
| 7 | Integration & Polish | ✅ Done |

---

## Phase 1: Data – Unit Costs ✅

**Classes**: `RTSDataTypes.h`, `FUnitRow`, DT_Units_MVP.csv

- [x] RecruitCostMoney, ConvertPopulationCost in FUnitRow
- [x] DT_Units_MVP.csv: Humans 30–120, Vampires 15–60, Werewolves 20–40

---

## Phase 2: Economy Config ✅

- [x] Hardcoded in URTSEconomySubsystem: BaseRate 5, PerPop 0.1, PerStab 2, PerControl 5, Tick 5s

---

## Phase 3: Economy Subsystem ✅

**Classes**: `URTSEconomySubsystem` (GameInstanceSubsystem)

- [x] MoneyByFaction, AddMoney, SpendMoney, GetMoney, CanAffordRecruit(Faction, UnitId, Region)

---

## Phase 4: Money Gain (Humans) ✅

- [x] TickEconomy 5s: Humans with Hero+control gain Money (Region GetPopulation/GetStability)

---

## Phase 5: Recruitment Logic ✅

- [x] TryRecruitUnit: region, faction, afford check
- [x] **Vampires/Werewolves:** ARTSHumanNPC in 600 UU → capture, spawn at Hero+150, destroy NPC (1 NPC = 1 unit). Else Region.SetPopulation(-Cost)
- [x] **Humans:** SpendMoney, spawn at Hero+150
- [x] Spawn ARTSUnitCharacter, InitializeFromRegistry, AddUnitToFactionSquad

---

## Phase 6: Recruit Input & Flow ✅

- [x] Key R bound; OnInputRecruit: Hero selected → GetFirstRecruitableUnitIdForFaction → TryRecruitUnit
- [x] On-screen feedback: success / fail

---

## Phase 7: Integration & Polish ✅

- [x] Region default Population=100, Stability=50
- [x] ARTSRegionVolume: GetPopulation, SetPopulation, GetStability (public accessors)
- [x] RTSUnitCharacter: BodyMeshComponent (visible capsule for spawned units)
- [x] GetOrderIssuer: when only regular units selected, find Hero of faction for orders

---

## Dependencies

| Phase | Depends On |
|-------|------------|
| 1 | P0 (FUnitRow, DataRegistry) |
| 2 | Phase 1 |
| 3 | Phase 1, 2 |
| 4 | Phase 3, P2 (Region, Hero in region) |
| 5 | Phase 3, P1 (GetOrderIssuer), P3 (SquadManager) |
| 6 | Phase 5 |
| 7 | Phases 1–6 |

---

## Related Documents

- `PLANNING_P4_economy.md` – Full P4 plan (Epics)
- `PLANNING_P2_phases_1_7.md` – Region, GetRegionAtLocation
- `P3_DEPENDENCIES_STATUS.md` – RTSSquadManagerSubsystem (AddUnitToFactionSquad)

---

## Definition of Done (P4) ✅

- [x] Humans: Money from region; recruit costs Money
- [x] Vampires/Werewolves: Physical capture (ARTSHumanNPC) or abstract Population
- [x] Key R: Recruit when Hero selected and in region
- [x] Recruited unit: visible (BodyMesh), in squad, controllable (GetOrderIssuer fallback)

---

## Implementation Summary

| Component | File | Notes |
|-----------|------|-------|
| FUnitRow | RTSDataTypes.h | RecruitCostMoney, ConvertPopulationCost |
| Economy | RTSEconomySubsystem.h/cpp | Money, TryRecruitUnit, physical capture |
| Region | RTSRegionVolume.h | GetPopulation, SetPopulation, GetStability |
| NPC | RTSHumanNPC.h/cpp | EHumanNPCState, TryCapture, HomeRegion |
| Input | RTSPlayerController | Key R, OnInputRecruit |
| Unit | RTSUnitCharacter | BodyMeshComponent, SetSelected |
| Order | RTSPlayerController | GetOrderIssuer finds Hero when only units selected |
