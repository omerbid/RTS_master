# אינדקס מסמכים – RTS Monsters

**מטרה:** ריכוז כל המסמכים שמוזכרים בפרויקט (במצב פרויקט, בתוכניות, בניתוח פערים) עם נתיב ותיאור קצר. מסמכים אלה **קיימים בריפו** אך לא תמיד סופקו להקשר חיצוני – זה הקובץ שמאגד ומסביר "מה זה מה".

**מקור:** נוצר בעקבות "ניתוח פערים וקונפליקטים בתכנון ובמימוש מול המסמכים הקיימים למשחק" – המסמכים הרשומים כאן צוינו שם כ"לא סופקו" ולכן מופיעים באינדקס יחד עם מיקומם ותפקידם.

---

## 1. מסמכי עיצוב ותכנון (Docs)

| מסמך | נתיב | תיאור |
|------|------|--------|
| **GDD ראשי MVP** | `Docs/MASTER_GDD_MVP.md` | מושגי ליבה, עמודי משחק, מפת אזורים, משתני Region, פעולות גיבור, רודמאפ. |
| **מכניקות ליבה** | `Docs/CORE_GAME_MECHANICS.md` | פילוסופיית משחק, כלכלה, CarryingCapacity, מאפייני Region, פעולות גיבור. |
| **מפרט שכבת קמפיין** | `Docs/campaign_layer_spec.md` | שדות Region, אירועי Region, פעולות גיבור, לולאת קמפיין. |
| **מערכת כלכלה** | `Docs/economy_system.md` | אוכלוסייה, FoodCapacity, סוגי יישובים, משתני ליבה. |
| **README ל־Cursor** | `Docs/README_FOR_CURSOR.md` | מבנה תיעוד, כלל: הרחבה בלי להחליף קוד קיים. |
| **יישור P0** | `Docs/P0_ALIGNMENT.md` | מיפוי מונחי מסמכים ↔ קוד נוכחי (מפנה ל־PLANNING_P0_revised §3). |
| **מצב פרויקט** | `Docs/PROJECT_STATUS.md` | סטטוס P0–P6, קוד C++, DataTables, מקשים, תוכניות, מקורות חיצוניים. |
| **מקורות חיצוניים + P0–P3** | `Docs/EXTERNAL_SOURCES_AND_P0_P3_PLAN.md` | RTS_FULL_MASTER_PACK, zip איזון, החלטות כלכלה/Role, תכנון P0–P3. |
| **ייבוא איזון יחידות** | `Docs/UNIT_BALANCE_IMPORT.md` | מודל כלכלה (כסף/דם-בשר/NPC/Upkeep), מיפוי Role, הנחיות ייבוא CSV ל־DT_Units_MVP. |
| **חוזה GameResult ו־Respawn** | `Docs/GAMERESULT_AND_RESPAWN_CONTRACT.md` | Single source of truth: סדר אירועים Win/Lose/Respawn, מכונת מצבים. |
| **החלטה סקוואד אחד לפאקשן** | `Docs/ONE_SQUAD_PER_FACTION_DECISION.md` | החלטה מוצרית: MVP עם סקוואד אחד לכל פאקשן + הרחבה עתידית. |
| **Spec עליית שליטה (Region)** | `Docs/REGION_CONTROL_GAIN_SPEC.md` | תנאי עליית שליטה: טיימר, contested, גיבור באזור, מקסימום 4 (5 רק via Secure). |

---

## 2. תיקיית AI_Knowledge (Docs/AI_Knowledge)

| מסמך | נתיב | תיאור |
|------|------|--------|
| **מדריך ארכיטקטורה** | `Docs/AI_Knowledge/ARCHITECTURE_GUIDE.md` | שכבות, קלאסים מרכזיים, כללי C++/Blueprint/Data. |
| **כללי קוד** | `Docs/AI_Knowledge/CODING_RULES.md` | לא למחוק, להרחיב, data-driven, ללא Tick כבד, שימוש ב־Subsystems. |
| **פייזות מימוש** | `Docs/AI_Knowledge/IMPLEMENTATION_PHASES.md` | Phase 1–7 (Region simulation → Recolonization). |
| **מוח הפרויקט** | `Docs/AI_Knowledge/PROJECT_BRAIN.md` | שכבת עולם, Region/Settlement/אוכלוסייה/Wildlife, ארכיטקטורה טכנית. |
| **סקירת פרויקט** | `Docs/AI_Knowledge/PROJECT_OVERVIEW.md` | עמודי משחק, כלל Cursor: extend don't replace. |
| **מערכת Region** | `Docs/AI_Knowledge/REGION_SYSTEM.md` | משתני Region, pulse, סימולציה. |
| **סימולציית עולם** | `Docs/AI_Knowledge/WORLD_SIMULATION.md` | סימולציה גלובלית, pulse. |
| **דשבורד AI/פיתוח** | `Docs/AI_Knowledge/RTS_AI_GAME_DEV_DASHBOARD.md` | דשבורד מידע (אם קיים). |

