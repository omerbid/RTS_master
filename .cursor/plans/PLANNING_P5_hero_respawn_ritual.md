# Plan: P5 Hero Respawn Ritual

**Created:** 2026-03-07  
**Status:** Implemented  
**GDD:** `.cursor/GDD.md` – HERO DEATH & RESPAWN (MVP), Hero Respawn Ritual, Definition of Done  
**Depends on:** P0 (Hero, Region, Victory), P2 (NotifyHeroDeath, CheckLoseCondition), P3 (morale for shock)

---

## 1. Goal

Implement **Hero respawn ritual** per GDD:

- On Hero death: if faction has at least one region with control ≥ 3 → **do not lose**; start **respawn timer** (90s).
- When timer ends → player can **Perform Ritual** (input).
- Ritual = **10s channel**. Cancelled if interrupted (e.g. move / take damage – define in task).
- On channel success → **spawn Hero** in the region with **highest faction control** (tie-break: first found).
- Hero death causes **morale shock** to nearby squads.

**GDD quotes:**  
"All heroes respawn identically." / "Respawn timer: 90 seconds (tunable)." / "After timer → player presses Perform Ritual." / "Ritual channel: 10 seconds." / "Requires at least one region at Control ≥ 3." / "On success → spawn in region with highest control." / "Death causes morale shock to nearby squads."

---

## 2. Current State (Do Not Remove)

| System | Existing behavior |
|--------|-------------------|
| **Hero death** | `ARTSHeroCharacter::Destroyed()` → `URTSVictorySubsystem::NotifyHeroDeath(this)` |
| **Lose** | `CheckLoseCondition(Faction)`: no living Hero **and** no region with control ≥ 3 → lose |
| **Morale** | `URTSMoraleComponent`, squad `RecalcMorale()`, Captain death → morale penalty |
| **Secure Region** | `URTSSecureRegionComponent` – 15s channel; cancel if hero leaves or enemy enters (reference for ritual channel) |

**Rule:** Extend only. Respawn is an **additional** path when Hero dies and faction has region ≥ 3; lose only when no Hero **and** no such region.

---

## 3. Respawn Flow

```
Hero dies
    → NotifyHeroDeath(Hero)
    → If no region with control ≥ 3 for Faction: CheckLoseCondition(Faction) [unchanged]
    → If has region ≥ 3:
          1. Store Faction, HeroId for respawn
          2. Apply morale shock to squads near Hero (optional radius, e.g. CommandAuthority range)
          3. Start respawn timer (90s, tunable)
    → When timer ends: set state "Ritual available" for that Faction
    → Player presses "Perform Ritual" (no target; or context on region/camera)
          4. Start ritual channel (10s)
          5. If channel cancelled (e.g. move, or define "no cancel" for MVP): abort
          6. If channel completes: SpawnHeroInBestRegion(Faction, HeroId)
          7. Clear respawn state for Faction
```

**MVP simplification:** Ritual can be "press key and hold for 10s" with no movement, or a dedicated key that starts channel (cancel on move/interrupt). Same pattern as Secure Region (channel + cancel conditions).

---

## 4. Design Choices

| Topic | Choice | Notes |
|-------|--------|--------|
| **Who can perform ritual?** | The **player** whose Hero died (that faction). No unit/Hero needed – timer then key. | GDD: "player performs ritual action". |
| **Spawn location** | Region with **max control** for that faction (≥ 3). Use region actor location or center of volume. | GDD: "spawn in region with highest control". |
| **Hero identity** | Same **HeroId** as dead Hero; load from DT_Heroes_MVP + unit row; `InitializeFromRegistry()`. | GDD: "all heroes respawn identically". |
| **Timer / channel config** | 90s respawn, 10s channel – **tunable** (e.g. UStruct or DataAsset; default 90 / 10). | GDD: tunable. |
| **Morale shock** | On Hero death, apply morale penalty to squads within range (e.g. 2500 UU). Value from data or constant (e.g. -15). | GDD: "Death causes morale shock to nearby squads." |

---

## 5. Architecture

### 5.1 Option A – Extend URTSVictorySubsystem

