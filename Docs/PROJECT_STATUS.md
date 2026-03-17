# מצב פרויקט – RTS Monsters (מפורט)

**עדכון אחרון:** 2026-03-07  
**מנוע:** Unreal Engine 5  
**שפה:** C++ + Blueprint

**הערות:** P6 דחוי. אינדקס מסמכים: `Docs/DOCS_INDEX.md`. חוזה Win/Lose/Respawn: `Docs/GAMERESULT_AND_RESPAWN_CONTRACT.md`. תגובה לניתוח פערים: `Docs/GAP_ANALYSIS_RESPONSE.md`.

---

## 1. סדר פיתוח (GDD) וסטטוס

| שלב | מוקד | סטטוס | הערות |
|-----|------|--------|--------|
| **P0** | בסיס: Data, Units, Heroes, Squads, Region, Morale, Orders, Victory, Input, GameMode | ✅ הושלם | כולל placeholders ל־Region (FoodCapacity, WildlifeCount, ThreatLevel, GarrisonPower, SettlementState, RegionType). |
| **P1** | Command radius + פקודות (בחירה, Move/Attack, טווח פקודה) | ✅ הושלם | LMB/Shift+LMB, Box select, RMB Move/Attack, UnitInfoWidget. |
| **P2** | Region control 0–5, Secure Region, Win/Lose | ✅ הושלם | Secure מקש G, ניצחון ב־5, הפסד כשאין גיבור + אין Region ≥ 3. |
| **P3** | Squad + Morale (0–100, Captain, Detached) | ✅ הושלם | Captain (Rank ≥3, Shift+RMB), הלם מורל במות Captain, detached drain. |
| **P4** | Economy (כסף לבני אדם, המרה למפלצות) | ✅ הושלם | גיוס מקש R, RecruitCostMoney / ConvertPopulationCost / RecruitCostResource, ARTSHumanNPC capture. |
| **P5** | Hero respawn ritual | ✅ הושלם | טיימר 90s, מקש B ל־Perform Ritual, ערוץ 10s, spawn באזור עם השליטה הגבוהה, הלם מורל במות גיבור. |
| **P6** | Day/Night + polish | ⏸ דחוי | תוכנית קיימת: `.cursor/plans/PLANNING_P6_day_night_polish.md`. **דחינו את P6** – לא ממומש כרגע; DayNightSubsystem קיים כקובץ אך לא נדרש ל־MVP נוכחי. |

---

## 2. קוד C++ – קבצים ומערכות

### 2.1 Data & Types

| קובץ | תפקיד |
|------|--------|
| `RTSDataTypes.h` | Enums: EFactionId, ERegionType, EUnitRole, ESpecialTag, ERTSPostCombatBehavior, ERTSOrderType, EHumanNPCState, ERTSGameResult, ERTSRespawnState. Structs: FRTSOrderPayload, FUnitRow, FHeroRow, FHeroXPRow, FMoraleThresholdRow. |
| `RTSDataRegistry.h/.cpp` | GameInstanceSubsystem. טוען DT_Units_MVP, DT_Heroes_MVP, DT_HeroXP_Prototype, DT_MoraleThresholds. GetUnitRow(UnitId), GetHeroRow(HeroId), GetUnitRowForHero, GetFirstRecruitableUnitIdForFaction. |

**FUnitRow (יחידות):** Faction, UnitId, DisplayName, Role, PowerScale, HP, Damage, Range, AttackSpeed, Armor, MoveSpeed, MoraleBase, Upkeep, RecruitCostMoney, ConvertPopulationCost, RecruitCostResource, Special, PostCombatBehavior, Rank, Level; **Combat target (data-driven):** PreferredTargetRoles, TargetPriorityBias, FocusFireFactor (Docs/COMBAT_CONTRACT).

**כלכלה (מתאים ל־GDD):** בני אדם – גיוס בכסף (RecruitCostMoney) + תחזוקה (Upkeep). מפלצות – גיוס במשאב סיעה (RecruitCostResource) + NPC (ConvertPopulationCost), תחזוקה במשאב.

### 2.2 Characters

