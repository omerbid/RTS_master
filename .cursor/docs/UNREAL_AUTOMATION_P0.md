# טסט אוטומטי ראשון ב-Unreal (P0)

מבוסס על **תיעוד Epic הרשמי** – Automation System (UE 5.4). לינקים: `.cursor/docs/UNREAL_LINKS.md` → סעיף "Automation & Testing".

---

## מה נוסף לפרויקט

1. **מודול:** ב־`RTS_Monsters.Build.cs` נוספה תלות ב־`AutomationTest` (רק ב־Editor build). אם הקומפילציה נכשלת עם "AutomationTest not found", נסה להסיר את הבלוק עם `AutomationTest` – לפעמים ה־Engine כבר מספק את ההגדרות.
2. **קובץ טסטים:** `Source/RTS_Monsters/RTSP0AutomationTests.cpp` – שני **Smoke Tests** (מהירים, מתאימים ל־SmokeFilter של Epic):
   - **CommandRadius default** – בודק ש־`URTSCommandAuthorityComponent` CDO עם `CommandRadius == 2500` (P0.4.1).
   - **EFactionId** – בודק שיש 3 ערכים ב־enum (GDD: Humans, Vampires, Werewolves).

---

## איך להריץ מהעורך (Editor) – לפי Epic

1. פתח את הפרויקט ב־Unreal Editor.
2. **Session Frontend:** תפריט **Tools → Session Frontend** (תיעוד Epic: Automation tab נמצא ב־Session Frontend).
3. בלשונית **Automation** לחץ **Refresh Tests** (עדכון רשימת הטסטים).
4. מצא את הקטגוריה **RTS_Monsters.P0** (או סינון לפי שם).
5. סמן את הטסטים (או את **RTS_Monsters.P0** לסימון כולם) ולחץ **Start Tests** / **Run Tests**.

התוצאות יופיעו באותה חלונית. סוגי הטסטים הרשמיים (Epic): Unit, Feature, **Smoke** (מהירים), Content Stress, Screenshot – הטסטים שלנו מסומנים כ־SmokeFilter.

---

## איך להריץ משורת הפקודה (Command Line) – לפי Epic

לפי התיעוד: הרצת טסטים עם `-ExecCmds="Automation RunTest ..."` (או `RunTests` בגרסאות מסוימות). אפשר להגדיר **Groups** ב־`DefaultEngine.ini` תחת `[Automation]` ולקרוא לקבוצה.

1. **נתיב לעורך:** למשל  
   `C:\Program Files\Epic Games\UE_5.4\Engine\Binaries\Win64\UnrealEditor.exe`
2. **הרצה (דוגמה):**

   ```batch
   "C:\Program Files\Epic Games\UE_5.4\Engine\Binaries\Win64\UnrealEditor.exe" ^
     "C:\path\to\RTS_Monsters\RTS_Monsters.uproject" ^
     -ExecCmds="Automation RunTest RTS_Monsters.P0" ^
     -unattended -nopause -nosplash -NullRHI
   ```

   - `-NullRHI` – ללא GPU (מתאים לשרת/CI).
   - להחליף את נתיב ה־`.uproject` בנתיב המלא של הפרויקט.
   - אם הפקודה היא `RunTests` (עם s) בגרסת ה־Engine שלך – השתמש בה.

3. **הגדרת Group ב־DefaultEngine.ini (אופציונלי):**  
   תחת `[Automation]` אפשר להוסיף למשל:
   ```ini
   +Groups=(Name="RTS_P0", Filters=((Contains="RTS_Monsters.P0")))
   ```
   ואז להריץ: `-ExecCmds="Automation RunTest Group:RTS_P0"`.

4. הפלט מופיע ב־log (למשל ב־`Saved/Logs/`).

---

## הוספת טסטים נוספים (P0 ו-P1/P2)

1. **באותו קובץ** – להוסיף בלוקים חדשים של:

   ```cpp
   IMPLEMENT_SIMPLE_AUTOMATION_TEST(
       FRTSP0_YourTestName,
       RTS_MONSTERS_P0_TEST_CATEGORY ".Group.TestName",
       EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::SmokeFilter
   )
   bool FRTSP0_YourTestName::RunTest(const FString& Parameters)
   {
       // TestTrue, TestEqual, TestNotNull וכו' (Automation Technical Guide)
       return true;
   }
   ```

   **דגלי Epic (EAutomationTestFlags):** `ApplicationContextMask` = זמין ב־Editor; `SmokeFilter` = טסט מהיר (עד כ־1 שנייה). פרטים: תיעוד Epic → Automation Technical Guide.

2. **טסטים שצריכים עולם (מפה):** צריך **Latent** / **Complex** automation (טעינת מפה עם `FLoadGameMapLatentCommand` ואז assertions). מתועד ב־Automation Technical Guide – אפשר להוסיף בהמשך.

3. **P1 / P2:** ליצור קבצים דומים, למשל `RTSP1AutomationTests.cpp` עם קטגוריה `RTS_Monsters.P1`, ו־`RTSP2AutomationTests.cpp` עם `RTS_Monsters.P2`.

---

## קישור ל־test_results.yaml

אחרי הרצת Automation, לעדכן ידנית (או בסקריפט) את  
`.cursor/tests/test_results.yaml`  
למשל:

```yaml
results:
  P0.4.1: pass   # CommandRadius – עבר ב-Automation
  P0.1.1: pass  # נבדק ידנית ב-PIE
```

סקריפט `run_tests.ps1` יציג את הסטטוס ואת הסיכום (pass/fail/skip) לפי הקובץ הזה.