- Add respawn state per faction: **WaitingForTimer** | **RitualAvailable** | **ChannelingRitual**.
- Add: `RespawnTimerSeconds`, `RitualChannelSeconds`, `RespawnHeroIdPerFaction`, `RespawnStatePerFaction`, `RespawnTimerHandle`, `RitualChannelHandle`.
- `NotifyHeroDeath(Hero)`:
  - If no region ≥ 3 → `CheckLoseCondition` (unchanged).
  - Else: save Hero's `HeroId` and `FactionId`, apply morale shock, start 90s timer.
- On timer expiry: set state **RitualAvailable** for that faction; broadcast optional `OnRespawnRitualAvailable(Faction)` for UI.
- New API: `bool CanPerformRitual(EFactionId Faction)`, `bool StartRitualChannel(EFactionId Faction)` (returns false if not RitualAvailable), `CancelRitualChannel(Faction)`, internal `OnRitualChannelComplete(Faction)` → `SpawnHeroInBestRegion(Faction, HeroId)`.

### 5.2 Option B – New URTSRespawnSubsystem

- GameInstanceSubsystem: holds all respawn state and timers; calls into Victory only for `CheckLoseCondition` when no region ≥ 3.
- Victory’s `NotifyHeroDeath` delegates to RespawnSubsystem when faction has region ≥ 3; Respawn calls back to Victory only if needed (e.g. never for win/lose; lose still via Victory when no Hero and no region ≥ 3).

**Recommendation:** **Option A** (extend Victory) for MVP – one place for win/lose and respawn; fewer subsystems. If respawn grows (e.g. multiple heroes, different rituals), split to Option B later.

---

## 6. Key Implementation Details

### 6.1 NotifyHeroDeath (updated)

```text
1. If GameResult != Playing: return
2. Faction = Hero->FactionId, HeroIdToRespawn = Hero->HeroId
3. Check: any region with GetControlLevelForFaction(Faction) >= 3?
4. If NO  → CheckLoseCondition(World, Faction); return
5. If YES → ApplyMoraleShockToSquadsNear(Hero); StartRespawnTimer(Faction, HeroIdToRespawn)
```

(Do not call `CheckLoseCondition` when respawn is possible.)

### 6.2 Respawn timer

- Use `FTimerHandle` (e.g. on World or subsystem). On expiry: set state RitualAvailable for Faction; optional broadcast for UI (e.g. "Press X to Perform Ritual").

### 6.3 Perform Ritual (input)

- **Input:** GDD: "Perform Ritual (via mouse actions / context)". MVP: dedicated key (e.g. **R** when respawn available, or **B** for ritual) or Hold + key. PlayerController checks `CanPerformRitual(LocalFaction)` and calls `StartRitualChannel(LocalFaction)`.
- **Channel:** 10s. Same pattern as Secure Region: start channel, tick/countdown, cancel on condition (e.g. player moves camera / unit selection action = cancel; or MVP: no cancel once started). On success → `SpawnHeroInBestRegion(Faction, HeroId)`.

### 6.4 Spawn Hero

- **Find region:** Iterate all `ARTSRegionVolume`; pick one with `GetControlLevelForFaction(Faction)` max and ≥ 3 (first if tie).
- **Location:** `Region->GetActorLocation()` or center of region volume (if available). Optionally add small random offset to avoid stacking.
- **Spawn:** `World->SpawnActor<ARTSHeroCharacter>(HeroClass, Location, Rotator, SpawnParams)`. Need **Hero class** per faction or from data – e.g. from GameMode or a small map Faction → Hero Blueprint class; set `NewHero->HeroId = HeroIdToRespawn`, `NewHero->FactionId = Faction`, then `NewHero->InitializeFromRegistry()`.
- **Hero class:** Currently Hero is placed in level; for respawn we need a C++ or Blueprint class to spawn. Use `ARTSHeroCharacter::StaticClass()` or a registry/GameMode table (Faction → UClass*) so the correct Blueprint is used (e.g. BP_Hero_Human, BP_Hero_Vampire).

### 6.5 Morale shock

- In `NotifyHeroDeath`, after deciding respawn: get Hero location; for each squad (e.g. via `URTSSquadManagerSubsystem` or iterate units), if squad center or any unit in range (e.g. 2500 UU) of Hero location → apply morale delta (e.g. -15). Use existing `ApplyMoraleDeltaToAll` or equivalent on squad.

