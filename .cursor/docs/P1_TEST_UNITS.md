# P1 – בדיקת יחידות וגיבור במפה

**המחלקות C++ לא מופיעות ב־Place Actors.** צריך ליצור פעם אחת Blueprint יחידה ו־Blueprint גיבור – הוראות מפורטות: **[CREATE_BP_UNIT_HERO.md](CREATE_BP_UNIT_HERO.md)**. אחרי ש־BP_RTSUnit ו־BP_RTSHero קיימים – גרור אותם למפה.

---

## 1. DataTables

וודא שיש לך DataTables עם לפחות שורה אחת:

- **DT_Units_MVP** (מבנה `FUnitRow`) – נתיב: `/Game/RTS/Data/Data_Tables/DT_Units_MVP`
- **DT_Heroes_MVP** (מבנה `FHeroRow`) – נתיב: `/Game/RTS/Data/Data_Tables/DT_Heroes_MVP`

אם אין – צור את הטבלאות וייבא CSV או הזן שורה ידנית (למשל **Row Name** = `Militia` ליחידה, `HighCommander` לגיבור, והשדות לפי המבנה).

---

## 2. הצבת יחידות במפה

1. **אם עדיין לא יצרת:** עקוב אחרי [CREATE_BP_UNIT_HERO.md](CREATE_BP_UNIT_HERO.md) כדי ליצור **BP_RTSUnit** ו־**BP_RTSHero**.
2. ב־**Content Browser** גרור **BP_RTSHero** למפה (הגיבור).
3. גרור **BP_RTSUnit** פעם או יותר למפה (בטווח ~2000–2500 מהגיבור).
4. Unit Id / Hero Id ו־Faction Id מוגדרים ב־Blueprint; אם צריך – בחר Actor במפה וערוך ב־**Details**.

ב־**Begin Play** הקוד קורא אוטומטית ל־**Initialize From Registry** כש־Unit Id / Hero Id מוגדרים.

---

## 3. בדיקת P1

1. **Play**.
2. **LMB** על הגיבור – אמור להיבחר.
3. **Shift+LMB** על יחידה – מתווספת ל־selection.
4. **RMB** על הרצפה – פקודת **Move** ליחידות שנמצאות **בטווח הגיבור** (2500 UU).
5. **RMB** על יחידת אויב (אקשן שונה) – פקודת **Attack**.

אם אין לך שורות ב־DataTables, הוסף שורה אחת ב־DT_Units_MVP ו־DT_Heroes_MVP והשתמש ב־**Row Name** כ־Unit Id / Hero Id ב־Details.
