# Plan: P2 Region Control – Phases 1–7

**Created**: 2026-02-21  
**Status**: Phases 1–6 complete; Phase 7 partial  
**GDD**: `.cursor/GDD.md` (Region Control System, Secure Region, Win/Lose, MVP P2)  
**Parent**: `PLANNING_P2_region_control.md`

---

## Overview

P2 is split into **7 phases** aligned with the current implementation and remaining work. Phases 1–4 are implemented; Phases 5–7 complete the MVP.

| Phase | Focus | Status |
|-------|--------|--------|
| 1 | Region Queries & Contestation | ✅ Done |
| 2 | Control Progression (0→4) | ✅ Done |
| 3 | Secure Region – Core Logic | ✅ Done |
| 4 | Win/Lose Conditions | ✅ Done |
| 5 | Secure Region Input | ✅ Done |
| 6 | Win/Lose UI & Game End | ✅ Done |
| 7 | Integration & Polish | ⏳ Partial |

---

## Phase 1: Region Queries & Contestation

**Status**: ✅ Complete  
**Classes**: `ARTSRegionVolume`

### Tasks

- [x] `IsPointInRegion(FVector WorldLocation)` – RegionBounds box containment
- [x] Overlap on RegionBounds – `OnRegionBoundsBeginOverlap` / `OnRegionBoundsEndOverlap`; `ActorsInRegion`
- [x] `GetHeroesInRegion()` – filter overlapping actors to `ARTSHeroCharacter`
- [x] `HasHeroOfFaction(EFactionId)` – any Hero in region with that FactionId
- [x] `IsContested()` – two or more factions have Hero in region (Humans, Vampires, Werewolves)
- [x] `SetControlLevelForFaction(Faction, Level)` – clamp 0–5, RecalcDominantFaction
- [x] `RecalcDominantFaction()` – set DominantFaction from highest control
- [x] `GetControlLevelName(int32)` – static helper 0=None … 5=Dominance
- [x] `GetRegionAtLocation(WorldContextObject, Location)` – find region containing point

---

## Phase 2: Control Progression (0→4)

**Status**: ✅ Complete  
**Classes**: `ARTSRegionVolume`

### Tasks

- [x] `ControlGainIntervalSeconds` (default 30) – tunable on Region
- [x] `ControlGainMaxLevel = 4` – passive gain stops at 4; 5 only via Secure
- [x] Timer in BeginPlay – `EvaluateControlGain` every N seconds
- [x] `EvaluateControlGain()` – if not contested, for each faction with Hero in region and control < 4, increment by 1
- [x] No passive decay (GDD leaves open)

---

## Phase 3: Secure Region – Core Logic

**Status**: ✅ Complete  
**Classes**: `URTSSecureRegionComponent`, `ARTSHeroCharacter`

### Tasks

- [x] `URTSSecureRegionComponent` on Hero
- [x] `CanStartSecureRegion()` – Hero in region, control == 4, not contested
- [x] `StartSecureRegion()` – 15s channel; validation every 0.5s
- [x] Cancel on: Hero leaves region, enemy Hero enters region
- [x] On success: `SetControlLevelForFaction(HeroFaction, 5)` → triggers VictorySubsystem
- [x] `TryStartSecureRegion()` on Hero – calls `SecureRegionComponent->StartSecureRegion()`
- [x] Delegates: `OnSecureRegionStarted`, `OnSecureRegionCancelled`, `OnSecureRegionCompleted`

---

## Phase 4: Win/Lose Conditions

**Status**: ✅ Complete  
**Classes**: `URTSVictorySubsystem`, `ARTSHeroCharacter`, `ARTSRegionVolume`

### Tasks

- [x] `URTSVictorySubsystem` (GameInstanceSubsystem)
- [x] `NotifyControlReachedFive(Region, Faction)` – set win state
- [x] Region calls subsystem when `SetControlLevelForFaction(_, 5)`
- [x] `NotifyHeroDeath(Hero)` – Hero `Destroyed()` calls this
- [x] `CheckLoseCondition(World, Faction)` – no living Hero + no region ≥ 3 → lose
- [x] Multi-faction: check lose per faction when any Hero dies

---

## Phase 5: Secure Region Input

**Status**: ✅ Complete  
**Classes**: `ARTSPlayerController`

### Tasks

- [x] Bind key S to trigger Secure Region when Hero selected
- [x] `OnInputSecureRegion()`: GetOrderIssuer → Cast to Hero → TryStartSecureRegion()
- [x] On-screen feedback: "Secure Region started" / "Cannot Secure: Hero must be in region, control 4, not contested"
- [x] Skip when game over (IsGameOver check)

---

## Phase 6: Win/Lose UI & Game End

**Status**: ✅ Complete  
**Classes**: `ARTSPlayerController`, `URTSVictorySubsystem`

### Tasks

- [x] VictorySubsystem exposes `IsGameOver()`, `GetWinningFaction()`, `GetLosingFaction()`, `OnGameWon`, `OnGameLost`
- [x] PlayerController subscribes to `OnGameWon` / `OnGameLost` in BeginPlay
- [x] On Win: AddOnScreenDebugMessage "*** VICTORY! [Faction] ***", SetPause(true)
- [x] On Lose: AddOnScreenDebugMessage "*** DEFEAT! [Faction] ***", SetPause(true)
- [x] Command range indicator skipped when game over

---

## Phase 7: Integration & Polish

**Status**: ⏳ Partial  
**Scope**: Single-region MVP, docs, optional hooks

### Tasks

- [x] Single-region MVP: one `ARTSRegionVolume` in level; `GetRegionAtLocation` finds it
- [ ] Blueprint hooks (optional): `OnControlLevelChanged`, `OnRegionSecured`, `OnGameWon` / `OnGameLost` – for UI/FX
- [ ] Documentation: comment Secure rules (15s, cancel conditions) and win/lose in code
- [ ] Place region: Place Actors → All Classes → RTS Region Volume; set Region Bounds (Box) extent to cover play area
- [ ] Optional: Visual/audio for Secure channel (progress bar, VFX, SFX) – wire to `OnSecureRegionStarted` / `OnSecureRegionCompleted`

---

## Dependencies

| Phase | Depends On |
|-------|------------|
| 1 | P0 (ARTSRegionVolume, ARTSHeroCharacter, EFactionId) |
| 2 | Phase 1 |
| 3 | Phase 1, P1 (selection so Hero can be in region) |
| 4 | Phase 1, Phase 3 |
| 5 | Phase 3, P1 (GetOrderIssuer, selection) |
| 6 | Phase 4 |
| 7 | Phases 1–6 |

---

## Related Documents

- `PLANNING_P2_region_control.md` – Full P2 plan (Epics 1–5)
- `PLANNING_P2_epics_1_2_4.md` – Detailed Epics 1, 2, 4 tasks
- `P0_P1_P2_REMAINING.md` – What remains (Input, UI)
- `PLANNING_P1_command_and_orders.md` – P1 (selection, GetOrderIssuer)
- `PLANNING_P4_economy.md` – P4 depends on Region (Population, Stability)

---

## Definition of Done (P2)

- [x] Control 0–5 per faction; passive gain 0→4; Secure for 4→5
- [x] Secure Region: 15s channel, cancel on leave/contested
- [x] Win on control 5; Lose on no Hero + no region ≥ 3
- [x] **Input:** Secure Region bound to key S
- [x] **UI:** Win/Lose feedback (on-screen message, SetPause)
