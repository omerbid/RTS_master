# Unit Info Widget – אם הפאנל האדום לא מופיע ב-Play

הווידג'ט עם פרטי היוניט (UnitId, Faction, Morale, Rank, Captain/Detached) מופיע **רק במצב Play** – לא בחלון האדיטור.

## אם אחרי Play עדיין לא רואים כלום

השתמש ב-**Blueprint** שיורש מ־C++:

### 1. ליצור Widget Blueprint

1. **Content Browser** → ימין-קליק → **User Interface** → **Widget Blueprint**.
2. תן שם (למשל `WBP_UnitInfo`).
3. **Class Settings** (בפאנל Details) → **Parent Class** → **RTSUnitInfoWidget**.

### 2. לבנות את הלייאאוט

1. פתח את ה-Widget Blueprint.
2. מה-**Palette** גרור:
   - **Canvas Panel** (אם אין Root – הוסף כ-Root).
   - לתוך ה-Canvas: **Vertical Box**.
   - לתוך ה-Vertical Box: **5 × Text** (חשוב: **Text** / Text Block, לא **TextBox** – כי הקוד מעדכן טקסט לקריאה בלבד).

3. **שמות**: לכל Text בחר אותו. ב-**Details** (למטה) יש שדה **Name** (לפעמים תחת **Identifiers** או בראש ה-Details). תן בדיוק את השמות האלה (אחד מהאפשרויות עובד):
   - `UnitId` או `TextBlock_UnitId`
   - `Faction` או `TextBlock_Faction`
   - `Morale` או `TextBlock_Morale`
   - `Rank` או `TextBlock_Rank`
   - `Extra` או `TextBlock_Extra`

4. (אופציונלי) צבע אדום: בחר כל Text → **Appearance** → **Color and Opacity** → אדום.

5. **Compile** ו-**Save**.

### 3. לחבר את WBP_UnitInfo ל-Player Controller (אחת משתי דרכים)

**אפשרות א – Config (בלי Blueprint PC):**

1. ב-**Content Browser** ימין-קליק על **WBP_UnitInfo** → **Copy Reference**.
2. פתח **Config/DefaultGame.ini** בפרויקט.
3. תחת `[/Script/RTS_Monsters.RTSPlayerController]` יש שורה `UnitInfoWidgetPath=`.
4. הדבק את הנתיב. אם הנתיב נגמר ב־`"` (בלי `_C`), הוסף **.WBP_UnitInfo_C** לפני המרכאות.  
   דוגמה: `UnitInfoWidgetPath=/Game/UI/WBP_UnitInfo.WBP_UnitInfo_C`
5. שמור את הקובץ והרץ שוב את הפרויקט (אין צורך ב-Build).

**אפשרות ב – Blueprint Player Controller + Game Mode:**

1. **Blueprint Player Controller:** Content Browser → ימין-קליק → **Blueprint Class** → חיפוש **RTS Player Controller** → בחר ויצור (למשל `BP_RTSPlayerController`). פתח → **Class Defaults** → תחת **RTS | UI** הגדר **Unit Info Widget Class** = `WBP_UnitInfo`.
2. **Blueprint Game Mode:** ימין-קליק → **Blueprint Class** → **RTS Game Mode Base** → צור (למשל `BP_RTSGameMode`). פתח → **Class Defaults** → **Player Controller Class** = `BP_RTSPlayerController`.
3. **במפה:** **Window → World Settings** → **Game Mode Override** = `BP_RTSGameMode`. שמור את המפה.

אחרי Build (אם שינית C++) ו-Play, הפאנל האדום עם הפרטים אמור להופיע מימין כשבוחרים יוניט.
