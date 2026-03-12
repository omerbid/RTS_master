# RTS_Monsters – ארכיטקטורה מלאה וניתוח פרויקט

**תאריך:** מרץ 2026  
**גרסת Unreal:** 5.4  
**מודול:** RTS_Monsters (Runtime)

---

## 1. סקירת פרויקט

| פריט | ערך |
|------|-----|
| **שם פרויקט** | RTS_Monsters |
| **סוג** | RTS עם 3 פאקשנים (בני אדם, ערפדים, אנשי זאב) |
| **מפה** | Campaign map רציפה, אזורים עם שליטה מדורגת |
| **פקודה** | Hero-centric (גיבור מפקד על יחידות בטווח) |
| **נתונים** | Data-driven (DataTables + DataRegistry) |

---

## 2. מבנה תיקיות

```
RTS_Monsters/
├── Config/                    # הגדרות מנוע ומשחק
│   ├── DefaultEngine.ini      # GameMode, מפות, redirects
│   ├── DefaultGame.ini        # ProjectName (לעדכן!)
│   ├── DefaultInput.ini       # Enhanced Input
│   └── DefaultEditor.ini     # SimpleMap (להתאים!)
├── Content/
│   ├── Data/                  # DataTables ל־Registry (ליצור!)
│   │   └── DT_Units_MVP, DT_Heroes_MVP, DT_HeroXP_Prototype, DT_MoraleThresholds
│   ├── RTS/                   # כל התוכן של המשחק
│   │   ├── Blueprints/        # BP_RTSGameMode, BP_RTSGameState, BP_SettlementBase
│   │   ├── Core/              # Enums (E_Faction, E_UnitBranch...), Structs (F_ResourceBundle...)
│   │   ├── Data/
│   │   │   ├── Archetypes/    # DA_Faction*, DA_Hero*, DA_RuleBook
│   │   │   ├── Heroes/        # DA_Hero_Paladin, DA_Hero_HighVampireElder...
│   │   │   ├── Rules/         # DA_RuleBook
│   │   │   └── Settlements/   # DA_SettlementArchetype, BP_SettlementBase
│   │   ├── UI/                # (ריק/להרחבה)
│   │   └── World/             # (ריק/להרחבה)
│   ├── TopDown/               # מפת התבנית
│   │   └── Maps/TopDownMap.umap
│   └── StarterContent/
├── Source/RTS_Monsters/       # C++ – כל הלוגיקה הליבה
│   ├── RTS_Monsters.Build.cs
│   ├── RTSDataTypes.h        # EFactionId, EUnitRole, FUnitRow, FHeroRow, FHeroXPRow, FMoraleThresholdRow
│   ├── RTSDataRegistry.*     # GameInstanceSubsystem – טעינת DataTables וחיפוש לפי ID
│   ├── RTSUnitCharacter.*    # ACharacter – יחידה, MoraleComponent, InitializeFromRegistry
│   ├── RTSHeroCharacter.*   # יורש Unit – גיבור, CommandAuthorityComponent, IssueOrderToUnitsInRange
│   ├── RTSRegionVolume.*     # AVolume – שליטה לפי פאקשן, יציבות, אוכלוסייה
│   ├── RTSSquadState.*       # UObject – רשימת יחידות, ממוצע מורל, RecalcMorale
│   ├── RTSMoraleComponent.*  # ActorComponent – CurrentMorale, ApplyMoraleDelta
│   ├── RTSCommandAuthorityComponent.*  # טווח פקודה, CanIssueOrderToUnit
│   └── MyActor.*             # שארית תבנית – להסרה
└── .cursor/
    ├── docs/                  # GDD, RTS_Pack, DATA_TABLES_CHECKLIST, RTS_ARCHITECTURE (זה)
    └── plans/                 # P0 Core Setup
```

---

## 3. מערכות ליבה (C++)

### 3.1 נתונים (Data Backbone)

