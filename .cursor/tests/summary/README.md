# פלט מסכם – טסטים RTS_Monsters

תיקייה זו מכילה פלט מסכם של כל טסטי P0, P1 ו-P2.

## קבצים

| קובץ | שלב | מספר טסטים |
|------|-----|------------|
| [P0_summary.txt](P0_summary.txt) | P0 Core Setup | 28 |
| [P1_summary.txt](P1_summary.txt) | P1 Command + Orders + Input | 17 |
| [P2_summary.txt](P2_summary.txt) | P2 Region Control | 23 |

**סה"כ:** 68 טסטים

## עדכון הפלט

להרצה מחדש ויצירת הפלט המסכם:

```powershell
cd <נתיב_הפרויקט>
.\run_tests.ps1 P0 -OutFile .cursor\tests\summary\P0_summary.txt
.\run_tests.ps1 P1 -OutFile .cursor\tests\summary\P1_summary.txt
.\run_tests.ps1 P2 -OutFile .cursor\tests\summary\P2_summary.txt
```

או הרצת שלב בודד, למשל:

```powershell
.\run_tests.ps1 P0 -OutFile .cursor\tests\summary\P0_summary.txt
```

## מקורות הטסטים

- **P0:** GDD, PLANNING_P0_core_setup, RTS_CHECKLIST_EPIC_VS_MVP  
- **P1:** GDD, PLANNING_P1_command_and_orders, RTS_CHECKLIST_EPIC_VS_MVP  
- **P2:** GDD, PLANNING_P2_region_control, PLANNING_P2_epics_1_2_4  

קבצי המקור (YAML): `.cursor/tests/P0_tests.yaml`, `P1_tests.yaml`, `P2_tests.yaml`
