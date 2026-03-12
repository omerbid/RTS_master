# RTS_Monsters – טסטים נדרשים ל־P0 ו־P1

**מטרה:** מסמך טסטים מלא לאימות P0 (Core Setup) ו־P1 (Command Radius + Orders + Input).  
**שימוש:** הרצה ידנית ב־PIE ו/או אוטומציה עתידית (Unreal Automation).

---

# חלק א׳ – טסטי P0 (Core Setup)

**תלות:** אין. פרויקט נבנה, DataTables ב־Content/Data/.  
**Definition of Done (P0):** C++ core (Hero, Unit, SquadState, Region, Morale, CommandAuthority) קיים, מתקמפל, data-driven, מוכן ל־P1.

---

## P0.1 Data Backbone & Registry

| # | תיאור | צעדים | תוצאה מצופה | ✓ |
|---|--------|--------|-------------|---|
| P0.1.1 | DataTables נטענים ללא שגיאות | הפעל PIE; בדוק Output Log | אין "table not found" או "Units table not found" / "Heroes table not found" | |
| P0.1.2 | Registry מחזיר שורות לפי ID | ב־Blueprint או Console: קרא GetUnitRow(UnitId), GetHeroRow(HeroId) | ערכים חוזרים תואמים ל־CSV/DataTable; אין crash | |
| P0.1.3 | שורה חסרה מחזירה false | קרא GetUnitRow עם UnitId שלא קיים בטבלה | פונקציה מחזירה false; OutRow לא מתמלא/לא משמש | |
| P0.1.4 | GetUnitRowForHero עובד | קרא GetUnitRowForHero עם FHeroRow תקין | FUnitRow חוזר (לפי פאקשן הגיבור או fallback) | |

---

## P0.2 Core Characters (Unit / Hero)

| # | תיאור | צעדים | תוצאה מצופה | ✓ |
|---|--------|--------|-------------|---|
| P0.2.1 | Spawn Unit – אתחול מ־Registry | שים BP יחידה במפה עם UnitId מוגדר; הפעל PIE; ב־Begin Play קורא InitializeFromRegistry | HP, Damage, MoveSpeed, MoraleBase, PowerScale (מ־FUnitRow) מאותחלים; אין אזהרות | |
| P0.2.2 | Spawn Hero – אתחול מ־Registry | שים BP גיבור עם HeroId; הפעל PIE; InitializeFromRegistry | סטטים מ־FHeroRow + בסיס מ־FUnitRow; CommandAuthorityComponent קיים ופעיל | |
| P0.2.3 | Unit ללא UnitId / Registry חסר | Spawn יחידה בלי UnitId או עם UnitId לא קיים; הפעל PIE | InitializeFromRegistry מחזיר false; אין crash; לוג/התנהגות צפויה | |
| P0.2.4 | FactionId ו־UnitId גלויים ב־Blueprint | פתח BP של Unit/Hero | FactionId, UnitId ניתנים לעריכה/קריאה; CachedUnitData (או מקביל) גלוי לאחר אתחול | |

---

## P0.3 MoraleComponent

| # | תיאור | צעדים | תוצאה מצופה | ✓ |
|---|--------|--------|-------------|---|
| P0.3.1 | אתחול מורל ב־BeginPlay | Spawn יחידה עם InitializeFromRegistry | MoraleComponent.CurrentMorale = BaseMorale (או ערך מ־FUnitRow); בטווח [MinMorale, MaxMorale] | |
| P0.3.2 | ApplyMoraleDelta | קרא ApplyMoraleDelta(10) ואז ApplyMoraleDelta(-5) | ערך מתעדכן clamped ל־[MinMorale, MaxMorale]; פונקציה מחזירה ערך חדש | |
| P0.3.3 | SetMorale | קרא SetMorale(150) עם MaxMorale=100 | ערך clamped ל־100; אין overflow | |
| P0.3.4 | יחידה ללא MoraleComponent | Spawn יחידה בלי הקומפוננטה (אם אפשרי) או השבת | התנהגות מוגדרת (אזהרה או fallback); אין crash | |

---

## P0.4 CommandAuthorityComponent

