# תיקון קריסת Play + אזהרות OrderComponent + Failed to spawn player controller

## "Failed to spawn player controller"

### 1. בדוק Override במפה (testlevel)

המפה יכולה לדרוס את ה־GameMode של הפרויקט:

1. פתח **testlevel** (או את המפה שבה אתה מריץ Play).
2. **Window → World Settings**.
3. גלול ל־**GameMode**.
4. אם יש **GameMode Override** או **Selected GameMode** – בחר **None** (כדי להשתמש בברירת המחדל של הפרויקט) או **RTS Game Mode Base**.
5. **שמור** את המפה (Ctrl+S).

### 2. וודא שה־ini נטען

אם שינית את DefaultEngine.ini – **סגור את האדיטור לגמרי** ופתח מחדש.

### 3. Build מלא

אם עדיין נכשל – **Build** את הפרויקט (לא Live Coding), סגור אדיטור, פתח מחדש.

### 4. איך למצוא RTS Camera Pawn ו־RTS Player Controller

אם לא מופיעים ב־dropdown:

1. **Build** את הפרויקט (סגור אדיטור → Build → פתח שוב).
2. ב־dropdown של Class: **בחר "All Classes"** (למטה ברשימה) או **הקלד "RTS"** בחיפוש.
3. **Default Pawn Class** → חפש **RTS Camera Pawn** (או RTSCameraPawn).
4. **Player Controller Class** → חפש **RTS Player Controller** (או RTSPlayerController).

**או** – **Game Mode Override** → **None** (כדי להשתמש ב־RTSGameModeBase שכבר מגדיר את שניהם).

---

## 3. קריסת Play (Cast Default__Object) – TopDownMap

**הקריסה קורית ב־TopDownMap** (יש לה World Partition). **testlevel עובד.**

**חשוב:** כשאתה משנה Game Mode ל־None – הפרויקט משתמש ב־RTSGameModeBase. הקריסה קורית כשמריצים Play על **TopDownMap** (לא על testlevel).

### פתרון מיידי: השתמש ב־testlevel ל־Play

1. פתח **testlevel** (לא TopDownMap).
2. **Window → World Settings** → **Game Mode Override** = **None** (כדי להשתמש ב־RTSGameModeBase).
3. **Play** – אמור לעבוד (מצלמה + בחירה).
4. **Project Settings → Maps & Modes** – וודא ש־**Editor Startup Map** ו־**Game Default Map** הם **testlevel**.

### אם אתה חייב להשתמש ב־TopDownMap

#### א. כיבוי World Partition

1. פתח **TopDownMap**.
2. **Window → World Settings**.
3. גלול ל־**World Partition**.
4. בטל סימון של **Enable World Partition** (אם קיים).
5. שמור. **ייתכן שיהיה צורך ב־Convert** – האדיטור יבקש.

#### ב. אם א' גורם למסך שחור – צור מפת בדיקה חדשה

**כיבוי World Partition על TopDownMap שובר תאורה** – המפה נבנתה עם WP. במקום זה:

1. **File → New Level → Basic** (יש תאורה ורצפה).
2. הוסף **Player Start** (Place Actors → Basic → Player Start).
3. **File → Save Current Level As** → שמור כ־`/Game/RTS/Maps/RTS_TestMap`.
4. **Window → World Settings** → **Game Mode Override** = **None**.
5. **Project Settings → Maps & Modes**:
   - **Editor Startup Map** → RTS_TestMap
   - **Game Default Map** → RTS_TestMap
6. **Play** – בדוק מצלמה (WASD) ובחירה (LMB/RMB).
7. (אופציונלי) גרור **BP_RTSUnit** או **BP_RTSHero** למפה כדי לבדוק בחירה ופקודות.

**החזר World Partition** ב־TopDownMap (סמן שוב Enable World Partition) כדי שהמפה תחזור למצב תקין.

---

## 4. אזהרות OrderComponent

```
Unable to load OrderComponent with outer BP_RTSHero_C ... because its class (RTSOrderComponent) does not exist
```

### סיבה

ה־Blueprint נשמר כשהמודול RTS_Monsters (שמכיל RTSOrderComponent) לא היה טעון. זה קורה לעיתים ב־World Partition streaming או ב־load order.

### מה לעשות

#### שלב 1: Build מלא

1. **סגור** את Unreal Editor.
2. **Build** את הפרויקט (Visual Studio או Rider).
3. **פתח** את הפרויקט מחדש.

#### שלב 2: Recompile Blueprints

1. פתח **BP_RTSUnit** (Content Browser → /Game/RTS/).
2. לחץ **Compile** (או Ctrl+Alt+F11).
3. **Save**.
4. חזור על השלבים עבור **BP_RTSHero**.

#### שלב 3: אם האזהרה נשארת – Clean Build

1. סגור את האדיטור.
2. מחק את התיקיות:
   - `RTS_Monsters/Binaries/`
   - `RTS_Monsters/Intermediate/`
3. **Build** מחדש.
4. פתח את הפרויקט.

### הערה

ה־OrderComponent נוצר ב־C++ (ARTSUnitCharacter constructor). ה־Blueprint יורש אותו. אם האזהרה מופיעה רק ב־Log ולא משפיעה על Play – אפשר להמשיך; ה־component אמור לעבוד ב־runtime.

---

## 5. שליטה ביחידות (Hero-centric)

**להזיז יחידה** (לא רק גיבור):

1. **בחר את הגיבור** (LMB על הגיבור).
2. **הוסף יחידה לבחירה** – **Shift + LMB** על היחידה.
3. **RMB על הקרקע** – פקודת Move תישלח לגיבור ולכל היחידות בטווח הפקודה.

**חשוב:** פקודות (RMB) דורשות **גיבור בבחירה**. אם בחרת רק יחידה – RMB לא יעבוד. בחר תמיד את הגיבור (או גיבור + יחידות).
