# RTS PROJECT – Unreal Engine Critical Documentation Links

מסמך זה מרכז את מקורות הידע הרשמיים של Unreal Engine שעליהם Cursor חייב להתבסס במהלך הפיתוח.

**יש להעדיף תמיד תיעוד רשמי של Epic Games על פני מקורות קהילתיים.**

---

## Unreal C++ Programming

- **Programming Guide:** https://docs.unrealengine.com/5.3/en-US/programming-with-cplusplus-in-unreal-engine/
- **Gameplay Framework:** https://docs.unrealengine.com/5.3/en-US/gameplay-framework-in-unreal-engine/

---

## Actors & Components

- **Actors:** https://docs.unrealengine.com/5.3/en-US/actors-in-unreal-engine/
- **Actor Components:** https://docs.unrealengine.com/5.3/en-US/actor-components-in-unreal-engine/
- **GameMode & GameState:** https://docs.unrealengine.com/5.3/en-US/game-mode-and-game-state-in-unreal-engine/

---

## Subsystems (Critical for Managers)

- **Subsystems Overview:** https://docs.unrealengine.com/5.3/en-US/subsystems-in-unreal-engine/

---

## Artificial Intelligence (AI)

- **AI Overview:** https://docs.unrealengine.com/5.3/en-US/artificial-intelligence-in-unreal-engine/
- **Behavior Trees:** https://docs.unrealengine.com/5.3/en-US/behavior-trees-in-unreal-engine/
- **Environment Query System (EQS):** https://docs.unrealengine.com/5.3/en-US/environment-query-system-in-unreal-engine/

---

## Enhanced Input System

- **Enhanced Input:** https://docs.unrealengine.com/5.3/en-US/enhanced-input-in-unreal-engine/

---

## Gameplay Tags

- **Using Gameplay Tags:** https://docs.unrealengine.com/5.3/en-US/using-gameplay-tags-in-unreal-engine/

---

## Data Assets

- **Data Assets:** https://docs.unrealengine.com/5.3/en-US/data-assets-in-unreal-engine/

---

## Collision & Overlap (Command Radius)

- **Collision Overview:** https://docs.unrealengine.com/5.3/en-US/collision-in-unreal-engine/

---

## Performance & Optimization

- **Performance Guidelines:** https://docs.unrealengine.com/5.3/en-US/performance-and-optimization-in-unreal-engine/

---

## Networking (Future Expansion)

- **Multiplayer Overview:** https://docs.unrealengine.com/5.3/en-US/networking-and-multiplayer-in-unreal-engine/

---

## Automation & Testing (Critical for P0/P1/P2)

תיעוד רשמי של Epic למערכת האוטומציה – Unit Tests, Feature Tests, Smoke Tests, הרצה מהעורך ומשורת הפקודה.

- **Automation System (Overview):** https://dev.epicgames.com/documentation/en-us/unreal-engine/automation-system-in-unreal-engine/?application_version=5.4  
  סוגי טסטים (Unit, Feature, Smoke, Content Stress, Screenshot), Session Frontend, הרצה מ־Editor ו־CLI.
- **Automation Technical Guide:** https://dev.epicgames.com/documentation/en-us/unreal-engine/automation-technical-guide?application_version=5.4  
  `IMPLEMENT_SIMPLE_AUTOMATION_TEST`, `EAutomationTestFlags`, כתיבת טסטים ב־C++.
- **Automation System User Guide:** https://dev.epicgames.com/documentation/en-us/unreal-engine/automation-system-user-guide-in-unreal-engine?application_version=5.4  
  שימוש ב־Session Frontend, סינון והרצת טסטים.
- **Configure Automation Tests:** https://dev.epicgames.com/documentation/en-us/unreal-engine/configure-automation-tests-in-unreal-engine?application_version=5.4  
  `DefaultEngine.ini`: `[Automation]`, `Groups`, `ExcludeTest`; הרצה עם `-ExecCmds="Automation RunTest ..."`.

**בפרויקט:** `.cursor/docs/UNREAL_AUTOMATION_P0.md` – התאמה לטסטי P0 ו־`run_tests.ps1`.

---

## Cursor Implementation Directive

1. יש להעדיף ארכיטקטורה התואמת ל-Gameplay Framework.
2. אין לבצע Hardcoding של פאקשנים.
3. יש להשתמש ב-DataAssets לכל ערכי איזון.
4. יש להימנע מחישובים יקרים ב-Tick.
5. יש להפריד Core Systems (C++) מתוכן (Blueprint).
6. במידה ויש קונפליקט בין גישות – לבחור בגישה הרשמית המתועדת ע"י Epic.
