# Save Migration Policy – RTS Monsters

**מטרה:** מפת גרסאות Schema ומדיניות מיגרציה ל־Save/Load, כדי ששמירות ישנות ימשיכו להיטען אחרי שינויי קוד.

**קשור:** `Docs/save_load_spec.md` (Versioning §7), מסמך הניתוח "ניתוח פערים ותכנון מפורט ל־Save_Load, QA וביצועים".

---

## עקרונות

- לכל SaveGame יש **SaveSchemaVersion** (int32) ב־Header.
- כל שינוי "ששובר שחזור" (breaking change) מגדיל גרסה.
- טעינה תמיד: **LoadRaw → ValidateHeader → אם גרסה ישנה: MigrateStepwise(vN→vN+1→...→Current) → ValidateAgain → Rehydrate**.
- תמיכה לאחור: מומלץ לפחות N‑2 גרסאות; ב־MVP אפשר N‑1 עם התראה ברורה למשתמש ("שמירה מגרסה ישנה – ייתכן אובדן נתונים").

---

## מפת מיגרציה (טבלה B)

| גרסה | שינויי schema עיקריים | מדיניות מיגרציה | Fallback אם נכשל |
|------|------------------------|------------------|-------------------|
| **v1** | בסיס Tactical Save: Regions, Units/Hero, Economy(Money), Victory(GameResult + Respawn timers) | אין (baseline) | הצעת "Start New Project" + שמירת קובץ קרוס ל־diagnostics |
| **v2** | הוספת Day/Night (phase, elapsed, duration) | אם שדה חסר: ברירת מחדל Day, elapsed=0, duration=300. אם game logic תלוי: disable day/night עד שמירה מחדש | — |
| **v3** | הוספת Orders persist (OrderType + payload) | אם חסר: OrderType=None; יחידות נעמדות/ממשיכות AI ברירת מחדל. אם payload שבור: sanitize ל־None | — |
| **v4** | מעבר ל־Multi‑Squads (עתידי) | יצירת SquadId ברירת מחדל לפאקשן; שיוך כל היחידות לסקוואד 0. אם captain guid לא קיים: קפטן=null, Bypass morale shock זמנית | — |
| **v5** | Phase 1+ World Simulation pulse (Region simulation meta) | הוספת LastPulseTime + counters; אם חסר: להתחיל מחדש. אם mismatch קשה: reset simulation layer בלבד, Tactical intact | — |

---

## כללי יישום

1. **FCustomVersion / GUID-based version keys** (Unreal) – שימושי ל־migration עקבי.
2. **ValidateHeader** – בדיקת SaveSchemaVersion, CRC אם קיים; דחיית קובץ corrupt.
3. **MigrateStepwise** – פונקציות נפרדות v1→v2, v2→v3 וכו', כדי שלא לדלג גרסאות.
4. **אין דילוג גרסאות** – טעינת v1 תעבור v1→v2→v3 אם Current=3.

---

## קישורים

- `Docs/save_load_spec.md` – סכמה מלאה ו־Versioning.
- `Docs/existing_project_mode_spec.md` – חוויית "פרויקט קיים" ו־slots.