---

## 7. Data / Config

| Item | Type | Default | Notes |
|------|------|--------|--------|
| RespawnTimerSeconds | float | 90 | Tunable |
| RitualChannelSeconds | float | 10 | Tunable |
| MoraleShockRadius | float | 2500 | UU; optional |
| MoraleShockDelta | int | -15 | Per squad in range |

Can live in subsystem as UPROPERTY(EditDefaultsOnly) or in a small DataAsset/GameMode.

---

## 8. Input

- **Perform Ritual:** When `CanPerformRitual(Faction)` is true, bind key (e.g. **B** or **R** when in "respawn available" state) to call `StartRitualChannel(Faction)`. UI hint: "Press [B] to Perform Ritual" when RitualAvailable.

---

## 9. Tasks (Phased)

### Phase 1 – Respawn state and timer

- [x] Add to Victory: respawn state per faction, RespawnTimerSeconds (90), stored HeroId per faction.
- [x] In `NotifyHeroDeath`: if faction has region ≥ 3, store Faction + HeroId, start 90s timer; else call `CheckLoseCondition` only.
- [x] On timer expiry: set RitualAvailable for that faction; broadcast `OnRespawnRitualAvailable(Faction)`.

### Phase 2 – Ritual channel and spawn

- [x] Add `CanPerformRitual(Faction)`, `StartRitualChannel(Faction)`, `CancelRitualChannel(Faction)` (MVP: no cancel during channel).
- [x] On channel complete: `SpawnHeroInBestRegion(Faction, HeroId)` – find region with max control ≥ 3, spawn at region location, set HeroId + FactionId, `InitializeFromRegistry()`.
- [x] Hero class: `HeroRespawnClass` on VictorySubsystem (default null = ARTSHeroCharacter::StaticClass()); set in Blueprint to use BP_RTSHero.

### Phase 3 – Input and UI

- [x] Bind **B** to Perform Ritual when RitualAvailable; call `GetFactionWithRitualAvailable` then `StartRitualChannel(Faction)`.
- [ ] Optional: simple UI message when RitualAvailable ("Press [B] to Perform Ritual") and during channel.

### Phase 4 – Morale shock

- [x] On Hero death (when starting respawn): `ApplyMoraleShockToSquadsNear` – same-faction squad members in MoraleShockRadius (2500 UU) get MoraleShockDelta (-15).

---

## 10. Definition of Done (P5)

- [x] Hero death when faction has region ≥ 3 does **not** trigger lose; respawn timer (90s) starts.
- [x] When timer ends, player can press **B** (Perform Ritual).
- [x] Ritual: 10s channel; on success Hero spawns in region with highest control for that faction; same HeroId, initialized from registry.
- [x] Hero death applies morale shock to same-faction squad in range (MoraleShockRadius 2500, MoraleShockDelta -15; tunable).
- [x] If Hero dies and faction has **no** region ≥ 3, lose condition unchanged (current behavior).
- [x] Respawn timer and channel duration tunable on VictorySubsystem (RespawnTimerSeconds, RitualChannelSeconds).

---

## 11. Files to Touch (Estimated)

| File | Changes |
|------|--------|
| `RTSVictorySubsystem.h/.cpp` | Respawn state, timer, channel, `NotifyHeroDeath` update, `SpawnHeroInBestRegion`, morale shock call |
| `RTSPlayerController.cpp` | Bind Perform Ritual key; check `CanPerformRitual` / `StartRitualChannel` |
| `RTSGameModeBase` or config | Optional: Faction → Hero class for spawn |
| Optional: `RTSRespawnSubsystem` | If Option B chosen instead of extending Victory |

---

## 12. References

- **GDD:** `.cursor/GDD.md` – HERO DEATH & RESPAWN (MVP), Hero Respawn Ritual, Input (Perform Ritual), Win/Lose.
- **Secure Region:** `URTSSecureRegionComponent` – channel + cancel pattern.
- **P2:** `PLANNING_P2_region_control.md`, `PLANNING_P2_epics_1_2_4.md` – NotifyHeroDeath, CheckLoseCondition.
- **P3:** `PLANNING_P3_squad_morale.md` – morale, squad, ApplyMoraleDeltaToAll.
