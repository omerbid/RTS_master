# Plan: P2 Epics 1, 2, 4 — Region Queries, Control Progression, Win/Lose

**Created**: 2026-03-08  
**Status**: In Progress (Epics 1, 2, 4 implemented; build pending — close editor or disable Live Coding to compile)  
**Parent**: `PLANNING_P2_region_control.md` (Epics 1, 2, 4 only — no Secure Region input/Epic 3)  
**GDD**: `.cursor/GDD.md` (Region Control System, Win/Lose, MVP P2)

---

## 1. Task Summary

Implement the **safe-to-parallel** slice of P2: **(Epic 1)** region spatial queries and contestation, **(Epic 2)** control progression 0→4 from uncontested presence, and **(Epic 4)** win/lose conditions. No Hero input or Secure Region channel (Epic 3) in this plan — that can be added after P1 or in a follow-up.

**Scope:** ARTSRegionVolume extensions, a region/presence manager (or timer on region), and a victory/lose checker (subsystem or GameState). All implementable without touching P1’s selection/input.

**GDD alignment:** Region control 0–5 (passive gain to 4); win on control 5; lose when no living hero and no region ≥ 3.

---

## 2. Execution Plan

### 2.1 Logic & Architecture

**Epic 1 — Region**
- **IsPointInRegion(FVector)** — Use `RegionBounds` (UBoxComponent): get world transform and box extent, build FBox, test `IsInsideOrOn(WorldLocation)`. No overlap needed for this.
- **Tracking who is inside** — Use `RegionBounds` overlap events. Set `RegionBounds->SetCollisionEnabled(QueryOnly or CollisionEnabled::QueryOnly)` and `SetGenerateOverlapEvents(true)`; add `OnComponentBeginOverlap` / `OnComponentEndOverlap`. In Overlap, filter for `ARTSHeroCharacter` and optionally `ARTSUnitCharacter`; maintain a `TSet<AActor*>` or TArray of “actors in region” (or only heroes). Provide `GetHeroesInRegion()` / `HasHeroOfFaction(EFactionId)` for contestation.
- **IsContested()** — True if at least two different factions have a Hero inside the region. MVP: Humans vs Vampires; if both have a hero in region → contested.
- **SetControlLevelForFaction(Faction, Level)** — Clamp Level 0–5, set the corresponding ControlLevel* member, call **RecalcDominantFaction()**.
- **RecalcDominantFaction()** — Set `DominantFaction` to the faction with highest control (break tie by enum order or leave as-is). Private or BlueprintCallable.
- **Control level name (optional)** — Static helper or small table: 0=None, 1=Presence, 2=Influence, 3=Control, 4=Established, 5=Dominance for UI/debug.

**Epic 2 — Control progression**
- **Rule:** Control increases when a faction has at least one Hero (and optionally units) in the region and the region is **not** contested. Increase only up to **4**; level 5 is only via Secure (Epic 3).
- **Rate:** Tunable (e.g. +1 level every 30 seconds of continuous uncontested presence). Store in region or in a small config (e.g. `ControlGainIntervalSeconds` on region or DataAsset).
- **Implementation:**  
  - **Option A (recommended):** Timer on **ARTSRegionVolume** (e.g. `FTimerHandle`; set in `BeginPlay`). Every N seconds, for each faction: if region not contested and that faction has a hero inside, and that faction’s control < 4, increment control by 1.  
  - **Option B:** World subsystem that holds a list of regions and runs the same logic on a timer.  
  Use Option A to avoid new subsystem for this slice; keep Option B for later if we want central “region manager.”
- **Presence:** Use the same “heroes in region” set from Epic 1. “Faction has presence” = at least one `ARTSHeroCharacter` with that `FactionId` is in the region.
- **No decay** in this plan (GDD leaves it open; P2 minimal = no passive decay).

