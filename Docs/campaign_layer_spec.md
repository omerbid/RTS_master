
# Campaign Layer Specification (MVP)

Purpose:
Transform RTS map into a continuous campaign world.

Each Region stores:

OwnerFaction
ControlLevel (0-5)
Population
FoodCapacity
ThreatLevel
WildlifeCount
GarrisonPower
SettlementState

Hero Actions:
SecureRegion
RaiseMilitia
FortifySettlement
SuppressRevolt
HuntMonsters
ClaimRuins
EscortCaravan

Region Events:
Raid
RefugeeWave
SettlementCollapse
Recolonization
WildlifeMigration

Campaign Loop:
Population grows → Monsters raid → Refugees move → Cities grow → New settlements emerge.
