# תגובה לניתוח פערים וקונפליקטים

**מקור:** ניתוח פערים וקונפליקטים בתכנון ובמימוש מול המסמכים הקיימים למשחק (מסמך Word שסיפק המשתמש).  
**תאריך תגובה:** 2026-03-07

המסמך המקורי התבסס על **מסמך מצב פרויקט** בלבד (ללא גישה ל־Docs האחרים וללא קוד). להלן מה שבוצע בעקבות ההמלצות, ומה נותר כהמלצה לעתיד.

---

## 1. מה בוצע

### 1.1 אינדקס מסמכים ("המון מסמכי תכנון מוזכרים אך לא סופקו")

- **נוצר:** `Docs/DOCS_INDEX.md`  
- **תוכן:** רשימת כל המסמכים שמוזכרים בפרויקט – Docs/*, Docs/AI_Knowledge/*, .cursor/plans/*, .cursor/GDD.md – עם **נתיב** ו**תיאור קצר** ("מה זה מה"). מאפשר למי שלא קיבל את הקבצים לדעת איפה כל מסמך ומה תפקידו.

### 1.2 חוזה Win/Lose ו־Respawn (מכונת מצבים)

- **נוצר:** `Docs/GAMERESULT_AND_RESPAWN_CONTRACT.md`  
- **תוכן:** Single source of truth לסדר אירועים: HeroDeath → (אין Region ≥ 3 → Lose) או (יש Region ≥ 3 → RespawnPending → RitualAvailable → Channeling → Spawn → Alive). מתאר את המימוש הנוכחי ב־`NotifyHeroDeath` ומכונת המצבים (Alive / Dead / RespawnPending / RitualAvailable / ChannelingRitual / Lost).  
- **מצב קוד:** הלוגיקה **כבר ממומשת** כך ב־VictorySubsystem (קודם בודקים HasRegionWithControlAtLeast3; רק אם אין – קוראים CheckLoseCondition). המסמך מקבע את החוזה ומאפשר אוטומציה עתידית.

### 1.3 RegionId יציב (לשמירה/טעינה)

- **בוצע:** הוספת שדה **RegionId** (FName) ל־`ARTSRegionVolume`.  
- **שימוש:** מזהה יציב לאזור למפה ולתשתית Save/Load עתידית (כפי שהניתוח המליץ). ניתן לעריכה ב־Editor (EditAnywhere).  
- **API:** `GetRegionId()`, `SetRegionId(InRegionId)`.

### 1.4 עדכון P6 – דחייה

- **עודכן:** במסמך מצב הפרויקט (`Docs/PROJECT_STATUS.md`) ו־`Docs/DOCS_INDEX.md`: **P6 (Day/Night + polish) דחוי**.  
- **הערה:** תוכנית P6 קיימת (`.cursor/plans/PLANNING_P6_day_night_polish.md`); המימוש נדחה. DayNightSubsystem קיים כקובץ אך לא נדרש ל־MVP נוכחי.

### 1.5 תגובה למסמך "ניתוח פערים ותכנון מפורט ל־Save_Load, QA וביצועים"

בעקבות המסמך הנוסף (מצב "פרויקט קיים") בוצעו תיקונים מהפערים **הקריטיים** ו**עדיפות גבוהה** שאפשר ליישם כעת:

| פער | חומרה | מה בוצע |
|-----|--------|---------|
| אין Spec מלא ל־Save/Load | קריטית | **נוצרו** `Docs/save_load_spec.md` (ארכיטקטורה, סכמה, Atomicity, Rehydration, Channel State) ו־`Docs/save_migration_policy.md` (גרסאות ומיגרציה v1–v5). |
| אין Spec ל"פרויקט קיים" | קריטית | **נוצר** `Docs/existing_project_mode_spec.md` (זרימות UI, naming, slots, חוזה MVP ומקרי קצה). |
| IDs יציבים: UnitGuid + RegionId | קריטית | **RegionId** כבר קיים ב־RegionVolume. **נוסף** `PersistentUnitGuid` (FGuid) ב־`ARTSUnitCharacter` (כולל גיבור) עם `GetPersistentUnitGuid()`/`SetPersistentUnitGuid()`, ו־`PersistentNpcGuid` ב־`ARTSHumanNPC` עם getter/setter. נוצרים אוטומטית ב־BeginPlay אם invalid. |
| אי־עקביות P6 / תיעוד | גבוהה | כבר טופל: P6 מסומן דחוי ב־PROJECT_STATUS ו־DOCS_INDEX. |
| Channel State (Secure/Ritual) mid‑save | גבוהה | **מתועד** ב־save_load_spec (Channel State Model) ו־existing_project_mode_spec (חוזה התנהגות). מימוש בפועל יבוא עם Vertical Slice Save/Load. |
| אינדקס מסמכים + Save/Load | גבוהה | **עודכן** `Docs/DOCS_INDEX.md`: סעיף 5 הוחלף ברשימת Save/Load ומצב "פרויקט קיים" עם קישורים לשלושת המסמכים החדשים. |

**לא בוצע (דורש מימוש):** URTSSaveSubsystem, Vertical Slice Save/Load, A/B slots, Migration pipeline, QA Harness ומטריצת הבדיקות, Orders persist (v3). אלה מפורטים במסמך הניתוח כרשימת משימות עם הערכת זמן.

---

## 2. מה לא בוצע (המלצות לעתיד)

הבאים נותרו כהמלצות מהניתוח; ביצועם תלוי בעדיפויות הפרויקט.

| נושא | המלצת הניתוח | סטטוס |
|------|----------------|--------|
| **Spec מצב "פרויקט קיים"** | מסמך 2–4 עמודים: הגדרה, מה נשמר, מסכים/זרימות, מקרי קצה | **בוצע.** נוצר `Docs/existing_project_mode_spec.md` (יחד עם save_load_spec ו־save_migration_policy). |
| **תשתית SaveGame מינימלית** | Vertical slice: שמירה/טעינה של ControlLevels, Money, Hero state, Squad member IDs | לא ממומש. |
| **אוטומציה ל־Victory/Respawn/RegionControl** | סט בדיקות שחוזר רגרסיות | קיים RTSP0AutomationTests; לא הורחב ל־P2–P5. |
| **יישור כלכלה Data↔Runtime** | תמיכה מלאה ב־RecruitCostResource + Upkeep drain, או צמצום שדות לא נתמכים | RecruitCostResource קיים ב־FUnitRow; הוצאה בפועל (דם/בשר) לא ממומשת. Upkeep לא נגרע ב־Runtime. תיעוד ב־UNIT_BALANCE_IMPORT. |
| **CommandRadius Data-driven** | שדה ב־HeroRow/קונפיג + UI | **בוצע.** שדה `CommandRadius` ב־`FHeroRow` (ברירת מחדל 0 = 2500 מהקומפוננטה). ב־`InitializeFromHeroRow` מופעל override אם > 0. |
| **החלטה מוצרית "סקוואד אחד"** | החלטה כתובה + API שמאפשר הרחבה | **בוצע.** נוצר `Docs/ONE_SQUAD_PER_FACTION_DECISION.md`. |
| **UI למשאבים/עלויות** | Widget/HUD: עלות גיוס, חוסר משאבים | לא נוסף. |
| **פירוט תנאי עליית שליטה (Region)** | Spec: מה מפעיל טיימר (נוכחות גיבור, לא contested), tie-breakers | **בוצע.** נוצר `Docs/REGION_CONTROL_GAIN_SPEC.md`. |
| **Fallback ל־SpawnHeroInBestRegion** | אם אין Region (או contested) – fallback כדי למנוע סופט־לוק | **בוצע.** אם `SpawnHeroInBestRegion` מחזיר nullptr, המצב חוזר ל־RitualAvailable (הגיבור לא "נצרך"), מוצגת הודעה, והשחקן יכול לנסות שוב. |

---

## 3. סיכום

- **בוצע:** אינדקס מסמכים (DOCS_INDEX), חוזה GameResult/Respawn (GAMERESULT_AND_RESPAWN_CONTRACT), RegionId ב־RegionVolume, עדכון שדחינו את P6; מסמכי Save/Load ו"פרויקט קיים"; PersistentUnitGuid/PersistentNpcGuid; Fallback ל־SpawnHeroInBestRegion (החזרה ל־RitualAvailable); CommandRadius מ־FHeroRow; החלטה "סקוואד אחד" (ONE_SQUAD_PER_FACTION_DECISION); Spec עליית שליטה (REGION_CONTROL_GAIN_SPEC).  
- **לא בוצע:** Vertical Slice Save/Load, הרחבת אוטומציה ל־P2–P5, יישור מלא כלכלה (RecruitCostResource/Upkeep runtime), UI משאבים.  
- המסמכים שצוינו בניתוח כ"לא סופקו" (GDD, CORE_GAME_MECHANICS, campaign_layer_spec, economy_system, AI_Knowledge, וכו') **קיימים בפרויקט** ומופיעים ב־`Docs/DOCS_INDEX.md` עם נתיב ותיאור.
