# V33 input response

## Source findings

The previous DS loop scanned keys only after returning from synchronous rendering and other main-loop work. `keysDown()` compares two sampled held states. A release and repress between those scans can therefore disappear, making separate quick taps look like one hold. This is a verified failure mechanism in that polling design, not a measurement proving the cause of every reported delay on hardware.

`tap_ticks` and `feedback_ticks` only control visual feedback; neither blocks taps. Simulation and ordinary rendering run every other loop iteration. Input originally sampled each loop, but its visible result could wait for the next rendering iteration. Periodic diagnostics also wrote synchronously to the SD card during play.

## Changes

- Sample held keys and cached touch coordinates every VBlank, independent of main-loop rendering. An eight-entry bounded queue retains separate press edges and the original contact coordinates. Main-loop consumption still dispatches one input at a time.
- Holds never repeat. The next sampled release/repress is eligible immediately; there is no debounce cooldown. Overflow drops newest extra events, and events older than 30 samples (about half a second at normal VBlank cadence) expire instead of executing late.
- Present immediately after consumed input, including on the simulation's off iteration. Simulation rate and weapon timings are unchanged.
- A changed screen, modal, confirmation, event or targeting space discards queued presses from the old view. Capture stays blocked only while that new view is being drawn/presented; held-state sampling continues, requiring a fresh press after the transition. Ordinary room/crew/menu selection, power adjustments, resources and feedback do not form a barrier. Selection on the zoomed map does because it moves beacon coordinates.
- Gameplay diagnostic polling now keeps its bounded memory log without automatic SD writes. Explicit Save, diagnostic-view access and entering the main menu flush the log. Older entries can be dropped and the most recent log can be lost on a crash before a flush; saved campaigns retain their existing persistence behavior. Host logging retains its old automatic-flush default unless explicitly opted into deferral.

## Evidence and limits

Strict C99 and undefined-behavior-trap tests exercise eight quick contacts while consumption is delayed, press-coordinate preservation, held-touch suppression, modal barriers, overflow, expiration and counter wrap. Context tests verify ordinary selection/power/feedback does not invalidate input, while new confirmations/events/camera/targeting spaces do. The logger regression covers deferred polling versus explicit persistence.

The installed BlocksDS `libnds9.a` disassembly was inspected: `scanKeys` performs bounded cached-data/register work and restores its previous interrupt state; `touchRead` copies 12 cached bytes and restores its previous interrupt state. Neither waits on touchscreen hardware, FIFO, storage or another interrupt. The VBlank handler performs only this sampling and bounded queue work; no renderer, filesystem, diagnostics or game actions execute there. Main queue access uses short critical sections.

This does not establish actual hardware latency, frame time or SD performance. Very short contacts/releases between two VBlank/ARM7 samples can still be missed. Slow rendering can still delay visible response, even though sampling now continues. Physical DS checks: rapid separate taps, long holds, power adjustment, successive room orders, map selection, reward/sale/new-run confirmation, menu return and SD save remain pending.
