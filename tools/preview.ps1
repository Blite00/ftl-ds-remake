param()
$ErrorActionPreference = 'Stop'
$projectRoot = Split-Path -Parent $PSScriptRoot
$compilerDir = Join-Path $projectRoot '.tools/msys64/ucrt64/bin'
$previousPath = $env:PATH
try {
    $env:PATH = $compilerDir + ';' + $env:PATH
    Push-Location $projectRoot
    try {
        New-Item -ItemType Directory -Force -Path build | Out-Null
        & (Join-Path $compilerDir 'gcc.exe') -std=c99 -O2 -Wall -Wextra -Werror -Isource -Igenerated tools/render_preview.c source/render.c source/shot_path.c source/menu.c source/setup.c source/player_art.c source/hangar.c source/profile.c source/tutorial.c source/diagnostics.c source/campaign.c source/rewards.c source/game.c source/fire.c source/boarding.c source/breach.c source/subsystems.c source/species.c source/beam.c source/drones.c source/achievements.c source/outcome.c source/audio.c -o build/render_preview.exe
        if ($LASTEXITCODE -ne 0) { throw 'Preview compilation failed.' }
        & ./build/render_preview.exe
        if ($LASTEXITCODE -ne 0) { throw 'Preview rendering failed.' }
        python tools/preview_png.py
        if ($LASTEXITCODE -ne 0) { throw 'PNG conversion failed; Python/Pillow are required.' }
    } finally { Pop-Location }
} finally { $env:PATH = $previousPath }
