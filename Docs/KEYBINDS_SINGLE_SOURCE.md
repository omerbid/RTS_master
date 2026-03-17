# מקור אמת יחיד למקשי קלט (Keybinds)

**מטרה:** להכריע סתירות בין מסמכים (למשל Secure Region במקש G מול S) ולשמש Doc of Record למקשי משחק. המימוש בפועל ב־`ARTSPlayerController`.

**מקור:** דוח פערים – "Secure Region Input: במסמך סטטוס G, ב־P2 Phases S; ב־P2 הראשי key/hold+click ללא החלטה אחידה".

---

## מקשים מחייבים (מימוש נוכחי)

| פעולה | מקש | הערה |
|--------|-----|------|
| **Secure Region** | **G** | גיבור נבחר, בתוך Region עם control 4, לא contested. (S לא בשימוש – מתנגש עם camera backward.) |
| **Perform Ritual (Respawn)** | **B** | כשמצב RitualAvailable (אחרי 90s מהמות גיבור). |

**מימוש:** `RTSPlayerController.cpp` – `BindKey(EKeys::G, ... OnInputSecureRegion)`; Ritual triggered מממשק/לוגיקה (למשל B או כפתור UI).

---

## המלצות

- **תיעוד:** כל מסמך שמזכיר "מקש Secure" או "keybind" – להפנות ל־מסמך זה או ל־RTSPlayerController.
- **QA:** בדיקת smoke – אחרי build, לוודא שלחיצה על G עם גיבור נבחר באזור תקף מתחילה Secure (או מציגה הודעה עקבית).
- **תפריט עזרה/הגדרות:** אם י נוסף מסך Keybinds, הערכים יילקחו ממסמך זה / מקוד כ־default.

---

## קישורים

- `Source/RTS_Monsters/RTSPlayerController.cpp` – Binding בפועל.
- `Docs/GAMERESULT_AND_RESPAWN_CONTRACT.md` – Ritual flow.
- `Docs/GAP_ANALYSIS_RESPONSE.md` – פער Secure G vs S.
