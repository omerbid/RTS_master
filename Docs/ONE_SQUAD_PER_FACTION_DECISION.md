# החלטה מוצרית: סקוואד אחד לכל פאקשן (MVP)

**מטרה:** להקבע בכתב את ההחלטה שהמשחק פועל ב־MVP עם **סקוואד אחד לכל פאקשן**, ו־API שמאפשר הרחבה עתידית ל־Multi‑Squads.

**מקור:** ניתוח פערים (GAP_ANALYSIS), PROJECT_STATUS.

---

## החלטה

- **MVP:** לכל פאקשן (Humans, Vampires, Werewolves) יש **סקוואד אחד**.
- כל היחידות והגיבורים של הפאקשן משויכים לאותו סקוואד (למשל דרך `URTSSquadManagerSubsystem::AddUnitToFactionSquad`).
- קפטן ומורל (P3) פועלים במסגרת הסקוואד הזה; טווח סמכות (Command Radius) של הגיבור/קפטן חל על יחידות באותו סקוואד.

---

## מימוש נוכחי

- `URTSSquadManagerSubsystem` – מחזיק סקוואד אחד לכל `EFactionId` (מבנה נתונים פנימי: סקוואד־לכל־אקשן).
- `URTSSquadState` – מייצג סקוואד; יחידות מקושרות דרך `SquadReference` / `AddMember`/`RemoveMember`.
- אין כרגע "בחירת סקוואד" או "מספר סקוואדים לפאקשן" – זה עקבי עם ההחלטה.

---

## הרחבה עתידית

- אם יוחלט על Multi‑Squads (למשל סקוואד A/B לפאקשן), יש להרחיב:
  - מזהה סקוואד (SquadId) ב־Save/Load (v4 במפת המיגרציה ב־save_migration_policy).
  - API ליצירת סקוואד נוסף, העברת יחידות בין סקוואדים, וקביעת "סקוואד פעיל" לשחקן.
- הקוד הנוכחי לא חוסם הרחבה כזו; יש להגדיר חוזה (שמות, DTs) כשמתחילים את Phase הרלוונטי.

---

## קישורים

- `Docs/PROJECT_STATUS.md` – סטטוס מערכות כולל Squad.
- `Docs/save_migration_policy.md` – v4 Multi‑Squads.
- `Docs/GAP_ANALYSIS_RESPONSE.md` – טבלת פערים והחלטה זו.
