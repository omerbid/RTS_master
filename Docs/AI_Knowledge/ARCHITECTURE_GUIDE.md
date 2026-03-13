
# Architecture Guide

Main architecture layers:

Game Layer
World Simulation Layer
Region Layer
Actor Layer
Data Layer

Key classes:

URTSWorldSubsystem
ARTSRegionVolume
ARTSSettlement
ARTSHumanNPC
ARTSAnimal
ARTSRefugeeGroup
ARTSCaravan

Rules:

Heavy simulation logic → C++
Actors and visuals → Blueprint
Balance data → DataTables
