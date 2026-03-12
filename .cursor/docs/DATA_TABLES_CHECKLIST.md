# DataTables Checklist – RTS_Monsters

ה־**URTSDataRegistry** טוען ארבעה DataTables בנתיבים קבועים. צריך ליצור את האססטים האלה באנריל כדי שהנתונים ייטענו.

---

## 1. תיקייה

- ב־**Content Browser**: צור תיקייה **Data** תחת **Content** (כלומר הנתיב יהיה `Content/Data/`).
- אם כבר קיימת – דלג.

---

## 2. ארבעה DataTables

צור **DataTable** חדש (Right‑click → Miscellaneous → Data Table) עבור כל שורה למטה. **שם האססט** חייב להיות **בדיוק** כמו ב־"Asset Name".

| Asset Name              | Row Structure (Struct) |
|-------------------------|-------------------------|
| **DT_Units_MVP**        | `FUnitRow`              |
| **DT_Heroes_MVP**       | `FHeroRow`              |
| **DT_HeroXP_Prototype** | `FHeroXPRow`            |
| **DT_MoraleThresholds** | `FMoraleThresholdRow`   |

- **מיקום**: שמור את כולם ב־`Content/Data/`.
- **Row Structure**: בעת יצירת ה־Data Table בחר את ה־Struct מהטבלה (מהמודול RTS_Monsters).

---

## 3. ייבוא CSV

הקבצים נמצאים בפרויקט ב־`.cursor/data/RTS_Pack/`:

- `DT_Units_MVP.csv`
- `DT_Heroes_MVP.csv`
- `DT_HeroXP_Prototype.csv`
- `DT_MoraleThresholds.csv`

**ייבוא:**

1. בחר את ה־DataTable ב־Content Browser.
2. בתפריט: **File → Import** (או גרור את ה־CSV לתוך האססט).
3. בחר את קובץ ה־CSV המתאים.
4. וודא ש־**Import Row Names** / עמודת מפתח מתאימה (אם האנריל שואל). אפשר להשאיר ברירת מחדל – הקוד מחפש לפי `UnitId` / `HeroId` וכו' בתוך השורות.

אם אין ייבוא מ־CSV: אפשר להזין שורות ידנית ב־DataTable (לפי השדות של כל Struct).

---

## 4. וידוא

- אחרי יצירה וייבוא, הנתיבים הבאים צריכים להתקיים:
  - `/Game/Data/DT_Units_MVP`
  - `/Game/Data/DT_Heroes_MVP`
  - `/Game/Data/DT_HeroXP_Prototype`
  - `/Game/Data/DT_MoraleThresholds`
- הרץ PIE (Play In Editor). אם ה־Registry טוען בהצלחה, **לא** תופיע ב־Output Log האזהרה:
  - `RTSDataRegistry: Units table not found at ...`
  - `RTSDataRegistry: Heroes table not found at ...`
- יחידה/גיבור: הגדר **UnitId** או **HeroId** ב־Details, ואז קרא **Initialize From Registry** (מ־Blueprint או מקוד) – הנתונים ייטענו מה־DataTables.

---

## 5. שימוש מ־Blueprint

- **Unit**: על ה־Actor של היחידה (Blueprint שיורש מ־`RTSUnitCharacter`) – הגדר **Unit Id** (למשל `Militia`, `Swordsman`) ובאירוע **Begin Play** (או אחרי Spawn) קרא ל־**Initialize From Registry**.
- **Hero**: אותו דבר עם **Hero Id** (למשל `HighCommander`, `BloodLord`) ו־**Initialize From Registry**.
- **נתונים גולמיים**: מ־**Get Game Instance** → **Get Subsystem** → **RTS Data Registry** אפשר לקרוא ל־**Get Unit Row** / **Get Hero Row** וכו' עם מפתח (שם השורה/מזהה).

---

סיום הצ'קליסט = אפיק 1 (Data Backbone & Registry) מוכן; אפשר להמשיך ל־P1 (פקודות, טווח פקודה, וכו').
