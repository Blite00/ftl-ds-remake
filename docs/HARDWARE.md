# V45 host measurements and physical checklist

These are host and static figures, not DS measurements. Physical acceptance is pending; see [DS_TEST_CHECKLIST.md](DS_TEST_CHECKLIST.md).

- **Memory:** the static ARM9 union is 984,356 bytes. Add 660,340 bytes of resident effects and the 4,096-byte stream buffer, about 1.65 MB in total, against the 4 MiB main RAM. That leaves roughly 2.3 MB for libnds, the FAT/NitroFS caches and heap.
- **Stack** (`python tools/stack_report.py`, build/v45-stack.txt): the worst known main-loop path is 3,508 bytes, about 21% of the 16 KiB DTCM stack. The deepest path runs from the touch handler through a jump and its surrender planning. The VBlank handler needs 72 bytes and the audio fill callback 72 bytes. SDK and libc callees, including the FAT writer, count as 0, so the save path, 992 bytes plus FAT, is the main unknown. No unbounded frames; three event helpers recurse over the bounded compiled event tree.
- **Host cost** (`tools/host_cost_probe.c`, build/v45-host-cost.txt):

  | Work | Host cost |
  |---|---|
  | One simulation step, mean | 0.8–3.1 µs |
  | Worst 4-step batch | 155 µs |
  | Full two-screen render | ~125 µs mean, 368 µs max |
  | 2048-sample audio fill (music + 4 resampled effects) | 68 µs max |
  | Save on the PC disk | 6.4 ms |

- **What that means for the frame budget:** a worst main-loop iteration costs about 0.6 ms on this PC.
  - The DS could be about 55 times slower before missing the 33 ms update budget, and about 150 times slower before the 93 ms Maxmod ring underruns.
  - An ARM946E-S at 67 MHz is plausibly 50–200 times slower than this PC for this code, so frame pacing is likely but unproven. Rendering is about 60% of the cost.
  - DMA and cache flushes aren't in the host figures.
- **Audio underrun risk:** saving runs synchronously in the main loop. A 16 KiB write, verify and backup rename on a flashcart SD can exceed the ~93 ms ring, so a short music gap on save is expected. Checklist item 3 records it.

# V37 AI: static measurement

ARM9 union 912,284 bytes; high-water 912,336. Game/Campaign sizes unchanged; V36 dynamic streaming allocation remains additional. Crew assignment is bounded to eight owned crew and runs once per simulated second. ROM builds and host previews/checks do not establish physical DS frame timing or audio acceptance. See V37_CHECKPOINT.md.

# V36 streaming audio: validation pending

Static ARM9 union 909,868 bytes; high-water 909,904. Effects allocate 660,340 bytes, plus a 4,096-byte stream sample buffer and library overhead. ROM 40,924,160 bytes. Build and host checks pass; real DS audio, flashcart NitroFS access, speaker volume, runtime memory and sustained frame timing remain unmeasured. See V36_CHECKPOINT.md and V36_AUDIO.md.

# V35 combat timing and setup

Elapsed VBlank scheduling replaces iteration-count timing; bitmap drawing clips once per image. Static ARM9 union 901,940 bytes, high-water 901,992; Game/Campaign sizes unchanged. Hardware speedup and frame smoothness are not measured. Validate setup keyboard, deselection, charging/crew speed and pause/resume on DS. See V35_CHECKPOINT.md and V35_PACING.md.

# V33 input and menu: static measurement

ARM9 static allocated union: 898,660 bytes; high-water: 898,696. Game: 17,876 bytes; Campaign: 22,024. ROM: 615,424 bytes. Save format24 is unchanged. Input sampling now runs in a bounded VBlank handler; game/render/storage work remains in the main loop. See build/v33-memory.txt and INPUT_RESPONSE.md.

The ROM builds and host framebuffers/tests pass. No hardware latency or frame-time measurements have been obtained. Validate rapid separate taps, long holds, successive power/room actions, screen/confirmation transitions, menu Continue/New Run and flashcart saves on the real DS. Recent diagnostics can be lost on a crash before an explicit flush; gameplay saves retain their existing rules. Earlier measurements follow.

# V32 surrender and map: static measurement

ARM9 allocated union: 868,012 bytes; image high-water: 868,040. Game: 17,876 bytes; Campaign: 22,024. The map uses cached bounded layout scratch without modifying campaign state. These static counts do not establish runtime free memory or frame pacing. See build/v32-memory.txt and V32_CHECKPOINT.md.

