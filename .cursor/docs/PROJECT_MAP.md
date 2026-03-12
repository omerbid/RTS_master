# RTS_Monsters – מפת פרויקט מלאה

**תאריך עדכון:** פברואר 2026  
**מקורות:** ניתוח קוד, Export Pack, Docs Pack, GDD

---

## 1. סיכום מנהלים

| נושא | סטטוס |
|------|--------|
| **פרויקט** | TopDown Template → RTS_Monsters, UE 5.4 |
| **מערכות P1** | Selection, Orders, Command Authority, Camera – מוכנות |
| **קריסת Play** | Cast Default__Object – תוקן חלקית (הוסר WorldContext) |
| **Export Pack** | נתונים חדשים (Regions, Tech, Perks, Events) – **לא משולבים** |
| **קונפליקטים** | גיבורים, יחידות, אזורים – Export שונה מהפרויקט |

---

## 2. מבנה הפרויקט הנוכחי

### 2.1 C++ Classes

| מחלקה | תפקיד |
|-------|--------|
| ARTSUnitCharacter | יחידה – OrderComponent, MoraleComponent, InitializeFromRegistry |
| ARTSHeroCharacter | גיבור – CommandAuthority, SecureRegion |
| ARTSRegionVolume | אזור – Control 0–5, Population, Stability, GetRegionAtLocation |
| ARTSPlayerController | בחירה, LMB/RMB, הוצאת פקודות |
| ARTSCameraPawn | מצלמה + WASD/Arrows |
| ARTSGameModeBase | GameMode בסיס |
| URTSOrderComponent | Move/Attack, PostCombatBehavior |
| URTSCommandAuthorityComponent | טווח פקודה, CanIssueOrderToUnit |
| URTSMoraleComponent | מורל 0–100 |
| URTSSecureRegionComponent | Secure Region (Hero) |
| URTSDataRegistry | DataTables – Units, Heroes, XP, Morale |
| URTSVictorySubsystem | ניצחון/הפסד |
| URTSSquadState | מצב Squad (MVP) |

### 2.2 Blueprints

| Blueprint | מיקום | יורש מ־ |
|-----------|--------|----------|
| BP_RTSUnit | /Game/RTS/ | RTSUnitCharacter |
| BP_RTSHero | /Game/RTS/ | RTSHeroCharacter |
| BP_RTSGameMode | /Game/RTS/Blueprints/ | RTSGameModeBase |
| BP_RTSGameState | /Game/RTS/Blueprints/ | (Blueprint only) |
| BP_SettlementBase | /Game/RTS/Data/Settlements/ | Actor |

### 2.3 DataTables (Registry)

| DataTable | מיקום | Struct |
|-----------|--------|--------|
| DT_Units_MVP | /Game/RTS/Data/Data_Tables/ | FUnitRow |
| DT_Heroes_MVP | /Game/RTS/Data/Data_Tables/ | FHeroRow |
| DT_HeroXP_Prototype | /Game/RTS/Data/Data_Tables/ | FHeroXPRow |
| DT_MoraleThresholds | /Game/RTS/Data/Data_Tables/ | FMoraleThresholdRow |

### 2.4 DataAssets (לא ב-Registry)

| DataAsset | מיקום |
|-----------|--------|
| DA_Faction_Humans, DA_Faction_Vampires, DA_Faction_Werewolves | /Game/RTS/Data/Archetypes/ |
| DA_Hero_Paladin, DA_Hero_HighVampireElder, DA_Hero_WarriorKing | /Game/RTS/Data/Heroes/ |
| DA_Settlement_Small/Medium/Large | /Game/RTS/Data/Settlements/ |
| DA_RuleBook | /Game/RTS/Data/Rules/ |

### 2.5 ARTSRegionVolume – שדות נוכחיים

- ControlLevelHumans, ControlLevelVampires, ControlLevelWerewolves (0–5)
- Population, Stability, DominantFaction
- **חסר:** RegionType, RegionTraits, RegionID, BasePopulation, BaseStability

---

## 3. Export Pack (RTS_All_Files_Export)

### 3.1 קבצי CSV

