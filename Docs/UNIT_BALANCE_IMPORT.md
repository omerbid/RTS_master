# ייבוא טבלת איזון יחידות (unit_balance_table)

**מקור:** `rts_units_dataassets_and_balance.zip`  
**החלטות:** מיפוי Role ל־EUnitRole קיים; כלכלה: כסף (בני אדם) / דם-בשר + NPC (מפלצות); Upkeep במשאב רלוונטי.

---

## 1. מודל כלכלה (מתאים ל־FUnitRow)

| סוג | גיוס | תחזוקה (Upkeep) |
|-----|------|-------------------|
| **בני אדם** | כסף (RecruitCostMoney) + אופציונלי NPC (ConvertPopulationCost, למשל מיליציה) | כסף (Upkeep) |
| **מפלצות (Vampire/Werewolf)** | דם/בשר (RecruitCostResource) + NPC (ConvertPopulationCost) | דם/בשר (Upkeep) |

- **RecruitCostMoney** – עלות בכסף לגיוס (בני אדם). 0 = לא ניתן לגייס בכסף.
- **ConvertPopulationCost** – עלות ב־NPC/אוכלוסייה לגיוס (מיליציה אנושית או המרה למפלצת). 0 = לא משתמש באוכלוסייה.
- **RecruitCostResource** – עלות במשאב הסיעה: דם (Vampire), בשר (Werewolf). 0 = בני אדם או לא רלוונטי.
- **Upkeep** – תחזוקה לכל סבב במשאב של הסיעה: כסף / דם / בשר.

---

## 2. מיפוי Role (zip → EUnitRole)

| Role בטבלת האיזון | EUnitRole בפרויקט |
|--------------------|-------------------|
| Frontline          | MeleeMedium       |
| Ranged             | RangedLight       |
| AntiLarge          | Tank              |
| EliteRanged        | RangedLight       |
| Flanker            | Assassin          |
| Elite              | Tank              |
| Assault            | Bruiser           |
| Skirmisher         | MeleeSkirmisher   |
| EliteFrontline     | Tank              |
| Support            | Support           |

---

## 3. 12 שורות הייבוא (מקור → מיפוי)

| Faction   | Unit         | Role (zip)   | → Role      | HP  | Damage | Range | AttackCooldown | → AttackSpeed | Speed | PopCost → ConvertPopulationCost |
|-----------|--------------|--------------|-------------|-----|--------|-------|----------------|---------------|-------|----------------------------------|
| Human     | Swordsman    | Frontline    | MeleeMedium | 120 | 16     | 150   | 1.4            | 0.714         | 420   | 2                                |
| Human     | Archer       | Ranged       | RangedLight | 80  | 14     | 900   | 1.6            | 0.625         | 410   | 2                                |
| Human     | Spearman     | AntiLarge    | Tank        | 110 | 15     | 170   | 1.5            | 0.667         | 415   | 2                                |
| Human     | MonsterHunter| EliteRanged  | RangedLight | 100 | 24     | 700   | 1.7            | 0.588         | 430   | 3                                |
| Vampire   | ThrallGuard  | Frontline    | MeleeMedium | 130 | 18     | 150   | 1.4            | 0.714         | 425   | 2                                |
| Vampire   | Duelist      | Flanker      | Assassin    | 90  | 22     | 160   | 1.2            | 0.833         | 450   | 2                                |
| Vampire   | BloodArcher  | Ranged       | RangedLight | 85  | 17     | 850   | 1.5            | 0.667         | 420   | 2                                |
| Vampire   | Knight       | Elite        | Tank        | 160 | 26     | 170   | 1.6            | 0.625         | 430   | 3                                |
| Werewolf  | ClawWarrior  | Assault      | Bruiser     | 150 | 22     | 150   | 1.3            | 0.769         | 460   | 3                                |
| Werewolf  | Hunter       | Skirmisher   | MeleeSkirmisher | 105 | 19 | 650   | 1.4            | 0.714         | 455   | 2                                |
| Werewolf  | AlphaGuard   | EliteFrontline | Tank     | 180 | 28     | 160   | 1.6            | 0.625         | 445   | 4                                |
| Werewolf  | Shaman       | Support      | Support     | 95  | 10     | 600   | 1.8            | 0.556         | 420   | 2                                |

AttackSpeed ב־FUnitRow = 1 / AttackCooldown מהטבלה.

---

## 4. איך לייבא ל־DT_Units_MVP

1. **ב־Unreal Editor:** פתח את `Content/RTS/Data/Data_Tables/DT_Units_MVP`.
2. **ייבוא CSV:** Reimport או גרור את `Data/DT_Units_MVP_BalanceImport.csv` (או העתק את התוכן ל־CSV ו־Import).
3. **לוודא:** שמות עמודות תואמים ל־FUnitRow (כולל RecruitCostMoney, ConvertPopulationCost, RecruitCostResource, PostCombatBehavior, Rank, Level).
4. **ערכי placeholder:** RecruitCostResource ו־RecruitCostMoney (למפלצות/בני אדם בהתאמה) – למלא מאיזון עתידי; בייבוא הזה: RecruitCostMoney=50 ל־Humans, 0 ל־מפלצות; RecruitCostResource=0 לכולם (לעדכן מאוחר).

קובץ CSV מוכן: `Content/RTS/Data/DT_Units_MVP_BalanceImport.csv` (או בתיקיית Data/Data_Tables לפי המבנה בפרויקט).