ROM build and host framebuffer validation are complete. Physical DS and emulator input/audio/storage acceptance remain pending, including V30 speaker volume and controller comfort. Earlier measurements and user observations follow.

# V31 enemy loadouts: static measurement

ARM9 allocated union: 851,908 bytes; image high-water: 851,952. Game: 17,876 bytes; Campaign: 22,016. Encounter staging uses a static Game buffer to avoid another full Game on the DS stack. ROM: 570,368 bytes. See build/v31-memory.txt and V31_CHECKPOINT.md.

The ROM builds and affected host framebuffers were inspected. These figures do not measure runtime free memory or frame time. Emulator input/audio/storage, stack/heap profiling and physical DS checks remain pending. Prior real-DS feedback and historical measurements follow.

# V30 real-DS feedback and current measurement

The user reports running the previous build on a real DS: maximum physical volume was too quiet, text/controls were confusing and opening fights too easy. These observations motivate V30; they do not establish full hardware acceptance. Revised volume, controller ergonomics, native-screen readability, frame pacing and flashcart saves still need explicit confirmation.

V30 static ARM9 union: 827,764 bytes; high-water offset: 827,816. Game 17,708 / Campaign 21,848 bytes. ROM builds; host controller/renderer/balance checks pass. See V30_CHECKPOINT.md and build/v30-memory.txt. Earlier measurements below are historical.

# V29 combat/audio polish: static measurement

ARM9 allocated section union: 822428 bytes; high-water offset: 822472. Game/Campaign remain 17708/21848 bytes. A 64-byte sine PCM loop replaces PSG tones and uses the same four reserved sound channels; no streaming or heap allocation is added. Runtime frame/stack/heap profiling and physical DS tests remain pending. See V29_COMBAT_POLISH.md and build/v29-polish-memory.txt.

# V29 loot and recruitment: current static measurement

ARM9 allocated section union: 820460 bytes; high-water offset: 820488. Game is 17708 bytes; Campaign is 21848 bytes. These static figures do not measure runtime memory or performance. The ROM builds; the existing SDK executable-stack metadata warning remains.

Runtime profiling, emulator input/audio/DLDI and physical DS acceptance remain pending. See V29_CHECKPOINT.md and build/v29-release-memory.txt. Earlier measurements below are historical.

# V29 surrender foundation: development measurement

Static ARM9 union: 810732 bytes; high-water offset: 810760. Game remains 17708 bytes and Campaign 21752 bytes. Current playable content is unchanged; unused surrender APIs may be removed by link optimization until campaign integration. This measurement does not estimate the future complete surrender runtime.

The ROM builds and fixture framebuffers were inspected. Runtime memory/frame timing, emulator input/audio/storage and physical DS acceptance remain pending. See V29_SURRENDER_CHECKPOINT.md.

# V28 retreat integration: current measurement

Static ARM9 allocation union: 810412 bytes; high-water offset: 810440. Game remains 17708 bytes; Campaign is 21752 bytes. Atomic travel uses one static Campaign staging buffer instead of placing it on the DS stack. See build/v28-retreat-memory.txt.

The ROM builds with the existing SDK executable-stack metadata warning. Host renderer previews are inspected separately from emulator behavior. Runtime stack/heap, frame pacing, emulator input/audio/DLDI, physical DS controls/audio/lid and interrupted-SD durability remain pending.

Earlier measurements below are historical.

# V28 intruder persistence: development measurement

Static ARM9 union 781028 bytes; high-water offset 781056. Game 17708 / Campaign 21748 bytes are unchanged. Two bounded save buffers increase from 8192 to 16384 bytes each. A compile-time check reserves the old 8192-byte payload bound plus a 6148-byte maximum carried appendix. The full 48-record fixture is 9864 bytes; an empty roster adds four bytes. Evidence: build/v28-persistence-memory.txt.

The ROM builds with the existing SDK executable-stack metadata warning. The restored last-slot intruder framebuffer was inspected and corrected to show no enemy contact. All 104 existing previews remain unchanged. Focused persistence and UI checks pass undefined-behavior trap instrumentation. Runtime stack/heap, frame timing, emulator input/audio/DLDI and physical DS acceptance remain pending.

# V28 intruder foundation: development measurement

Static ARM9 union 760740 bytes; high-water offset 760768. Game 17708 / Campaign 21748 bytes. The 48 extra Crew records add 4224 bytes per Game instance; total static union grows by 13056 bytes versus the charge foundation, including all static Game copies and code. Melee scratch storage now has 56 entries per faction. Evidence: build/v28-intruder-memory.txt.

