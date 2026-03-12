# Fix "remote end hung up" - track ONLY Content/RTS, Content/TopDown, Content/Data (~3.5 MB)
# Rest of Content (~3.7 GB) stays on disk but is not pushed to GitHub.
# Run from project root: .\fix_git_push.ps1

$ErrorActionPreference = "Stop"
Set-Location $PSScriptRoot

Write-Host "1. Removing ALL Content from Git index (files stay on disk)..." -ForegroundColor Cyan
git rm -r --cached Content/ 2>$null
if ($LASTEXITCODE -ne 0) {
    Write-Host "   (Content already untracked or path issue - continuing)" -ForegroundColor Gray
}

Write-Host "`n2. Staging .gitignore and re-adding only Content/RTS, Content/TopDown, Content/Data..." -ForegroundColor Cyan
git add .gitignore
git add Content/
git status -s Content/

Write-Host "`n3. Staging rest of project..." -ForegroundColor Cyan
git add -A

Write-Host "`n4. Committing..." -ForegroundColor Cyan
$count = git rev-list --count HEAD 2>$null
if (-not $count -or [int]$count -le 1) {
    git commit --amend -m "Initial commit: P0 P1 P2 complete - MVP baseline (Content: RTS+TopDown+Data only)"
} else {
    git commit -m "Track only Content/RTS, TopDown, Data - fix GitHub push"
}

Write-Host "`n5. Setting Git HTTP buffer to 500MB..." -ForegroundColor Cyan
git config http.postBuffer 524288000

Write-Host "`n6. Pushing to origin main..." -ForegroundColor Cyan
git push --set-upstream origin main
if ($LASTEXITCODE -ne 0) {
    Write-Host "`nIf this is a rewritten/amended history, try: git push --set-upstream origin main --force" -ForegroundColor Yellow
    exit 1
}

Write-Host "`nDone. Repo should be small enough for GitHub now." -ForegroundColor Green