| קובץ | תפקיד |
|------|--------|
| `RTSUnitCharacter.h/.cpp` | ARTSUnitCharacter – UnitId, FactionId, CachedUnitData, MoraleComponent, OrderComponent, Squad, Rank, Captain, Detached. InitializeFromUnitRow, ApplyDamage, Destroyed (NotifyCaptainDeath / morale). |
| `RTSHeroCharacter.h/.cpp` | ARTSHeroCharacter (יורש Unit). HeroId, CommandAuthorityComponent (2500), SecureRegionComponent. TryStartSecureRegion, IssueOrderToUnitsInRange, InitializeFromRegistry, Destroyed → NotifyHeroDeath. |
| `RTSHumanNPC.h/.cpp` | ARTSHumanNPC – NPC אנושי (Phase 3); states: Idle, Working, Fleeing, Captured. לשימוש ב־P4 capture. |

### 2.3 Components

| קובץ | תפקיד |
|------|--------|
| `RTSMoraleComponent.h/.cpp` | URTSMoraleComponent – ערך 0–100, thresholds, ApplyMoraleDelta. |
| `RTSCommandAuthorityComponent.h/.cpp` | URTSCommandAuthorityComponent – CommandRadius, CanIssueOrderToUnit. |
| `RTSOrderComponent.h/.cpp` | URTSOrderComponent – Move/Attack, SetCurrentOrder, FRTSOrderPayload. |
| `RTSSecureRegionComponent.h/.cpp` | URTSSecureRegionComponent – ערוץ 15s ל־Secure Region, ביטול ביציאה/אויב. OnSecureRegionStarted/Cancelled/Completed. |
| `RTSUnitInfoWidget.h/.cpp` | URTSUnitInfoWidget – פאנל מידע ליחידה (UnitId, Faction, Morale, Rank). |

### 2.4 Squad & Morale

| קובץ | תפקיד |
|------|--------|
| `RTSSquadState.h/.cpp` | URTSSquadState – Members, CaptainUnit, AverageMorale, RecalcMorale, ApplyMoraleDeltaToAll, SetCaptain. |
| `RTSSquadManagerSubsystem.h/.cpp` | URTSSquadManagerSubsystem – GetOrCreateSquadForFaction, AddUnitToFactionSquad. סקד אחד לכל faction. |

### 2.5 Region & Victory

| קובץ | תפקיד |
|------|--------|
| `RTSRegionVolume.h/.cpp` | ARTSRegionVolume – ControlLevel 0–5 לכל faction, Population, Stability, DominantFaction. IsPointInRegion, IsContested, SetControlLevelForFaction, RecalcDominantFaction. Placeholders: FoodCapacity, WildlifeCount, ThreatLevel, GarrisonPower, SettlementState, RegionType. Overlap לגיבורים, טיימר עלייה 0→4, Secure→5. GetControlLevelForFaction, GetHeroesInRegion, HasHeroOfFaction. GetRegionAtLocation (static). |
| `RTSVictorySubsystem.h/.cpp` | URTSVictorySubsystem – NotifyControlReachedFive (ניצחון), NotifyHeroDeath (הפסד או P5 respawn). **P5:** Respawn state per faction, טיימר 90s, RitualAvailable, ערוץ 10s, SpawnHeroInBestRegion, ApplyMoraleShockToSquadsNear. CanPerformRitual, StartRitualChannel, CancelRitualChannel, GetFactionWithRitualAvailable. OnRespawnRitualAvailable. |

### 2.6 Economy

| קובץ | תפקיד |
|------|--------|
| `RTSEconomySubsystem.h/.cpp` | URTSEconomySubsystem – Money per faction (בני אדם), CanAffordRecruit, TryRecruitUnit. גיוס כסף (Humans) או המרת אוכלוסייה/לכידת ARTSHumanNPC (Vampires/Werewolves). AddMoney, SpendMoney. |

### 2.7 Combat & Orders (מנהלים)

| קובץ | תפקיד |
|------|--------|
| `RTSCombatManagerSubsystem.h/.cpp` | ניהול קרב (התקפה, נזק). |
| `RTSCombatResolverLibrary.h/.cpp` | פונקציות עזר לפתרון קרב. |
| `RTSCombatTypes.h` | טיפוסים לקרב. |
| `RTSOrderManagerSubsystem.h/.cpp` | ניהול פקודות. |

### 2.8 Input & Game

| קובץ | תפקיד |
|------|--------|
| `RTSPlayerController.h/.cpp` | ARTSPlayerController – בחירה (LMB, Shift+LMB, Box), RMB Move/Attack. מקש G – Secure Region, מקש R – Recruit, מקש B – Perform Ritual (P5). UnitInfoWidget, Win/Lose feedback (OnGameWon/OnGameLost). |
| `RTSGameModeBase.h/.cpp` | ARTSGameModeBase – PlayerController, CameraPawn. |
| `RTSCameraPawn.h/.cpp` | ARTSCameraPawn – תנועת מצלמה, zoom. |
| `RTSHUD.h/.cpp` | HUD. |
| `RTSSelectionBoxWidget.h/.cpp` | וידג'ט לבחירה במלבן. |