The ROM builds with the existing SDK executable-stack metadata warning. All 104 existing previews match V27; a new last-slot home-intruder framebuffer was inspected. Four focused tests also pass undefined-behavior trap instrumentation. These checks do not measure DS stack/heap high-water, frame time, input/audio/DLDI or physical hardware; those gates remain pending.

# V28 charge foundation: development measurement

ARM9 static union 747684 bytes; high-water offset 747712. Game 13484 / Campaign 17524 bytes. The new transient charge counter adds four bytes to each Game instance. Evidence: build/v28-charge-memory.txt. The ROM builds with the existing SDK executable-stack metadata warning. All 104 regenerated renderer previews are byte-identical to V27; the escape charging framebuffer was also visually inspected. No new player jump controls are exposed yet.

Runtime stack/heap, frame timing, emulator input/audio/DLDI and physical DS acceptance remain pending. This development checkpoint does not complete retreat or a hardware gate.

# Latest release measurement: V27

ARM9 static union 747300 bytes; high-water offset 747328. Game 13480 / Campaign 17520 bytes. Evidence: build/v27-memory.txt. Five transient escape integers add 20 bytes. Existing bounded framebuffers/text buffers support the warning and countdown. Runtime stack/heap, frame pacing, emulator input/audio/DLDI and physical DS/SD durability remain pending. SDK executable-stack metadata warning persists.

# Latest release measurement: V26

ARM9 static union 742188 bytes; high-water offset 742216. Game 13460 / Campaign 17500 bytes. Evidence: build/v26-memory.txt. Two new transient event integers add eight Campaign bytes; the new hull and compiled event data remain bounded. Actual renderer previews fit station choices, wrapped reward hints and Rigger views. Runtime stack/heap, frame pacing, emulator input/audio/DLDI and physical SD durability remain pending. The SDK executable-stack metadata linker warning persists.

# Latest release measurement: V25

ARM9 static union 724836 bytes; high-water offset 724864. Game 13460 / Campaign 17492 bytes. Evidence: build/v25-memory.txt. Event narrative is bounded at 2048 bytes and source nodes are compiled constants; the runtime does not allocate event heap memory. Four-choice and paged screens fit inspected framebuffers. Stack/heap high-water, frame pacing, audio/input/DLDI and physical SD durability remain pending.

# Latest release measurement: V24

ARM9 static union 711548 bytes; high-water offset 711576. Game 13460 / Campaign 15688 bytes. Evidence: build/v24-memory.txt. Branch panels/maps fit inspected framebuffers. Physical input comfort, runtime memory/frame timing, audio/DLDI and SD durability remain pending.

# Latest release measurement: V23

ARM9 static union 708268 bytes; high-water offset 708296. Game 13460 / Campaign 15644 bytes. Evidence: build/v23-memory.txt. A 23-beacon overview fits the DS framebuffer; physical stylus/controller usability, runtime memory/frame time and SD durability remain pending.

# Latest release measurement: V22

ARM9 static union 698612 bytes; high-water offset 698640. Game 13460 / Campaign 14724 bytes. Evidence: build/v22-memory.txt. Runtime memory/frame time, emulator input/audio/DLDI and physical DS acceptance remain pending.

# Latest release measurement: V21

ARM9 static union 697412 bytes; high-water offset 697440. Game 13460 bytes; Campaign 14676 bytes. Evidence: build/v21-memory.txt. Runtime stack/heap, frame time, emulator input/audio/DLDI and physical DS validation remain pending.

# Latest development measurement: V21 drone foundation

ARM9 static union 687652 bytes; high-water offset 687680. Game 13460 bytes; Campaign 14516 bytes. Evidence: build/v21-foundation-memory.txt. Runtime and physical DS acceptance remain pending.

# Latest static measurement: V20

ARM9 allocated sections total 684484 bytes; high-water offset 684512 bytes. ARM ABI Game is 13360 bytes and Campaign 14416 bytes, already included in static accounting. Evidence: build/v20-memory.txt. Runtime stack/heap, frame time, input/audio and physical SD durability remain unverified.

# DS hardware baseline

## Measurement status

This is static build accounting and profiler documentation. It does not establish

that this build meets its frame budget on physical DS hardware. Host or emulator

timing is not a DS performance measurement.

