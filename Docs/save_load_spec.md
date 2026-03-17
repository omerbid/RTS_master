# Save/Load Spec – RTS Monsters ("פרויקט קיים")

**מטרה:** Spec מלא ל־Save/Load עבור מצב "פרויקט קיים": ארכיטקטורה, סכמה, Atomicity/Consistency, Rehydration APIs. מסמך זה משלים את ניתוח הפערים ("ניתוח פערים ותכנון מפורט ל־Save_Load, QA וביצועים") ומשמש בסיס ליישום ו־QA.

**מקור אמת:** `Docs/GAMERESULT_AND_RESPAWN_CONTRACT.md` (Win/Lose/Respawn), `Docs/PROJECT_STATUS.md` (מערכות קיימות), מסמך הניתוח המפורט.

---

## 1. ארכיטקטורה – שתי שכבות שמירה

- **Project Meta (Index):** רשימת פרויקטים/slots, שמות, timestamps, "last played", flags (migration required). נשמר ב־`URTSProjectIndexSaveGame` / Index slot.
- **Project State (Tactical/Run):** מצב המשחק: Regions, Units/Heroes, Orders, Victory/Respawn timers, Economy, Day/Night, Channels (Secure/Ritual). נשמר ב־`URTSProjectSaveGame` לכל ProjectId.

**שער יחיד:** `URTSSaveSubsystem` (GameInstanceSubsystem) – Single entry point:
- `RequestManualSave(ProjectId)`, `RequestAutoSave(ProjectId, Reason)`, `RequestLoad(ProjectId, DesiredCheckpoint)`
- `ListProjects()`, `CreateProject()`, `DeleteProject()`

**גבול ברור:** State שמחושב מחדש (derived) vs State שחובה לשמר (persistent). רק ה־persistent נכתב ל־SaveGame.

---

## 2. מזהים יציבים (חובה ל־Rehydration)

- **RegionId** (FName) – ב־`ARTSRegionVolume`. קובע matching ל־Region בשמירה/טעינה. **ממומש.**
- **UnitGuid / PersistentId** (FGuid) – לכל יחידה/גיבור/NPC. משמש ל־AttackTarget, Captain, Squad membership, Channel (Hero). **נדרש ב־Unit/Hero/NPC.**

בלי IDs יציבים אי אפשר לפתור references לאחר Load.

---

## 3. Atomicity ו־Consistency

- **Consistency snapshot:** כל ה־Subsystems וה־Actors נלכדים באותו "רגע לוגי" (אותו frame boundary). Freeze קצר או Save Lock שמונע mutating בזמן Gather. **Combat:** בזמן SaveLock לא להריץ Resolve step (או לאסוף Snapshot רק בין Resolve intervals) כדי ש־HP ב־Combat ו־Units תואמים; ראה `Docs/COMBAT_CONTRACT.md`.
- **No UObject access off‑thread:** Snapshot נאסף על Game Thread ל־pure structs; כתיבה/דחיסה ברקע מותרת רק על הנתונים הסריאליזביליים.
- **A/B slots:** שמירה לסירוגין ל־`ProjectId_A` ו־`ProjectId_B`; Header עם Timestamp + Version + CRC/Checksum. ב־Load בוחרים את העדכני התקין.
- **Invariant validation לפני commit:** אם snapshot שובר invariant (למשל Unit עם AttackTargetGuid לא קיים), policy מוגדרת: fail save / auto‑repair / sanitize (למשל OrderType=None).

---

## 4. סכמת SaveGame (סיכום – פרטים ב־טבלה A במסמך הניתוח)

- **Header/Meta:** SaveSchemaVersion, ProjectId, ProjectDisplayName, MapName, CreatedUtc, LastSavedUtc, BuildId, Checksum/CRC.
- **Regions:** `Regions[]` – RegionId, ControlByFaction, Population, Stability, וכו'.
- **Units:** `Units[]` – UnitGuid, UnitId, FactionId, bIsHero, Transform, HP, Morale, Rank, bIsCaptain, CurrentOrderType, MoveDestination, AttackTargetGuid (אופציונלי v3).
- **Hero (per Faction):** HeroStateByFaction – HeroId, bIsAlive, HeroGuidIfAlive.
- **VictorySubsystem:** GameResult, WinnerFaction, RespawnByFaction (state, timer remaining, ritual remaining, StoredHeroId).
- **EconomySubsystem:** MoneyByFaction, EconomyTickAccumulator.
- **SquadManager:** SquadsByFaction (MemberGuids, CaptainGuid).
- **DayNightSubsystem (אופציונלי v2):** bIsNight, PhaseDurationSeconds, ElapsedInPhase.
- **Channels (Secure/Ritual):** SecureChannelStateByHero – TargetRegionId, RemainingSeconds; Ritual state ב־RespawnByFaction.

---

## 5. Channel State Model (mid‑channel save/load)

כדי למנוע soft locks ו־desync:
- **Secure:** נשמרים TargetRegionId (FName), RemainingSeconds (float). Load משחזר טיימר או מאפס לפי חוזה (מפורש ב־existing_project_mode_spec).
- **Ritual:** נשמרים ב־RespawnByFaction: RespawnState, RitualChannelRemaining, StoredHeroId. Load משחזר channel או מאפס עקבי.

---

## 6. Rehydration APIs – חוזה

**לכל Subsystem מרכזי:**
- `GatherSaveData(OUT Struct)`
- `ApplySaveData(const Struct&)`
- `PostLoadFixup()` – בניית derived state, recalcs, timers, caches

**לכל Actor שמוקם מחדש:**
- `GetPersistentId()` / `SetPersistentId()` (או UnitGuid)
- `ToSaveStruct()` / `FromSaveStruct()`
- `ResolveReferences(Resolver)` – שלב שני: פתרון AttackTargetGuid, CaptainGuid וכו' אחרי שכל ה־Actors נוצרו.

**כלל זהב:** Load בשני שלבים: (1) Spawn/Assign + transforms + שדות בסיס; (2) Resolve cross‑refs.

---

## 7. Versioning ו־Migration

- כל SaveGame כולל **SaveSchemaVersion** (int) ב־Header.
- שינוי "שובר שחזור" מגדיל גרסה.
- Pipeline טעינה: LoadRaw → ValidateHeader → אם ישן: MigrateStepwise(vN→vN+1→...) → ValidateAgain → Rehydrate.
- תמיכה לאחור: לפחות N‑2 גרסאות (MVP: N‑1 עם התראה למשתמש).
- מפת מיגרציות מפורטת: `Docs/save_migration_policy.md`.

---

## 8. קישורים

- `Docs/save_migration_policy.md` – גרסאות ומיגרציה.
- `Docs/existing_project_mode_spec.md` – חוויית תפריט, slots, חוזה מוצרי.
- `Docs/GAMERESULT_AND_RESPAWN_CONTRACT.md` – state machine ל־Victory/Respawn.
- `Docs/DOCS_INDEX.md` – אינדקס מסמכים כולל Save/Load.
