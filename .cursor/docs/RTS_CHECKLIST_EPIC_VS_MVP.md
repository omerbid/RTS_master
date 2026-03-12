# RTS_Monsters – צ'קליסט: MVP מול Epic

**מטרה:** צ'קליסט להורדה/הדפסה – MVP (מינימום למוצר שמיש) מול Epic (פיצ'רים גדולים Post-MVP).

---

# חלק א׳ – MVP (Minimum Viable Product)

## 1. Data Backbone & Registry

- [ ] תיקייה `Content/Data/` קיימת
- [ ] DataTable **DT_Units_MVP** (FUnitRow) ב־Content/Data/
- [ ] DataTable **DT_Heroes_MVP** (FHeroRow) ב־Content/Data/
- [ ] DataTable **DT_HeroXP_Prototype** (FHeroXPRow) ב־Content/Data/
- [ ] DataTable **DT_MoraleThresholds** (FMoraleThresholdRow) ב־Content/Data/
- [ ] ייבוא CSV (או הזנה ידנית) לכל טבלה
- [ ] PIE ללא אזהרות "table not found" ב־Output Log
- [ ] Unit/Hero מתאתחל מ־InitializeFromRegistry בהצלחה

## 2. Core Characters & Components

- [ ] ARTSUnitCharacter – סטטים מ־FUnitRow (HP, Damage, MoveSpeed, MoraleBase, PowerScale)
- [ ] ARTSHeroCharacter – סטטים מ־FHeroRow, CommandAuthority
- [ ] UMoraleComponent על Unit/Hero – CurrentMorale, ApplyMoraleDelta
- [ ] UCommandAuthorityComponent על Hero – טווח 2500, CanIssueOrderToUnit
- [ ] Blueprint ליחידה ולגיבור (יורש מ־C++) עם UnitId/HeroId מוגדרים

## 3. Squad & Region (Skeleton)

- [ ] URTSSquadState – רשימת Members, RecalcMorale
- [ ] מנהל/בעלים לסקוואדים (GameState או Subsystem)
- [ ] ARTSRegionVolume – ControlLevel לפי פאקשן, Stability, Population, DominantFaction
- [ ] אפשרות להציב Region ב־מפה ולקרוא GetControlLevelForFaction

## 4. Game Mode & Map

- [ ] BP_RTSGameMode מוגדר כ־GlobalDefaultGameMode
- [ ] BP_RTSGameState קיים
- [ ] מפת משחק ברירת מחדל תקינה (DefaultEngine + DefaultEditor מתואמים)
- [ ] Config: ProjectName = RTS Monsters (או שם תצוגה)

## 5. פקודות וקלט (P1)

- [ ] Input: בחירת יחידות (לחיצה / גרירה)
- [ ] Input: פקודת תנועה (לחיצה על קרקע)
- [ ] Input: פקודת התקפה (לחיצה על אויב)
- [ ] IssueOrderToUnitsInRange ממומש (או proxy ל־move/attack)
- [ ] CommandAuthority בודק טווח לפני מתן פקודה

## 6. MVP Scope (GDD)

- [ ] 3 פאקשנים: Humans, Vampires, Werewolves
- [ ] קרב RTS בסיסי (יחידות נלחמות, נזק, מורל)
- [ ] XP גיבור בסיסי (לפחות רמות 1–25, טבלת XP)
- [ ] שליטה אזורית בסיסית (Region עם רמות 0–5)
- [ ] מורל בסיסי (השפעה על נזק/בריחה, 90+ Buff)

## 7. ניקוי ותיעוד

- [ ] MyActor הוסר (אם לא בשימוש)
- [ ] תיעוד: Squad = URTSSquadState (לא ARTSQuadActor) מעודכן ב־GDD/ארכיטקטורה

---

# חלק ב׳ – Epic (Post-MVP / אפיקים גדולים)

## E1. מערכות משחק מתקדמות

- [ ] Weather – השפעה על קרב/מורל
- [ ] PTSD / נפשי – השפעה על יחידות אחרי קרבות
- [ ] Mythic – שדרוגים/יכולות מיתיות
- [ ] Multi-region מתקדם – מסע בין אזורים, מסכי טעינה או מפה רציפה מורכבת
- [ ] Multi-hero scaling – מספר גיבורים, איזון כוח

## E2. אנשי זאב (Werewolves) ייחודי

- [ ] מדד טורף-נטרף (Predator/Prey) – נתונים ב־DataTable/DataAsset
- [ ] Hunted → נטייה לבריחה
- [ ] Cornered → Berserk
- [ ] אינטגרציה עם FPredatorPreyStateRow / לוגיקת משחק

## E3. מבנים וכלכלה

- [ ] התפתחות לפי שליטה/אוכלוסיה/יציבות (לא איסוף משאבים קלאסי)
- [ ] מבנים נותנים בונוסים קרביים (מורל, התקפה וכו')
- [ ] שדרוג נעול כאשר שליטה/יציבות לא מספיקים (מרמה 3)
- [ ] BP_SettlementBase / DA_Settlement* מחוברים ל־Region וללוגיקת שליטה

## E4. AI

- [ ] Defensive AI ליחידות מנותקות (ללא גיבור/קפטן בטווח)
- [ ] Behavior Trees / AI Controller ליחידות
- [ ] דילוג מורל ליחידות מנותקות

## E5. UI/UX

- [ ] HUD: מורל, בריאות, טווח פקודה
- [ ] UI בחירת יחידות/גיבורים
- [ ] מיפוי אזורים (שליטה, יציבות) במפה/מיני-מפה
- [ ] תפריטים ומעברי מסכים

## E6. תוכן ועיצוב

- [ ] לפחות גיבור אחד לכל פאקשן (MVP: מינימום 3)
- [ ] יכולות גיבור בשלבים (L1, L6, L11, L16, L21)
- [ ] איזון PowerScale: אדם=1, ערפד=2, איש זאב=3
- [ ] ESpecialTag ממומשים (ShieldWall, Charge, FearAura וכו') לפי GDD

## E7. איכות ופריסה

- [ ] בדיקות PIE לכל מערכת ליבה
- [ ] Package / Build לדמו
- [ ] תיעוד משתמש / מדריך התקנה (אם רלוונטי)

---

# סיכום שימוש

- **MVP:** סמן את כל הסעיפים בחלק א׳ כדי להגדיר "גרסה 1 שמישה" (מפה אחת, 3 פאקשנים, גיבור, יחידות, מורל, אזורים, פקודות בסיס).
- **Epic:** השתמש בחלק ב׳ לתכנון ספרינטים/אפיקים אחרי MVP – כל אפיק (E1–E7) יכול להיות sprint נפרד או משימות במקביל.

**קבצים קשורים:**  
`.cursor/docs/DATA_TABLES_CHECKLIST.md` | `.cursor/docs/RTS_ARCHITECTURE.md` | `.cursor/plans/PLANNING_P0_core_setup.md`