Run `python tools/memory_report.py` after building. The script reads allocated

ELF sections and compiles `sizeof(Game)` and `sizeof(Campaign)` markers for the ARM ABI without running

the game. Debug sections and ELF file size do not measure runtime memory use.

The original DS has a 4 MiB main-RAM ceiling shared with system/ARM7 needs.

Two 256x192 RGB555 CPU buffers occupy 196,608 bytes in `.bss`. Two configured

256x256 bitmap VRAM banks occupy 262,144 bytes of separate VRAM. Do not subtract

VRAM from main RAM or count CPU buffers twice. `Campaign`, including `Game`, is static in `main` and already in `.bss`.

Reported type sizes must not be added to the ELF sum again. Temporary copies in

combat initialization/save validation add stack demand; stack high-water and

dynamic heap still need runtime observation. The ELF section union is not a free-heap measurement.

## Work timing integration

Call `performance_init()` once. Current integration times `campaign_step`,

`diagnostics_poll`, render, cache maintenance, and DMA together. Diagnostic writes

inside that span are deliberately included in the frame-work cost. VBlank waits

are outside the span. Every 600 display frames format a 160-byte buffer using

`performance_report(buffer,sizeof buffer)`, then `diagnostics_action(frame,buffer)`.

The report contains cumulative sample count and minimum/average/maximum elapsed

work in microseconds. Interrupt time is included. The 30 Hz update target gives

roughly 33.3 ms per update; that is a budget, not a measured result. DMA, cache maintenance, and diagnostic polling are included in the current span;

menu-triggered save work outside this span is not measured by these samples.

### Timer ownership

The bundled BlocksDS `libs/libnds/include/nds/timers.h` documents a 33,513,982 Hz

source (`BUS_CLOCK`), `cpuStartTiming(base)`, `cpuEndTiming()`, and

`timerTicks2usec()`. It reserves timer 2 as `LIBNDS_TIMER_SYSTEM_COUNTER` and

lists timer 3 as the ARM9 DSWiFi default.

This profiler therefore uses the documented timer 0/1 pair. It refuses to start

when either is already enabled. The application must reserve that pair while

profiling; future libraries needing them require revisiting timer ownership.

It leaves timers 2/3 untouched and installs no timer interrupts. The 32-bit timer

wraps after approximately 128 seconds; work spans must remain shorter. Host

builds provide no-op begin/end and explicitly report DS timing unavailable.

Local evidence: `nds/timers.h`, `nds/memory.h` (main RAM base), and

`sys/crts/ds_arm9.ld` (stack reservations and linker symbols). SDK linker support

includes DSi; its largest region must not be treated as original DS capacity.

## Physical DS acceptance still needed

Observe profiler output on the intended DS/flashcart through a long combat with

crew movement, both weapons, damage/repair, and diagnostic writes enabled. Record

worst work span and inspect input response. Static memory fit does not establish

frame pacing, stack safety, or filesystem compatibility.

## Provisional engineering budgets

These are project limits to guide additions, not measured allocations or hardware

claims. Revisit them after real-hardware logs and stack measurements.

| Resource | Proposed limit | Accounting |

|---|---:|---|

| ARM9 static sections | 1 MiB | Includes code, original art, Campaign, diagnostics and both CPU framebuffers |

| Dynamic asset cache | 512 KiB | Future decompressed/loaded assets; no whole PC archive in RAM |

| Audio working data | 128 KiB | Future stream double buffers and short effects; no full music tracks |

| Other temporary heap | 256 KiB | Save/load/filesystem temporary work, separate from static storage |

| ARM9 application stack high-water | 8 KiB target | Preserve room inside 16 KiB DTCM for IRQ/SVC/system use; inspect SDK configuration |

| DS SD save file | 8212 bytes V10 maximum | 8192-byte payload capacity plus 20-byte header; actual saves use less |

| Diagnostic log | 256 KiB current cap | Existing diagnostics stops file growth at its cap |

| SD free working allowance | 2 MiB minimum project target | ROM excluded; permits save/replacement/log overhead |

The main-RAM envelopes total less than 2 MiB before platform reservations; this

leaves headroom, not a promise of that much free heap. V10 audio uses synthesized PSG patterns without PCM buffers. The 128 KiB reservation remains available for a future streaming path; it is not a measured requirement. Save-file size and log growth follow source constants; filesystem cluster

allocation can consume more storage than file byte lengths.

Large automatic `Game`, `Campaign`, and 2048-byte save buffers can exceed the

