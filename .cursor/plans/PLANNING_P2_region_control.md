# Plan: P2 Region Control (0–5, Secure Action, Victory at 5)

**Created**: 2026-03-08  
**Updated**: 2026-03-07 (synced with code)  
**Status**: Phases 1–6 complete; Phase 7 partial. **Single source of truth for phase status:** `PLANNING_P2_phases_1_7.md`. Input (Secure key G), UI, Polish per PROJECT_STATUS §1. Last synced with code: 2026-03-07.  
**GDD**: `.cursor/GDD.md` (Region Control System, Secure Region, Win/Lose, MVP Locked Values P2)

**Related docs:** `PLANNING_P2_phases_1_7.md` (Phases 1–7) | `P0_P1_P2_REMAINING.md` (מה נשאר) | `P0_P1_QUICK_REFERENCE.md` | `RTS_ARCHITECTURE.md` | `PROJECT_MAP.md` | `PLANNING_P2_epics_1_2_4.md`

---

## Implementation status (aligned with current code)

| Area | Classes / files | Notes |
|------|------------------|--------|
| **Region** | `ARTSRegionVolume` | IsPointInRegion, SetControlLevelForFaction, RecalcDominantFaction, GetRegionAtLocation(WorldContext, Location), overlap → GetHeroesInRegion, HasHeroOfFaction, IsContested, GetControlLevelName; timer → EvaluateControlGain (0→4). |
| **Secure Region** | `URTSSecureRegionComponent`, `ARTSHeroCharacter::TryStartSecureRegion()` | 15s channel, validate every 0.5s, cancel on leave/contested; success → SetControlLevelForFaction(5). |
| **Win/Lose** | `URTSVictorySubsystem` | NotifyControlReachedFive → Win; NotifyHeroDeath → CheckLoseCondition (no hero + no region ≥ 3 → Lose). Hero Destroyed() calls NotifyHeroDeath. |
| **Remaining** | Input, UI | **Input:** Bind key/hold+click to `GetOrderIssuer()->TryStartSecureRegion()`. **UI:** Show Win/Lose result, optionally pause/disable input. See `P0_P1_P2_REMAINING.md`. |

**Placing a region:** Place **RTS Region Volume** in the level (Place Actors / All Classes). Select it → **Region Bounds** (Box) → set **Box Extent** in Details or scale with **R** in viewport so the green box covers the play area.

---

## 1. Task Summary

Implement P2: **region control levels 0–5** with rules for gaining control (Hero + units present, uncontested), the **Secure Region** action (available at level 4; 15s channel; Hero in region, no enemy hero; cancel on leave or enemy entry; success → level 5), and **win/lose conditions** (reach control 5 = win; no living hero AND no region ≥ 3 = lose).

**GDD alignment:** REGION CONTROL SYSTEM (levels 0–5: None, Presence, Influence, Control, Established, Dominance), Secure Region (GDD locked values), Win / Lose table, Definition of Done (Secure region to level 5).

**Depends on:** P0 done (ARTSRegionVolume, ARTSHeroCharacter, EFactionId). P1 recommended (selection, orders) so player can move Hero into region and trigger Secure.

---

## 2. Execution Plan

### 2.1 Logic & Architecture

- **Region (ARTSRegionVolume – existing)**
  - Already has: ControlLevelHumans, ControlLevelVampires, ControlLevelWerewolves (0–5), Population, Stability, DominantFaction, RegionBounds (BoxComponent).
  - **Add:**
    - `IsPointInRegion(FVector WorldLocation) const` – use RegionBounds (box) to test containment.
    - `IsContested()` – true if at least one Hero of a **different** faction is inside the region (both Human and Vampire heroes in = contested).
    - `GetDominantFaction()` / `RecalcDominantFaction()` – set DominantFaction from highest control level (MVP: single region; tie-break by design).
    - Optional: level name helper (0=None, 1=Presence, … 5=Dominance) for UI/debug.

- **Control progression (0–4, no Secure)**
  - Control **increases** when: faction has Hero + units present in region and region is **not contested** (no enemy Hero inside).
  - Implementation: periodic check (e.g. every N seconds via timer or GameState tick) or event when Hero/unit enters/exits region.
  - Per GDD: “Control increases if Hero + units present uncontested.” Define rate (e.g. +1 level per X seconds of uncontested presence) – can be DataAsset/tunable; MVP can use simple step (e.g. every 30s → +1 up to 4). **Level 5 only via Secure**, not by passive gain.
  - **Decrease:** GDD does not spell out decay; optional for P2: if contested or no presence, slow decay or hold. Simplest: no passive decay in P2; only Secure can push 4→5.

