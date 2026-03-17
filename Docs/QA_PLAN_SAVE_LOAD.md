# תוכנית QA לרגרסיית Save/Load ומצב "פרויקט קיים"

**מטרה:** בסיס לתוכנית QA אוטומטית ל־Save/Load – עקרונות, מטריצת תרחישים, והפניה למסמכי הניתוח המפורטים. מסמך זה ממלא את הפער "אין תוכנית QA" שצוין בדוח הפערים.

**מקור:** דוח פערים ותכנון מעמיק (טבלה C, עקרונות אוטומציה), תיעוד Unreal Automation/Gauntlet.

---

## 1. עקרונות

- **Automation First:** רוב הבאגים של Save/Load הם "מצביים" (stateful); בדיקות ידניות לא מספיקות לרגרסיה. יש להריץ מטריצת תרחישים ב־Automation.
- **Golden Saves:** לכל גרסת schema (v1, v2…) – קובץ שמירה "זהב" שנטען בכל build כדי לוודא migration.
- **Test Groups:** ב־DefaultEngine.ini ניתן להגדיר Groups ולהריץ רק סט SaveLoad ב־CI: `-ExecCmds="Automation RunTest Group:SaveLoad"`.
- **Harness:** מפת טסט קטנה ודטרמיניסטית (Region אחד, RegionId ידוע, spawn points קבועים), Fixture Builder ליצירת מצבים (idle, mid-secure, mid-ritual, contested), Mock DataRegistry אופציונלי.

---

## 2. מטריצת תרחישים (סיכום – פרטים בדוח הפערים טבלה C)

| עדיפות | תרחיש | אוטומציה |
|--------|--------|----------|
| קריטית | IndexSave בסיסי (2 פרויקטים, טעינת רשימה) | כן |
| קריטית | Save/Load Idle (Regions/Units/Economy/Victory זהים) | כן |
| קריטית | Save באמצע Secure (t=7/15) → Load → ערוץ ממשיך/מתאפס עקבי | כן |
| קריטית | Save באמצע RitualChannel (t=4/10) → Load → לא נתקע | כן |
| קריטית | Save בזמן RespawnTimer → Remaining נשמר | כן |
| קריטית | AttackTarget resolve אחרי Load; אם יעד חסר → sanitize ל־None | כן |
| קריטית | Version mismatch v1→v2 → migration + Load מצליח | כן |
| קריטית | Corrupt slot A → Load בוחר B או הודעת error | כן |
| גבוהה | Contested, Orders Move/Attack, Economy tick boundary, Partial data | כן |
| בינונית | Save+Load במקביל → SaveLock מונע | כן |
| גבוהה | Stress 500/1000 units | כן (profiling) |

---

## 3. תזמון ומשאבים (גנרי)

- Harness + Fixtures + כ־10–15 תרחישים קריטיים: ~7–15 אדם־ימים.
- Golden saves + migration tests: ~3–8 אדם־ימים.
- Stress/perf בסיסי: ~2–4 אדם־ימים.
(הערכה גסה; דורש כיול לפי צוות.)

---

## 4. קישורים

- `Docs/save_load_spec.md` – סכמה, Rehydration.
- `Docs/existing_project_mode_spec.md` – חוזה מוצרי.
- דוח פערים (טבלה C) – מטריצה מלאה.
- Unreal: Configure Automation Tests, Run Automation Tests, Gauntlet.