stack target when calls nest. Recheck save verification and load call chains

whenever temporary state is added; static staging areas or bounded heap storage

must be accounted in their corresponding budgets instead of ignored.

## V8 integrated build result

The rebuilt ARM9 ELF reports **464020 bytes** in the union of main-RAM allocated sections (about453KiB), below the provisional1MiB static budget. High-water offset is464048 bytes. This includes the static Campaign and save staging buffers. CPU framebuffers196608 bytes are already included; VRAM262144 bytes is separate. ARM ABI Game is5412 bytes; Campaign5724 bytes. Runtime stack/heap high-water and actual update times remain unmeasured. The save module's large workspaces were moved to static storage after stack review; start_combat still uses a temporary Game on stack.

## V9 integrated build result

ARM9 allocated main-RAM section union: **568156 bytes** (about 555 KiB), below the provisional 1 MiB static budget. High-water offset: 568184 bytes. Game: 5420 bytes; Campaign: 5732 bytes. The increase includes imported UI/background art. CPU framebuffer and VRAM sizes are unchanged. Runtime stack/heap, frame work timing, touch comfort and SD reliability remain unmeasured in this revision.

## V10 campaign build â€” current

Final ARM9 allocated main-RAM section union: **571916 bytes** (about 559 KiB).

Main-RAM high-water offset: 571944 bytes. ARM ABI Game: 5784 bytes;

Campaign: 6692 bytes. The two CPU framebuffers remain 196608 bytes inside BSS;

VRAM bitmap banks remain 262144 bytes outside main RAM. This is below the

provisional 1 MiB static envelope.

Save staging is static and bounded; encounter creation now updates enemy state

in place instead of copying an entire Game on the DS stack. Campaign generation

and event dispatch use fixed arrays. V10 save payload capacity is 8192 bytes

plus a 20-byte header, with a separate campaign filename. Synthesized audio uses

four sound channels and no PCM buffers or ARM9 timers.

Build passed with clean application compilation and the existing SDK startup

GNU-stack warning. Runtime stack/heap high-water, update timings, audio output,

touchscreen behavior and SD durability remain pending. The emulator executable

started as a process, but no current live framebuffer was independently observed.

## V11 feedback/Assault checkpoint

Current ELF main-RAM allocated section union593964 bytes; high-water593992. ARM ABI Game6636 and Campaign7544 bytes, measured before final presentation-only edits. CPU framebuffer196608 and separate VRAM262144 unchanged. This includes fire state and one enemy drone descriptor plus original Assault art. Still below the provisional1MiB static envelope. Runtime timing/heap/stack and physical DS verification remain pending. Final section totals were independently parsed after the full memory helper stalled at nm; see V11_CHECKPOINT.md.

## V12 named crew and living enemies

Current ARM9 allocated main-RAM union: **624164 bytes**, high-water offset 624192. ARM ABI Game is 9604 bytes and Campaign is 10512 bytes. These state types are already included in static allocations, not additional RAM. CPU framebuffer196608 and separate VRAM262144 are unchanged. Enemy roster, door graph and original Fighter art account for the increase. Below the provisional1MiB static envelope; this does not measure free memory or runtime performance. The memory report now parses compiled ELF symbol sizes directly and completed successfully. Frame timing, stack/heap high-water and physical DS checks remain pending.

## V13 static checkpoint

October 3, 2026: ARM9 main-RAM allocated section union 638740 bytes; image high-water offset 638768. ARM ABI Game 9748 bytes and Campaign 10660 bytes (already included in static allocations). ROM 444416 bytes. See build/v13-memory.txt and docs/V13_CHECKPOINT.md. Runtime stack/heap, frame timing and physical DS validation remain pending.

## V14 static checkpoint

October 3, 2026: ARM9 main-RAM allocated union 644156 bytes; image high-water offset 644184. ARM ABI Game 10584 bytes and Campaign 11496 bytes (already included). ROM 447488 bytes. See build/v14-memory.txt and V14_CHECKPOINT.md. Runtime frame timing, stack/heap and physical DS validation remain pending.

## V15 static checkpoint

October 3, 2026: ARM9 main-RAM allocated union 653292 bytes; image high-water offset 653320. ARM ABI Game 11784 bytes and Campaign 12696 bytes (already included). ROM 452608 bytes. See build/v15-memory.txt and V15_CHECKPOINT.md. Runtime memory/frame timing and physical DS validation remain pending.

