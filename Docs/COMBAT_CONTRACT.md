# חוזה Combat מרכזי (Single Source of Truth)

**מטרה:** להגדיר מקור אמת אחד לנזק (Damage), מחזור חיים של קרב (Engagement), ואינטגרציה עם Orders/Morale/Save-Load. מסמך זה משלים את "דוח פערים ותכנון מעמיק... מערכת Combat מרכזית".

**מקור קוד:** `RTSCombatManagerSubsystem`, `RTSCombatResolverLibrary`, `RTSCombatTypes.h`.

---

## 1. Single Damage Authority

- **מקור נזק יחיד:** כל שינוי HP כתוצאה מקרב עובר **רק** דרך `URTSCombatResolverLibrary::ResolveCombatRound` → `ApplyDamageToGroup`.
- **יחידה כ־Sink:** ה־Unit לא מחשב נזק בעצמו; `ApplyDamageToGroup` מעדכן את ה־Snapshot ואז מסנכרן ל־Unit דרך `Unit->OverrideHP = NewHP`. אם HP ≤ 0 קוראים `Unit->Destroy()`.
- **אין נתיב נזק כפול:** אין קריאה נפרדת ל־`Unit::TakeDamage` או `ApplyDamage` ממקור אחר במסגרת Combat. UI ושאר מערכות קוראים ל־OverrideHP / CachedUnitData לצורך תצוגה.
- **משמעות ל־Save/Load:** בשחזור מספיק לשמור/לטעון `OverrideHP` (או CurrentHP בשכבת Save); אין "שני מקורות אמת" שיכולים להתנגש אחרי Load.

---

## 2. מחזור חיים של Engagement (קרב)

- **התחלה:** `StartCombat(SideAUnits, SideBUnits, CombatCenter)` – בונה Groups מ־BuildGroupFromUnits (כולל Day/Night multipliers), רושם יחידות, broadcast `OnCombatStarted`.
- **תוך כדי:** `UpdateCombat` כל Tick; כל `ResolveInterval` (למשל 0.2–0.5s) קורא `ResolveCombatRound` (חישוב DPS מאוגד, `ApplyDamageToGroup`, סנכרון ל־Units).
- **סיום:** כאשר `LivingCount == 0` בצד אחד או Routed – קוראים `EndCombat` → `UnregisterUnitsFromCombat`, `Combat.bFinished = true`, broadcast `OnCombatEnded`.
- **Events:** `OnCombatStarted`, `OnCombatRoundResolved`, `OnCombatEnded` – מאפשרים ל־PostCombatBehavior, Morale, UI להתחבר בלי לסקור את כל ה־Combat בעצמם.

---

## 3. אינטגרציה עם Orders / Morale

- **Orders:** Attack order מוביל להזנת יחידות ל־Combat (מי שקורא ל־StartCombat – למשל OrderManager או לוגיקת קרב). Combat Manager לא יוזם Attack בעצמו; הוא רק מפתר קרב שכבר הוגדר.
- **Morale / Detached:** מכפילי מורל (למשל OrderResponsiveness, דמג') נכללים ב־BuildGroupFromUnits אם ממומשים שם; Detached לא מקבל Attack **חדש** (ב־Orders). אם אחרי Load יחידה במצב Detached עם Attack order שמור – ב־PostLoadFixup יש לסניטזר: Attack → None/Hold/Retreat לפי חוזה (מתועד ב־save_load_spec).
- **Day/Night:** מוחל ב־BuildGroupFromUnits (GetDayNightDamageMultiplier) על ה־Snapshot – מקור יחיד למכפיל בתוך Combat.

---

## 4. מדיניות Save/Load ל־Combat

- **המלצה:** לא לשחזר "מצב קרב פעיל" כ־struct נשמר; אחרי Load לבנות מחדש engagements מה־Orders והמרחקים (Recompute).
- **אלטרנטיבה:** אם בעתיד יישמר CombatId + SideA/SideB Guids, יש להבטיח ש־Rehydrate יוצר מחדש את ה־Groups ורק אז ממשיך Resolve – כדי למנוע אי־התאמה בין HP ב־Save ל־state נוכחי.
- **Snapshot Save / Combat freeze:** בזמן Gather Snapshot (שמירה) – **חוזה:** לא להפעיל Resolve step של Combat באותו frame; כלומר Snapshot נאסף על **גבול עקבי** (לאחר Resolve או לפני). כך HP ב־Combat ו־OverrideHP ב־Units תואמים. אלטרנטיבה: "Combat freeze" – השהאת UpdateCombat בזמן SaveLock (אם URTSSaveSubsystem מחזיק Lock עד סיום Gather). ממומש כשמימוש Save/Load יוגדר.

---

## 5. רשימת בדיקה (Checklist)

| נושא | סטטוס |
|------|--------|
| Single Damage Authority (רק CombatResolver → OverrideHP) | ממומש |
| OnCombatStarted / OnCombatEnded | ממומש |
| BuildGroupFromUnits כולל Day/Night | ממומש |
| Target selection / Preferences data-driven | שדות ב־FUnitRow (PreferredTargetRoles, TargetPriorityBias, FocusFireFactor) – מוכן; שימוש ב־scoring בעתיד |
| PostCombatBehavior מחובר ל־OnCombatEnded | תלוי מימוש Orders |
| Save/Load: Recompute engagements vs persist | מתועד – Recompute מומלץ |
| Detached + Attack ב־PostLoadFixup | מתועד ב־save_load_spec |

---

## קישורים

- `Docs/save_load_spec.md` – Rehydration, PostLoadFixup.
- `Docs/GAMERESULT_AND_RESPAWN_CONTRACT.md` – Win/Lose/Respawn.
- `Docs/REGION_CONTROL_GAIN_SPEC.md` – Region control.