- **Secure Region action**
  - **Availability:** Faction’s control in that region is **4**. (Not 5; at 5 already won.)
  - **Requirements:** Hero is inside the region; region is not contested (no enemy Hero in region).
  - **Execution:** Player initiates “Secure Region” (input: e.g. Hold key + click region, or context action on region). Hero performs a **channel** for **15 seconds**.
  - **Cancel conditions:** Hero leaves the region, OR an enemy Hero enters the region → channel cancels.
  - **Success:** After 15s, set that faction’s control in the region to **5** → trigger **Victory** for that faction (skirmish win).
  - Implementation: `URTSSecureRegionComponent` on Hero, or ability/action in Hero BP that:
    - Validates: Hero in region, control == 4, not contested.
    - Starts 15s channel (timer or ability task).
    - Each tick/frame or short timer: re-validate; if invalid, cancel channel.
    - On completion: region->SetControlLevelForFaction(HeroFaction, 5); notify game to check win.

- **Win / Lose**
  - **Win:** Any faction reaches control level **5** in the (MVP) single region. → End game, declare winner (e.g. GameState or GameMode sets state, UI shows).
  - **Lose:** “No living hero AND no region ≥ 3” – i.e. player has no Hero alive and has no region with their control ≥ 3 (so cannot respawn in acceptable region). → End game, declare defeat.
  - Ownership: GameMode or dedicated subsystem (e.g. `URTSVictorySubsystem`) that:
    - Subscribes to control-level changes and Hero death.
    - On control 5: set win.
    - Periodically or on Hero death: check “any living Hero for faction?” and “any region with control ≥ 3 for that faction?”; if no Hero and no such region → lose.

- **Data flow**
  - Region volumes register with a manager or are found by overlap/query (e.g. “get all ARTSRegionVolume”, or single region in MVP).
  - Hero/Unit overlap with RegionBounds: trigger enter/exit events or polling (e.g. “which region is Hero in?”).
  - Secure action lives on Hero (component or ability); reads region under Hero and calls region API.

### 2.2 Complexity Assessment

| Aspect       | Level  | Notes |
|-------------|--------|--------|
| Logic       | Medium | Control rules, Secure channel, win/lose branching. |
| Integration | Medium | Region, Hero, GameMode/GameState; input for Secure. |
| Testing     | Medium | PIE: gain control to 4, Secure to 5, win; Hero death + no region ≥ 3 = lose. |

### 2.3 Performance Considerations

- No per-frame sweep of all units in all regions if avoidable; use overlap events (OnBeginOverlap/OnEndOverlap on RegionBounds) for “who is in region.”
- Control progression: timer-based (e.g. 30s) rather than every tick.
- Secure channel: check every 0.5–1s for “still in region / still uncontested” instead of every frame if desired.
- Win/lose: evaluate on control change and on Hero death, not every frame.

### 2.4 Build Strategy

- **Step 1 – Region queries:** Add `IsPointInRegion`, overlap handling or “get region at location,” and `IsContested(World)` (query Heroes in region by faction). Optionally `SetControlLevelForFaction(Faction, Level)` with clamp 0–5 and RecalcDominantFaction.
- **Step 2 – Control progression:** Implement uncontested presence → increase control 0→1→2→3→4 over time (timer or event-driven). No passive 4→5; Secure only.
- **Step 3 – Secure Region:** Add Secure action to Hero (component or ability). Input binding (e.g. Hold + click or context menu). Validation + 15s channel; cancel on leave or enemy Hero enter. On success: set control 5 and notify.
- **Step 4 – Win/Lose:** GameMode or subsystem checks on control 5 (win) and on Hero death + no region ≥ 3 (lose). Hook into game end flow (set state, show result).
- **GDD alignment:** Region Control System, Secure Region, Win/Lose table.

---

## 3. Epics & Tasks

### Epic 1: Region Queries & Contestation

- [x] Add `IsPointInRegion(FVector WorldLocation) const` to ARTSRegionVolume (use RegionBounds box).
- [ ] Add overlap or spatial query so region knows which actors (Heroes/units) are inside; or provide `GetActorsInRegion()` / “is Hero in this region?” using existing bounds.
- [ ] Implement `IsContested()`: true if any Hero of a faction **other than** the one with highest control (or “any two factions with Heroes inside”) is in region. MVP: two factions; contested = Human Hero and Vampire Hero both in region.
- [x] Add `SetControlLevelForFaction(EFactionId Faction, int32 Level)` with clamp 0–5; update DominantFaction after change.
- [x] Optional: helper for control level name (0=None … 5=Dominance) for UI.

### Epic 2: Control Progression (0–4)

- [ ] Define rule: control increases when faction has Hero (+ optionally units) in region and region not contested. Rate: tunable (e.g. +1 per 30s up to 4).
- [ ] Implement periodic evaluation (timer on GameState, or on Region, or subsystem) to add “presence time” and increment level. Ensure 0–4 only; 5 only via Secure.
- [ ] Optional: decay or stagnation when contested or no presence (GDD leaves open; can be “no change” for P2).

