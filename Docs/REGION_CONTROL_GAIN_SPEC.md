# Spec עליית שליטה (Region Control Gain)

**מטרה:** פירוט תנאי עליית שליטה באזור – מה מפעיל טיימר, מתי אין עלייה, ו־tie-breakers. מיישר תיעוד עם המימוש ב־`ARTSRegionVolume`.

**מקור:** GDD, PLANNING_P2, ניתוח פערים (GAP_ANALYSIS).

---

## 1. תנאים לעליית שליטה (פסיבית)

- **טיימר:** `ControlGainIntervalSeconds` (ברירת מחדל 30s). כל X שניות נקראת `EvaluateControlGain()`.
- **תנאי לעלייה:** עבור פאקשן נתון, **רק אם**:
  1. **יש גיבור של הפאקשן באזור** – `HasHeroOfFaction(Faction)` (גיבור בתוך ה־RegionBounds).
  2. **האזור לא contested** – `IsContested()` = false.  
     Contested = לפחות **שתי** פאקשנים עם גיבור באזור. במצב כזה **אף פאקשן** לא עולה.
  3. **רמת השליטה הנוכחית של הפאקשן** נמוכה מ־`ControlGainMaxLevel` (ברירת מחדל 4).

- **פעולה:** העלאת רמת השליטה של הפאקשן ב־1 (עד מקסימום ControlGainMaxLevel). רמה 5 **לא** מושגת בדרך זו – רק דרך **Secure Region** (פעולת גיבור).

---

## 2. Contested ו־Tie-breakers

- **Contested:** אם שני גיבורים (אקשנים שונים) או יותר נמצאים באזור – האזור contested, ו־`EvaluateControlGain()` יוצא מיד בלי לעדכן אף פאקשן.
- **אין tie-breaker בין פאקשנים** בעלייה פסיבית: כל פאקשן שמקיים "גיבור באזור + לא contested + רמה < max" עולה באותו tick. בפועל ב־tick נתון רק פאקשן אחד יכול לקיים את זה כי אם יש גיבור של פאקשן אחר – contested.

---

## 3. פרמטרים (Editor / Level)

| שדה | תיאור | ברירת מחדל |
|-----|--------|-------------|
| `ControlGainIntervalSeconds` | מרווח בין בדיקות (שניות) | 30 |
| `ControlGainMaxLevel` | מקסימום רמה שהעלייה הפסיבית מגיעה אליה (0–5) | 4 |

רמה 5 (Dominance) מושגת **רק** כאשר גיבור משלים פעולת Secure Region (טיימר 15s) – מפורט ב־PLANNING_P2 ו־RTSSecureRegionComponent.

---

## 4. קוד רלוונטי

- `ARTSRegionVolume::EvaluateControlGain()` – לוגיקת העלייה.
- `HasHeroOfFaction`, `IsContested` – תנאים.
- `SetControlLevelForFaction` – עדכון + קריאה ל־`NotifyControlReachedFive` כשמגיעים ל־5.

---

## קישורים

- `Docs/GAMERESULT_AND_RESPAWN_CONTRACT.md` – Win/Lose.
- `.cursor/plans/PLANNING_P2_region_control.md` – Region control ו־Secure.
- `Docs/GAP_ANALYSIS_RESPONSE.md` – פערים ותיקונים.
