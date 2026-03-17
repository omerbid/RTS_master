# חוזה GameResult ו־Respawn (Single Source of Truth)

**מטרה:** להגדיר סדר אירועים חד־משמעי בין תנאי הפסד (P2) לבין Respawn Ritual (P5), כדי למנוע סתירות ("טקס זמין אבל המשחק כבר הוכרע") וסופט־לוקים.

**מימוש נוכחי:** `URTSVictorySubsystem::NotifyHeroDeath` – הלוגיקה below היא **כפי שממומש בקוד**.

---

## 1. סדר אירועים במות גיבור

```
HeroDestroyed (Hero->Destroyed() → NotifyHeroDeath(Hero))
    │
    ▼
┌─────────────────────────────────────────────────────────┐
│ יש לאחת הסיעות (Faction) אזור עם ControlLevel ≥ 3?     │
└─────────────────────────────────────────────────────────┘
    │
    ├── לא  ──► CheckLoseCondition(Faction)  ──► אם אין גיבור חי + אין Region ≥ 3  ──► Lose (GameResult = Lost)
    │
    └── כן   ──► לא קוראים ל־CheckLoseCondition
                 │
                 ├── ApplyMoraleShockToSquadsNear (הלם מורל לסקד באותו faction בטווח)
                 └── StartRespawnTimer(Faction, HeroId)  ──► 90s
                              │
                              ▼ (אחרי 90s)
                 RespawnState = RitualAvailable  ──► OnRespawnRitualAvailable.Broadcast(Faction)
                              │
                              ▼ (שחקן לוחץ B)
                 StartRitualChannel(Faction)  ──► 10s channel
                              │
                              ├── ביטול (אם יוחלט בעתיד)  ──► חזרה ל־RitualAvailable
                              └── השלמה  ──► SpawnHeroInBestRegion(Faction, HeroId)  ──► RespawnState = None
```

**כלל:** Lose נקבע **רק** כאשר קוראים ל־`CheckLoseCondition` – וזה קורה **רק** כאשר במות הגיבור **אין** לאף אזור ControlLevel ≥ 3 עבור אותה סיעה. אם יש Region ≥ 3, נכנסים לזרימת Respawn ולא מפסידים.

---

## 2. מכונת מצבים (לפי Faction)

| מצב | משמעות |
|-----|--------|
| **Alive** | יש גיבור חי לסיעה. |
| **Dead (מיד אחרי HeroDestroyed)** | אין גיבור חי. אם אין Region ≥ 3 → **Lost**. אם יש Region ≥ 3 → **RespawnPending**. |
| **RespawnPending (WaitingForTimer)** | טיימר 90s רץ. המשחק לא הוכרע. |
| **RitualAvailable** | הטיימר הסתיים; השחקן יכול ללחוץ B (Perform Ritual). |
| **ChannelingRitual** | ערוץ 10s רץ. בהשלמה → Spawn; אז **Respawned** → בחזרה ל־Alive. |
| **Lost** | GameResult = Lost; המשחק הסתיים להפסד. |

**החלטות עיצוביות (מנוסחות):**

- ב־RespawnPending/RitualAvailable/ChannelingRitual **אין** גיבור חי – לכן פקודות שדורשות "גיבור נבחר" (למשל Secure Region, Recruit) לא זמינות עד שה־Hero spawn מחדש. זה מכוון (GDD: Hero-centric).
- אם SpawnHeroInBestRegion נכשל (אין Region עם control ≥ 3 – לא אמור לקרות כי נכנסנו ל־Respawn רק כשיש Region ≥ 3): במימוש נוכחי יש בדיקה; אם אין BestRegion לא spawn ולא קורסים. **המלצה:** להוסיף fallback (למשל הודעה + החזרה ל־RitualAvailable) אם יידרש.

---

## 3. איפה נאכף בקוד

| שלב | קובץ/פונקציה |
|-----|---------------|
| Hero death | `ARTSHeroCharacter::Destroyed()` → `URTSVictorySubsystem::NotifyHeroDeath(this)` |
| בחירה Lose vs Respawn | `URTSVictorySubsystem::NotifyHeroDeath` – `HasRegionWithControlAtLeast3(World, Faction)`; אם false → `CheckLoseCondition`; אם true → `ApplyMoraleShockToSquadsNear` + `StartRespawnTimer` |
| Lose | `CheckLoseCondition` – בודק "גיבור חי?" ו־"Region ≥ 3?"; אם שניהם לא → GameResult = Lost, OnGameLost.Broadcast |
| טיימר 90s | `StartRespawnTimer` → OnRespawnTimerExpired → RespawnState = RitualAvailable, OnRespawnRitualAvailable |
| ערוץ 10s | `StartRitualChannel` → OnRitualChannelComplete → `SpawnHeroInBestRegion` |

---

## 4. בדיקות מומלצות (אוטומציה)

- Hero מת, אין Region ≥ 3 → Lose.
- Hero מת, יש Region ≥ 3 → לא Lose; אחרי 90s CanPerformRitual == true.
- אחרי Perform Ritual והשלמת ערוץ → גיבור חדש ב־World, אותו HeroId; RespawnState = None.

מסמך זה הוא ה־**Single Source of Truth** ללוגיקת GameResult/Respawn/WinLose בהתאם לניתוח הפערים.
