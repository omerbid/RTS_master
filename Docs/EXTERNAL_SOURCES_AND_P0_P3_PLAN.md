# מקורות חיצוניים + תכנון P0–P3 מעודכן

**נוצר:** 2026-03-07  
**מטרה:** תיעוד חבילות התוכן החיצוניות, השפעה על P0–P3, קונפליקטים לאישור, ותכנון P0–P3 מעודכן.

---

## 1. מקורות חיצוניים (שמורים אצלנו – הפניות)

### 1.1 RTS_FULL_MASTER_PACK

**נתיב:** `c:\Users\shodi\Desktop\תוכן נוסף ועדכון\RTS_FULL_MASTER_PACK`

**תוכן (46 קבצי MD):**

| קבוצה | קבצים |
|--------|--------|
| **Core** | MASTER_00_README, MASTER_01_FULL_ARCHITECTURE_OVERVIEW, MASTER_02_FULL_ARCHITECTURE_DIAGRAM, MASTER_03_IMPLEMENTATION_SEQUENCE |
| **Systems** | MASTER_10 Economy, MASTER_11 Population, MASTER_12 Building Progression, MASTER_13 Faction, MASTER_14 **Morale**, MASTER_15 **Hero Command**, MASTER_16 Building/Tech, MASTER_17 **Region Simulation**, MASTER_18 Garrison, MASTER_19 Building List MVP |
| **AI** | MASTER_20 Army AI, MASTER_21 Squad AI, MASTER_22 AI Risk Notes |
| **Content** | MASTER_30–33 Art/Mesh/Rigging/Naming |
| **Class/Data** | MASTER_40 CPP Class List, MASTER_41 BP Class List, MASTER_42 **Data Table and Asset Plan** |
| **Cursor** | MASTER_90 Cursor Master Prompt |
| **ARCH (ארכיטקטורה)** | ARCH_00–02 README/Overview/Diagram, ARCH_03 **RTS Unit**, ARCH_04 Unit Manager, ARCH_05 **Squad**, ARCH_06 Combat Manager, ARCH_07 Selection and Orders, ARCH_08 Collision, ARCH_09 Hero, ARCH_10 **Region and Map**, ARCH_11 **Implementation Roadmap**, ARCH_12 Cursor Implementation, ARCH_13 **Data Asset Schema**, ARCH_14 **Example Unit Tables**, ARCH_15–18 Class lists, Performance |

**הערה:** Phase numbering ב־Master Pack שונה משלנו: אצלם Phase 1 = Combat Slice, 2 = Infrastructure, 3 = Buildings+Economy, 4 = Heroes, 5 = Regions. אצלנו P0 = Foundation, P1 = Selection/Orders, P2 = Region/Secure/Win-Lose, P3 = Squad/Morale/Captain.

---

### 1.2 rts_units_dataassets_and_balance.zip

