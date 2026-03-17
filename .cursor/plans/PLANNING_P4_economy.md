# Plan: P4 Economy (Humans: Money / Vampires & Werewolves: Convert)

**Created**: 2026-02  
**Updated**: 2026-02-21 (Implementation complete; Phase 3 NPC integrated)  
**Status**: ✅ Complete  
**GDD**: `.cursor/GDD.md` (Economy MVP, Definition of Done: Recruit units)  
**Phases**: `PLANNING_P4_phases.md` (Phases 1–7 breakdown)

---

## 1. Task Summary

Implement P4: **Economy** – Humans use **Money** from region for recruitment; **Vampires and Werewolves** both **Convert** population into units. Hierarchy: Population → Stability → Overlord Resource. Exact rates via DataAssets.

**GDD alignment:** ECONOMY (MVP), "Use Money for recruitment" (Humans), "Convert population into units" (Vampires), Definition of Done ("Recruit units").

**Docs alignment (CORE_GAME_MECHANICS, economy_system, PROJECT_BRAIN):**
- **Physical NPC conversion:** בני אדם הם NPC פיזיים בעולם – "Humans exist as physical NPCs. Monsters must capture humans to convert them." Flow: find → capture → convert. See `CaptureHuman()` in Phase 3 (PLANNING_P0_and_Phases_1_to_7).
- **Werewolves:** CORE_GAME_MECHANICS §5 – "יכולים גם ללכוד בני אדם" – also capture humans; same Convert model as Vampires.
- **Phase 3 NPC:** ARTSHumanNPC implemented. Vampires/Werewolves: try physical capture first (600 UU), else abstract Population.

**Depends on:** P0 done (ARTSRegionVolume, ARTSHeroCharacter, FUnitRow). P2 recommended (region control, Population, Stability). P1 for selection/orders. P3 optional.

---

## 2. Execution Plan

### 2.1 Logic & Architecture

- **Resource model**
  - **Humans:** Money (int/float). Gained from region: Population + Stability + Control level → Money rate. Spent on recruitment (unit cost from data).
  - **Vampires:** No Money. Convert region Population into units. Each unit type has ConvertPopulationCost (or equivalent). **Physical NPC:** CORE_GAME_MECHANICS – find human NPC → capture → convert.
  - **Werewolves:** Same Convert model as Vampires. CORE_GAME_MECHANICS §5: "יכולים גם ללכוד בני אדם". ConvertPopulationCost per unit type. **Physical NPC:** capture human NPC → convert.

- **Data**
  - **FUnitRow:** Add `RecruitCostMoney` (Humans) and `ConvertPopulationCost` (Vampires, Werewolves). Or single `RecruitCost` with faction-specific interpretation. Upkeep remains for ongoing cost (optional P4).
  - **Economy config:** DataAsset or DataTable: `MoneyPerPopulationPerSecond`, `MoneyPerStabilityMultiplier`, `ControlLevelMoneyBonus`, base rates. Vampire/Werewolf: `PopulationPerUnit` or per-unit-type from FUnitRow.

- **Economy subsystem**
  - **URTSEconomySubsystem** (GameInstanceSubsystem) or store in **ARTSGameState**.
  - Holds per-faction: `Money` (Humans), optionally cached "available Population" for Vampires/Werewolves (read from region).
  - `AddMoney(Faction, Amount)`, `SpendMoney(Faction, Amount)`, `CanAffordRecruit(Faction, UnitId)`.
  - `TickEconomy()` or timer: Humans gain Money from regions (Hero in region with control → rate). Vampires/Werewolves: no passive gain; conversion consumes region Population (or physical NPC when ARTSHumanNPC exists).

- **Region → Money (Humans)**
  - Rule: Faction gains Money from a region when Hero (or presence) is in region and faction has control.
  - Rate = f(Population, Stability, ControlLevel). Example: `BaseRate + Population * 0.1 + Stability * 2 + ControlLevel * 5` (tunable).
  - Implementation: periodic (e.g. every 5–10s) or on timer; for each region, for each faction with control and Hero in region, add Money.