| קובץ | תוכן |
|------|------|
| RegionTypes.csv | 12 סוגי אזור (HumanRegion, City, Village, Forest, Mountain...) |
| RegionTraits.csv | 17 תכונות (Fertile, Wild, Haunted, Corrupted...) |
| SampleRegions.csv | 8 אזורים לדוגמה (Ironvale, Blackwood...) |
| HeroPerks.csv | 9 גיבורים × 6 רמות (Sir Aldric, Malkor, Kargar...) |
| HumanTech.csv | 7 מבנים (Town Hall, Barracks, Archery Range...) |
| VampireTech.csv | 7 מבנים (Dark Court, Night Barracks...) |
| WerewolfTech.csv | 7 מבנים (Den, War Den, Moon Shrine...) |
| LegendaryArtifacts.csv | 15 חפצים (Blade of First King...) |
| WorldEvents.csv | 30 אירועים (Peasant Unrest, Blood Cult...) |

### 3.2 Docs Pack (RTS_UE54_Updated_Docs_Pack)

- RTS_Cursor_MVP_Implementation_Updated.docx
- RTS_MVP_Adapted_UE54.docx
- RTS_Unreal_Architecture_Adapted.docx

---

## 4. קונפליקטים ובעיות

### 4.1 גיבורים – אי-התאמה מלאה

| מקור | גיבורים |
|------|---------|
| **פרויקט DT_Heroes** | HighCommander, BloodLord, AlphaPrime (3 – אחד לכל פאקשן) |
| **פרויקט DA_Hero** | Paladin, HighVampireElder, WarriorKing (שמות שונים!) |
| **Export HeroPerks** | Sir Aldric, Marcus, Archmage (Human); Malkor, Isabella, Draven (Vampire); Kargar, Volgar, Toran (Werewolf) – 9 גיבורים |

**בעיה:** שלוש מערכות גיבורים שונות. ה-Registry משתמש ב-DT_Heroes בלבד. DA_Hero ו-HeroPerks לא משולבים.

### 4.2 יחידות – אי-התאמה

| מקור | יחידות Human לדוגמה |
|------|----------------------|
| **פרויקט DT_Units** | Militia, Swordsman, Archer, Knight |
| **Export HumanTech** | Militia, Hunters; Footman, Town Spearmen; Longbowmen; Light Cavalry, Knight; Mage Apprentice, Battle Priest; Ballista, Trebuchet; Order of the Dawn, War Griffin |

**בעיה:** Export מגדיר מבנים שמשחררים יחידות – הפרויקט לא תומך במבנים או ב-unlock chain.

### 4.3 אזורים – חסרים ב-Export, חסרים בפרויקט

**SampleRegions.csv – שגיאות:**
- R04 (Stonepass): SecondaryTrait = **Harsh** – **לא קיים** ב-RegionTraits.csv
- R05 (Ashfields): SecondaryTrait = **BloodSoaked** – **לא קיים** ב-RegionTraits.csv

**ARTSRegionVolume – חסר:**
- RegionType (HumanRegion, City, Forest...)
- RegionTraits (Fertile, Wild, Corrupted...)
- RegionID
- BasePopulation, BaseStability, BaseControlDifficulty
- GrowthModifier, StabilityModifier, CorruptionRate...

### 4.4 נתיבי DataTables

- **RTS_ARCHITECTURE.md** מציין `/Game/Data/` – **שגוי**.
- **קוד (RTSDataRegistry.cpp)** משתמש ב-`/Game/RTS/Data/Data_Tables/` – **נכון**.

### 4.5 OrderComponent – אזהרות Log

```
Unable to load OrderComponent with outer BP_RTSHero_C ... because its class (RTSOrderComponent) does not exist
```

**סיבה:** Blueprints נוצרו לפני שה-RTSOrderComponent הוגדר ב-C++. פתרון: פתח BP_RTSUnit ו-BP_RTSHero, Compile, שמור – או וודא שה-Build כולל את RTSOrderComponent.

### 4.6 IsContested – Werewolves לא נספרים

```cpp
bool ARTSRegionVolume::IsContested() const
{
    bool bHuman = HasHeroOfFaction(EFactionId::Humans);
    bool bVampire = HasHeroOfFaction(EFactionId::Vampires);
    return bHuman && bVampire;  // Werewolves לא נכללים!
}
```

