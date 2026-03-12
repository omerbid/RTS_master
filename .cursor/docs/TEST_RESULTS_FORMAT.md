# פורמט תוצאות טסטים – RTS_Monsters

## סקירה

תוצאות ההרצה (עובר/נכשל/דילוג) נשמרות **בקובץ נפרד** מהמפרט, כדי לא לשנות את קבצי ה־YAML של הטסטים.

**הקשר ל-Unreal:** מערכת ה־**Automation** של Epic מדווחת עובר/נכשל בחלון Session Frontend וב־Log. הפורמט כאן (YAML עם `results:`) הוא **משלים לפרויקט** – מאחד תוצאות מטסטים אוטומטיים (Automation) ומבדיקות ידניות (PIE) לסיכום אחד ב־`run_tests.ps1`. Epic לא מגדירה קובץ תוצאות בסגנון הזה; זה תקן פנימי של RTS_Monsters.

---

## קובץ התוצאות

**נתיב:** `.cursor/tests/test_results.yaml`

**מבנה:**

```yaml
# תוצאות טסטים – מתעדכן ידנית או מסקריפט/אוטומציה
# result: pass | fail | skip
# עודכן: YYYY-MM-DD (אופציונלי)

updated: "2026-03-10"

results:
  P0.1.1: pass
  P0.1.2: pass
  P0.1.3: fail
  P0.1.4: skip
  # ...
```

### שדות

| שדה | חובה | תיאור |
|-----|------|--------|
| `updated` | לא | תאריך עדכון אחרון (מחרוזת) |
| `results` | כן | מפתח = `id` של טסט (למשל `P0.1.1`), ערך = `pass` / `fail` / `skip` |

### ערכי `result`

- **pass** – הטסט עבר.
- **fail** – הטסט נכשל.
- **skip** – דילוג (לא הורץ, או לא רלוונטי כרגע).

---

## קובץ הערות (אופציונלי)

אם רוצים להשאיר הערה לכל טסט (למשל סיבת כישלון):

**נתיב:** `.cursor/tests/test_results_notes.yaml`

```yaml
notes:
  P0.1.3: "Registry not found in context – check GameInstance"
  P0.2.1: "BP_Unit missing in map"
```

הסקריפט `run_tests.ps1` יכול להציג הערות כשמוצג סיכום תוצאות.

---

## שימוש עם run_tests.ps1

אחרי ש־`test_results.yaml` קיים (עם מפתח `results:` ולפחות חלק מה־ids), הרצת:

```powershell
.\run_tests.ps1 P0
```

- ליד כל טסט יוצג סטטוס: **[PASS]** (ירוק), **[FAIL]** (אדום), **[SKIP]** (צהוב), או בלי סימון (עדיין לא הורץ).
- בסוף יוצג **Summary:** מספר pass, fail, skip, ו־"not run".
- נתיב קובץ התוצאות יוצג כדי שיהיה קל לערוך אותו.

לעדכון תוצאות: לערוך את `.cursor/tests/test_results.yaml` ידנית (תחת `results:`), או להריץ אוטומציה ולתעד את התוצאות בקובץ.
