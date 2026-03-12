# מה נשאר לסיים ב־P0, P1, P2

**עדכון:** לפי הצ'קליסט, התכניות והקוד הנוכחי.

---

## P0 – Core Setup (Data, Characters, Squad, Region, Game Mode)

### כבר ממומש בקוד
- Data Registry (URTSDataRegistry), טיפוסי נתונים (FUnitRow, FHeroRow וכו')
- ARTSUnitCharacter, ARTSHeroCharacter, InitializeFromRegistry
- URTSSquadState, URTSSquadManagerSubsystem
- URTSMoraleComponent, URTSCommandAuthorityComponent
- ARTSRegionVolume (ControlLevel, IsPointInRegion, IsContested, SetControlLevelForFaction)
- RTSGameModeBase (PlayerController + CameraPawn), GlobalDefaultGameMode ב־DefaultEngine.ini

### מה נשאר (תוכן / הגדרות)
| משימה | איפה | הערה |
|--------|------|------|
| **DataTables ב־Content** | Content/Data/ | צור את הטבלאות: **DT_Units_MVP**, **DT_Heroes_MVP**, **DT_HeroXP_Prototype**, **DT_MoraleThresholds** (מבני FUnitRow, FHeroRow וכו'). ייבוא מ־CSV מ־`.cursor/data/RTS_Pack/` או הזנה ידנית. |
| **PIE בלי "table not found"** | בדיקה | אחרי יצירת הטבלאות – וודא ב־Output Log שאין אזהרות טעינה. |
| **Blueprint ליחידה ולגיבור** | Content | יורש מ־RTSUnitCharacter / RTSHeroCharacter עם **UnitId** / **HeroId** מוגדרים ו־Initialize From Registry. |
| (אופציונלי) BP_RTSGameMode | Content | הצ'קליסט מזכיר BP; כרגע עובד גם עם C++ RTSGameModeBase. |
| (אופציונלי) הסרת MyActor | Content/Source | אם MyActor לא בשימוש – למחוק או להשאיר. |

**סיכום P0:** בעיקר **תוכן** – DataTables ו־Blueprints של Unit/Hero. הקוד ליבה מוכן.

---

## P1 – פקודות וקלט (Selection, Orders, Input)

### כבר ממומש
- בחירה: LMB (SetSelection), Shift+LMB (AddToSelection), קליק על קרקע (ClearSelection)
- פקודות: RMB על קרקע → Move, RMB על אויב → Attack; טווח פקודה (CommandAuthority) נאכף
- קלט: WASD/חצים להזזת מצלמה (RTSCameraPawn), עכבר לבחירה ולפקודות
- GetHitUnderCursor, GetOrderIssuer (גיבור מועדף), פקודות מתמשכות (היחידה ממשיכה גם אחרי יציאה מטווח)

### מה נשאר
| משימה | איפה | הערה |
|--------|------|------|
| **הרצת טסטים** | Session Frontend / Automation | להריץ את טסטי P1 מ־RTS_TESTS_P0_P1.md (ידנית או Automation) ולסמן עבר/נכשל. |
| (אופציונלי) Captain טווח 1200 | כבר ב־P3 | אם רוצים Captain ב־P1 – מופיע כבר ב־P3. |

**סיכום P1:** הליבה **מלאה**. נשאר **אימות** (טסטים/בדיקות PIE).

---

## P2 – Region Control (0–5, Secure, Win/Lose)

### כבר ממומש
- **Epic 1:** IsPointInRegion, IsContested, SetControlLevelForFaction, RecalcDominantFaction, overlap למעקב גיבורים ב־Region
- **Epic 2:** התקדמות שליטה 0→4 (טיימר על Region – נוכחות לא contested מעלה רמה)
- **Epic 3:** Secure Region – URTSSecureRegionComponent, TryStartSecureRegion (15s channel, ביטול ביציאה/אויב נכנס), עלייה ל־5 בהצלחה
- **Epic 4:** URTSVictorySubsystem – NotifyControlReachedFive (ניצחון ב־5), NotifyHeroDeath + CheckLoseCondition (אין גיבור + אין Region ≥ 3 → הפסד), Destroyed() על Hero קורא ל־NotifyHeroDeath

### מה נשאר
| משימה | איפה | הערה |
|--------|------|------|
| **קלט ל־Secure Region** | RTSPlayerController / Input | אין עדיין **מיפוי קלט** שמפעיל את TryStartSecureRegion. צריך: מקש/החזקה + קליק על Region (או מקש ייעודי) שקורא ל־GetOrderIssuer()->TryStartSecureRegion(). |
| **סיום משחק (UI/לוג)** | VictorySubsystem / UI | אחרי Win/Lose – להציג תוצאה (מסך/לוג/הפסקת input). כרגע הלוגיקה קיימת; חסר חיווי למשתמש. |
| (אופציונלי) ויזואל/סאונד ל־Secure | Blueprint / FX | OnSecureRegionStarted / Cancelled / Completed – לחבר אפקטים. |
| (אופציונלי) תיעוד ב־Plan | PLANNING_P2 | לעדכן סימון משימות שהושלמו ב־PLANNING_P2_region_control.md. |

**סיכום P2:** הליבה **מלאה**. נשאר **קלט ל־Secure Region** + **חיווי Win/Lose** (UI/לוג/הפסקת משחק).

---

## סדר עבודה מומלץ

1. **P0:** ליצור DataTables ב־Content/Data ו־Blueprint Unit/Hero עם UnitId/HeroId – כך PIE יעבוד עם נתונים ו־Registry.
2. **P1:** להריץ טסטים/בדיקות PIE לפי RTS_TESTS_P0_P1.md ולסמן Done.
3. **P2:** להוסיף מיפוי קלט ל־Secure Region (למשל מקש + קליק על Region → TryStartSecureRegion), ולהוסיף חיווי Win/Lose (מסך/לוג/הפסקת input).

**קבצים רלוונטיים:**  
`RTS_CHECKLIST_EPIC_VS_MVP.md` | `DATA_TABLES_CHECKLIST.md` | `RTS_TESTS_P0_P1.md` | `PLANNING_P2_region_control.md`