**בעיה:** GDD מדבר על Humans vs Vampires ב-MVP, אבל הקוד מתעלם מ-Werewolves. אם 3 פאקשנים – צריך `(bHuman && bVampire) || (bHuman && bWerewolf) || (bVampire && bWerewolf)`.

---

## 5. פערים (מה חסר)

| מערכת | Export | פרויקט | פער |
|-------|--------|--------|-----|
| Region Types | ✅ 12 | ❌ | צריך להוסיף RegionType ל-ARTSRegionVolume |
| Region Traits | ✅ 17 | ❌ | צריך FRegionTraitRow + DataTable |
| Hero Perks | ✅ 54 שורות | ❌ | אין מערכת Perks |
| Tech/Buildings | ✅ 21 מבנים | ❌ | אין מערכת בנייה |
| World Events | ✅ 30 אירועים | ❌ | אין Event System |
| Artifacts | ✅ 15 חפצים | ❌ | אין מערכת Artifacts |
| Economy | GDD | ❌ | Humans=Money, Vampires=Convert – לא מיושם |

---

## 6. המלצות

### עדיפות גבוהה

1. **תיקון RTS_ARCHITECTURE.md** – לעדכן נתיבי DataTables ל-`/Game/RTS/Data/Data_Tables/`.
2. **תיקון SampleRegions.csv** – להחליף Harsh → Fortified או להוסיף Harsh ל-RegionTraits; BloodSoaked → Corrupted או להוסיף.
3. **תיקון IsContested** – לכלול Werewolves אם MVP תומך ב-3 פאקשנים.
4. **קריסת Play** – להמשיך לפי CRASH_Default__Object_PLAY.md (Level Blueprint, מפת בדיקה).

### עדיפות בינונית

5. **החלטת גיבורים** – לבחור מקור אחד: DT_Heroes (MVP) או HeroPerks (Post-MVP). אם Post-MVP – ליצור DT_Heroes_Perks ולהרחיב FHeroRow.
6. **RegionType/Traits** – להוסיף FRegionTypeRow, FRegionTraitRow, DataTables; להרחיב ARTSRegionVolume ב-RegionTypeId, PrimaryTraitId, SecondaryTraitId.
7. **OrderComponent warnings** – Recompile Blueprints אחרי Build מלא.

### עדיפות נמוכה / Post-MVP

8. **Tech/Buildings** – מערכת בנייה + unlock chain.
9. **World Events** – Event System + SpawnUnits, SpawnMonster.
10. **Artifacts** – מערכת חפצים לגיבורים.

---

## 7. טבלת מיפוי גיבורים (החלטה)

**MVP:** 3 גיבורים = **Faction Leaders** (HighCommander, BloodLord, AlphaPrime).  
**Post-MVP:** 9 גיבורים מ־HeroPerks = **גיבורי משנה** (Sub-heroes) – יוגדרו כיחידות/גיבורים נפרדים.

| Export HeroPerks | פרויקט DT (Faction Leader) | הערה |
|-----------------|----------------------------|------|
| Sir Aldric Valemor (H_H1) | HighCommander | MVP |
| Marcus Blightwood, Archmage Lucian | – | גיבורי משנה (Post-MVP) |
| Malkor the Eternal (V_H1) | BloodLord | MVP |
| Countess Isabella, Draven Mournight | – | גיבורי משנה (Post-MVP) |
| Kargar Ironmoon (W_H1) | AlphaPrime | MVP |
| Volgar the Grey Hunter, Toran | – | גיבורי משנה (Post-MVP) |

---

## 8. מסמכים קשורים

- `.cursor/GDD.md` – GDD נעול
- `.cursor/docs/RTS_ARCHITECTURE.md` – ארכיטקטורה
- `.cursor/docs/CRASH_Default__Object_PLAY.md` – תיקון קריסה (רקע)
- `.cursor/docs/FIX_CRASH_AND_ORDERCOMPONENT.md` – **מדריך מעשי** לקריסה + OrderComponent
- `.cursor/docs/DATA_TABLES_CHECKLIST.md` – DataTables
- `RTS_All_Files_Export/README_RTS_Export_Pack.md` – תיאור Export