| # | תיאור | צעדים | תוצאה מצופה | ✓ |
|---|--------|--------|-------------|---|
| P0.4.1 | טווח ברירת מחדל לגיבור | בדוק CommandRadius על Hero אחרי spawn | 2500 UU (או ערך מ־GDD) | |
| P0.4.2 | CanIssueOrderToUnit – יחידה בטווח | גיבור ויחידה באותו פאקשן, מרחק < CommandRadius | CanIssueOrderToUnit(Unit) == true | |
| P0.4.3 | CanIssueOrderToUnit – יחידה מחוץ לטווח | גיבור ויחידה באותו פאקשן, מרחק > CommandRadius | CanIssueOrderToUnit(Unit) == false | |
| P0.4.4 | CanIssueOrderToUnit – פאקשן שונה | גיבור פאקשן A, יחידה פאקשן B (בטווח) | CanIssueOrderToUnit(Unit) == false | |
| P0.4.5 | bAuthorityEnabled = false | השבת Authority על הגיבור | CanIssueOrderToUnit(Unit) == false גם אם בטווח | |

---

## P0.5 SquadState (URTSSquadState)

| # | תיאור | צעדים | תוצאה מצופה | ✓ |
|---|--------|--------|-------------|---|
| P0.5.1 | יצירת Squad והצטרפות יחידות | צור Squad; קרא AddMember ל־2–3 יחידות עם מורל שונה | Members מכיל את היחידות; RecalcMorale() מחזיר ממוצע סביר | |
| P0.5.2 | RecalcMorale אחרי שינוי מורל | שנה מורל של יחידה אחת (ApplyMoraleDelta); קרא RecalcMorale על ה־Squad | AverageMorale (או שדה מקביל) מתעדכן | |
| P0.5.3 | RemoveMember | הסר יחידה מה־Squad; קרא RecalcMorale | היחידה לא ב־Members; ממוצע מורל מתעדכן | |
| P0.5.4 | Squad ללא members | RecalcMorale על Squad ריק | אין crash; ערך ברירת מחדל (למשל 0 או BaseMorale) | |

---

## P0.6 Region (ARTSRegionVolume)

| # | תיאור | צעדים | תוצאה מצופה | ✓ |
|---|--------|--------|-------------|---|
| P0.6.1 | הצבת Region במפה | גרור RTS Region Volume למפה; הפעל PIE | אין crash; Volume נראה/ניתן לבחירה | |
| P0.6.2 | GetControlLevelForFaction | הגדר ControlLevel לפאקשן (0–5); קרא GetControlLevelForFaction | ערך תואם (0–5) | |
| P0.6.3 | GetDominantFaction | הגדר שליטה כךאקשן אחד דומיננטי; קרא GetDominantFaction | FactionId הנכון חוזר | |
| P0.6.4 | Stability / Population | קרא/הגדר Stability, Population (אם exposed) | ערכים נשמרים; אין שגיאות | |

---

## P0.7 Game Mode & Map

| # | תיאור | צעדים | תוצאה מצופה | ✓ |
|---|--------|--------|-------------|---|
| P0.7.1 | GlobalDefaultGameMode | הפעל PIE ממפת ברירת מחדל | BP_RTSGameMode נטען; אין "failed to find GameMode" | |
| P0.7.2 | GameState | במהלך PIE בדוק ש־GameState מסוג BP_RTSGameState (או מקביל) | GameState קיים; מנהל סקוואדים זמין אם הוגדר | |
| P0.7.3 | תיקיית Content/Data/ ונתיבים | וודא DataTables ב־Content/Data/ עם שמות: DT_Units_MVP, DT_Heroes_MVP, DT_HeroXP_Prototype, DT_MoraleThresholds | Registry מוצא טבלאות; P0.1.1 עובר | |

---

# חלק ב׳ – טסטי P1 (Command Radius + Orders + Input)

**תלות:** P0 הושלם.  
**Definition of Done (P1):** טווח פקודה נאכף לפקודות חדשות; פקודת Move מתמשכת; בחירה והנחת פקודות בעכבר; מקלדת רק להזזת מצלמה.

---

## P1.1 Selection (בחירת יחידות)

| # | תיאור | צעדים | תוצאה מצופה | ✓ |
|---|--------|--------|-------------|---|
| P1.1.1 | LMB על יחידה – בחירה בודדת | לחץ שמאלי על יחידה | Selection מתעדכן ליחידה הזו בלבד (SetSelection); אין יחידות אחרות נבחרות | |
| P1.1.2 | Shift+LMB – הוספה לבחירה | בחר יחידה; Shift+לחיצה על יחידה שנייה | שתי היחידות נבחרות (AddToSelection); Selection מכיל את שתיהן | |
| P1.1.3 | LMB על קרקע – ביטול/ריקון | בחר יחידה; לחץ על קרקע (לא על actor) | Selection מתנקה או מתאפס (לפי GDD – אפשר ClearSelection) | |
| P1.1.4 | בחירת גיבור | LMB על גיבור | הגיבור נבחר; ניתן להנפיק פקודות כ־Issuer | |