### Epic 3: Secure Region Action

- [ ] Add input binding for “Secure Region” (e.g. Hold + LMB on region, or key + click, per GDD “Hold, … Secure Region”). **→ Bind P1 input to call selected Hero’s `TryStartSecureRegion()` (C++/Blueprint).**
- [x] Implement Secure logic on Hero: validate Hero in region, control level for Hero’s faction == 4, not contested. (`URTSSecureRegionComponent` + `TryStartSecureRegion()`)
- [x] Start 15-second channel; re-validate periodically (Hero still in region, no enemy Hero entered). Cancel channel if invalid.
- [x] On channel success: call region->SetControlLevelForFaction(HeroFaction, 5); broadcast “region secured” / “control 5 reached” for win condition.
- [ ] Visual/audio feedback (optional): use `OnSecureRegionStarted` / `OnSecureRegionCancelled` / `OnSecureRegionCompleted` delegates.

### Epic 4: Win / Lose Conditions

- [ ] On any SetControlLevelForFaction(_, 5): trigger win for that faction (GameMode or subsystem).
- [ ] On Hero death (or periodically): check for each faction – “has living Hero?” and “has any region with control ≥ 3?”. If no living Hero and no region ≥ 3 for that faction → trigger lose for that faction.
- [ ] Game end flow: set game state (e.g. Won/Lost), show result in UI or log; optionally disable input / return to menu.

### Epic 5: Integration & Polish

- [ ] Ensure single-region MVP: one ARTSRegionVolume in level; all “region” logic uses it (or finds it).
- [ ] Blueprint hooks where useful (e.g. OnControlLevelChanged, OnRegionSecured, OnGameWon/OnGameLost).
- [ ] Documentation: comment Secure rules (15s, cancel conditions) and win/lose in code.

---

## 4. Tests & Validation

- [x] Hero and units in region, no enemy: control increases over time (0→1→2→3→4), not to 5 without Secure.
- [x] At control 4: Secure Region available; Hero inside, no enemy. Channel 15s completes → control 5, win triggers.
- [x] Secure channel: Hero leaves region mid-channel → channel cancels.
- [x] Secure channel: Enemy Hero enters region mid-channel → channel cancels.
- [x] Win: control 5 → game reports win for that faction (VictorySubsystem).
- [x] Lose: Hero dead and no region with control ≥ 3 for that faction → game reports lose (VictorySubsystem).

---

## 5. Documentation

- [x] Inline comments: Secure Region rules (15s, cancel), win/lose conditions (GDD).
- [x] Plan status updated when epics complete.
- [x] DataAsset or config for control gain rate and Secure duration if tunable (ControlGainIntervalSeconds, SecureChannelDurationSeconds on Region/Component).

---

## 6. Dependencies & Risks

### Dependencies

- P0: ARTSRegionVolume, ARTSHeroCharacter, EFactionId.
- P1 (recommended): selection and orders so player can move Hero into region and trigger Secure.
- GameMode or GameState that can set “game over” and winner/loser.

### Risks

| Risk | Mitigation |
|------|------------|
| Overlap vs polling | Prefer overlap events on RegionBounds for “who is in region”; fallback to periodic overlap query. |
| Multiple regions (post-MVP) | P2 assumes one region; design APIs (e.g. “get region at location”) so multi-region can be added later. |
| Input for Secure | Use same input style as P1 (context action or Hold); document in Input (MVP). |

---

## 7. Acceptance Criteria

- [x] Control level 0–5 per faction in region; passive gain only up to 4 when Hero (+ units) present and uncontested.
- [x] Secure Region: available at 4, Hero in region, not contested; 15s channel; cancel if Hero leaves or enemy Hero enters; success → level 5.
- [x] Reach control 5 → skirmish win for that faction.
- [x] No living Hero and no region with control ≥ 3 for that faction → lose.
- [x] Single region MVP: one region in map; win/lose and Secure work with it.
- [ ] **Input:** Secure Region bound to key/hold+click (call `TryStartSecureRegion()` on selected Hero). **UI:** Win/Lose feedback (see P0_P1_P2_REMAINING).

**Definition of Done (P2):** Region control 0–5 with uncontested presence gain; Secure Region action working; win on 5, lose on no Hero and no region ≥ 3.

---

## 8. Rollback / Fallback

- If Secure channel is complex: implement as simple “hold key for 15s” first; refine cancel conditions after.
- If win/lose is not yet in GameMode: implement as subsystem or Blueprint that sets a flag and logs; wire to UI later.
- Control progression: minimal version = manual test commands to set level 0–4; then add timer-based gain in a follow-up task.
