# RTS PROJECT – MASTER GDD (Cursor Implementation Edition)

**Source**: RTS_Master_GDD_Cursor_Edition_v1.0.docx  
**Status**: MVP Scope – Locked

---

## PROJECT PHILOSOPHY

This document defines the LOCKED systems, MVP scope, and implementation architecture for a Hero-Centric RTS built in Unreal Engine 5 (C++ + Blueprint hybrid).

- All military command flows through Heroes.
- New orders require Hero or Captain command range.
- Units evolve through combat.
- Permanent unit death.
- Region Control 0–5 determines victory.

---

## ARCHITECTURAL RULE

Every gameplay system must influence at least one of the following loops:

- **Combat Loop**
- **Army Loop**
- **Region Loop**
- **Macro Loop**

No isolated systems.

---

## CORE ENTITIES (C++ BASE, Blueprint EXTENDABLE)

### Hero (ARTSHeroCharacter)

- CommandRange
- Level (MVP simplified)
- FactionTag
- IssueOrderToUnitsInRange()

### Unit (ARTSUnitCharacter)

- Rank (1–3 MVP)
- Health
- Damage
- bIsCaptain
- CurrentOrder
- SquadReference

### Captain

- Unit with bIsCaptain = true
- Has LocalCommandRange (smaller than Hero)

### Squad (ARTSQuadActor)

- Members[]
- SquadMorale (0–100)
- Captain reference
- RecalcMorale()

### Region (ARTSRegionVolume)

- ControlLevelHuman (0–5)
- ControlLevelVampire (0–5)
- Population
- Stability
- DominantFaction

---

## HERO-CENTRIC COMMAND DOCTRINE (LOCKED)

- Only units within Hero or Captain command radius can receive NEW orders.
- Destination of order does not need to be in range.
- Existing orders persist when unit leaves range.
- Detached units (no authority nearby):
  - Defensive AI only
  - Slow morale drain
  - Reduced performance at low morale

---

## CAPTAIN SYSTEM (MVP)

- Unit eligible at Rank >= 3.
- Player manually promotes unit.
- Captain provides local command radius.
- Captain death causes morale penalty to squad.

---

## HERO DEATH & RESPAWN (MVP)

- All heroes respawn identically.
- On death: start respawn timer.
- When timer ends: player performs ritual action.
- Hero respawns in region with highest faction control.
- Death causes morale shock to nearby squads.

---

## REGION CONTROL SYSTEM

**Control Levels:**

| Level | Name |
|-------|------|
| 0 | None |
| 1 | Presence |
| 2 | Influence |
| 3 | Control |
| 4 | Established |
| 5 | Dominance |

- Control increases if Hero + units present uncontested.
- 4→5 requires active 'Secure Region' action.
- Skirmish victory: reach level 5.

---

## ECONOMY (MVP)

**Hierarchy:** Population → Stability → Overlord Resource

**Humans:**
- Use Money for recruitment.
- **Conscription Decree (צו גיוס):** Player issues decree that sets price for soldiers. People come from city/town/village (and other regions as hero influence grows), offer themselves; player chooses who to recruit. Stronger player → higher chance of higher quality/rank units offering themselves (higher cost and upkeep).
- Emergency decree cheaper but reduces stability.

**Vampires:**
- No money.
- Convert population into units.

---

## COMBAT RULES (MVP)

- Permanent unit death.
- Rank system 1–3.
- Morale affects:
  - Movement speed
  - Order execution speed
  - Auto-retreat chance at low threshold.
- Day/Night cycle active.
- Night grants vampires bonus; day slight penalty.

---

## MVP SCOPE (STRICT)

### Included

- Humans vs Vampires
- 1 Region
- 1 Hero per faction
- 2–3 unit types per faction
- Command radius system
- Squad morale
- Region control 0–5
- Hero respawn ritual

### Excluded

- Weather system
- PTSD system
- Multi-region map
- Mythic units
- Multi-hero scaling