---

## P1.2 Orders – Move (פקודת תנועה)

| # | תיאור | צעדים | תוצאה מצופה | ✓ |
|---|--------|--------|-------------|---|
| P1.2.1 | RMB על קרקע – Move ליחידות בטווח | בחר גיבור + יחידות; וודא כולן בטווח 2500 UU; RMB על קרקע | כל היחידות **בטווח** מקבלות פקודת Move ל־hit location; מתחילות לנוע | |
| P1.2.2 | יחידות מחוץ לטווח לא מקבלות פקודה חדשה | בחר גיבור + יחידה בטווח + יחידה מחוץ לטווח; RMB על קרקע | רק היחידה בטווח מקבלת Move; היחידה הרחוקה **לא** מקבלת פקודה חדשה (שומרת על פקודה קיימת אם יש) | |
| P1.2.3 | פקודה מתמשכת – יציאה מטווח | תן Move ליחידה; הזז/המתן עד שהיחידה יוצאת מטווח הגיבור | היחידה **ממשיכה** לבצע את פקודת התנועה (לא מתבטלת אוטומטית) | |
| P1.2.4 | פקודה חדשה רק עם Issuer בטווח | עם יחידה מחוץ לטווח, נסה לתת פקודה חדשה (RMB) | היחידה לא מקבלת את הפקודה; אם נכנסת שוב לטווח ואז ניתנת פקודה – אז מקבלת | |
| P1.2.5 | Issuer = גיבור נבחר | בחר רק גיבור; RMB על קרקע | אין יחידות נבחרות מלבד הגיבור; אין crash; אופציונלי: הגיבור עצמו מקבל Move (אם ממומש) | |

---

## P1.3 Orders – Attack (אופציונלי ב־P1)

| # | תיאור | צעדים | תוצאה מצופה | ✓ |
|---|--------|--------|-------------|---|
| P1.3.1 | RMB על אויב – Attack | בחר גיבור + יחידות בטווח; RMB על actor אויב | יחידות בטווח מקבלות פקודת Attack (target); מתקרבות/תוקפות לפי מימוש | |
| P1.3.2 | Attack לא ליחידות מחוץ לטווח | בחר יחידות שחלקן מחוץ לטווח; RMB על אויב | רק יחידות בטווח מקבלות Attack | |

---

## P1.4 Input (קלט)

| # | תיאור | צעדים | תוצאה מצופה | ✓ |
|---|--------|--------|-------------|---|
| P1.4.1 | WASD / חצים – רק הזזת מצלמה | השתמש ב־WASD או בחצים | המצלמה נעה (pan); **אין** שליטה על יחידות (לא תנועה, לא בחירה) | |
| P1.4.2 | LMB / RMB ממופים | בדוק Input mapping (Enhanced Input או Legacy) | LMB = Select; Shift+LMB = Add Select; RMB = Context (Move/Attack) | |
| P1.4.3 | Trace תקף – קרקע vs actor | RMB על קרקע vs RMB על יחידה/אויב | קרקע → Move; actor ידיד → select/add; actor אויב → Attack (אם scope) | |

---

## P1.5 Authority & Persistence (סיכום התנהגות)

| # | תיאור | צעדים | תוצאה מצופה | ✓ |
|---|--------|--------|-------------|---|
| P1.5.1 | Hero CommandRadius = 2500 | בדוק בעורך או ב־runtime | ערך 2500 UU (או מקור מ־data) | |
| P1.5.2 | Captain 1200 (אם ממומש ב־P1) | אם Captain קיים ב־P1: בדוק טווח | 1200 UU; Issuer Captain מפקד רק על יחידות ב־1200 | |
| P1.5.3 | אותה פאקשן – תנאי להנפקת פקודה | יחידהאקשן שונה (בטווח) | CanIssueOrderToUnit false; לא מקבלת פקודה ב־RMB | |

---

# סיכום סימון

- **P0:** סמן כל טסט שעבר (✓) עד שכל P0.1–P0.7 מסומנים.
- **P1:** סמן כל טסט שעבר עד שכל P1.1–P1.5 מסומנים.

**קבצים קשורים:**  
`PLANNING_P0_core_setup.md` | `PLANNING_P1_command_and_orders.md` | `RTS_CHECKLIST_EPIC_VS_MVP.md` | `GDD.md`