| רכיב | תפקיד |
|------|--------|
| **RTSDataTypes.h** | Enums: EFactionId, EUnitRole, ESpecialTag. Structs: FUnitRow, FHeroRow, FHeroXPRow, FMoraleThresholdRow, FPredatorPreyStateRow. |
| **URTSDataRegistry** | GameInstanceSubsystem. טוען 4 DataTables מ־`/Game/Data/`. מספק GetUnitRow, GetHeroRow, GetHeroXPRow, GetMoraleThresholdRow, GetUnitRowForHero. |

**נתיבי DataTables (חובה):**

- `/Game/RTS/Data/Data_Tables/DT_Units_MVP`
- `/Game/RTS/Data/Data_Tables/DT_Heroes_MVP`
- `/Game/RTS/Data/Data_Tables/DT_HeroXP_Prototype`
- `/Game/RTS/Data/Data_Tables/DT_MoraleThresholds`

**הערה:** ה־Registry טוען מ־`Content/RTS/Data/Data_Tables/`.

### 3.2 יחידות וגיבורים

| מחלקה | בסיס | תפקיד עיקרי |
|--------|------|--------------|
| **ARTSUnitCharacter** | ACharacter | Faction, UnitId, FUnitRow, MoraleComponent; InitializeFromRegistry / InitializeFromUnitRow. |
| **ARTSHeroCharacter** | ARTSUnitCharacter | HeroId, FHeroRow, CommandAuthorityComponent; IssueOrderToUnitsInRange (stub). |

### 3.3 סקוואד ומורל

| רכיב | סוג | תפקיד |
|------|-----|--------|
| **URTSSquadState** | UObject | SquadId, FactionId, Members (weak), AverageMorale; AddMember, RemoveMember, RecalcMorale. |
| **URTSMoraleComponent** | ActorComponent | CurrentMorale, BaseMorale, Min/Max; ApplyMoraleDelta, SetMorale. |

### 3.4 פקודה ואזורים

| רכיב | תפקיד |
|------|--------|
| **URTSCommandAuthorityComponent** | CommandRadius, bAuthorityEnabled; CanIssueOrderToUnit (טווח + פאקשן). |
| **ARTSRegionVolume** | שליטה לפי פאקשן (ControlLevel), Population, Stability, DominantFaction; GetControlLevelForFaction. |

### 3.5 Game Mode / State

- **BP_RTSGameMode** – מוגדר כ־GlobalDefaultGameMode ב־DefaultEngine.ini.
- **BP_RTSGameState** – אין עדיין מחלקת C++; הלוגיקה ב־Blueprint.

---

## 4. זרימת נתונים ופקודות

1. **אתחול:** ב־BeginPlay (או אחרי Spawn) – Unit/Hero קורא ל־InitializeFromRegistry עם UnitId/HeroId; ה־Registry מחזיר שורה ומאתחל HP, Damage, MoveSpeed, MoraleBase וכו'.
2. **פקודות:** גיבור קורא ל־IssueOrderToUnitsInRange; CommandAuthorityComponent בודק טווחאקשן; ביצוע הפקודה (P1) – עדיין stub.
3. **מורל:** MoraleComponent מתעדכן מאירועים (נזק, נוכחות גיבור); SquadState.RecalcMorale מחשבת ממוצע ומשפיעה על התנהגות (בשלבים מאוחרים).

---

## 5. מוסכמות שמות

| סוג | דוגמה |
|-----|--------|
| C++ Classes | ARTSUnitCharacter, URTSDataRegistry |
| Blueprint | BP_RTSGameMode, BP_SettlementBase |
| Data Asset | DA_FactionArchetype, DA_Hero_Paladin |
| Enum | E_Faction, E_UnitBranch |
| Struct | F_ResourceBundle, F_UnitStats |
| API | RTS_MONSTERS_API |

---

## 6. ניתוח – פערים ואי-התאמות

### 6.1 Config

- **DefaultGame.ini:** `ProjectName=Top Down BP Game Template` – לא תואם ל־RTS_Monsters. **המלצה:** לעדכן ל־`ProjectName=RTS Monsters` (או שם התצוגה הרצוי).
- **DefaultEditor.ini:** `SimpleMapName=/Game/RTS_Monsters/Maps/TopDownExampleMap` – המפה לא קיימת. **המלצה:** להפנות ל־`/Game/TopDown/Maps/TopDownMap` או ליצור מפת RTS ייעודית ולעדכן.

