# V63 original-style replies and the DS save fix

## Completed
- **Event replies without boxes (Blite's request).** All replies sit in one original-style event window as numbered lines ("1. ..."). Their resource changes are shown in cyan underneath.
  - **Colours:** white normally, blue for blue (requirement) options, grey when unavailable.
  - **Selection:** the selected reply turns yellow and gets a yellow marker.
  - Long replies wrap with an indent, and the touch areas are unchanged.
- **DS save fix, found in melonDS.** BlocksDS's `nitroFSInit()` (called by `audio_init`) switches the current directory to the read-only `nitro:/` drive.
  - **Effect:** every write after start-up (campaign save, profile, log flushes) went to NitroFS and failed with "Save failed. Check SD storage". This would have happened on real hardware too.
  - **Fix:** the start-up directory is now restored after NitroFS initialises, in `main.c` and in the art loader's fallback init.
  - **Verified in melonDS 1.1** (DLDI, SD folder sync): boot, new run (hangar → crew → map), "Campaign saved", `/ftl-ds-campaign.sav` written to the card, relaunch shows "Saved campaign available", and Continue resumes the run. Combat runs at 60/60 fps.
- **Quicker checks.** `tools/check.ps1` runs a quick set by default: it skips the 14 long scripted full-campaign bot runs (129 checks, about 5.5 minutes). `-Full` runs everything; that's planned once, before the final package.

## Evidence
- **Results:** 129 quick host checks and the event UI checks passed.
- **ROM:** 43,701,248 bytes. SHA-256 `188ae83376a608d235a50bcc078dbfa7d6c6714f904317f042b9e923deb9379e`. Audio is byte-exact.
- **Memory:** the static ARM9 union is 2,189,804 bytes.
- **Previews inspected:** `v62-long-choices`, `v53-event-many` and `v53-event-blue` (all `-v63`).

Recovery: releases/v62/ftl-ds.nds. Current ROM: releases/v63/ftl-ds.nds.
