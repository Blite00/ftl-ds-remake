# Windows build entry point. Same as setup.bat, kept for existing workflows.
#   .\build.ps1                         rebuild the ROM from already converted assets
#   .\build.ps1 -FtlArchive <ftl.dat>   convert assets from your copy of FTL, then build
#   .\build.ps1 -Setup                  auto-detect FTL, convert, then build
param([string]$FtlArchive, [switch]$Setup)
$ErrorActionPreference = 'Stop'
Push-Location $PSScriptRoot
try {
    $py = (Get-Command py -ErrorAction SilentlyContinue)
    $pyArgs = @()
    if ($py) { $pyArgs = @('-3') } else { $py = Get-Command python -ErrorAction Stop }
    if ($FtlArchive) { & $py.Source @pyArgs tools/setup_assets.py --ftl $FtlArchive }
    elseif ($Setup) { & $py.Source @pyArgs tools/setup_assets.py }
    else { & $py.Source @pyArgs tools/setup_assets.py --build-only }
    if ($LASTEXITCODE -ne 0) { throw 'Setup/build failed.' }
} finally { Pop-Location }