**נתיב ZIP:** `c:\Users\shodi\Desktop\תוכן נוסף ועדכון\rts_units_dataassets_and_balance.zip`  
**נתיב אחרי חילוץ:** `c:\Users\shodi\Desktop\תוכן נוסף ועדכון\rts_units_dataassets_and_balance\`

| קובץ | תיאור |
|------|--------|
| **unit_balance_table.csv** | 12 יחידות: Faction, Unit, Role, HP, Damage, Range, AttackCooldown, Speed, PopulationCost |
| **unit_balance_table.md** | אותו טבלה בפורמט Markdown |
| **DataAsset_Template.txt** | תבנית UUnitDataAsset: UnitID, DisplayName, Faction, Mesh, AnimBP, MaxHealth, AttackDamage, AttackRange, AttackCooldown, Armor, MoveSpeed, PopulationCost, Capsule, Abilities, Tags |

**יחידות בטבלת האיזון:**

- **Human:** Swordsman (Frontline), Archer (Ranged), Spearman (AntiLarge), MonsterHunter (EliteRanged)
- **Vampire:** ThrallGuard (Frontline), Duelist (Flanker), BloodArcher (Ranged), Knight (Elite)
- **Werewolf:** ClawWarrior (Assault), Hunter (Skirmisher), AlphaGuard (EliteFrontline), Shaman (Support)

---

## 2. השפעה על P0–P3 וקונפליקטים

### 2.1 P0 (Data, Units, Heroes, Squads, Regions, Morale, Orders, Victory)

| מקור | השפעה | קונפליקט? | החלטה מוצעת |
|------|--------|-----------|-------------|
| **Master Pack – Data/Asset** | MASTER_42 + ARCH_13/14 ממליצים DataAssets ליחידות; אנחנו משתמשים ב־DataTables (FUnitRow, DT_Units_MVP). | **אין מחליף:** המסמכים תומכים גם ב־DataTables ל־balance. DataAssets הם אופציונליים (אפשר להוסיף מאוחר). | להשאיר DataTables כבסיס; טבלת האיזון מה־zip יכולה להזין/להרחיב DT_Units_MVP. |
| **Zip – unit_balance_table** | ערכי HP, Damage, Range, AttackCooldown, Speed, PopulationCost ל־12 יחידות. | **מיפוי תפקידים:** ב־zip: Frontline, Ranged, AntiLarge, EliteRanged, Flanker, Elite, Assault, Skirmisher, EliteFrontline, Support. אצלנו EUnitRole: MeleeLight, MeleeMedium, RangedLight, CavalryHeavy, Assassin, Tank, MeleeSkirmisher, Bruiser, Support. | **לאשר איתך:** להרחיב EUnitRole בערכים חדשים (Frontline, AntiLarge, EliteRanged, Flanker, Elite, Assault, EliteFrontline) או למפות ל־Roles קיימים (למשל Frontline→MeleeMedium, Ranged→RangedLight). |
| **Zip – PopulationCost** | שדה ב־CSV. אצלנו: RecruitCostMoney (כסף), ConvertPopulationCost (NPC), RecruitCostResource (דם/בשר), Upkeep. | הוחלט: בני אדם גיוס בכסף+NPC, מפלצות דם/בשר+NPC; Upkeep במשאב סיעה. PopulationCost מה־zip → ConvertPopulationCost. נוסף RecruitCostResource. | בוצע. |
| **Master Pack – Region** | MASTER_17: owner, control, development, settlements, garrison, unrest. אנחנו: ControlLevel 0–5, FoodCapacity, WildlifeCount, ThreatLevel, GarrisonPower, SettlementState, RegionType. | development level לא קיים אצלנו; unrest לא. | P0 כבר מכיל placeholders; development/unrest יכולים להיכנס כ־placeholder או ב־Phase 1. **אין קונפליקט חוסם.** |

### 2.2 P1 (Selection, Orders, Input)

| מקור | השפעה | קונפליקט? |
|------|--------|-----------|
| ARCH_07 Selection and Orders | Move/Attack, SelectionDecal – תואם למה שבנו. | אין. |
| Master Pack Phase 1 | Selection, Squad, Combat – כבר קיים. | אין. |

### 2.3 P2 (Region Control, Secure, Win/Lose)

| מקור | השפעה | קונפליקט? |
|------|--------|-----------|
| MASTER_17 Region Simulation | control, garrison – תואם ל־ControlLevel ו־GarrisonPower placeholder. | אין. |
| Victory | Win on 5, lose – כבר ב־URTSVictorySubsystem. | אין. |

### 2.4 P3 (Squad, Morale, Captain, Detached)

| מקור | השפעה | קונפליקט? | החלטה מוצעת |
|------|--------|-----------|-------------|
| **ARCH_05 Squad** | FormationType (Line, Box, Wedge, Loose, Circle), SquadState (Idle, Moving, Reforming, Engaging, Holding, Retreating, Defending). אצלנו: URTSSquadState עם Members, Captain, RecalcMorale; אין FormationType או squad-level orders. | **הרחבה:** ה־Pack מציע formations ו־squad states שלא ממומשים. | להשאיר P3 כמו שהוא; Formations = הרחבה עתידית (לא להחליף קוד קיים). |
| **MASTER_14 Morale** | 0–100, thresholds, faction flavor (Human discipline, Vampire fear, Werewolf frenzy). אצלנו: URTSMoraleComponent, 0–100, thresholds, Captain death, detached drain. | **התאמה:** עקרונות דומים. Faction flavor = תוכן לעתיד. | אין קונפליקט; אפשר להוסיף faction flavor מאוחר. |
| **MASTER_15 Hero Command** | XP, levels, auras, abilities, unlocks. אצלנו: Hero עם CommandAuthority, SecureRegion – בלי XP/levels. | **הרחבה:** XP/levels הם post-P3. | אין קונפליקט ל־P0–P3. |

---

## 3. החלטות (סופי)

1. **תפקידי יחידות (Role):** **מיפוי** ל־EUnitRole קיים (לא הרחבת enum). טבלת מיפוי ו־12 שורות: `Docs/UNIT_BALANCE_IMPORT.md`.

2. **כלכלה:**
   - **בני אדם:** גיוס = כסף (RecruitCostMoney) + אופציונלי NPC (ConvertPopulationCost). תחזוקה = Upkeep בכסף.
   - **מפלצות:** גיוס = דם/בשר (RecruitCostResource) + NPC (ConvertPopulationCost). תחזוקה = Upkeep בדם/בשר.
   - ב־FUnitRow: נוסף שדה `RecruitCostResource`; הערות עודכנו על Upkeep, RecruitCostMoney, ConvertPopulationCost.

3. **ייבוא:** **כן.** קובץ ייבוא: `Content/RTS/Data/Data_Tables/DT_Units_MVP_BalanceImport.csv`. הנחיות: `Docs/UNIT_BALANCE_IMPORT.md`.

---

## 4. תכנון P0–P3 מעודכן (אחרי עדכון המקורות)

### P0 – Core Setup (כפי שמתועד ב־PLANNING_P0_revised_with_new_docs.md)

| סטטוס | תוכן |
|--------|------|
| **בוצע** | DataRegistry, DataTables (DT_Units_MVP, DT_Heroes_MVP, DT_HeroXP_Prototype, DT_MoraleThresholds), RTSDataTypes (כולל ERegionType), Units, Heroes, Squads, Morale, Command, Regions (Control 0–5 + placeholders: FoodCapacity, WildlifeCount, ThreatLevel, GarrisonPower, SettlementState, RegionType), Secure, Victory, Orders, Input, GameMode. |
| **הוחלט** | ייבוא 12 שורות מטבלת האיזון (מיפוי Role, RecruitCostResource נוסף). קובץ: `Data/Data_Tables/DT_Units_MVP_BalanceImport.csv`, הנחיות: `Docs/UNIT_BALANCE_IMPORT.md`. |

### P1 – Selection, Orders, Input

| סטטוס | תוכן |
|--------|------|
| **בוצע** | Selection, Move/Attack orders, ARTSPlayerController, UnitInfoWidget. |
| **מהמקורות** | אין שינוי נדרש ל־P1 מהחבילות. |

### P2 – Region Control, Secure, Win/Lose

| סטטוס | תוכן |
|--------|------|
| **בוצע** | Region control 0–5, SecureRegion (channel 15s), URTSVictorySubsystem (win 5 / lose no hero + no region ≥3). |
| **מהמקורות** | MASTER_17 תואם; development/unrest – placeholder או Phase 1. |

### P3 – Squad, Morale, Captain, Detached

| סטטוס | תוכן |
|--------|------|
| **בוצע** | URTSSquadState, Captain, morale recalc, detached drain, Rank, promotion. |
| **מהמקורות** | Formations (Line/Box/Wedge וכו') ו־SquadState מורחב – **הרחבה עתידית**, לא דרישה ל־P3 נוכחי. |

---

## 5. חסר אחרי העדכון

### לא חסר למימוש P0–P3 הנוכחי

- כל מה שנדרש ל־P0–P3 **כבר בפרויקט** או מתועד ב־Docs וב־PLANNING_P0_revised / PLANNING_P0_and_Phases_1_to_7.
- החבילות **מחזקות** את אותו כיוון (Data-driven, Morale, Squad, Region, Hero command) ולא דורשות החלפת מערכות.

### תוכן אופציונלי (לא חובה ל־P0–P3)

| מקור | תוכן | הערה |
|------|------|------|
| Zip | 12 שורות איזון ל־DT_Units_MVP | מומלץ אחרי אישור מיפוי Role ו־PopulationCost. |
| Zip | DataAsset template | רלוונטי אם בעתיד נוסיף UUnitDataAsset בנוסף ל־DataTables. |
| Master Pack | Formation types, SquadState מורחב | Phase אחרי P3. |
| Master Pack | Hero XP, levels, auras, abilities | Phase אחרי P3. |
| Master Pack | Region development, unrest | Phase 1 או placeholder. |

### בוצע (החלטות משתמש)

- **Role:** מיפוי ל־EUnitRole (לא הרחבת enum).
- **כלכלה:** RecruitCostMoney (בני אדם), RecruitCostResource (דם/בשר למפלצות), ConvertPopulationCost (NPC), Upkeep (משאב סיעה).
- **ייבוא:** CSV מוכן ב־`Content/RTS/Data/Data_Tables/DT_Units_MVP_BalanceImport.csv`; הנחיות ב־`Docs/UNIT_BALANCE_IMPORT.md`. ב־Unreal: לפתוח DT_Units_MVP → Reimport / Import מהקובץ.
