# P0 / P1 – הפניה מהירה

## P0 – טבלאות שהקוד מזהה

ה־**RTSDataRegistry** טוען ארבע טבלאות בנתיבים **קבועים**. אם הטבלאות לא בנתיבים האלה, תופיע ב־Output Log אזהרה "table not found".

| טבלה | נתיב מלא (מהקוד) |
|------|-------------------|
| Units | `/Game/RTS/Data/Data_Tables/DT_Units_MVP.DT_Units_MVP` |
| Heroes | `/Game/RTS/Data/Data_Tables/DT_Heroes_MVP.DT_Heroes_MVP` |
| Hero XP | `/Game/RTS/Data/Data_Tables/DT_HeroXP_Prototype.DT_HeroXP_Prototype` |
| Morale Thresholds | `/Game/RTS/Data/Data_Tables/DT_MoraleThresholds.DT_MoraleThresholds` |

כלומר ב־Content: תיקייה **RTS → Data → Data_Tables**, ובתוכה האססטים עם השמות האלה. אם המבנה שונה (למשל רק Content/Data/) – צריך לעדכן את הנתיבים ב־`RTSDataRegistry.cpp`.

---

## P0 – גיבור והדאטה בייס

הגיבור משתמש ב־**HeroId** (שם שורה ב־DT_Heroes_MVP):

- **BeginPlay:** אם `HeroId != NAME_None` קורא ל־`InitializeFromRegistry()`.
- **InitializeFromRegistry:** קורא `GetHeroRow(HeroId)` → מקבל FHeroRow (DisplayName, Faction, BaseHP, BaseDamage, MoveSpeed, PowerScale וכו'), ואז `GetUnitRowForHero` → מקבל FUnitRow סינתטי (HP, Damage, MoveSpeed, MoraleBase וכו').
- **InitializeFromHeroRow:** ממלא `CachedHeroData`, ומאתחל את הבסיס דרך `InitializeFromUnitRow` (סטטים של יחידה).

**פער אפשרי:** אם ב־Blueprint של הגיבור **Hero Id** לא מוגדר (ריק), הוא לא יאתחל מ־Registry. וודא שב־Details ל־Blueprint הגיבור יש **Hero Id** = שם שורה שקיימת ב־DT_Heroes_MVP (למשל `HighCommander`, `BloodLord`).

---

## P0 – שינוי דאטה ליחידה (לא תקוע על מיליציה)

- **Unit Id** – עכשיו **BlueprintReadWrite**. אפשר להגדיר **שונה לכל instance** במפה (בחר את ה־Unit במפה → Details → RTS | Data → **Unit Id** = Militia / Swordsman / וכו'). כל יחידה במפה יכולה להיות טיפוס אחר.
- **דריסות (Overrides):** אם אתה רוצה ש**יחידה אחת** תישאר "מיליציה" מהטבלה אבל עם ערכים אחרים, השתמש ב־**Override Move Speed**, **Override HP**, **Override Damage** (תחת RTS | Data). אם ערך > 0, הוא **דורס** את מה שהגיע מהטבלה אחרי האתחול.
- **Cached Unit Data** – עכשיו **BlueprintReadWrite**: אפשר לקרוא/לכתוב מ־Blueprint אחרי אתחול (למשל באירוע אחרי Begin Play). באדיטור הוא עדיין Visible (לא Edit) כדי לא לבלבל עם הטבלה; הדריסה נעשית עם Override או מ־Blueprint.

---

## P1 – איך להפעיל Attack

**Attack ממומש:** לחיצה ימנית (RMB) על **אויב** (Actor עם **FactionId שונה** מהגיבור הנבחר) נותנת פקודת Attack.

**שימוש:**
1. בחר יחידות (כולל גיבור, כדי שיהיה Issuer).
2. **לחיצה ימנית על יחידה/גיבור של פאקשן אחר** (לא על הקרקע).
3. היחידות **בטווח** הגיבור יקבלו פקודת Attack ויתקרבו ליעד. כשהן בטווח התקפה – נשארות (כרגע נזק הוא stub; התנהגות PostCombat כשהמטרה מתה).

**אם לא קורה כלום:** וודא ש־**האובייקט עליו לוחצים** הוא `ARTSUnitCharacter` (או גיבור) **עם FactionId שונה** מהגיבור שלך (למשל אתה Humans, הוא Vampires). אם ה־trace לא פוגע ב־Unit (למשל Collision מוסתר) – לא יינתן Attack. הודעת מסך כתומה **"[RTS] Attack order issued"** תופיע כשפקודת Attack ניתנת.

---

**P2:** הוספתי הדפסה במשחק כשגיבור נכנס לאזור: הודעת מסך בציאן **"[Region] Hero entered region: ... (Faction: ...)"** ל־4 שניות.
