# קריסה: Cast Default__Object to Actor בלחיצה על Play

## מה קורה

הקריסה מופיעה ב־**GenerateStreaming** (World Partition) מיד אחרי לחיצה על Play. המנוע מעביר אובייקט לא נכון (למשל `Default__Object`) למקום שמצפה ל־Actor.

## מה תוקן בקוד

1. **GetRegionAtLocation** – בדיקה ש־WorldContextObject לא CDO ולא UObject; לא קוראים ל־GetWorldFromContextObject במקרה כזה.
2. **הוסר WorldContext מה־meta** של GetRegionAtLocation – המנוע לא ממלא אוטומטית Context (שגרם ל־Cast ל־Actor במהלך GenerateStreaming). **מ־Blueprint** יש לחבר ידנית את ה־Context (למשל Get Player Controller 0 או Self) ל־**World Context Object**.

## מה לעשות עכשיו

1. **בנה מחדש** (סגור אדיטור → Build → פתח שוב).
2. **Level Blueprint של TopDownMap**: פתח **Window → Blueprints → Level Blueprint**. אם ה־Event Graph **ריק** – המקור לא כאן, עבור ל־3. אם יש nodes עם World Context – מחק או חבר ל־Get Player Controller (0).
3. **מפת בדיקה בלי World Partition**: צור מפה חדשה (**File → New Level → Empty Level**), הוסף **Player Start**, שמור כ־**TestMap**. ב־**Project Settings → Maps & Modes** הגדר **Editor Startup Map** ו־**Game Default Map** ל־**TestMap** זמנית. הרץ Play – אם זה עובד, הבעיה ב־TopDownMap/World Partition.
4. אם ב־TopDownMap יש **World Partition** ואתה רוצה לכבות: **Window → World Settings** → **World** → תחת **Partition** חפש **Enable World Partition** ובטל סימון (אם המפה תומכת בכך).
