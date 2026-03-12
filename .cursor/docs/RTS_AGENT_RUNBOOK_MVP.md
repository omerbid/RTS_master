# RTS_Monsters – חלוקת משימות לסוכנים (הרצת MVP מהר)

**מטרה:** אפיקים נפרדים ללא תלות – כל סוכן עובד בתיקיות/קבצים משלו. מתי להפעיל כמה סוכנים מפורט בהמשך.

---

## תרשים תלויות (מתי כל אפיק יכול להתחיל)

```
שלב 1 (מקביל, 4 סוכנים):
  [סוכן A: Data]     [סוכן B: Core+BP]   [סוכן C: Squad+Region]   [סוכן D: Map]
        |                     |                      |                     |
        v                     v                      v                     v
   DataTables מוכנים    Unit/Hero BPs + ניקוי    Squad manager + Region   מפה + spawn
        |                     |                      |                     |
        +---------------------+----------------------+---------------------+
                                    |
                                    v
שלב 2 (אחרי שאין "table not found" ב־PIE, 2 סוכנים):
  [סוכן E: Input+Commands]          [סוכן F: UI מינימלי]
                                    |
                                    v
שלב 3 (1 סוכן): אינטגרציה, תיקון באגים, סימון צ'קליסט
```

---

# שלב 1 – הרצה במקביל (4 סוכנים)

**תנאי התחלה:** הפרויקט נבנה; C++ קיים (Unit, Hero, Morale, CommandAuthority, SquadState, Region, DataRegistry).

**מטרה:** DataTables עובדים, Blueprints ליחידה/גיבור, מנהל סקוואדים, Region במפה, מפה מוכנה ל־PIE.

---

## סוכן A – Data Backbone (רק Content + CSV)

**תיקיות/קבצים:** רק `Content/Data/` (חדש) ו־קבצי CSV מ־`.cursor/data/RTS_Pack/`. **לא לגעת ב־Source.**

