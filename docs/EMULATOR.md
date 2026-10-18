## User playtest feedback, October 3, 2026

The user reports rough audio, a text-heavy interface and unclear room-targeting effects in the current build. This provides user-reported screen/audio/input experience, but does not establish a complete emulator test pass or physical DS validation. The combat polish checkpoint addresses these concerns; listening and hands-on confirmation of the revised build remain pending.

# Emulator baseline and remaining checks

The local executable is `.tools/melonds/melonDS.exe`, version 1.1 according to its
successful `--help` output. It accepts a ROM as its first positional argument and
`-b auto|always|never` for firmware boot behavior. Full executable and ROM paths,
with `.tools/melonds` as the working directory, successfully started a persistent
process during investigation. This establishes process startup only; a live
window, running frames, controls and sound require separate observation.

The inspected local `melonDS.toml` has:

- ConsoleType 0, internal BIOS and direct boot enabled.
- DLDI disabled: SD save/log validation is not possible with this configuration.
- All DS keyboard and joystick bindings set to -1: configure input before tests.
- 60 FPS limiting, AudioSync false and instance audio volume 256.

Configuration was inspected without editing it. The initial Computer Use app
launch did not expose a targetable window. CLI `--help` exited successfully,
which rules out a basic missing executable dependency for that invocation.
Launching the ROM by full path subsequently left a running process; UI validation
must document its own results rather than infer them from process existence.

## Runtime checklist

1. Confirm visible dual screens and map startup with the current ROM.
2. Configure DS buttons, test touch, pause, target selection and screen changes.
3. Verify a complete encounter and return to map; inspect work_us diagnostics.
4. Listen to map/combat music, weapon/hull cues and victory/defeat stingers.
5. Configure writable homebrew DLDI storage, then test save/relaunch and backup
   recovery on a disposable copy of the storage image. Preserve existing saves.
6. Play through sectors and final encounter, including a fresh run after defeat.

Emulator measurements do not establish original DS frame pacing or flashcart
compatibility. Physical DS checks remain pending.

The CLI ROM probe reported Responding=true but MainWindowHandle=0 and no title; Computer Use still found no window. Hidden launch may contribute to visibility, so this is not evidence that emulation failed. The probe process was stopped after inspection. No screen/input/audio runtime result is claimed.