**Epic 4 — Win / Lose**
- **Win:** When any region’s control for any faction reaches **5**. Trigger immediately in `SetControlLevelForFaction` when Level == 5 (broadcast delegate or call into a victory subsystem).
- **Lose:** “No living hero AND no region with control ≥ 3 for that faction.” Evaluate per faction (e.g. Humans and Vampires in MVP). When to check: (1) when a Hero dies, (2) optionally on a short timer as backup. If for faction F: no living `ARTSHeroCharacter` with `FactionId == F` in the world AND no region has `GetControlLevelForFaction(F) >= 3` → trigger lose for F.
- **Ownership:** Use a **GameInstanceSubsystem** (e.g. `URTSVictorySubsystem`) so we don’t depend on a specific GameMode:
  - Subscribe to: control level changes (region calls subsystem when setting level 5), Hero death (need a way to notify — e.g. `OnHeroDeath` broadcast from Hero or from a simple death hook).
  - API: `NotifyControlReachedFive(ARTSRegionVolume* Region, EFactionId Faction)`, `NotifyHeroDeath(ARTSHeroCharacter* Hero)`. Subsystem evaluates win/lose and sets state (e.g. `EWorldState::Playing | Won | Lost`) and which faction won/lost. Optionally broadcast `OnGameWon(EFactionId)` / `OnGameLost(EFactionId)` for UI.
- **Hero death:** If there is no existing “OnDeath” on Hero, add a minimal hook: e.g. override or delegate when health reaches 0 / when `Destroy()` is called, and call `URTSVictorySubsystem::NotifyHeroDeath(this)`. If Hero doesn’t have health yet, subsystem can poll “all Heroes in world” on a slow timer (e.g. every 5s) and compare to previous frame to detect death; less ideal but no Hero change.

### 2.2 Build Order (implementation sequence)

1. **Epic 1.1** — `IsPointInRegion`, `SetControlLevelForFaction`, `RecalcDominantFaction` (no overlap yet).
2. **Epic 1.2** — Enable overlap on `RegionBounds`; track overlapping actors; implement `GetHeroesInRegion()` / `HasHeroOfFaction`, `IsContested()`.
3. **Epic 4.1** — Create `URTSVictorySubsystem`; implement win on control 5 (region calls subsystem when setting 5). No lose yet.
4. **Epic 2.1** — Timer on region; every N seconds increment control for factions with presence and not contested, cap at 4.
5. **Epic 4.2** — Lose condition: Hero death notification or polling; check “no living Hero and no region ≥ 3” per faction; call `OnGameLost(Faction)`.
6. **Epic 1.3 (optional)** — Control level name helper for UI.

### 2.3 File / Class Changes (summary)

| File / Class | Changes |
|--------------|---------|
| `RTSRegionVolume.h/.cpp` | IsPointInRegion, SetControlLevelForFaction, RecalcDominantFaction; overlap registration; HeroesInRegion set + GetHeroesInRegion, HasHeroOfFaction, IsContested; timer + control progression (Epic 2). |
| `RTSVictorySubsystem.h/.cpp` (new) | GameInstanceSubsystem; NotifyControlReachedFive, NotifyHeroDeath; win/lose state; optional delegates. |
| `RTSHeroCharacter` (optional) | Only if we add NotifyHeroDeath on death; else subsystem can poll. |
| `RTSDataTypes.h` (optional) | Optional enum or table for control level names. |

### 2.4 Dependencies

- P0: ARTSRegionVolume, ARTSHeroCharacter, ARTSUnitCharacter (FactionId), EFactionId.
- No P1 dependency for this slice.
- Engine: TimerManager (from World or Actor), GameInstanceSubsystem.

### 2.5 Complexity

| Aspect | Level | Notes |
|--------|--------|--------|
| Logic | Low–Medium | Straightforward spatial + timer + one win/lose pass. |
| Integration | Low | Region and new subsystem; minimal Hero touch. |
| Testing | Medium | PIE: move Hero in/out, verify control 0→4 and contested; set 5 via debug → win; kill Hero + no region ≥ 3 → lose. |

---

## 3. Epics & Tasks (detailed)

### Epic 1: Region Queries & Contestation

#### Task 1.1 — IsPointInRegion and control setter
- [ ] **1.1.1** In `ARTSRegionVolume`, add `bool IsPointInRegion(FVector WorldLocation) const`. Use `RegionBounds->GetScaledBoxExtent()` and `GetComponentTransform()` (or Actor transform) to build an `FBox` in world space; return `FMath::PointBoxIntersection(WorldLocation, Box)` or equivalent (e.g. transform point to local and test against extent).
- [ ] **1.1.2** Add `void SetControlLevelForFaction(EFactionId Faction, int32 Level)`. Clamp Level to 0–5. Set the corresponding `ControlLevelHumans` / `ControlLevelVampires` / `ControlLevelWerewolves`. Call `RecalcDominantFaction()`.
- [ ] **1.1.3** Add `void RecalcDominantFaction()`. Set `DominantFaction` to the faction with the highest control level (tie: use existing or first in enum order). Keep BlueprintReadWrite so editor can still override if needed.

