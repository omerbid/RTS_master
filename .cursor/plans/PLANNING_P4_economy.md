# Plan: P4 Economy (Humans: Money / Vampires: Convert)

**Created**: 2026-02  
**Status**: Draft  
**GDD**: `.cursor/GDD.md` (Economy MVP, Definition of Done: Recruit units)

---

## 1. Task Summary

Implement P4: **Economy** – Humans use **Money** from region for recruitment; Vampires **Convert** population into units. Hierarchy: Population → Stability → Overlord Resource. Exact rates via DataAssets.

**GDD alignment:** ECONOMY (MVP), "Use Money for recruitment" (Humans), "Convert population into units" (Vampires), Definition of Done ("Recruit units").

**Depends on:** P0 done (ARTSRegionVolume, ARTSHeroCharacter, FUnitRow). P2 recommended (region control, Population, Stability). P1 for selection/orders. P3 optional.

---

## 2. Execution Plan

### 2.1 Logic & Architecture

- **Resource model**
  - **Humans:** Money (int/float). Gained from region: Population + Stability + Control level → Money rate. Spent on recruitment (unit cost from data).
  - **Vampires:** No Money. Convert region Population directly into units. Each unit type has ConvertPopulationCost (or equivalent).
  - **Werewolves (MVP):** GDD says Humans vs Vampires; Werewolves = post-MVP. For P4, either exclude or mirror Vampires (Convert) for consistency.

- **Data**
  - **FUnitRow:** Add `RecruitCostMoney` (Humans) and `ConvertPopulationCost` (Vampires). Or single `RecruitCost` with faction-specific interpretation. Upkeep remains for ongoing cost (optional P4).
  - **Economy config:** DataAsset or DataTable: `MoneyPerPopulationPerSecond`, `MoneyPerStabilityMultiplier`, `ControlLevelMoneyBonus`, base rates. Vampire: `PopulationPerUnit` or per-unit-type from FUnitRow.

- **Economy subsystem**
  - **URTSEconomySubsystem** (GameInstanceSubsystem) or store in **ARTSGameState**.
  - Holds per-faction: `Money` (Humans), optionally cached "available Population" for Vampires (read from region).
  - `AddMoney(Faction, Amount)`, `SpendMoney(Faction, Amount)`, `CanAffordRecruit(Faction, UnitId)`.
  - `TickEconomy()` or timer: Humans gain Money from regions (Hero in region with control → rate). Vampires: no passive gain; conversion consumes region Population.

- **Region → Money (Humans)**
  - Rule: Faction gains Money from a region when Hero (or presence) is in region and faction has control.
  - Rate = f(Population, Stability, ControlLevel). Example: `BaseRate + Population * 0.1 + Stability * 2 + ControlLevel * 5` (tunable).
  - Implementation: periodic (e.g. every 5–10s) or on timer; for each region, for each faction with control and Hero in region, add Money.

- **Recruitment flow**
  - **Where:** Hero in region. Player initiates "Recruit" (input: e.g. key + click, or context menu, or dedicated UI).
  - **How:** Player selects unit type (from faction's available units). System checks: CanAfford? Hero in region? Spawn point valid?
  - **Cost:** Humans: deduct Money (RecruitCostMoney). Vampires: deduct region Population (ConvertPopulationCost).
  - **Spawn:** Spawn unit near Hero (or at region spawn point). Add to squad via RTSSquadManagerSubsystem. Initialize from registry.

- **Emergency decree (optional)**
  - GDD: "Emergency decree cheaper but reduces stability." Defer to P4.1 if time-constrained.

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

- [ ] Add `RecruitCostMoney` (int32) to FUnitRow for Human units. Default from existing Upkeep or new column.
- [ ] Add `ConvertPopulationCost` (int32) to FUnitRow for Vampire units. Default e.g. 10–50.
- [ ] Create `FEconomyConfigRow` or `DA_EconomyConfig`: MoneyPerPopulationPerSecond, MoneyPerStabilityBonus, ControlLevelMoneyBonus, or equivalent formula.
- [ ] Load config in DataRegistry or new EconomySubsystem.

### Epic 2: Economy Subsystem

- [ ] Create `URTSEconomySubsystem` (GameInstanceSubsystem).
- [ ] Per-faction `Money` (TMap<EFactionId, int32> or similar). Vampires: no Money; read Population from region when recruiting.
- [ ] `AddMoney(Faction, Amount)`, `SpendMoney(Faction, Amount)`, `GetMoney(Faction)`.
- [ ] `CanAffordRecruit(Faction, UnitId, Region)` – Humans: Money >= RecruitCostMoney; Vampires: Region.Population >= ConvertPopulationCost.
- [ ] Optional: `CanAffordConvert(Region, UnitId)` for Vampires.

### Epic 3: Money Gain (Humans)

- [ ] Timer (e.g. 5s): for each ARTSRegionVolume, for each faction with Hero in region and control > 0:
  - Compute rate from Population, Stability, ControlLevel (config).
  - AddMoney(Faction, Rate * DeltaTime).
- [ ] Ensure single-region MVP: one region in map; all logic uses it.

### Epic 4: Recruitment Logic

- [ ] `TryRecruitUnit(ARTSHeroCharacter* Hero, FName UnitId)` or `TryRecruitUnit(Hero, UnitId, Region)`.
- [ ] Validate: Hero in region; region valid; faction matches unit; CanAffordRecruit.
- [ ] Deduct: Humans: SpendMoney; Vampires: Region.Population -= ConvertPopulationCost.
- [ ] Spawn: SpawnActor at Hero location + offset (or region spawn point). InitializeFromRegistry.
- [ ] Add to squad: RTSSquadManagerSubsystem->AddUnitToFactionSquad(NewUnit).
- [ ] Return success/failure.

### Epic 5: Input & UI

- [ ] Add input binding for "Recruit" (e.g. R key, or Hold + click, or context menu).
- [ ] When Recruit pressed: if Hero selected and in region, show recruit options (unit types + cost). Minimal: list in log or simple widget.
- [ ] On select unit type: call TryRecruitUnit. If success, spawn unit; if fail, log reason.
- [ ] Optional: Simple recruit UI (UMG or Blueprint) showing available units and costs.

---

## 4. Tests & Validation

- [ ] Human: Hero in region, control > 0 → Money increases over time.
- [ ] Human: Enough Money → Recruit unit → Money deducted, unit spawned.
- [ ] Human: Not enough Money → Recruit fails.
- [ ] Vampire: Region has enough Population → Convert → unit spawned, Population deducted.
- [ ] Vampire: Not enough Population → Convert fails.
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
| Werewolves | Exclude or use Vampire conversion model; document. |
| Emergency decree | Defer to P4.1. |

---

## 7. Acceptance Criteria

- [ ] Humans: Money gained from region when Hero present and faction has control.
- [ ] Humans: Recruit unit costs Money; unit spawns when affordable.
- [ ] Vampires: Convert region Population into unit; Population deducted.
- [ ] Vampires: Convert fails when Population insufficient.
- [ ] Recruited unit: correct faction, initialized, added to squad (if P3 done).
- [ ] Input: Recruit action bound; player can recruit at least one unit type per faction.

**Definition of Done (P4):** Economy subsystem with Money (Humans) and Population conversion (Vampires); recruitment flow working; player can recruit units.

---

## 8. Rollback / Fallback

- If EconomySubsystem is heavy: store Money in GameState first; extract to subsystem later.
- If recruitment UI is complex: ship with log-only or key + unit-type hardcoded; add UI in P4.1.
- Emergency decree: skip for P4; add in follow-up.