---

## MVP LOCKED VALUES (from Companion v1.1)

### Development Order

| Phase | Focus |
|-------|-------|
| P0 | C++ base classes (Hero, Unit, Squad, Region), GameplayTags, basic map |
| P1 | Command radius + orders (persistent orders, units outside range ignore NEW) |
| P2 | Region control (0–5, Secure action, victory at 5) |
| P3 | Squad + Morale (container, 0–100, detached behavior) |
| P4 | Economy (Humans: Money / Vampires: Convert) |
| P5 | Hero respawn ritual |
| P6 | Day/Night + polish |

### Secure Region

- Available at Control Level 4
- Hero must be inside Region
- Region must not be contested (no enemy hero)
- Channel: 15 seconds
- Cancelled if hero leaves or enemy hero enters
- On success → Level 5 immediately

### Hero Respawn Ritual

- Respawn timer: 90 seconds (tunable)
- After timer → player presses "Perform Ritual"
- Ritual channel: 10 seconds
- Requires at least one region at Control ≥ 3
- On success → spawn in region with highest control

### Command Range

| Authority | Range (UU) |
|-----------|------------|
| Hero | 2500 |
| Captain | 1200 |

Receiving unit must be in range. Destination does NOT need to be in range. Orders persist when leaving range.

### Morale (0–100)

| Threshold | Effect |
|-----------|--------|
| < 30 | -15% Move Speed, -15% Order responsiveness |
| < 15 | 20% chance every 5s to Auto-Retreat |

**Detached units:** Defensive AI only. Morale drain: -1 every 5 seconds.

**Affected by:** Casualties, Hero presence, detached state.

### Rank Progression

| Rank | XP to next |
|------|------------|
| 1 → 2 | 100 |
| 2 → 3 | 300 |

**XP sources:** Combat participation, kill contribution, surviving combat. Rank 3 = Captain eligible.

### Input (MVP)

**Control: mouse only.** Arrow keys or WASD = map pan only.

| Input | Action |
|-------|--------|
| LMB | Select |
| Shift+LMB | Multi-select |
| RMB | Context Move/Attack |
| Mouse | Hold, Retreat, Promote, Secure Region, Perform Ritual (via mouse actions / context) |

### Camera

- Top-down only. Zoom in/out. Pan. No rotate in MVP.

### Win / Lose

| Condition | Result |
|-----------|--------|
| Reach Control Level 5 | Win |
| No living hero AND no region ≥ 3 | Lose |

---

## UNREAL IMPLEMENTATION GUIDELINES

- Core systems in C++ (Subsystems, Components).
- Tunables via DataAssets.
- Use GameplayTags for Factions and Orders.
- No hardcoded faction logic in base classes.
- Keep systems modular and testable.
- **Cursor must prioritize Epic official documentation over community sources.**
- **If conflicting implementation approaches exist, prefer Gameplay Framework-aligned architecture.**

---

## DEFINITION OF DONE (MVP)

**Playable Skirmish:**

- [ ] Move hero
- [ ] Recruit units
- [ ] Split squads
- [ ] Promote captain
- [ ] Fight
- [ ] Morale impacts behavior
- [ ] Secure region to level 5
- [ ] Hero death + ritual respawn works

---

## NOTES

- **Typo in source**: ARTSQuadActor → likely intended **ARTSSquadActor**
- **Werewolves**: GDD mentions Humans vs Vampires for MVP; existing project has Werewolves – defer to post-MVP

---

# CURSOR ANALYSIS (Reference)

*Lightweight reference – main rules are in locked sections above.*

**Project alignment:** DA_Faction_Humans, DA_Faction_Vampires; DA_Hero_Paladin, DA_Hero_HighVampireElder. Extend F_UnitStats/F_MoraleFearBundle for Rank, Morale. Werewolves = post-MVP.

**Economy MVP:** Humans = Money from region. Vampires = Convert population. Exact rates via DataAssets.
