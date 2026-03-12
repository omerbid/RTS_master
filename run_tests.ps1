# RTS_Monsters - Test runner for P0, P1, P2
# Usage: .\run_tests.ps1 -Phase P0   (or P1, P2)

# Phase: first P0/P1/P2 in args. Optional: -OutFile path (writes summary to file)
$Phase = $null
$OutFilePath = $null
for ($ai = 0; $ai -lt $args.Count; $ai++) {
    if ($args[$ai] -eq '-OutFile' -and ($ai+1) -lt $args.Count) { $OutFilePath = $args[$ai+1]; $ai++ }
    elseif ($args[$ai] -in 'P0','P1','P2') { $Phase = $args[$ai] }
}
if (-not $Phase) { Write-Error 'Usage: .\run_tests.ps1 P0  or  .\run_tests.ps1 P0 -OutFile path'; exit 1 }

$ErrorActionPreference = 'Stop'
$ProjectRoot = $PSScriptRoot
if (-not $ProjectRoot) { $ProjectRoot = Get-Location }
$TestsDir = Join-Path (Join-Path $ProjectRoot '.cursor') 'tests'
$TestFile = Join-Path $TestsDir ($Phase + '_tests.yaml')
$ResultsFile = Join-Path $TestsDir 'test_results.yaml'

if (-not (Test-Path $TestFile)) {
    Write-Error ('Test file not found: ' + $TestFile)
    exit 1
}

# Load results (pass/fail/skip) if present
$resultsMap = @{}
if (Test-Path $ResultsFile) {
    $resLines = Get-Content $ResultsFile
    $inResults = $false
    foreach ($rl in $resLines) {
        if ($rl -match '^results:\s*$') { $inResults = $true; continue }
        if ($inResults -and $rl -match '^\s+([A-Z0-9.]+\.[0-9.]+)\s*:\s*(pass|fail|skip)') {
            $resultsMap[$Matches[1]] = $Matches[2]
        }
    }
}

$content = Get-Content $TestFile -Raw
$dod = ''
if ($content -match 'phase:\s*(\S+)') { $phaseName = $Matches[1] }
if ($content -match 'definition_of_done:\s*(.+)') { $dod = $Matches[1].Trim().Trim('"').Trim("'") }

$outLines = [System.Collections.ArrayList]@()
$add = { param($s) Write-Host $s; [void]$outLines.Add($s) }
$addC = { param($s,$c) Write-Host $s -ForegroundColor $c; [void]$outLines.Add($s) }

$addC.Invoke('========================================', 'Cyan')
$addC.Invoke(' RTS_Monsters - ' + $Phase + ' Tests', 'Cyan')
$addC.Invoke('========================================', 'Cyan')
$addC.Invoke('Definition of Done: ' + $dod, 'Gray')
$add.Invoke('')

$tests = [System.Collections.ArrayList]@()
$lines = Get-Content $TestFile
$i = 0
$inTests = $false
$curId = ''; $curTitle = ''; $curExpected = ''

while ($i -lt $lines.Count) {
    $line = $lines[$i]
    if ($line -match '^tests:\s*$') { $inTests = $true; $i++; continue }
    if (-not $inTests) { $i++; continue }

    if ($line -match '^\s*-\s*id:\s*(.+)') {
        if ($curId) { [void]$tests.Add([PSCustomObject]@{ id = $curId; title = $curTitle; expected = $curExpected }) }
        $curId = $Matches[1].Trim(); $curTitle = ''; $curExpected = ''
    }
    elseif ($line -match '^\s+title:\s*(.+)') {
        $curTitle = $Matches[1].Trim()
    }
    elseif ($line -match '^\s+expected:\s*(.+)') {
        $curExpected = $Matches[1].Trim()
    }
    $i++
}
if ($curId) { [void]$tests.Add([PSCustomObject]@{ id = $curId; title = $curTitle; expected = $curExpected }) }

$num = $tests.Count
$addC.Invoke('Total tests: ' + $num, 'Yellow')
$add.Invoke('')

$passCount = 0; $failCount = 0; $skipCount = 0; $noResultCount = 0
for ($j = 0; $j -lt $tests.Count; $j++) {
    $t = $tests[$j]
    $res = $resultsMap[$t.id]
    if ($res -eq 'pass') { $passCount++ }
    elseif ($res -eq 'fail') { $failCount++ }
    elseif ($res -eq 'skip') { $skipCount++ }
    else { $noResultCount++ }
    $status = ''
    $color = 'White'
    if ($res -eq 'pass') { $status = ' [PASS]'; $color = 'Green' }
    elseif ($res -eq 'fail') { $status = ' [FAIL]'; $color = 'Red' }
    elseif ($res -eq 'skip') { $status = ' [SKIP]'; $color = 'Yellow' }
    $addC.Invoke('[' + ($j + 1) + '/' + $num + '] ' + $t.id + ' - ' + $t.title + $status, $color)
    if ($t.expected) { $addC.Invoke('    Expected: ' + $t.expected, 'Gray') }
    $add.Invoke('')
}

$addC.Invoke('----------------------------------------', 'Cyan')
$add.Invoke('Summary: ' + $passCount + ' pass, ' + $failCount + ' fail, ' + $skipCount + ' skip, ' + $noResultCount + ' not run')
$addC.Invoke('Results file: ' + $ResultsFile, 'Gray')
$add.Invoke('Run in PIE (or Automation) and update test_results.yaml with pass/fail/skip.', 'Gray')
$addC.Invoke('Test file: ' + $TestFile, 'Gray')

$uproject = Get-ChildItem -Path $ProjectRoot -Filter '*.uproject' -ErrorAction SilentlyContinue | Select-Object -First 1
if ($uproject) {
    $filter = 'RTS_Monsters.' + $Phase
    $addC.Invoke('Automation filter: ' + $filter, 'Gray')
}

if ($OutFilePath) {
    $OutFilePath = $ExecutionContext.SessionState.Path.GetUnresolvedProviderPathFromPSPath($OutFilePath)
    $outLines -join "`r`n" | Set-Content -Path $OutFilePath -Encoding UTF8
}

exit 0
