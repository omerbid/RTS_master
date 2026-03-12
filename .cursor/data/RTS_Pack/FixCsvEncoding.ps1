# Unreal: first CSV column = Row Name only (not mapped to struct).
# Columns 2+ must match struct order: FUnitRow=Faction,UnitId,... FHeroRow=Faction,HeroId,... FHeroXPRow=TableId,Level,... FMoraleThresholdRow=Name,Min,Max,Effect
$dir = $PSScriptRoot
$utf8NoBom = New-Object System.Text.UTF8Encoding $false
$nl = "`n"

# Morale: RowName, Name, Min, Max, Effect
$morale = "RowName,Name,Min,Max,Effect${nl}Broken,Broken,0,30,High retreat chance${nl}Normal,Normal,31,69,No modifier${nl}Inspired,Inspired,70,89,Minor buff${nl}Heroic,Heroic,90,100,Major buff"

# Units: RowName, Faction, UnitId, DisplayName, Role, PowerScale, HP, Damage, Range, AttackSpeed, Armor, MoveSpeed, MoraleBase, Upkeep, Special
$units = "RowName,Faction,UnitId,DisplayName,Role,PowerScale,HP,Damage,Range,AttackSpeed,Armor,MoveSpeed,MoraleBase,Upkeep,Special${nl}Militia,Humans,Militia,Militia,MeleeLight,1,100,10,0,1.0,0,420,60,1,None${nl}Swordsman,Humans,Swordsman,Swordsman,MeleeMedium,1,150,18,0,0.95,2,410,70,2,ShieldWall${nl}Archer,Humans,Archer,Archer,RangedLight,1,90,22,900,1.1,0,430,65,2,Ranged${nl}Knight,Humans,Knight,Knight,CavalryHeavy,1,220,30,0,0.9,4,520,80,4,Charge${nl}Thrall,Vampires,Thrall,Thrall,MeleeLight,2,120,20,0,1.05,1,440,75,2,LifeLeech${nl}Nightblade,Vampires,Nightblade,Nightblade,Assassin,2,180,35,0,1.15,1,480,85,4,StealthBurst${nl}BloodGuard,Vampires,BloodGuard,Blood Guard,Tank,2,260,40,0,0.85,5,400,90,5,FearAura${nl}PackHunter,Werewolves,PackHunter,Pack Hunter,MeleeSkirmisher,3,180,28,0,1.2,1,500,70,4,PackBonus${nl}AlphaWarrior,Werewolves,AlphaWarrior,Alpha Warrior,Bruiser,3,300,50,0,1.0,3,470,85,6,BerserkTrigger${nl}Shaman,Werewolves,Shaman,Shaman,Support,3,160,15,650,1.0,0,450,80,5,SpiritHowl"

# Heroes: RowName, Faction, HeroId, DisplayName, Archetype, PowerScale, BaseHP, BaseDamage, MoveSpeed, MoraleAura, XPTableId, Ability_L1, ...
$heroes = "RowName,Faction,HeroId,DisplayName,Archetype,PowerScale,BaseHP,BaseDamage,MoveSpeed,MoraleAura,XPTableId,Ability_L1,Ability_L6,Ability_L11,Ability_L16,Ability_L21${nl}HighCommander,Humans,HighCommander,High Commander,Commander,1,600,35,480,10,HeroXP_Prototype,Inspire,FormationBonus,DefensiveDoctrine,EliteUpgrade,BattlefieldCommandAura${nl}BloodLord,Vampires,BloodLord,Blood Lord,CasterAssassin,2,700,45,500,0,HeroXP_Prototype,LifeDrain,FearWave,ShadowStep,BloodFrenzy,MassTerrorAura${nl}AlphaPrime,Werewolves,AlphaPrime,Alpha Prime,Brawler,3,850,55,520,5,HeroXP_Prototype,HowlBoost,PackCoordination,PredatorSense,ControlledBerserk,ApexForm"

# HeroXP: RowName, TableId, Level, XPRequired (struct order)
$heroxp = "RowName,TableId,Level,XPRequired${nl}Lv1,HeroXP_Prototype,1,0${nl}Lv2,HeroXP_Prototype,2,120${nl}Lv3,HeroXP_Prototype,3,260${nl}Lv4,HeroXP_Prototype,4,420${nl}Lv5,HeroXP_Prototype,5,500${nl}Lv6,HeroXP_Prototype,6,650${nl}Lv7,HeroXP_Prototype,7,850${nl}Lv8,HeroXP_Prototype,8,1100${nl}Lv9,HeroXP_Prototype,9,1300${nl}Lv10,HeroXP_Prototype,10,1500${nl}Lv15,HeroXP_Prototype,15,3000${nl}Lv20,HeroXP_Prototype,20,5500${nl}Lv25,HeroXP_Prototype,25,9000"

[System.IO.File]::WriteAllText((Join-Path $dir "DT_MoraleThresholds.csv"), $morale, $utf8NoBom)
[System.IO.File]::WriteAllText((Join-Path $dir "DT_Units_MVP.csv"), $units, $utf8NoBom)
[System.IO.File]::WriteAllText((Join-Path $dir "DT_Heroes_MVP.csv"), $heroes, $utf8NoBom)
[System.IO.File]::WriteAllText((Join-Path $dir "DT_HeroXP_Prototype.csv"), $heroxp, $utf8NoBom)
Write-Host "Done. First column = Row Name; rest = struct columns in order."
