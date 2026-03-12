# התקנת Unreal MCP – RTS_Monsters

מדריך צעד־אחר־צעד ל־Windows. אחרי ההתקנה Cursor יוכל לשלוט ב־Unreal Editor (אקטורים, Blueprints, Viewport).

---

## דרישות מוקדמות

- **Unreal Engine 5.5+** (אתה על 5.4 – בדוק תאימות; אם יש בעיה נסה גרסה חדשה יותר של ה־MCP).
- **Python 3.10+** ([python.org](https://www.python.org/downloads/)) – סמן "Add Python to PATH".
- **Git** (לשכפול הריפו).

---

## שלב 1 – הורדת unreal-mcp

1. פתח **PowerShell** או **Command Prompt**.
2. עבור לתיקייה נוחה (למשל Desktop):
   ```powershell
   cd %USERPROFILE%\Desktop
   ```
3. שכפל את הריפו:
   ```powershell
   git clone https://github.com/chongdashu/unreal-mcp.git
   ```
   אם אין Git – הורד ZIP מ־https://github.com/chongdashu/unreal-mcp ולחץ "Code → Download ZIP", then חלץ.

תקבל תיקייה: `unreal-mcp` עם תת־תיקיות `MCPGameProject`, `Python`, `Docs`.

---

## שלב 2 – התקנת ה־Plugin ב־Unreal

1. העתק את תיקיית ה־Plugin:
   - **מקור:** `unreal-mcp\MCPGameProject\Plugins\UnrealMCP`
   - **יעד:** `RTS_Monsters\Plugins\UnrealMCP`  
     כלומר: `c:\Users\shodi\Documents\Desktop\RTS_Monsters\Plugins\UnrealMCP`
2. אם אין לך תיקיית **Plugins** בפרויקט – צור אותה תחת `RTS_Monsters`.
3. פתח את **Unreal Editor** עם הפרויקט RTS_Monsters.
4. **Edit → Plugins** → חפש **UnrealMCP** (תחת Editor) → סמן **Enabled** → **Restart**.
5. אחרי הפעלה מחדש: **לחץ ימני על `RTS_Monsters.uproject`** → **Generate Visual Studio project files**.
6. פתח את ה־**.sln** ובנה **Development Editor** (או Build מהעורך). וודא שאין שגיאות.

---

## שלב 3 – התקנת שרת ה־Python

1. התקן **uv** (מנהל חבילות Python):
   - PowerShell (הרצה כ־Administrator אופציונלי):
     ```powershell
     powershell -ExecutionPolicy ByPass -c "irm https://astral.sh/uv/install.ps1 | iex"
     ```
   - או הורד מ־https://github.com/astral-sh/uv/releases והרץ.
2. עבור לתיקיית **Python** של unreal-mcp:
   ```powershell
   cd %USERPROFILE%\Desktop\unreal-mcp\Python
   ```
   (התאם את הנתיב אם שכפלת למקום אחר.)
3. צור סביבה וירטואלית והתקן תלויות:
   ```powershell
   uv venv
   .venv\Scripts\activate
   uv pip install -e .
   ```
4. בדיקה שהשרת עולה (**בלי** לסגור את החלון):
   ```powershell
   uv run unreal_mcp_server.py
   ```
   אם מופיעות שגיאות – וודא ש־**Unreal Editor פתוח** עם הפרויקט (ה־Plugin מתחבר לפורט 55557).

---

## שלב 4 – הגדרת Cursor (mcp.json)

1. בפרויקט RTS_Monsters צור/ערוך את הקובץ:
   **`.cursor\mcp.json`**  
   (נתיב מלא: `c:\Users\shodi\Documents\Desktop\RTS_Monsters\.cursor\mcp.json`)

2. שים בו את התוכן הבא – **והחלף את הנתיב ב־`--directory`** בנתיב האמיתי לתיקיית Python אצלך:

   ```json
   {
     "mcpServers": {
       "unrealMCP": {
         "command": "uv",
         "args": [
           "--directory",
           "C:\\Users\\shodi\\Documents\\Desktop\\unreal-mcp\\Python",
           "run",
           "unreal_mcp_server.py"
         ]
       }
     }
   }
   ```

   - אם שכפלת את unreal-mcp למקום אחר (למשל `D:\Dev\unreal-mcp`), שנה ל:
     `"D:\\Dev\\unreal-mcp\\Python"`  
     (ב־JSON כפול backslash: `\\`.)

3. אם אתה מעדיף **python** במקום **uv** (לאחר `uv pip install -e .` ו־activate):
   ```json
   "unrealMCP": {
     "command": "C:\\Users\\shodi\\Documents\\Desktop\\unreal-mcp\\Python\\.venv\\Scripts\\python.exe",
     "args": [
       "C:\\Users\\shodi\\Documents\\Desktop\\unreal-mcp\\Python\\unreal_mcp_server.py"
     ]
   }
   ```

4. שמור את הקובץ.

5. **הפעל מחדש את Cursor** (או Reload Window: Ctrl+Shift+P → "Developer: Reload Window") כדי שיטען את ה־MCP.

---

## שלב 5 – שימוש

1. **תמיד** להפעיל את **Unreal Editor** עם RTS_Monsters (וה־Plugin מופעל) **לפני** שאתה משתמש ב־MCP ב־Cursor.
2. ב־Cursor פתח את הפרויקט RTS_Monsters; ה־MCP אמור להתחבר אוטומטית כשתבקש פעולות Unreal.
3. אם משהו לא עובד – בדוק את הקובץ **unreal_mcp.log** בתיקיית Python (או בלוגים של Cursor).

---

## פתרון בעיות

| בעיה | מה לבדוק |
|------|-----------|
| "table not found" / אין חיבור | Unreal Editor פתוח? Plugin מופעל? פורט 55557 פנוי? |
| Cursor לא רואה את ה־MCP | קובץ `.cursor\mcp.json` בנתיב הנכון? הנתיב ב־`--directory` נכון? הפעלה מחדש ל־Cursor. |
| שגיאה ב־Python / uv | Python 3.10+ ב־PATH? הרצת `uv pip install -e .` מתוך תיקיית Python? |
| Build נכשל אחרי הוספת Plugin | Generate VS project files שוב; נסה Clean + Build. |

---

## סיכום נתיבים (התאם למחשב שלך)

| מה | נתיב לדוגמה |
|----|--------------|
| פרויקט | `c:\Users\shodi\Documents\Desktop\RTS_Monsters` |
| Plugin | `RTS_Monsters\Plugins\UnrealMCP` |
| Python (unreal-mcp) | `c:\Users\shodi\Documents\Desktop\unreal-mcp\Python` |
| mcp.json | `RTS_Monsters\.cursor\mcp.json` |

אחרי שכל השלבים עברו – אתה אמור לראות את כלי ה־Unreal MCP זמינים ב־Cursor כשהעורך פתוח.
