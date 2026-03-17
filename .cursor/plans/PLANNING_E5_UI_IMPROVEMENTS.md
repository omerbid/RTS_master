# Plan: E5 UI/UX Improvements

**Created**: 2026-02  
**Status**: Planning  
**GDD Reference**: RTS_CHECKLIST_EPIC_VS_MVP.md (E5. UI/UX)

---

## 1. Current State

### What Exists
| Component | Status | Location |
|-----------|--------|----------|
| **Unit Info Panel** | ✅ Basic | `RTSUnitInfoWidget` – UnitId, Faction, Morale, Rank, HP (Current/Max), Captain/Detached |
| **Box Selection** | ✅ | `RTSHUD` – gray semi-transparent rect when dragging |
| **Command Radius** | ✅ Debug | `DrawDebugCircle` in PlayerTick – cyan circle (debug only, not in shipping) |
| **Win/Lose** | ✅ | `AddOnScreenDebugMessage` + `SetPause` in OnGameWon/OnGameLost |
| **Selection Ring** | ✅ | `RTSUnitCharacter::SetSelected` – ring at feet + CustomDepth |

### Gaps (from E5 Checklist)
- [ ] HUD: מורל, בריאות, טווח פקודה – **partial** (unit panel has morale/HP; command radius is debug)
- [ ] UI בחירת יחידות/גיבורים – **basic** (no portrait/icon strip for multi-select)
- [ ] מיפוי אזורים (שליטה, יציבות) במפה/מיני-מפה – **missing**
- [ ] תפריטים ומעברי מסכים – **missing**

---

## 2. Epic Breakdown

### Epic 1 – HUD: Morale, Health, Command Range
**Goal:** Clear, always-visible HUD elements for selected unit(s) and game state.

| Task | Priority | Description |
|------|----------|-------------|
| **1.1 HP Bar** | High | Replace text "HP: 70/100" with visual progress bar (green→yellow→red by %). |
| **1.2 Morale Bar** | High | Add morale bar (0–100) with color gradient (red &lt;30, yellow 30–70, green &gt;70). |
| **1.3 Command Radius HUD** | Medium | Move command circle from `DrawDebugCircle` to HUD (DrawHUD or Widget) so it works in shipping builds. Optional: subtle ring, not full debug draw. |
| **1.4 Resource Bar** | Medium | Top bar: Money (Humans) / Population (Vampires/Werewolves) from EconomySubsystem. |
| **1.5 Layout Polish** | Low | Panel background, padding, font size. Optional: faction-colored accents. |

**Files:** `RTSUnitInfoWidget`, `RTSHUD`, `RTSPlayerController`, `RTSEconomySubsystem`

---

### Epic 2 – Selection UI (Unit/Hero Strip)
**Goal:** When multiple units selected, show a strip of portraits/icons.

| Task | Priority | Description |
|------|----------|-------------|
| **2.1 Selection Strip** | High | Horizontal strip of unit icons when 2+ selected. Click icon → focus/select that unit. |
| **2.2 Hero vs Unit** | Medium | Different icon style for Hero (e.g. larger, gold border). |
| **2.3 Group Stats** | Low | When multi-select: show aggregate (e.g. "5 units, Avg Morale: 72"). |

**Files:** New `RTSSelectionStripWidget` or extend `RTSUnitInfoWidget`, `RTSPlayerController`

---

### Epic 3 – Region / Mini-Map
**Goal:** Show region control and stability on map or mini-map.

| Task | Priority | Description |
|------|----------|-------------|
| **3.1 Region Info Panel** | High | When Hero in region: show Region name, Control 0–5 per faction, Stability, Population. |
| **3.2 Mini-Map** | Medium | Small corner map with region bounds, control color overlay, player/Hero position. |
| **3.3 Region Tooltip** | Low | Hover over region in mini-map → tooltip with details. |

**Files:** New `RTSRegionInfoWidget`, `RTSMiniMapWidget`, `RTSRegionVolume`, `RTSHUD`

---