**משימות:**
1. ליצור תיקייה **Content/Data/** (תחת Content ב־Unreal או במערכת קבצים).
2. ליצור 4 DataTables ב־Content/Data/ עם שמות מדויקים:
   - **DT_Units_MVP** → Row Structure: `FUnitRow`
   - **DT_Heroes_MVP** → Row Structure: `FHeroRow`
   - **DT_HeroXP_Prototype** → Row Structure: `FHeroXPRow`
   - **DT_MoraleThresholds** → Row Structure: `FMoraleThresholdRow`
3. לייבא CSV לכל DataTable (File → Import):
   - `.cursor/data/RTS_Pack/DT_Units_MVP.csv` → DT_Units_MVP
   - `.cursor/data/RTS_Pack/DT_Heroes_MVP.csv` → DT_Heroes_MVP
   - `.cursor/data/RTS_Pack/DT_HeroXP_Prototype.csv` → DT_HeroXP_Prototype
   - `.cursor/data/RTS_Pack/DT_MoraleThresholds.csv` → DT_MoraleThresholds
4. לוודא שהנתיבים הסופיים: `/Game/Data/DT_Units_MVP` וכו'.
5. להריץ PIE ולוודא שאין ב־Output Log: "RTSDataRegistry: Units table not found" (ואם יש – לתקן נתיב/שם).

**סיום:** ב־PIE אין אזהרות "table not found" ל־Units ו־Heroes. שאר הסוכנים יכולים להניח ש־DataTables קיימים.

---

## סוכן B – Core Characters + Blueprints + ניקוי

**תיקיות/קבצים:** `Source/RTS_Monsters/` (רק MyActor, אופציונלי קטן ב־Unit/Hero), ו־`Content/RTS/Blueprints/` (או תיקייה מתאימה ל־BP). **לא לגעת ב־Content/Data/.**

**משימות:**
1. **ניקוי:** אם אף Blueprint לא יורש מ־MyActor – למחוק את `MyActor.cpp` ו־`MyActor.h` (ולהסיר מהמודול אם צריך).
2. **Blueprint יחידה:** ליצור Blueprint שיורש מ־**RTSUnitCharacter** (Content/RTS/Blueprints או Characters):
   - שם מוצע: `BP_RTSUnit` או `BP_Unit_Militia`.
   - להגדיר **Unit Id** (למשל `Militia` או ערך מקביל מ־DT_Units_MVP).
   - ב־Event **Begin Play** – לקרוא **Initialize From Registry**.
3. **Blueprint גיבור:** ליצור Blueprint שיורש מ־**RTSHeroCharacter**:
   - שם מוצע: `BP_RTSHero` או `BP_Hero_Commander`.
   - להגדיר **Hero Id** (ערך מ־DT_Heroes_MVP).
   - ב־Begin Play – **Initialize From Registry**.
4. לוודא ש־**MoraleComponent** ו־**CommandAuthorityComponent** מחוברים (ברירת מחדל ב־C++ על Unit/Hero).
5. אופציונלי: אם חסר – לחשוף ב־C++ property ל־UnitId/HeroId כ־BlueprintReadOnly (אם כבר קיים – לדלג).

**סיום:** יש BP ליחידה ו־BP לגיבור; ב־PIE (אחרי שסוכן A סיים) Spawn של ה־BP מאתחל סטטים מה־Registry ללא crash.

---

## סוכן C – Squad Manager + Region במפה

**תיקיות/קבצים:** `Source/RTS_Monsters/` (רק אם מוסיפים Subsystem או משנים GameState), ו/או **Blueprint** של GameState; מפה – `Content/TopDown/Maps/TopDownMap` (או מפת RTS). **לא לגעת ב־Content/Data/ או ב־BP של Unit/Hero.**

**משימות:**
1. **מנהל סקוואדים:** להחליט איפה **URTSSquadState** מאוחסן:
   - **אפשרות 1 (Blueprint):** ב־**BP_RTSGameState** – להוסיף משתנה (array של objects) מסוג RTS Squad State, ופונקציות "Create Squad", "Register Unit to Squad" (קריאות ל־C++ אם קיימות).
   - **אפשרות 2 (C++):** ליצור **Subsystem** (למשל `URTSSquadManagerSubsystem`) שמחזיק TArray של URTSSquadState ומנהל יצירה/הצטרפות. לחשוף ל־Blueprint.
2. לחבר את **ARTSUnitCharacter** ל־Squad (קריאה מ־Unit ל־AddMember כשנוצר/מצטרף לסקוואד) – אם ה־API קיים ב־C++; אחרת לחשוף פונקציה ב־GameState/Subsystem.
3. **Region:** לוודא ש־**ARTSRegionVolume** ניתן להצבה במפה. לפתוח את מפת המשחק (TopDownMap או מפת RTS) ולהציב **RTS Region Volume** אחד לפחות; להגדיר Control Level ראשוני לפי פאקשן (אופציונלי).
4. לחשוף ל־Blueprint (אם חסר): GetControlLevelForFaction, GetDominantFaction – ב־ARTSRegionVolume כבר קיימים או להוסיף.

**סיום:** יש "בעלים" לסקוואדים (GameState או Subsystem); Region Volume במפה; אין קונפליקטים עם סוכנים A/B.

---

## סוכן D – מפה ו־Game Mode

**תיקיות/קבצים:** `Config/` (אם צריך), `Content/TopDown/Maps/` או מפת RTS, **BP_RTSGameMode**. **לא לגעת ב־Source/RTS_Monsters או ב־Content/Data/.**

**משימות:**
1. לוודא ש־**DefaultEngine.ini** מצביע על מפת ברירת מחדל ועל **GlobalDefaultGameMode** ל־BP_RTSGameMode (כבר מוגדר).
2. לוודא ש־**BP_RTSGameState** קיים ומחובר ל־Game Mode.
3. במפת המשחק: לוודא שיש **Player Start** (או ליצור). אופציונלי – להוסיף **Test spawn** של גיבור ו־2–3 יחידות ב־BeginPlay (מ־BP_RTSGameMode או מ־Level Blueprint) כדי שאפשר לבדוק PIE בלי להקליד ידנית.
4. אם יש מפת RTS נפרדת – להגדיר אותה כ־EditorStartupMap/GameDefaultMap; אחרת להשאיר TopDownMap.

**סיום:** PIE נטען עם GameMode ו־GameState; יש איפה שהשחקן מתחיל (ו־אופציונלי יחידות בדיקה).

---

## אחרי שלב 1

- **בדיקה משותפת:** להריץ PIE; לוודא שאין "table not found", ש־Unit/Hero מתאתחל, ש־Region קיים במפה וש־מנהל הסקוואדים קיים.
- **מעבר לשלב 2:** רק אחרי ש־DataTables עובדים ו־BP של Unit/Hero עובדים (סוכנים A ו־B לא חוסמים; C ו־D עצמאיים).

---

# שלב 2 – Input + UI (2 סוכנים)

**תנאי:** שלב 1 הושלם; ב־PIE יש גיבור ויחידות (לפחות מ־spawn בדיקה).

---

## סוכן E – Input ופקודות (P1)

**תיקיות/קבצים:** `Config/DefaultInput.ini`, `Source/RTS_Monsters/` (Input אם ב־C++), ו/או **Blueprints** ל־PlayerController / Input. **לא לגעת ב־Content/RTS/UI.**

**משימות:**
1. **Input Actions (Enhanced Input):** ליצור Input Actions – למשל: `IA_Select`, `IA_Move`, `IA_Attack` (או קליק שמאלי/ימני אם מתאים).
2. **Input Mapping:** לחבר ל־PlayerController (Blueprint או C++) – בחירת יחידות (לחיצה או גרירה), פקודת תנועה (לחיצה על קרקע), פקודת התקפה (לחיצה על אויב).
3. **לוגיקת פקודות:** לממש או לחבר את **IssueOrderToUnitsInRange** ב־RTSHeroCharacter:
   - לאסוף יחידות בטווח (CommandAuthorityComponent.CanIssueOrderToUnit).
   - להעביר להן פקודה (Move To / Attack) – ב־C++ או ב־Blueprint שקורא ל־C++.
4. **בחירה:** לשמור "נבחרים" ב־PlayerController או HUD; להציג סימון (decals/sprites) אופציונלי.

**סיום:** השחקן יכול לבחור יחידות, לתת פקודת תנועה ופקודת התקפה; הגיבור מפקד רק על יחידות בטווח.

---

## סוכן F – UI מינימלי (HUD)

**תיקיות/קבצים:** רק `Content/RTS/UI/` – Widget Blueprints, חומרים ל־HUD. **לא לגעת ב־Input או ב־Source.**

**משימות:**
1. ליצור **Widget** ל־HUD (למשל WBP_RTSHUD): טקסט/בר מורל, בר HP (לנבחר או לגיבור), אופציונלי – אינדיקציה לטווח פקודה.
2. לחבר את ה־Widget ל־Viewport (מ־PlayerController או GameMode ב־Begin Play).
3. לעדכן את הערכים מ־Unit/Hero נבחר (קריאות ל־Get Morale, Get Health וכו' – מה־C++ או Blueprint).

**סיום:** ב־PIE מוצג HUD בסיסי (מורל/HP) ליחידה או גיבור נבחר.

---

## אחרי שלב 2

- **בדיקה:** PIE – בחירה, תנועה, התקפה, HUD מעודכן.
- **מעבר לשלב 3:** כשהזרימה הבסיסית עובדת.

---

# שלב 3 – אינטגרציה (1 סוכן)

**מטרה:** לרכז תיקונים, למלא צ'קליסט MVP, לוודא שאין regressions.

**משימות לסוכן יחיד:**
1. להריץ PIE מקצה לקצה: טעינה → בחירת גיבור ויחידות → תנועה → התקפה → מורל.
2. לתקן באגים שנתגלו (או להקצות חזרה לאפיק מתאים אם דחוף).
3. לעבור על `.cursor/docs/RTS_CHECKLIST_EPIC_VS_MVP.md` – לסמן כל סעיף MVP שהושלם.
4. לעדכן תיעוד אם השתנו שמות/מבנה (למשל GDD – Squad = URTSSquadState).

**סיום:** צ'קליסט MVP מסומן; גרסת MVP שמישה לבדיקה.

---

# סיכום: כמה סוכנים ומתי

| שלב | מספר סוכנים | אפיקים |
|-----|-------------|--------|
| **1** | **4** | A=Data, B=Core+BP, C=Squad+Region, D=Map |
| **2** | **2** | E=Input+Commands, F=UI |
| **3** | **1** | אינטגרציה + צ'קליסט |

**הקטנת סוכנים:** אם יש רק 2 סוכנים – שלב 1: סוכן 1 = A+B (Data ואז Core+BP), סוכן 2 = C+D (Squad+Region + Map). שלב 2: אחד Input, אחד UI. שלב 3: אחד.

**הגדלת סוכנים:** בשלב 1 אפשר לפצל: סוכן נפרד רק ל־Blueprints (Unit/Hero) וסוכן רק ל־ניקוי C++ – אז 5 סוכנים. לא מומלץ יותר מ־5 בשלב 1 כדי לא להכפיל עריכה באותם קבצים.

**כלל:** כל סוכן עובד בתיקיות/קבצים שמופיעים במשימות שלו; לא לערוך את אותם קבצים בשני סוכנים במקביל.