### 2.9 Day/Night (P6)

| קובץ | תפקיד |
|------|--------|
| `RTSDayNightSubsystem.h/.cpp` | URTSDayNightSubsystem – Day/Night (אם הוטמע; תוכנית P6). |

### 2.10 Save/Load (״פרויקט קיים״ – שלד)

| קובץ | תפקיד |
|------|--------|
| `RTSSaveSubsystem.h/.cpp` | URTSSaveSubsystem – שער יחיד: AcquireSaveLock/ReleaseSaveLock, RequestManualSave, RequestLoad, ListProjects, CreateProject, DeleteProject. שלד (ללא I/O עד Vertical Slice). |
| `RTSProjectIndexSaveGame.h/.cpp` | URTSProjectIndexSaveGame – רשימת פרויקטים (FProjectMeta). |
| `RTSProjectSaveGame.h/.cpp` | URTSProjectSaveGame – Meta (SaveSchemaVersion, ProjectId, MapName, timestamps). להרחבה עם Regions/Units/Economy/Victory. |

### 2.11 בדיקות

| קובץ | תפקיד |
|------|--------|
| `RTSP0AutomationTests.cpp` | אוטומציה ל־P0. |

---

## 3. DataTables ונתונים

| טבלה | נתיב (יחסי ל־Content) | מבנה | הערות |
|------|------------------------|------|--------|
| **DT_Units_MVP** | RTS/Data/Data_Tables/DT_Units_MVP | FUnitRow | יחידות לפי UnitId. קובץ ייבוא מוכן: DT_Units_MVP_BalanceImport.csv (12 יחידות מטבלת האיזון). |
| **DT_Heroes_MVP** | RTS/Data/Data_Tables/DT_Heroes_MVP | FHeroRow | גיבורים לפי HeroId. |
| **DT_HeroXP_Prototype** | RTS/Data/Data_Tables/DT_HeroXP_Prototype | FHeroXPRow | אופציונלי. |
| **DT_MoraleThresholds** | RTS/Data/Data_Tables/DT_MoraleThresholds | FMoraleThresholdRow | אופציונלי. |

**נתיבי טעינה בקוד:** `/Game/RTS/Data/Data_Tables/DT_Units_MVP.DT_Units_MVP` (ודומה ליתר).

**ייבוא איזון:** ראה `Docs/UNIT_BALANCE_IMPORT.md` – מיפוי Role, כלכלה, CSV ל־12 יחידות.

---

## 4. קלט (מקשים)

| מקש | פעולה |
|-----|--------|
| LMB | בחירה (לחיצה / שחרור אחרי גרירה = box select) |
| Shift+LMB | הוספה לבחירה |
| RMB | פקודת הקשר (Move / Attack) |
| G | Secure Region (גיבור נבחר, באזור עם control 4, לא contested) |
| R | Recruit (גיבור נבחר, באזור, מספיק משאבים) |
| B | Perform Ritual (P5 – אחרי 90s ממות גיבור, כש־RitualAvailable) |
| Scroll | Zoom מצלמה |
| Middle Mouse + גרירה | Pitch מצלמה |

---

## 5. תוכניות (Plans)

| קובץ | תוכן |
|------|--------|
| `.cursor/plans/PLANNING_P0_revised_with_new_docs.md` | P0 מפורט, alignment למסמכים, §2 רשימת מערכות קיימות (לא למחוק). |
| `.cursor/plans/PLANNING_P0_and_Phases_1_to_7.md` | P0 + Phases 1–7 (Region simulation → Recolonization). |
| `.cursor/plans/PLANNING_P1_command_and_orders.md` | P1 פקודות וקלט. |
| `.cursor/plans/PLANNING_P2_region_control.md` | P2 Region, Secure, Win/Lose. |
| `.cursor/plans/PLANNING_P2_epics_1_2_4.md` | אפיקים P2. |
| `.cursor/plans/PLANNING_P2_phases_1_to_7.md` | פירוט Phase לפי P2. |
| `.cursor/plans/PLANNING_P3_squad_morale.md` | P3 Squad, Morale, Captain. |
| `.cursor/plans/PLANNING_P4_economy.md` | P4 Economy. |
| `.cursor/plans/PLANNING_P4_phases.md` | פייזות P4. |
| `.cursor/plans/PLANNING_P5_hero_respawn_ritual.md` | P5 Hero respawn – טיימר, טקס, spawn (Implemented). |
| `.cursor/plans/PLANNING_P6_day_night_polish.md` | P6 Day/Night + polish. |
| `.cursor/plans/PLANNING_E5_UI_IMPROVEMENTS.md` | שיפורי UI (אופציונלי). |

