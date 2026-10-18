param([switch]$Full)
# V63: quick mode by default skips the long scripted full-campaign bot runs;
# -Full runs everything (used once before the final package).
$longRuns = @('surrender_run_smoke','enemy_loadout_run_smoke','balance_probe','retreat_run_smoke','large_campaign_run_smoke','defense_run_smoke','drone_run_smoke','equipment_run_smoke','bomb_run_smoke','beam_run_smoke','ion_run_smoke','species_run_smoke','boarding_run_smoke','campaign_v10_smoke')
$ErrorActionPreference = 'Stop'
$projectRoot = Split-Path -Parent $PSScriptRoot
$compilerDir = Join-Path $projectRoot '.tools/msys64/ucrt64/bin'
$compiler = Join-Path $compilerDir 'gcc.exe'
if (!(Test-Path -LiteralPath $compiler)) { throw 'Portable host C compiler is missing.' }
$runDirectory = Join-Path $projectRoot ('build/check-' + [guid]::NewGuid().ToString('N'))
New-Item -ItemType Directory -Path $runDirectory | Out-Null
New-Item -ItemType Directory -Path (Join-Path $runDirectory 'tests') | Out-Null
$previousPath = $env:PATH
$checks = @(
    @{ Name='enemy_ai_smoke'; Sources=@('game') },
    @{ Name='enemy_weapon_ai_smoke'; Sources=@('game') },
    @{ Name='enemy_boarding_ai_smoke'; Sources=@('game') },
    @{ Name='enemy_boarding_v39_smoke'; Sources=@('game') },
    @{ Name='enemy_calm_ai_smoke'; Sources=@('game') },
    @{ Name='final_sector_smoke'; Sources=@('save','campaign','game') },
    @{ Name='flagship_smoke'; Sources=@('save','campaign','game') },
    @{ Name='flagship_drones_smoke'; Sources=@('game') },
    @{ Name='flagship_final_smoke'; Sources=@('game') },
    @{ Name='finale_choice_smoke'; Sources=@('menu','render','diagnostics','save','campaign','game','audio') },
    @{ Name='v46_combat_smoke'; Sources=@('render','diagnostics','campaign','game','audio') },
    @{ Name='v51_weapons_smoke'; Sources=@('campaign','game') },
    @{ Name='v52_ships_smoke'; Sources=@('render','diagnostics','campaign','game','audio') },
    @{ Name='v53_events_smoke'; Sources=@('save','campaign','game') },
    @{ Name='v55_cloak_augments_smoke'; Sources=@('save','campaign','game') },
    @{ Name='v56_drones_smoke'; Sources=@('save','campaign','game') },
    @{ Name='v57_hangar_smoke'; Sources=@('save','campaign','game','profile','hangar','setup') },
    @{ Name='v58_artillery_systems_smoke'; Sources=@('save','campaign','game') },
    @{ Name='v59_achievements_smoke'; Sources=@('save','campaign','game','profile') },
    @{ Name='v60_environment_smoke'; Sources=@('save','campaign','game') },
    @{ Name='v61_quests_smoke'; Sources=@('save','campaign','game') },
    @{ Name='v62_event_features_smoke'; Sources=@('save','campaign','game') },
    @{ Name='v63_secret_sector_smoke'; Sources=@('save','campaign','game') },
    @{ Name='tutorial_smoke'; Sources=@('tutorial','game') },
    @{ Name='v66_flagship_construction_smoke'; Sources=@('save','campaign','game') },
    @{ Name='v71_unlock_smoke'; Sources=@('save','campaign','game','hangar','profile') },
    @{ Name='v68_shot_path_smoke'; Sources=@('shot_path','game') },
    @{ Name='v70_damage_audit_smoke'; Sources=@('game') },
    @{ Name='audio_stream_smoke'; Sources=@('audio_stream') },
    @{ Name='audio_backend_smoke'; Sources=@('audio_stream') },
    @{ Name='v68_alarm_smoke'; Sources=@('audio_stream') },
    @{ Name='pacing_smoke'; Sources=@('pacing') },
    @{ Name='crew_setup_smoke'; Sources=@('setup','game') },
    @{ Name='crew_setup_view_smoke'; Sources=@('render','diagnostics','campaign','game','audio') },
    @{ Name='input_capture_smoke'; Sources=@('input_capture') },
    @{ Name='input_view_smoke'; Sources=@('input_view') },
    @{ Name='enemy_hull_smoke'; Sources=@('render','diagnostics','campaign','game','audio') },
    @{ Name='main_menu_smoke'; Sources=@('menu','render','diagnostics','save','campaign','game','audio') },
    @{ Name='surrender_event_smoke'; Sources=@('save','campaign','game') },
    @{ Name='save_surrender_policy_smoke'; Sources=@('save','campaign','game') },
    @{ Name='surrender_run_smoke'; Sources=@('save','campaign','game') },
    @{ Name='map_zoom_smoke'; Sources=@('render','diagnostics','campaign','game','audio') },
    @{ Name='surrender_offer_ui_smoke'; Sources=@('render','diagnostics','campaign','game','audio') },
    @{ Name='enemy_threat_ui_smoke'; Sources=@('render','diagnostics','campaign','game','audio') },
    @{ Name='enemy_loadout_save_smoke'; Sources=@('save','campaign','game') },
    @{ Name='enemy_loadout_run_smoke'; Sources=@('campaign','game') },
    @{ Name='enemy_loadout_smoke'; Sources=@('game') },
    @{ Name='controller_smoke'; Sources=@('render','diagnostics','campaign','game','audio') },
    @{ Name='v69_weapon_targets_smoke'; Sources=@('render','diagnostics','campaign','game','audio') },
    @{ Name='balance_probe'; Sources=@('campaign','game') },
    @{ Name='target_card_smoke'; Sources=@('render','diagnostics','campaign','game','audio') },
    @{ Name='room_damage_smoke'; Sources=@('game') },
    @{ Name='reward_ui_smoke'; Sources=@('render','diagnostics','campaign','game','audio') },
    @{ Name='reward_smoke'; Sources=@('rewards','game') },
    @{ Name='save_reward_smoke'; Sources=@('save','campaign','game') },
    @{ Name='reward_event_smoke'; Sources=@('campaign','game') },
    @{ Name='surrender_core_smoke'; Sources=@('game','outcome') },
    @{ Name='surrender_persistence_ui_smoke'; Sources=@('save','render','diagnostics','campaign','game','audio') },
    @{ Name='retreat_run_smoke'; Sources=@('save','campaign','game') },
    @{ Name='player_departure_smoke'; Sources=@('game') },
    @{ Name='player_retreat_smoke'; Sources=@('campaign','game') },
    @{ Name='retreat_home_service_smoke'; Sources=@('campaign','game') },
    @{ Name='save_boss_retreat_smoke'; Sources=@('save','campaign','game') },
    @{ Name='player_retreat_ui_smoke'; Sources=@('render','diagnostics','campaign','game','audio') },
    @{ Name='save_intruder_smoke'; Sources=@('save','campaign','game') },
    @{ Name='intruder_resume_smoke'; Sources=@('save','campaign','game') },
    @{ Name='carried_intruder_smoke'; Sources=@('game') },
    @{ Name='carried_hazards_smoke'; Sources=@('game') },
    @{ Name='carried_ui_save_smoke'; Sources=@('save','render','diagnostics','campaign','game','audio') },
    @{ Name='player_ftl_smoke'; Sources=@('game') },
    @{ Name='player_ftl_campaign_smoke'; Sources=@('save','campaign','game') },
    @{ Name='escape_ui_smoke'; Sources=@('render','diagnostics','campaign','game','audio') },
    @{ Name='enemy_escape_smoke'; Sources=@('game') },
    @{ Name='escape_event_smoke'; Sources=@('campaign','game') },
    @{ Name='save_escape_smoke'; Sources=@('save','campaign','game') },
    @{ Name='combat_event_smoke'; Sources=@('campaign','game') },
    @{ Name='save_combat_event_smoke'; Sources=@('save','campaign','game') },
    @{ Name='rebel_fat_smoke'; Sources=@('render','diagnostics','campaign','game','audio') },
    @{ Name='original_event_smoke'; Sources=@('campaign','game') },
    @{ Name='save_event_policy_smoke'; Sources=@('save','campaign','game') },
    @{ Name='original_event_ui_smoke'; Sources=@('render','diagnostics','campaign','game','audio') },
    @{ Name='sector_routes_smoke'; Sources=@('campaign','game') },
    @{ Name='save_sector_routes_smoke'; Sources=@('save','campaign','game') },
    @{ Name='sector_routes_ui_smoke'; Sources=@('render','diagnostics','campaign','game','audio') },
    @{ Name='large_map_smoke'; Sources=@('campaign','game') },
    @{ Name='save_large_map_smoke'; Sources=@('save','campaign','game') },
    @{ Name='fleet_ui_smoke'; Sources=@('render','diagnostics','campaign','game','audio') },
    @{ Name='large_campaign_run_smoke'; Sources=@('save','campaign','game') },
    @{ Name='defense_smoke'; Sources=@('game') },
    @{ Name='defense_ui_smoke'; Sources=@('render','diagnostics','campaign','game','audio') },
    @{ Name='defense_run_smoke'; Sources=@('save','campaign','game') },
    @{ Name='save_defense_smoke'; Sources=@('save','campaign','game') },
    @{ Name='drone_campaign_smoke'; Sources=@('campaign','game') },
    @{ Name='drone_ui_smoke'; Sources=@('render','diagnostics','campaign','game','audio') },
    @{ Name='drone_run_smoke'; Sources=@('save','campaign','game') },
    @{ Name='save_drone_inventory_smoke'; Sources=@('save','campaign','game') },
    @{ Name='drone_smoke'; Sources=@('game') },
    @{ Name='save_drone_smoke'; Sources=@('save','campaign','game') },
    @{ Name='equipment_run_smoke'; Sources=@('save','campaign','game') },
    @{ Name='equipment_smoke'; Sources=@('game') },
    @{ Name='equipment_ui_smoke'; Sources=@('render','diagnostics','campaign','game','audio') },
    @{ Name='save_equipment_smoke'; Sources=@('save','campaign','game') },
    @{ Name='bomb_run_smoke'; Sources=@('save','campaign','game') },
    @{ Name='bomb_smoke'; Sources=@('game') },
    @{ Name='bomb_ui_smoke'; Sources=@('render','diagnostics','campaign','game','audio') },
    @{ Name='save_bomb_smoke'; Sources=@('save','campaign','game') },
    @{ Name='beam_run_smoke'; Sources=@('save','campaign','game') },
    @{ Name='beam_smoke'; Sources=@('game') },
    @{ Name='beam_ui_smoke'; Sources=@('render','diagnostics','campaign','game','audio') },
    @{ Name='save_beam_smoke'; Sources=@('save','campaign','game') },
    @{ Name='ion_run_smoke'; Sources=@('save','campaign','game') },
    @{ Name='ion_smoke'; Sources=@('game') },
    @{ Name='ion_ui_smoke'; Sources=@('render','diagnostics','campaign','game','audio') },
    @{ Name='save_ion_smoke'; Sources=@('save','campaign','game') },
    @{ Name='species_run_smoke'; Sources=@('save','campaign','game') },
    @{ Name='species_smoke'; Sources=@('game') },
    @{ Name='species_ui_smoke'; Sources=@('render','diagnostics','campaign','game','audio') },
    @{ Name='save_species_smoke'; Sources=@('save','campaign','game') },
    @{ Name='subsystems_smoke'; Sources=@('game') },
    @{ Name='subsystems_ui_smoke'; Sources=@('render','diagnostics','campaign','game','audio') },
    @{ Name='save_subsystems_smoke'; Sources=@('save','campaign','game') },
    @{ Name='damage_breach_smoke'; Sources=@('game') },
    @{ Name='breach_ui_smoke'; Sources=@('render','diagnostics','campaign','game','audio') },
    @{ Name='save_breach_smoke'; Sources=@('save','campaign','game') },
    @{ Name='outcome_smoke'; Sources=@('outcome') },
    @{ Name='boarding_smoke'; Sources=@('game') },
    @{ Name='boarding_ui_smoke'; Sources=@('render','diagnostics','campaign','game','audio') },
    @{ Name='boarding_campaign_smoke'; Sources=@('campaign','game') },
    @{ Name='boarding_run_smoke'; Sources=@('campaign','game') },
    @{ Name='save_teleporter_smoke'; Sources=@('save','campaign','game') },
    @{ Name='crew_v12_smoke'; Sources=@('game') },
    @{ Name='crew_outcome_smoke'; Sources=@('campaign','game') },
    @{ Name='crew_ui_smoke'; Sources=@('render','diagnostics','campaign','game','audio') },
    @{ Name='save_crew_smoke'; Sources=@('save','campaign','game') },
    @{ Name='shields_smoke'; Sources=@('game') },
    @{ Name='feedback_smoke'; Sources=@('render','diagnostics','campaign','game','audio') },
    @{ Name='enemy_drone_smoke'; Sources=@('game') },
    @{ Name='save_fire_smoke'; Sources=@('save','campaign','game') },
    @{ Name='fire_smoke'; Sources=@('game') },
    @{ Name='smoke'; Sources=@('game') },
    @{ Name='doors_smoke'; Sources=@('game') },
    @{ Name='medbay_smoke'; Sources=@('game') },
    @{ Name='foundation_smoke'; Sources=@('game') },
    @{ Name='campaign_v10_smoke'; Sources=@('campaign','game') },
    @{ Name='campaign_resume_smoke'; Sources=@('save','campaign','game') },
    @{ Name='audio_smoke'; Sources=@() },
    @{ Name='logging_smoke'; Sources=@('diagnostics','game'); Log=$true },
    @{ Name='save_v10_smoke'; Sources=@('save','campaign','game') },
    @{ Name='ui_v10_smoke'; Sources=@('render','diagnostics','campaign','game','audio') },
    @{ Name='targeting_smoke'; Sources=@('render','diagnostics','campaign','game','audio') },
    @{ Name='autofire_smoke'; Sources=@('render','diagnostics','campaign','game','audio') }
)
$passed = 0
& python (Join-Path $projectRoot 'tests/enemy_weapon_import_test.py')
if ($LASTEXITCODE -ne 0) { throw 'Enemy weapon importer tests failed.' }
& python (Join-Path $projectRoot 'tests/event_compile_test.py')
if ($LASTEXITCODE -ne 0) { throw 'Original event compiler tests failed.' }
try {
    $env:PATH = $compilerDir + ';' + $env:PATH
    Push-Location $runDirectory
    try {
        foreach ($check in $checks) {
            if (-not $Full -and $longRuns -contains $check.Name) { continue }
            $testFile = Join-Path $projectRoot ('tests/' + $check.Name + '.c')
            if (!(Test-Path -LiteralPath $testFile)) {
                if ($check.Optional) { Write-Output ('SKIP (not yet present): ' + $check.Name); continue }
                throw ('Required test missing: ' + $testFile)
            }
            $executable = Join-Path $runDirectory ($check.Name + '.exe')
            $compilerArgs = @('-std=c99','-Wall','-Wextra','-Werror',('-I' + (Join-Path $projectRoot 'source')),('-I' + (Join-Path $projectRoot 'generated')),$testFile)
            foreach ($source in $check.Sources) { $compilerArgs += Join-Path $projectRoot ('source/' + $source + '.c') }
            if ($check.Sources -contains 'game') { $compilerArgs += Join-Path $projectRoot 'source/fire.c'; $compilerArgs += Join-Path $projectRoot 'source/achievements.c'; $compilerArgs += Join-Path $projectRoot 'source/boarding.c'; $compilerArgs += Join-Path $projectRoot 'source/breach.c'; $compilerArgs += Join-Path $projectRoot 'source/subsystems.c'; $compilerArgs += Join-Path $projectRoot 'source/species.c'; $compilerArgs += Join-Path $projectRoot 'source/beam.c'; $compilerArgs += Join-Path $projectRoot 'source/drones.c' }
            if ($check.Sources -contains 'campaign') { $compilerArgs += Join-Path $projectRoot 'source/outcome.c'; $compilerArgs += Join-Path $projectRoot 'source/rewards.c' }
            if (($check.Sources -contains 'render') -and -not ($check.Sources -contains 'menu')) { $compilerArgs += Join-Path $projectRoot 'source/menu.c' }
            if ($check.Sources -contains 'render') { $compilerArgs += Join-Path $projectRoot 'source/setup.c'; $compilerArgs += Join-Path $projectRoot 'source/shot_path.c' }
            if ($check.Sources -contains 'render') { $compilerArgs += Join-Path $projectRoot 'source/player_art.c'; $compilerArgs += Join-Path $projectRoot 'source/hangar.c'; $compilerArgs += Join-Path $projectRoot 'source/profile.c'; if (-not ($check.Sources -contains 'tutorial')) { $compilerArgs += Join-Path $projectRoot 'source/tutorial.c' } }
            $compilerArgs += @('-o',$executable)
            & $compiler @compilerArgs
            if ($LASTEXITCODE -ne 0) { throw ('Compilation failed: ' + $check.Name) }
            if ($check.Log) { & $executable (Join-Path $runDirectory 'logging-smoke.log') }
            else { & $executable }
            if ($LASTEXITCODE -ne 0) { throw ('Test failed: ' + $check.Name) }
            $passed++
            Write-Output ('PASS: ' + $check.Name)
        }
    } finally { Pop-Location }
} finally { $env:PATH = $previousPath }
Write-Output ("Passed $passed host checks. Disposable artifacts: $runDirectory")