#### Task 1.2 — Overlap tracking and contestation
- [ ] **1.2.1** In `ARTSRegionVolume::BeginPlay` (or constructor if needed), set `RegionBounds->SetCollisionEnabled(ECollisionEnabled::QueryOnly)` and `SetGenerateOverlapEvents(true)`. Ensure collision response to Pawn (or All) so that characters overlapping the box generate events. Bind `OnComponentBeginOverlap` / `OnComponentEndOverlap` to local handlers.
- [ ] **1.2.2** Add a `UPROPERTY` (e.g. `TSet<TWeakObjectPtr<AActor>> ActorsInRegion` or TArray) to track overlapping actors. In begin overlap: add actor if it’s `ARTSHeroCharacter` or `ARTSUnitCharacter` (or only heroes for contestation). In end overlap: remove. Handle actor destruction (e.g. remove when EndOverlap or when actor is pending kill).
- [ ] **1.2.3** Add `TArray<ARTSHeroCharacter*> GetHeroesInRegion() const` (or return TArray of actors and filter in callers). Implement by iterating overlapping actors and casting to `ARTSHeroCharacter`.
- [ ] **1.2.4** Add `bool HasHeroOfFaction(EFactionId Faction) const` — true if any hero in region has `FactionId == Faction`.
- [ ] **1.2.5** Add `bool IsContested() const` — true if at least two different factions have a hero in region (e.g. check HasHeroOfFaction for Humans and Vampires; if both true → contested). MVP: two factions.

#### Task 1.3 (optional) — Control level name
- [x] **1.3.1** Add `static FText GetControlLevelName(int32 Level)` or `static FName` returning None/Presence/Influence/Control/Established/Dominance for 0–5. Use for UI or debug.

---

### Epic 2: Control Progression (0–4)

#### Task 2.1 — Timer and progression rule
- [ ] **2.1.1** Add tunable: e.g. `UPROPERTY(EditAnywhere) float ControlGainIntervalSeconds = 30.f` on `ARTSRegionVolume`. Optionally `ControlGainMaxLevel = 4` (so we never auto-gain to 5).
- [ ] **2.1.2** In `BeginPlay`, call `GetWorld()->GetTimerManager().SetTimer(ControlGainTimerHandle, this, &ARTSRegionVolume::EvaluateControlGain, ControlGainIntervalSeconds, true)` (repeat every N seconds).
- [ ] **2.1.3** Implement `EvaluateControlGain()`:  
  - If `IsContested()`, do nothing (or optionally decay later).  
  - For each faction (Humans, Vampires; skip Werewolves if not in use): if `HasHeroOfFaction(Faction)` and `GetControlLevelForFaction(Faction) < 4`, call `SetControlLevelForFaction(Faction, GetControlLevelForFaction(Faction) + 1)`.
- [ ] **2.1.4** Ensure level 5 is never set by this path (cap at 4). Level 5 only via `SetControlLevelForFaction` from Secure (Epic 3) or from debug.

---

### Epic 4: Win / Lose Conditions

#### Task 4.1 — Victory subsystem and win on control 5
- [ ] **4.1.1** Create `URTSVictorySubsystem` (GameInstanceSubsystem). Add to module startup if needed (subsystems are auto-instantiated).
- [ ] **4.1.2** Add API `void NotifyControlReachedFive(ARTSRegionVolume* Region, EFactionId Faction)`. Set internal state e.g. `GameResult = EWon`, `WinningFaction = Faction`. Optionally broadcast a delegate `OnGameWon(EFactionId)` for UI.
- [ ] **4.1.3** In `ARTSRegionVolume::SetControlLevelForFaction`, when the new level is 5, get GameInstance from World and get `URTSVictorySubsystem`, call `NotifyControlReachedFive(this, Faction)`.
- [ ] **4.1.4** (Optional) Add getters: `bool IsGameOver()`, `EFactionId GetWinningFaction()`, or an enum `ERTSGameResult::Playing | Won | Lost` and which faction.