### 6.2 DataTables vs DataAssets

- הקוד טוען DataTables מ־`/Game/RTS/Data/Data_Tables/`.
- ב־Content/RTS/Data/ יש גם DataAssets (DA_*) – ארכיטיפים, גיבורים, settlements, rules. ה־Registry משתמש ב־DataTables בלבד, לא ב־DataAssets.
- **המלצה:** להשאיר הפרדה: DataTables ל־Registry; DataAssets לעיצוב ו־Blueprint.

### 6.3 קוד מיותר

- **MyActor** – שארית תבנית. **המלצה:** למחוק כשאף Blueprint לא תלוי בו.

### 6.4 מערכות חסרות (לפי GDD ו־P0)

| מערכת | סטטוס | המלצה |
|--------|--------|--------|
| **Input (RTS)** | רק Enhanced Input ב־Config; אין C++ bindings ל־RTS | להוסיף ב־P1: בחירת יחידות, פקודות movement/attack (Blueprint או C++). |
| **AI** | אין Behavior Trees / AI Controller ייעודי | Post-MVP או P2: defensive AI ליחידות מנותקות. |
| **UI** | תיקיית UI קיימת, ללא widgets מתועדות | ליצור HUD בסיסי ל־MVP: מורל, בריאות, טווח פקודה. |
| **GameMode/GameState (C++)** | רק Blueprint | אופציונלי: אם רוצים לוגיקה מורכבת (squads, regions) ב־C++ – להוסיף ARTSGameMode / ARTSGameState. |
| **ביצוע פקודות** | IssueOrderToUnitsInRange stub | P1: ליישם movement / attack orders עם CommandAuthority. |

### 6.5 GDD vs קוד

- ב־GDD מופיע "ARTSQuadActor"; ביישום – URTSSquadState (UObject). זה מתועד ב־P0 כ־מכוון. **המלצה:** לעדכן את GDD/ארכיטקטורה עם השם URTSSquadState כדי למנוע בלבול.

---

## 7. תיקונים והמלצות (סיכום לביצוע)

### עדיפות גבוהה

1. **יצירת Content/RTS/Data/Data_Tables/** ו־4 DataTables לפי DATA_TABLES_CHECKLIST – חובה ל־Registry.
2. **עדכון DefaultGame.ini:** ProjectName ל־RTS Monsters (או שם תצוגה).
3. **עדכון DefaultEditor.ini:** SimpleMapName למפה קיימת או ל־TopDownMap.

### עדיפות בינונית

4. **מחיקת MyActor** אם לא בשימוש.
5. **תיעוד:** במסמך אחד (GDD או Architecture) – "Squad = URTSSquadState (UObject), לא Actor".
6. **P1:** חיווט Input + לוגיקת פקודות (move/attack) עם CommandAuthority.

### עדיפות נמוכה / Post-MVP

7. C++ GameMode/GameState אם נדרש.
8. AIModule + Behavior Trees ל־defensive AI.
9. UMG widgets ל־HUD (מורל, HP, טווח פקודה).

---

## 8. תלויות Build

**RTS_Monsters.Build.cs:** Core, CoreUObject, Engine, InputCore.  
אין Slate/UMG/AIModule – תואם למצב נוכחי (ללא UI/AI ב־C++).

---

## 9. מסמכים קשורים

- `.cursor/docs/DATA_TABLES_CHECKLIST.md` – יצירת DataTables ו־Registry.
- `.cursor/docs/RTS_Pack/RTS_MVP_1_3_GDD.md` – היקף MVP.
- `.cursor/docs/RTS_Pack/RTS_Post_MVP_Roadmap.md` – Post-MVP.
- `.cursor/plans/PLANNING_P0_core_setup.md` – P0 Epics ו־Tasks.

---

*מסמך זה משמש כבסיס ארכיטקטורה מלא לפרויקט RTS_Monsters; יש לעדכן אותו עם שינויים במערכות או במבנה.*
