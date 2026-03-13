
# RTS Project – Master Design Document (MVP)

## Core Concept
A continuous campaign RTS where the world simulation (population, ecology, regions)
exists on the same map as tactical battles. There is no separation between campaign
map and battle map.

Factions:
- Humans
- Vampires
- Werewolves

## Core Pillars
1. Living World – population, wildlife, and settlements evolve dynamically.
2. Region Control – territory is the primary strategic resource.
3. Hero-Centric Command – heroes lead armies and stabilize regions.
4. Ecological Economy – monsters rely on humans and wildlife as resources.

---

# Core Gameplay Loop

Population grows
↓
Regions generate food and wealth
↓
Monsters raid or hunt
↓
Humans flee or fortify
↓
Cities grow or collapse
↓
Settlements rebuild or wilderness spreads
↓
Cycle repeats

---

# Map Structure (MVP)

Map Size: 4km x 4km

Region Types:
Capital (1)
Cities (2)
Towns (3)
Villages (~10)
Hamlets (~6)
Wild Forest (~5)
Ruins (~5)

Regions contain:
Population
FoodCapacity
WildlifeCount
ThreatLevel
GarrisonPower
SettlementState

---

# Campaign Layer

Each Region tracks:

OwnerFaction
ControlLevel (0–5)
Population
FoodCapacity
WildlifeCount
ThreatLevel
GarrisonPower
SettlementState

Hero actions:

SecureRegion
RaiseMilitia
FortifySettlement
SuppressRevolt
HuntMonsters
ClaimRuins
EscortCaravan

Region events:

Raid
RefugeeWave
SettlementCollapse
Recolonization
WildlifeMigration

---

# Economy

Humans are physical NPCs.

Settlements produce population.

Monsters capture humans for resources.

Food comes from:
- farms
- livestock
- wildlife

CarryingCapacity limits population growth.

---

# Wildlife System

Animals:

Deer
Boar
Wolf
Bear

Functions:

Food for werewolves
Predator ecosystem
Environmental pressure on frontier regions

Wildlife regenerates through regional simulation.

---

# Systems Included in MVP

Region simulation
Settlement population
Human NPC actors
Wildlife ecosystem
Refugee migration
Caravan trade routes
Settlement collapse and recolonization
Hero region actions

---

# Systems Post‑MVP

Advanced diplomacy
Political factions
Dynamic road building
Complex predator-prey chains
Large-scale city management

---

# Technical Architecture

Engine: Unreal Engine 5

Main Systems:

URTSWorldSubsystem – world simulation

ARTSRegionVolume – region state and simulation

ARTSSettlement – settlement actor

ARTSHumanNPC – population actors

ARTSAnimal – wildlife actors

ARTSRefugeeGroup – migration system

ARTSCaravan – trade routes

Simulation Pulse: every 120–300 seconds

Regions simulate themselves when not loaded.

---

# Performance Strategy

Simulated World:

~1000 humans
~200 wildlife

Active actors near player:

Humans: 60–120
Wildlife: 40–80
Units: up to 200

Far regions run abstract simulation instead of active actors.

---

# Development Roadmap (MVP)

Phase 1 – Region simulation
Phase 2 – Settlements
Phase 3 – Human population
Phase 4 – Wildlife ecosystem
Phase 5 – Refugee system
Phase 6 – Caravan routes
Phase 7 – Recolonization

---

# Project Vision

A hybrid between:

Warcraft‑style RTS combat
Total War‑style territorial strategy
Bannerlord‑style living world

All occurring on one continuous battlefield.
