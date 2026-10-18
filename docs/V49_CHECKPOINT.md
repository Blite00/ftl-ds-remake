# V49: a dead flagship gunner silences the room for good

Blite chose the fully silent option, because the original flagship artillery rooms have no doors.

## Completed
- The V48 replacement transfer is removed: the 10 s delay, the `MAN<n>` countdown and the gold blink.
- A dead gunner now silences their artillery room for the rest of the stage, the same as V47. The other artillery rooms keep firing.
- With sensors, the enemy weapon line shows `<kind> UNMAN`, as in V46/V47.
- The AI-run ship with no crew left still counts as manned.
- The V48 test-bot improvements are kept:
  - the healthiest crewmember repairs critical systems;
  - the bot fights boarders between fights;
  - the gunner leaves an airless weapons room.
- The station-manning audit in [V48_CHECKPOINT.md](V48_CHECKPOINT.md) still applies, except that flagship artillery now has no replacement.
- The game state is back to the V47 size: Game is 19,456 bytes and Campaign 23,748 bytes. There is no save format change.

## Evidence
- **Tests:** 132 host checks and 45 Python tests passed (build/v49-check.log).
  - `v46_combat_smoke` now checks that a sealed artillery room stays silent for 20 s after its gunner dies, while the other rooms stay manned.
- **TUNED balance:**
  - `flagship_final_smoke`: the strong DS build wins 10/9/10 stages of 10. The authentic-normal row is 9/0/0.
  - Eight-sector runs: 11 flagship engagements, 6 stage wins.
- **ROM:** 41,003,008 bytes. SHA-256 `c856975ad635ffb5c4454fdf1f0de376819f153d813ae2de352f4ffc87906779`. The only build warning is the existing SDK one.
- **Memory:** the static ARM9 union is 996,892 bytes. All 17 audio files are byte-exact.
- **Preview:** `v49-artillery-silent-v49.png` (shows `LAS UNMAN`).

Recovery: releases/v48/ftl-ds.nds. Current ROM: releases/v49/ftl-ds.nds. Snapshot: checkpoints/v49-silent-gunners.zip and build/v49-manifest.json.
