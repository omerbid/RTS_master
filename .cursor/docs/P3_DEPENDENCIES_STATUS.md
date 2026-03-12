# סטטוס תלויות P3 – מה סגור ומה לא

**עודכן:** לאחר תיקון כל הפערים (P3).

---

## טבלת סטטוס

| רכיב | סטטוס | פרטים |
|------|--------|--------|
| **URTSCommandAuthorityComponent** | ✅ סגור | טווח Hero 2500, Captain 1200, CanIssueOrderToUnit – ממומש. |
| **ARTSHeroCharacter** | ✅ סגור | CommandAuthorityComponent (יורש מ־Unit), IssueOrderToUnitsInRange. |
| **URTSMoraleComponent** | ✅ סגור | CurrentMorale, ApplyMoraleDelta, SetMorale. **השפעות סף:** מיושמות ב־Unit (UpdateMoraleEffects כל 1s): מורל &lt;30 → MaxWalkSpeed × 0.85; GetOrderResponsivenessMultiplier() → 0.85 ב־OrderComponent; מורל &lt;15 → 20% כל 5s Auto-Retreat. |
| **URTSSquadState** | ✅ סגור | AddMember/RemoveMember + סנכרון SquadReference; CaptainUnit; ApplyMoraleDeltaToAll(Delta); SetCaptain(Unit); RecalcMorale. |
| **ARTSUnitCharacter** | ✅ סגור | SquadReference, SetSquad; Destroyed() → קפטן -10, RemoveMember, נפגעים -5; טיימרים: UpdateMoraleEffects (1s), UpdateDetachedAndDrain (5s); bIsDetached, דילוג -1/5s כשמנותק; Auto-Retreat כשמורל &lt;15; דחיית Attack כשמנותק. |
| **מנהל סקוואדים** | ✅ סגור | **URTSSquadManagerSubsystem** (GameInstanceSubsystem): GetOrCreateSquadForFaction, AddUnitToFactionSquad; Unit ב־BeginPlay נרשם לסקוואד לפי FactionId. |
| **Captain** | ✅ סגור | bIsCaptain, Rank ≥3, TryPromoteToCaptain (יוצר CommandAuthority 1200); SetCaptain על הסקוואד; מוות קפטן → ApplyMoraleDeltaToAll(-10). קלט "Promote" – לקשור ב־Blueprint/Input ל־TryPromoteToCaptain. |

---

## סיכום שינויים שבוצעו

1. **URTSSquadState:** CaptainUnit, SetCaptain, ApplyMoraleDeltaToAll; AddMember מגדיר Captain אם Unit->bIsCaptain; RemoveMember מנקה Captain.
2. **ARTSUnitCharacter:** Destroyed – קפטן -10, RemoveMember, נפגעים -5; טיימר 1s ל־UpdateMoraleEffects (מהירות לפי מורל); טיימר 5s ל־UpdateDetachedAndDrain (חישוב bIsDetached, דילוג -1, 20% auto-retreat); GetOrderResponsivenessMultiplier; דחיית SetCurrentOrder(Attack) כשמנותק; רישום ל־RTSSquadManagerSubsystem ב־BeginPlay; TryPromoteToCaptain מעדכן Squad->SetCaptain.
3. **URTSOrderComponent:** TickMove מכפיל ב־GetOrderResponsivenessMultiplier().
4. **RTSSquadManagerSubsystem:** subsystem חדש – יצירת סקוואד לפי פאקשן, AddUnitToFactionSquad.

---

## מה נשאר אופציונלי (לא חובה ל־P3)

- **קלט Promote to Captain:** TryPromoteToCaptain קיים וניתן לקריאה מ־Blueprint; לחבר מקש/פעולה ב־Input או בתפריט הקשר.
- **בונוס מורל מגיבור:** (Hero presence) לא ממומש – אופציונלי ב־GDD.
- **נתוני קנס מורל:** כרגע קבועים (נפגעים -5, קפטן -10) – אפשר להעביר ל־DataTable בהמשך.

---

**Definition of Done (P3):** כל התלויות הרשומות למעלה ממומשות; סקוואד, מורל, מנותק וקפטן פעילים.

---

# דוח מצב – סיום תיקון פערי P3

## בוצע

| # | פער | תיקון |
|---|-----|--------|
| 1 | השפעות מורל (סף) | Unit: UpdateMoraleEffects כל 1s – MaxWalkSpeed × 0.85 כש־CurrentMorale &lt; 30; GetOrderResponsivenessMultiplier() ו־OrderComponent מכפיל תנועה; טיימר 5s – 20% auto-retreat כש־מורל &lt; 15 (נסיגה 400 UU אחורה). |
| 2 | Captain + קנס בסקוואד | URTSSquadState: CaptainUnit, SetCaptain, ApplyMoraleDeltaToAll. Unit: Destroyed() – אם קפטן ApplyMoraleDeltaToAll(-10), RemoveMember, אחר כך ApplyMoraleDeltaToAll(-5) (נפגעים). TryPromoteToCaptain קורא SetCaptain על הסקוואד. |
| 3 | מנהל סקוואדים | RTSSquadManagerSubsystem (GameInstanceSubsystem): GetOrCreateSquadForFaction, AddUnitToFactionSquad. Unit ב־BeginPlay קורא AddUnitToFactionSquad(this). |
| 4 | מנותק (detached) | Unit: טיימר 5s – חישוב bIsDetached (אין גיבור 2500 / קפטן 1200 באותה פאקשן); דילוג מורל -1/5s; SetCurrentOrder(Attack) נדחה כשמנותק. |
| 5 | Captain | כבר היה: Rank, bIsCaptain, TryPromoteToCaptain (הוספת CommandAuthority 1200). חובר: SetCaptain בסקוואד, קנס מורל במוות. קלט Promote – להגדיר ב־Blueprint/Input. |

## קבצים שנוגעו

- `RTSSquadState.h/cpp` – CaptainUnit, SetCaptain, ApplyMoraleDeltaToAll; עדכון AddMember/RemoveMember.
- `RTSUnitCharacter.h/cpp` – Destroyed, טיימרים, UpdateMoraleEffects, UpdateDetachedAndDrain, TickLowMoraleAutoRetreat, GetOrderResponsivenessMultiplier, דחיית Attack כשמנותק, רישום ל־SquadManager, TryPromoteToCaptain → SetCaptain.
- `RTSOrderComponent.cpp` – כפל ב־GetOrderResponsivenessMultiplier ב־TickMove.
- `RTSSquadManagerSubsystem.h/cpp` – **חדש** – מנהל סקוואדים לפי פאקשן.

## בנייה

- UHT עבר (7 written). אם מופיעה "Live Coding" – לסגור עורך או Ctrl+Alt+F11 ואז לבנות שוב.

## בדיקות מומלצות ב־PIE

1. יחידות נרשמות לסקוואד (SquadReference לא null אחרי BeginPlay).
2. מורל &lt; 30 – מהירות תנועה יורדת.
3. מורל &lt; 15 – אחרי זמן סביר (עד ~25s) יחידה נסוגה (20% כל 5s).
4. יחידה ללא גיבור/קפטן בטווח – bIsDetached true, מורל יורד כל 5s.
5. מוות יחידה – שאר הסקוואד מקבלים -5 מורל; מוות קפטן – לפני ההסרה -10 לסקוואד.
6. Promote to Captain (מ־Blueprint) – Rank ≥3, יחידה מקבלת CommandAuthority 1200 ומוגדרת כ־Captain בסקוואד.