### Epic 4 – Menus & Screen Transitions
**Goal:** Main menu, pause menu, win/lose screen.

| Task | Priority | Description |
|------|----------|-------------|
| **4.1 Main Menu** | Medium | Simple menu: Play, Settings (optional), Quit. Uses UMG. |
| **4.2 Pause Menu** | Medium | ESC → Pause menu (Resume, Restart, Quit to Menu). |
| **4.3 Win/Lose Screen** | High | Replace `AddOnScreenDebugMessage` with full-screen widget: "Victory!" / "Defeat" + faction, Restart, Quit to Menu. |
| **4.4 Loading Screen** | Low | Optional: simple loading screen between menu and game. |

**Files:** New `WBP_MainMenu`, `WBP_PauseMenu`, `WBP_GameOver`, `RTSGameModeBase`, `RTSVictorySubsystem`

---

## 3. Execution Order

### Phase 1 – Quick Wins (1–2 days)
1. **Epic 1.1–1.2** – HP bar + Morale bar in Unit Info (Progress Bar widgets).
2. **Epic 4.3** – Win/Lose screen widget (replace debug message).

### Phase 2 – Core HUD (1–2 days)
3. **Epic 1.3** – Command radius in HUD (DrawHUD circle or Widget).
4. **Epic 1.4** – Resource bar (Money / Population).
5. **Epic 1.5** – Layout polish.

### Phase 3 – Selection & Region (2–3 days)
6. **Epic 2.1–2.2** – Selection strip for multi-select.
7. **Epic 3.1** – Region info panel.

### Phase 4 – Menus & Mini-Map (2–3 days)
8. **Epic 4.1–4.2** – Main menu, Pause menu.
9. **Epic 3.2** – Mini-map (optional, higher effort).

---

## 4. Technical Notes

### Widget Architecture
- **C++ base + Blueprint child:** Use `URTSUnitInfoWidget` as parent; create `WBP_UnitInfo` with Progress Bars. Bind via `BindWidget` or `FindWidget`.
- **New widgets:** Prefer C++ base class for logic, Blueprint for layout (UMG Designer).

### Data Sources
| Data | Source |
|------|--------|
| HP, Morale, Rank | `ARTSUnitCharacter`, `URTSMoraleComponent` |
| Money | `URTSEconomySubsystem::GetMoney` |
| Population, Stability, Control | `ARTSRegionVolume` |
| Win/Lose | `URTSVictorySubsystem` |
| Command Radius | `URTSCommandAuthorityComponent` |

### DrawDebugCircle
- `DrawDebugCircle` is debug-only (not in Shipping). For HUD circle: use `AHUD::DrawHUD` with `DrawRect`/projected circle, or a Widget with a circular image/brush.

---

## 5. Definition of Done (E5 MVP)

- [ ] Unit Info: HP bar + Morale bar (visual, not just text).
- [ ] Resource bar: Money or Population visible.
- [ ] Win/Lose: Full-screen widget (not debug message).
- [ ] Command radius: Visible in game (HUD or Widget).
- [ ] Selection strip: When 2+ units selected, show icons.
- [ ] Region info: When Hero in region, show control/stability.

**Stretch:** Main menu, Pause menu, Mini-map.

---

## 6. Files to Create/Modify

| File | Action |
|------|--------|
| `RTSUnitInfoWidget.h/.cpp` | Add Progress Bar refs, HP/Morale bar logic |
| `WBP_UnitInfo` (Blueprint) | Add Progress Bar widgets, bind |
| `RTSHUD.cpp` | Command radius circle (or new Widget) |
| `RTSResourceBarWidget` (new) | Money/Population display |
| `RTSSelectionStripWidget` (new) | Multi-select icons |
| `RTSRegionInfoWidget` (new) | Region control/stability |
| `RTSGameOverWidget` (new) | Win/Lose full screen |
| `RTSVictorySubsystem` | Broadcast to GameOver widget |
| `RTSPlayerController` | Create/add GameOver, ResourceBar widgets |
| `WBP_MainMenu`, `WBP_PauseMenu` | New (Phase 4) |
