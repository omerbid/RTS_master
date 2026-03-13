
# README_FOR_CURSOR.md  
מדריך שילוב מסמכי התכנון בפרויקט

מסמך זה מיועד ל-Cursor (או לכל כלי AI אחר) כדי להבין כיצד לשלב את קבצי התכנון שהועלו לפרויקט Unreal הקיים.

המטרה:  
להכניס את הקבצים למקומות הנכונים בפרויקט, ולהבין את ההיררכיה ביניהם לפני שמבצעים שינויים בקוד.

---

# 1. כלל בסיסי

הקבצים שהועלו הם מסמכי תכנון בלבד.

אין למחוק או לשנות קוד קיים בפרויקט רק בגלל שמסמך לא מזכיר אותו.

אם קיימות מערכות בקוד שאינן מוזכרות במסמכים:

- לשמור עליהן
- לתעד אותן
- לשלב את המערכות החדשות איתן

לא להחליף מערכות קיימות בלי הוראה מפורשת.

---

# 2. מבנה תיקיות מומלץ בפרויקט

יש ליצור בפרויקט תיקייה:

/Docs/

ובתוכה:

/Docs/AI_Knowledge/

---

# 3. מיקום הקבצים

מסמכי עיצוב ראשיים → /Docs/

MASTER_GDD_MVP.md  
CORE_GAME_MECHANICS.md  
campaign_layer_spec.md  
economy_system.md  

מסמכי ארכיטקטורה → /Docs/AI_Knowledge/

ARCHITECTURE_GUIDE.md  
WORLD_SIMULATION.md  
REGION_SYSTEM.md  
PROJECT_OVERVIEW.md  
IMPLEMENTATION_PHASES.md  
CODING_RULES.md  

מסמכי מפת מערכת → /Docs/AI_Knowledge/

PROJECT_BRAIN.md  
RTS_AI_GAME_DEV_DASHBOARD.md  