#### Task 4.2 — Lose condition (no hero + no region ≥ 3)
- [ ] **4.2.1** Add `void NotifyHeroDeath(ARTSHeroCharacter* Hero)` to `URTSVictorySubsystem`. Store `EFactionId Faction = Hero->FactionId` (Hero inherits FactionId from Unit). Then call an internal `CheckLoseCondition(Faction)`.
- [ ] **4.2.2** Implement `CheckLoseCondition(EFactionId Faction)`:  
  - If game already over, return.  
  - Check “is there any living `ARTSHeroCharacter` with this FactionId?” (e.g. `UGameplayStatics::GetAllActorsOfClass(World, ARTSHeroCharacter::StaticClass(), Out)` and filter by FactionId). If at least one living → no lose.  
  - Check “is there any region with `GetControlLevelForFaction(Faction) >= 3`?” (iterate all `ARTSRegionVolume` in world or get from a registry). If at least one such region → no lose (respawn possible later).  
  - If no living hero and no region ≥ 3 → set state to Lost for that faction, call `OnGameLost(Faction)`.
- [ ] **4.2.3** Hook Hero death: either (A) in `ARTSHeroCharacter` override or in a place that calls when Hero dies (e.g. when health zero or Destroy), call `GetGameInstance()->GetSubsystem<URTSVictorySubsystem>()->NotifyHeroDeath(this)`, or (B) in subsystem use a timer (e.g. every 5s) to iterate all Heroes and detect if one disappeared (compare to cached set). Prefer (A) if a death hook exists or can be added without touching P1 logic.
- [ ] **4.2.4** On game start, ensure subsystem state is Playing. When Win or Lose is set, optionally disable input or show result (can be Blueprint or later).

#### Task 4.3 — Optional: multi-faction lose
- [x] **4.3.1** If multiple factions (e.g. 2 players), call `CheckLoseCondition` for each faction when any Hero dies. So when Human hero dies, check Human lose; when Vampire hero dies, check Vampire lose.

---

## 4. Tests & Validation

- [ ] **Epic 1:** Place region in level; call `IsPointInRegion` from console or Blueprint with point inside/outside → correct. Set control via `SetControlLevelForFaction` → DominantFaction updates. Move Hero into region → overlap fires; `GetHeroesInRegion` / `HasHeroOfFaction` / `IsContested` correct (second Hero of other faction → contested).
- [ ] **Epic 2:** One Hero in region, no enemy. Wait 30s (or tuned interval) → control for that faction increases by 1. Stops at 4. When second Hero (enemy) enters, next tick should not increase (contested).
- [ ] **Epic 4:** Set control to 5 via debug or future Secure → win triggers (subsystem state / delegate). Kill only Hero of faction F and ensure no region has F’s control ≥ 3 → lose for F triggers.

---

## 5. Documentation & Acceptance

- [ ] Comment in code: control 0–4 from presence, 5 only via Secure; win/lose rules per GDD.
- [ ] Update parent plan `PLANNING_P2_region_control.md`: mark Epic 1, 2, 4 tasks done when complete.

**Acceptance (Epics 1+2+4):**
- [ ] IsPointInRegion, SetControlLevelForFaction, RecalcDominantFaction implemented and tested.
- [ ] Overlap tracks heroes; IsContested and HasHeroOfFaction work.
- [ ] Control auto-increases 0→4 when faction has hero in region and not contested; never auto 5.
- [ ] Control 5 triggers win (VictorySubsystem).
- [ ] No living Hero + no region ≥ 3 for that faction triggers lose (VictorySubsystem).

---

## 6. Risks & Rollback

| Risk | Mitigation |
|------|------------|
| Overlap not firing | Ensure RegionBounds collision preset includes overlap with Pawn; test with single Hero. |
| Circular dependency Region ↔ Subsystem | Region only gets GameInstance from World and calls subsystem; subsystem does not hold strong ref to region. |
| Hero death not detectable | Use polling in subsystem (slow timer) if no death hook available. |

**Rollback:** Disable timer (control gain) and win/lose checks via cvar or bool if needed; revert Region and new subsystem files.