---

## 6. מסמכי עיצוב ותיעוד (Docs)

| קובץ | תוכן |
|------|--------|
| `Docs/README_FOR_CURSOR.md` | הנחיות ל־Cursor, הרחבה בלי להחליף קוד קיים. |
| `Docs/P0_ALIGNMENT.md` | מיפוי מונחי מסמכים ↔ קוד (מפנה ל־PLANNING_P0_revised §3). |
| `Docs/MASTER_GDD_MVP.md` | GDD ראשי ל־MVP. |
| `Docs/CORE_GAME_MECHANICS.md` | מכניקות ליבה. |
| `Docs/campaign_layer_spec.md` | מפרט שכבת קמפיין. |
| `Docs/economy_system.md` | מערכת כלכלה. |
| `Docs/EXTERNAL_SOURCES_AND_P0_P3_PLAN.md` | מקורות חיצוניים (RTS_FULL_MASTER_PACK, zip איזון), החלטות P0–P3, תכנון מעודכן. |
| `Docs/UNIT_BALANCE_IMPORT.md` | ייבוא טבלת איזון, מיפוי Role, מודל כלכלה, הנחיות ייבוא CSV. |
| `Docs/DOCS_INDEX.md` | אינדקס כל המסמכים (Docs, AI_Knowledge, Plans, GDD) – נתיב + תיאור ("מה זה מה"). |
| `Docs/GAMERESULT_AND_RESPAWN_CONTRACT.md` | חוזה Win/Lose/Respawn – סדר אירועים, מכונת מצבים. |
| `Docs/GAP_ANALYSIS_RESPONSE.md` | תגובה לניתוח פערים – מה בוצע, מה נותר. |
| `Docs/AI_Knowledge/` | ARCHITECTURE_GUIDE, CODING_RULES, IMPLEMENTATION_PHASES, PROJECT_BRAIN, REGION_SYSTEM, WORLD_SIMULATION, וכו'. |

---

## 7. מקורות חיצוניים (הפניה)

| מקור | נתיב | שימוש |
|------|------|--------|
| **RTS_FULL_MASTER_PACK** | `c:\Users\shodi\Desktop\תוכן נוסף ועדכון\RTS_FULL_MASTER_PACK` | 46 קבצי MD – ארכיטקטורה, מערכות, Data/Asset. תיעוד: Docs/EXTERNAL_SOURCES_AND_P0_P3_PLAN.md. |
| **rts_units_dataassets_and_balance.zip** | חילוץ: `…\תוכן נוסף ועדכון\rts_units_dataassets_and_balance\` | unit_balance_table.csv/md, DataAsset_Template. ייבוא: DT_Units_MVP_BalanceImport.csv + UNIT_BALANCE_IMPORT.md. |

---

## 8. כללי עבודה (ממסמכי הפרויקט)

- **לא למחוק/להחליף** מערכות קיימות (§2 ב־PLANNING_P0_revised). רק להוסיף או להרחיב.
- **Data-driven:** יחידות/גיבורים מ־DataTables (UnitId, HeroId); לוגיקה בקוד, תוכן ונתונים ב־Data/Assets.
- **Subsystems:** DataRegistry, VictorySubsystem, SquadManager, EconomySubsystem – GameInstanceSubsystem. WorldSubsystem (למשל סימולציה) – Phase 1+.

---

## 9. סיכום מצב נוכחי

- **P0–P5:** ממומשים בקוד; Win/Lose, Secure, Recruit, Captain, Morale, Respawn ritual פעילים.
- **תוכן:** DataTables ו־Blueprint Unit/Hero – להגדיר ב־Content (או לייבא מ־CSV). קובץ ייבוא יחידות: `Content/RTS/Data/Data_Tables/DT_Units_MVP_BalanceImport.csv`.
- **הבא בתור:** P6 דחוי. אופציונלי – Phase 1 (Region simulation pulse) מתוך PLANNING_P0_and_Phases_1_to_7; או תשתית Save/Load ו־Spec "פרויקט קיים" לפי ניתוח הפערים.
