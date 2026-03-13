
# CORE_GAME_MECHANICS.md

מסמך זה מרכז את עקרונות הליבה של המשחק.
זהו מסמך קצר אך סמכותי שמגדיר את המכניקות המרכזיות של המשחק.
כל מערכת במשחק צריכה להיות תואמת למסמך זה.

---

# 1. פילוסופיית המשחק

המשחק הוא:

Continuous Campaign RTS

כלומר:

- אין הפרדה בין מפת קמפיין למפת קרב
- הקרבות מתרחשים על אותה מפה שבה העולם מתקיים
- העולם ממשיך להשתנות גם בלי שהשחקן נוכח

השפעות:
- אזורים משתנים
- אוכלוסייה נודדת
- ישובים קורסים או גדלים

---

# 2. הכלכלה המרכזית

המשאב המרכזי במשחק הוא:

Population

אבל בניגוד ל‑RTS רגיל:

בני אדם הם **NPC פיזיים בעולם**.

הם לא משאב מספרי.

השחקן או המפלצות צריכים:
- למצוא אותם
- לתפוס אותם
- להרוג או להמיר אותם

דוגמא:

Village
↓
Villagers walking
↓
Monster raid
↓
CaptureHuman()
↓
Convert to unit

---

# 3. Carrying Capacity

לכל אזור יש קיבולת אוכלוסייה.

Variables:

Population
FoodCapacity

חוק בסיסי:

PopulationGrowth = BaseGrowth * (FoodCapacity / Population)

אם:
Population > FoodCapacity

אז:
- האוכלוסייה תברח
- ערים יתנפחו
- אזורים יינטשו

---

# 4. בריחת אוכלוסייה

כאשר ThreatLevel באזור עולה:

Villagers flee.

יעדים:
- Town
- City
- Capital

תוצאות:

ערים גדלות  
כפרים מתרוקנים  
גבולות נעשים פראיים

---

# 5. כלכלה אקולוגית

לכל פקשן כלכלה שונה.

Humans:
- משתמשים באוכלוסייה
- מגייסים חיילים

Vampires:
- צריכים בני אדם
- צדים ערים ושיירות

Werewolves:
- צדים חיות
- יכולים גם ללכוד בני אדם

משאבים:

Population
Food
Wildlife
Control

---

# 6. אזורים לא סימטריים

לא כל האזורים שווים.

Region properties:

RegionValue
PopulationCapacity
WildlifeDensity
TradeImportance
StrategicPosition

דוגמאות:

City region → אוכלוסייה גבוהה  
Forest region → חיות רבות  
Trade route → שיירות רבות

---

# 7. מחזור חיי ישוב

Village
↓
Raided
↓
Declining
↓
Ruins
↓
Recolonization
↓
Village

זה מאפשר לעולם להשתנות.

---

# 8. פעולות גיבור אזוריות

הגיבור הוא גם מפקד וגם מושל אזורי.

Hero actions:

SecureRegion
RaiseMilitia
FortifySettlement
SuppressRevolt
HuntMonsters
ClaimRuins
EscortCaravan

---

# 9. לולאת העולם

Population grows
↓
Settlements expand
↓
Monsters raid
↓
Refugees move
↓
Cities grow
↓
Frontiers collapse
↓
New settlements appear

---

# 10. החלטות MVP

כן:
Population economy
Wildlife system
Refugees
Settlement destruction
Recolonization
Hero regional actions

לא (Post‑MVP):
Diplomacy
Deep politics
City management UI
