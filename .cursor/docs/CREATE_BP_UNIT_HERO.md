# יצירת Blueprint יחידה ו־Blueprint גיבור (פעם אחת)

המחלקות C++ (**RTS Unit Character**, **RTS Hero Character**) לא מופיעות אוטומטית ב־Place Actors. צריך ליצור מהן **Blueprint** פעם אחת – ואז תוכל לגרור את ה־Blueprint למפה.

---

## 1. Blueprint יחידה (BP_RTSUnit)

1. ב־**Content Browser** → נווט לתיקייה (למשל `Content/RTS/Blueprints` או `Content/RTS`).
2. **Right‑click** → **Blueprint Class**.
3. בחלון "Pick Parent Class" לחץ על **All Classes** (או חפש).
4. בחיפוש הקלד: **RTS Unit**.
5. בחר **RTS Unit Character** (או **RTSUnitCharacter**) → **Select**.
6. תן שם: **BP_RTSUnit** → **Enter**.
7. פתח את ה־Blueprint (double‑click):
   - ב־**Details** (כשהרכיב **Self** נבחר) הגדר **Unit Id** לשם שורה מ־DT_Units_MVP (למשל `Militia` או שם השורה שלך).
   - **Faction Id** – למשל Humans.
   - שמור (Ctrl+S).

עכשיו יש לך **BP_RTSUnit** – אפשר לגרור אותו למפה מ־Content Browser.

---

## 2. Blueprint גיבור (BP_RTSHero)

1. **Right‑click** באותה תיקייה → **Blueprint Class**.
2. **All Classes** → חפש **RTS Hero**.
3. בחר **RTS Hero Character** → **Select**.
4. שם: **BP_RTSHero**.
5. פתח את ה־Blueprint:
   - **Hero Id** = שם שורה מ־DT_Heroes_MVP (למשל `HighCommander` או השם אצלך).
   - **Faction Id** = אותו פאקשן כמו היחידות (למשל Humans).
   - שמור.

עכשיו יש **BP_RTSHero** – גרור למפה.

---

## 3. שימוש במפה

- גרור **BP_RTSHero** למפה (זה הגיבור – נותן פקודות).
- גרור **BP_RTSUnit** פעם או כמה פעמים (בטווח ~2500 מהגיבור).
- **Play** – LMB לבחירה, RMB על הרצפה ל־Move.

אם אין שורות ב־DataTables (DT_Units_MVP, DT_Heroes_MVP), הוסף שורה אחת בכל טבלה והשתמש ב־**Row Name** כ־Unit Id / Hero Id ב־Blueprint.