---

## 3. תוכניות (Plans) – .cursor/plans

| מסמך | נתיב | תיאור |
|------|------|--------|
| **P0 מתוקן עם מסמכים** | `.cursor/plans/PLANNING_P0_revised_with_new_docs.md` | P0 מפורט, §2 רשימת מערכות קיימות (לא למחוק), מיפוי doc↔code. |
| **P0 + Phases 1–7** | `.cursor/plans/PLANNING_P0_and_Phases_1_to_7.md` | P0 סיכום + Phase 1 (Region simulation) עד Phase 7 (Recolonization). |
| **P1 פקודות וקלט** | `.cursor/plans/PLANNING_P1_command_and_orders.md` | Selection, Orders, Input. |
| **P2 Region / Secure / Win-Lose** | `.cursor/plans/PLANNING_P2_region_control.md`, `PLANNING_P2_epics_1_2_4.md`, `PLANNING_P2_phases_1_to_7.md` | Region control 0–5, Secure, Victory. |
| **P3 Squad ו־Morale** | `.cursor/plans/PLANNING_P3_squad_morale.md` | Squad, Morale, Captain, Detached. |
| **P4 Economy** | `.cursor/plans/PLANNING_P4_economy.md`, `PLANNING_P4_phases.md` | כסף (בני אדם), המרה/לכידה (מפלצות). |
| **P5 Hero Respawn Ritual** | `.cursor/plans/PLANNING_P5_hero_respawn_ritual.md` | טיימר 90s, טקס 10s, Spawn באזור הטוב. |
| **P6 Day/Night + Polish** | `.cursor/plans/PLANNING_P6_day_night_polish.md` | **דחוי** – תוכנית קיימת, לא ממומשת כרגע. |
| **שיפורי UI** | `.cursor/plans/PLANNING_E5_UI_IMPROVEMENTS.md` | אופציונלי. |

---

## 4. GDD ותצורה (.cursor)

| מסמך | נתיב | תיאור |
|------|------|--------|
| **GDD (גרסת Cursor)** | `.cursor/GDD.md` | סדר פיתוח P0–P6, Secure Region, Hero Respawn Ritual, Command Range, Morale, Win/Lose, ערכים נעולים. |

---

## 5. Save/Load ומצב "פרויקט קיים"

| מסמך | נתיב | תיאור |
|------|------|--------|
| **Save/Load Spec** | `Docs/save_load_spec.md` | ארכיטקטורה, סכמה, Atomicity/Consistency, Rehydration APIs, Channel State. |
| **מדיניות Migration** | `Docs/save_migration_policy.md` | גרסאות schema ומפת מיגרציה (v1–v5). |
| **Spec מצב "פרויקט קיים"** | `Docs/existing_project_mode_spec.md` | זרימות UI, naming, slots, חוזה מוצרי MVP ומקרי קצה. |

| **תוכנית QA Save/Load** | `Docs/QA_PLAN_SAVE_LOAD.md` | עקרונות אוטומציה, מטריצת תרחישים, Golden Saves, Test Groups. |
| **חוזה Combat** | `Docs/COMBAT_CONTRACT.md` | Single Damage Authority, מחזור חיים Engagement, אינטגרציה Orders/Morale, Save/Load. |
| **מקשי קלט (מקור אמת)** | `Docs/KEYBINDS_SINGLE_SOURCE.md` | Secure=G, Ritual=B; Doc of Record מול P2/סטטוס. |

תוכנית QA מפורטת (מטריצה C) – גם בדוח "ניתוח פערים ותכנון מפורט ל־Save_Load, QA וביצועים" ובדוח "דוח פערים... Combat מרכזית".

---

## 6. קוד מקור (הפניה)

הקוד נמצא ב־`Source/RTS_Monsters/`. רשימת קבצים ומערכות מפורטת ב־`Docs/PROJECT_STATUS.md` (§2).

**DataTables:** נתיבים ב־Content תחת `RTS/Data/Data_Tables/` – DT_Units_MVP, DT_Heroes_MVP, DT_HeroXP_Prototype, DT_MoraleThresholds. קובץ ייבוא איזון: `Content/RTS/Data/Data_Tables/DT_Units_MVP_BalanceImport.csv`.

---

## 7. מקורות חיצוניים (מחוץ לריפו)

| מקור | נתיב | תיאור |
|------|------|--------|
| **RTS_FULL_MASTER_PACK** | `c:\Users\shodi\Desktop\תוכן נוסף ועדכון\RTS_FULL_MASTER_PACK` | 46 קבצי MD – ארכיטקטורה, מערכות, Data/Asset. |
| **rts_units_dataassets_and_balance.zip** | חילוץ: `…\תוכן נוסף ועדכון\rts_units_dataassets_and_balance\` | unit_balance_table.csv/md, DataAsset_Template. |

**ניתוח הפערים:** התבסס על מסמך מצב פרויקט בלבד; המסמכים ברשימה למעלה קיימים בפרויקט וניתן לקרוא אותם ישירות מריפו.
