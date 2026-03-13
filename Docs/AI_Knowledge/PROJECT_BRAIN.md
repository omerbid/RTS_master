
# RTS Project Brain

This document maps **all major systems of the RTS project** so AI tools
(Cursor, LLM assistants, etc.) can quickly understand the entire game architecture.

Engine: Unreal Engine 5
Genre: Hero‑centric RTS with continuous campaign layer
Core concept: A living world where regions simulate population, ecology,
and conflict on the same map used for tactical battles.

---

# Core Game Loop

Population grows
↓
Regions produce food & wealth
↓
Monsters hunt / raid
↓
Humans flee or fortify
↓
Cities grow or collapse
↓
Settlements rebuild or wilderness expands
↓
Loop continues

---

# World Layers

1. World Simulation Layer
Handles global simulation pulses and region updates.

Main class:
URTSWorldSubsystem

Responsibilities:
- population growth
- wildlife growth
- threat decay
- migration
- settlement collapse
- recolonization

Simulation pulse interval:
120–300 seconds

---

2. Region Layer

Each strategic area is represented by:

ARTSRegionVolume

Core variables:
OwnerFaction
ControlLevel
Population
FoodCapacity
WildlifeCount
ThreatLevel
GarrisonPower
SettlementState

Regions simulate even when actors are unloaded.

---

3. Settlement Layer

Actor:
ARTSSettlement / BP_Settlement

Types:
Capital
City
Town
Village
Hamlet

Responsibilities:
- population growth
- food production
- spawning villagers
- managing collapse / recovery

---

4. Population Layer

Actor:
ARTSHumanNPC

States:
Idle
Working
Fleeing
Captured

Humans are physical resources used by monsters and armies.

---

5. Wildlife Layer

Actor:
ARTSAnimal

Types:
Deer
Boar
Wolf
Bear

Functions:
- food for werewolves
- ecosystem pressure
- frontier danger

Wildlife regenerates through regional simulation.

---

6. Migration Layer

Actor:
ARTSRefugeeGroup

Refugees spawn when settlements are attacked.
They travel along roads to safer regions.

Result:
Population redistribution.

---

7. Trade Layer

Actor:
ARTSCaravan

Moves along trade routes between settlements.

Contains:
Traders
Guards
Wealth

Can be ambushed by monsters.

---

# Factions

Humans
- defend settlements
- rely on population economy

Vampires
- feed on dense population
- prefer cities and caravans

Werewolves
- hunt animals and humans
- dominate wilderness regions

---

# Economy

Primary resources:

Population
Food
Wildlife
Control

Humans generate economy.
Monsters consume population or wildlife.

---

# Campaign Layer

Regions have ownership and stability.

Variables:

OwnerFaction
ControlLevel
Population
ThreatLevel
GarrisonPower

Hero actions:

SecureRegion
RaiseMilitia
FortifySettlement
SuppressRevolt
HuntMonsters
ClaimRuins
EscortCaravan

---

# Technical Architecture

Layers:

Game Layer
World Simulation Layer
Region Layer
Actor Layer
Data Layer

Data driven systems using DataTables.

Important classes:

URTSWorldSubsystem
ARTSRegionVolume
ARTSSettlement
ARTSHumanNPC
ARTSAnimal
ARTSRefugeeGroup
ARTSCaravan

---

# Performance Model

Simulated world:

~1000 humans
~200 animals

Active actors near player:

Humans: 60–120
Animals: 40–80
Units: up to 200

Regions outside player area run abstract simulation.

---

# Development Philosophy

Extend existing systems instead of rewriting them.
Keep simulation data‑driven.
Separate world simulation from active actors.
Maintain compatibility with World Partition.

This document acts as the **central mental model of the project**.
