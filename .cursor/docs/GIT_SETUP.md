# העלאת הפרויקט ל-GitHub והמשך עבודה בבראנצ'ים

## שגיאת "remote end hung up unexpectedly"

השגיאה נגרמת לרוב מ**קבצים גדולים** ב־Content (תוכן דוגמה כמו Mannequins, ParagonFengMao, StarterContent). GitHub מתקשה עם push ענק.

**פתרון (הרץ מהשורש של הפרויקט):**
```powershell
cd C:\Users\shodi\Documents\Desktop\RTS_Monsters
.\fix_git_push.ps1
```
הסקריפט: מוציא את **כל** Content מה־index, then מחזיר לעקיבה **רק** `Content/RTS`, `Content/TopDown`, `Content/Data` (~3.5 MB). שאר ה־Content (תבניות, Mannequins, Paragon וכו') נשאר על הדיסק אבל לא נדחף ל־GitHub. אחרי זה: commit, הגדלת buffer, ו־push.

אם ה־push עדיין נכשל – נסה `git push --set-upstream origin main --force` (רק אם ב־GitHub אין commits שאתה צריך).

---

## מה בוצע

- **Git:** ה־repo אותחל בתיקיית הפרויקט (`RTS_Monsters`).
- **.gitignore:** נוסף לפי סטנדרט Unreal (Saved, Intermediate, Binaries, וכו').
- **Remote:** נוסף `origin` → https://github.com/omerbid/RTS_master

## מה להריץ אצלך (פעם אחת)

נעילה של Git (`index.lock`) יכולה להישאר מהרצה קודמת. **סגור Cursor/Editor/כל תהליך שמשתמש ב-Git**, ואז:

1. **אם יש `.git/index.lock`** – מחק אותו ידנית (או פתח PowerShell בפרויקט והרץ):
   ```powershell
   Remove-Item "C:\Users\shodi\Documents\Desktop\RTS_Monsters\.git\index.lock" -Force -ErrorAction SilentlyContinue
   ```

2. **הרצת הסקריפט להעלאה:**
   ```powershell
   cd C:\Users\shodi\Documents\Desktop\RTS_Monsters
   .\push_to_github.ps1
   ```
   הסקריפט מבצע: `git add -A` → `git commit` → `git branch -M main` → `git push -u origin main`.

3. **אם ב-GitHub יש כבר קובץ (למשל README):** ה־push עלול לדרוש מיזוג:
   ```powershell
   git pull origin main --rebase
   git push -u origin main
   ```
   או אם אתה רוצה שהמקומי יהפוך ל־main ב-GitHub:
   ```powershell
   git push -u origin main --force
   ```
   (השתמש ב־`--force` רק אם אתה בטוח שאתה רוצה לדרוס את מה שב-GitHub.)

## עבודה בבראנצ'ים אחרי ה-push

- **יצירת בראנצ' לפיצ'ר (למשל P3):**
  ```bash
  git checkout -b feature/p3-squad-morale
  ```
- **לאחר סיום עבודה על הבראנצ':** commit ו-push של הבראנצ', ואז ב-GitHub פתיחת Pull Request ל־`main`.
- **שמירת main יציב:** למזג ל־main רק אחרי בדיקות / code review.