- **Recruitment flow**
  - **Where:** Hero in region. Player initiates "Recruit" (input: e.g. key + click, or context menu, or dedicated UI).
  - **How:** Player selects unit type (from faction's available units). System checks: CanAfford? Hero in region? Spawn point valid?
  - **Cost:** Humans: deduct Money (RecruitCostMoney). Vampires/Werewolves: deduct region Population (ConvertPopulationCost) or consume physical human NPC (CaptureHuman → convert).
  - **Spawn:** Spawn unit near Hero (or at region spawn point). Add to squad via RTSSquadManagerSubsystem. Initialize from registry.

- **Emergency decree (optional)**
  - GDD: "Emergency decree cheaper but reduces stability." Defer to P4.1 if time-constrained.

- **Human recruitment – Conscription Decree (צו גיוס) – Design spec**
  - Player issues **צו גיוס** that sets/offers a **price** for soldiers.
  - **People come** from the player's city/town/village (and from other regions as hero influence grows).
  - **They offer themselves** – NPCs present themselves as candidates; player **chooses** who to recruit.
  - **Player strength** → higher chance that higher quality/rank units will offer themselves.
  - **Higher quality** = higher recruitment cost and upkeep.
  - **P4 MVP:** May use simplified flow (direct pay + spawn). Full Conscription Decree flow in P4.1 (requires ARTSHumanNPC, settlements, influence).

### 2.2 Complexity Assessment

| Aspect       | Level  | Notes |
|-------------|--------|--------|
| Logic       | Medium | Money gain, conversion cost, recruitment validation. |
| Integration | Medium | Region, Hero, GameState, SquadManager, DataRegistry. |
| Testing     | Medium | PIE: gain Money, recruit Human unit; convert Population, recruit Vampire unit. |

### 2.3 Performance

- Economy tick: 5–10s interval, not every frame.
- Recruitment: one-time cost check + spawn.
- No per-unit economy tick; Upkeep can be deferred to post-P4.

### 2.4 Build Strategy

- **Step 1 – Data:** Add RecruitCostMoney, ConvertPopulationCost to FUnitRow. Create DA_EconomyConfig or DT row for rates.
- **Step 2 – Subsystem:** URTSEconomySubsystem with Money per faction, CanAffordRecruit, AddMoney, SpendMoney.
- **Step 3 – Money gain:** Timer on subsystem or GameState; for each region with Hero+control, add Money for Human faction.
- **Step 4 – Recruitment:** TryRecruitUnit(Hero, UnitId, Region) – validate, deduct cost, spawn unit, add to squad.
- **Step 5 – Input/UI:** Bind "Recruit" action (key + click or context). Minimal UI: list unit types + cost; click to recruit.

---

## 3. Epics & Tasks

### Epic 1: Data & Config

- [x] Add `RecruitCostMoney` (int32) to FUnitRow for Human units.
- [x] Add `ConvertPopulationCost` (int32) to FUnitRow for Vampire and Werewolf units.
- [x] Economy config: hardcoded in URTSEconomySubsystem (BaseRate 5, PerPopulation 0.1, PerStability 2, PerControlLevel 5, Tick 5s).

### Epic 2: Economy Subsystem

- [x] Create `URTSEconomySubsystem` (GameInstanceSubsystem).
- [x] Per-faction `Money`; Vampires/Werewolves: read Population from region.
- [x] `AddMoney`, `SpendMoney`, `GetMoney`, `CanAffordRecruit(Faction, UnitId, Region)`.

### Epic 3: Money Gain (Humans)

- [x] Timer 5s: for each region, Humans with Hero+control gain Money (Rate = Base + Pop*0.1 + Stability*2 + ControlLevel*5).

### Epic 4: Recruitment Logic

- [x] `TryRecruitUnit(Hero, UnitId)`: validate region, faction, afford.
- [x] **Vampires/Werewolves:** Try ARTSHumanNPC capture first (600 UU range); if found → capture, spawn at Hero+150, destroy NPC. Else deduct abstract Population.
- [x] **Humans:** SpendMoney, spawn at Hero+150.
- [x] Spawn unit, InitializeFromRegistry, AddUnitToFactionSquad.

### Epic 5: Input & UI

- [x] Key **R** bound; `OnInputRecruit()` – Hero selected, recruit first unit for faction (GetFirstRecruitableUnitIdForFaction).
- [x] On-screen feedback: "Unit recruited" / "Cannot recruit: not enough resources or Hero not in region".

---

## 4. Tests & Validation

- [ ] Human: Hero in region, control > 0 → Money increases over time.
- [ ] Human: Enough Money → Recruit unit → Money deducted, unit spawned.
- [ ] Human: Not enough Money → Recruit fails.
- [ ] Vampire/Werewolf: Region has enough Population → Convert → unit spawned, Population deducted.
- [ ] Vampire/Werewolf: Not enough Population → Convert fails.
- [ ] Recruited unit: added to squad, correct faction, initialized from registry.

---

## 5. Documentation

- [ ] Comment economy rules (Money gain formula, conversion cost) in code.
- [ ] Document RecruitCostMoney and ConvertPopulationCost in FUnitRow.
- [ ] Plan status updated when epics complete.

---

## 6. Dependencies & Risks

### Dependencies

- P0: ARTSRegionVolume, ARTSHeroCharacter, FUnitRow, DataRegistry.
- P2: Region control, Population, Stability, Hero-in-region tracking.
- P1: Selection (Hero selected for recruit).
- P3: RTSSquadManagerSubsystem (for adding recruited unit to squad). If P3 not done, spawn unit without squad assignment.

### Risks

| Risk | Mitigation |
|------|------------|
| Spawn point | Use Hero location + offset; avoid spawning inside geometry. |
| Multiple regions | P4 MVP: single region; design APIs for multi-region later. |
| Werewolves | Use same Convert model as Vampires (CORE_GAME_MECHANICS §5). |
| Emergency decree | Defer to P4.1. |

---

## 7. Acceptance Criteria

- [x] Humans: Money gained from region when Hero present and faction has control.
- [x] Humans: Recruit unit costs Money; unit spawns when affordable.
- [x] Vampires/Werewolves: Physical capture (ARTSHumanNPC in 600 UU) first; else abstract Population.
- [x] Recruited unit: correct faction, initialized, in squad, visible (BodyMeshComponent).
- [x] Key R: Recruit when Hero selected and in region.
- [x] Order issuer: when selecting only recruited unit, find Hero of faction for orders.

**Definition of Done (P4):** ✅ Economy subsystem with Money (Humans) and Population/NPC conversion (Vampires, Werewolves); recruitment flow working; physical capture integrated.

---

## 8. Rollback / Fallback

- If EconomySubsystem is heavy: store Money in GameState first; extract to subsystem later.
- If recruitment UI is complex: ship with log-only or key + unit-type hardcoded; add UI in P4.1.
- Emergency decree: skip for P4; add in follow-up.

---

## 9. Post-P4 (Deferred)

| Item | Status |
|------|--------|
| Conscription Decree (צו גיוס) | P4.1 – NPCs offer themselves, player chooses |
| Emergency decree (cheaper, -stability) | P4.1 |
| Recruit UI (unit list + costs) | P4.1 |
| DataAsset economy config | Optional – currently hardcoded |
