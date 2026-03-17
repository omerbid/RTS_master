# Spec מצב "פרויקט קיים" – חוויית תפריט וחוזה מוצרי

**מטרה:** הגדרה פורמלית של מצב "פרויקט קיים": זרימות UI, naming, slots policy, ומקרי קצה. MVP ברור בהיעדר Spec רשמי קודם.

**קשור:** `Docs/save_load_spec.md`, `Docs/save_migration_policy.md`, מסמך הניתוח המפורט.

---

## 1. זרימות UI

### תפריט ראשי
- **New Project** → יוצר ProjectId חדש + Save v1 ראשוני (או "ריק" עם MapName + CreatedUtc).
- **Existing Project** → מציג **Project List** מתוך IndexSave: שם פרויקט, LastSaved, Build, דגל "Needs Migration" אם רלוונטי.
- **Continue** (או בחירת פריט ברשימה) → טוען את ה־slot התקין האחרון (A או B) של הפרויקט הנבחר.

### בתוך משחק
- **Manual Save** – כפתור/מקש. שמירה סינכרונית עם אינדיקציה (למשל "Saving...").
- **Autosave** – באירועים מוגדרים (לא על כל שינוי כסף/מורל):
  - אחרי Secure מוצלח,
  - אחרי התחלת RespawnTimer,
  - אופציונלי: לפני מסך Win/Lose.
  - תדירות כללית מומלצת: אחת ל־3–5 דקות (או רק באירועים).

---

## 2. Naming ו־Slots

- **ProjectId:** מזהה יציב (FGuid או FString) – לא שם תצוגה. לא משתנה לאורך חיי הפרויקט.
- **ProjectDisplayName:** שם תצוגה בתפריט (עריכתי על ידי משתמש אם רלוונטי).
- **Slots:** לכל ProjectId שני slots: `ProjectId_A`, `ProjectId_B`. שמירה לסירוגין; Load בוחר את העדכני התקין (לפי Timestamp + CRC). אם אחד corrupt – נסיון Slot השני.

---

## 3. חוזה מוצרי מינימלי (MVP)

| תרחיש | התנהגות צפויה |
|--------|-----------------|
| Load אחרי Save "Idle" | מצב זהה: Regions, Units, Economy, Victory. |
| Save בזמן Move order | אחרי Load – היחידה ממשיכה Move או reset orders (חוזה מפורש בפרויקט). |
| Save עם Attack order | AttackTargetGuid נפתר ב־ResolveReferences; אם היעד לא קיים – order מנוקה (OrderType=None). |
| Save באמצע Secure (למשל t=7/15) | אחרי Load – הערוץ ממשיך עם Remaining≈8s **או** מתאפס (חוזה עקבי – לא soft lock). |
| Load כשבטווח Secure נכנס אויב | לפי כללי ביטול – הערוץ מתבטל מיד. |
| Save כשהגיבור מת ו־RespawnTimer רץ | אחרי Load – RespawnTimerRemaining משוחזר (לא restart ל־90). |
| Save באמצע RitualChannel | Channel ממשיך או מתאפס לפי חוזה; לא נתקע. |
| Save לפני Win / אחרי Win | GameResult נשמר; Win לא מוכפל/נעלם; חסימת input/UI עקבית. |
| Version mismatch (v1 נטען ב־v3) | Migration רץ; invariants נשמרים. |
| Corrupt save (CRC fail) | בחירת Slot B או הודעת שגיאה + recovery (הצעת "Start New Project"). |

---

## 4. מקרי קצה מתועדים

- **נתונים חלקיים (שדות חסרים):** Load מצליח עם defaults; אין crash.
- **Double save race:** Save lock מונע corruption; קריאה שנייה נדחית או מתוזמנת.
- **Orders persist:** חוזה – האם Load משחזר CurrentOrder לכל יחידה; אם כן – איך משחזרים AttackTarget בלי pointer (via UnitGuid + ResolveReferences).

---

## 5. קישורים

- `Docs/save_load_spec.md` – ארכיטקטורה, סכמה, Rehydration.
- `Docs/save_migration_policy.md` – גרסאות ומיגרציה.
- `Docs/GAMERESULT_AND_RESPAWN_CONTRACT.md` – Win/Lose/Respawn state machine.
- `Docs/DOCS_INDEX.md` – אינדקס מסמכים.
