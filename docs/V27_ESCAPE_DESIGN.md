# V27 source audit: warning scout escape

## Scope and provenance

Read-only audit of the installed `ftl.dat`, using `tools/import_ftl.py` and its XML parser. The installed archive remains untouched. Source files: `data/events_rebel.xml`, `data/events_ships.xml`, `data/autoBlueprints.xml`, and `data/text_events.xml`. This document records source facts and the compiler implementation; it does not establish original executable behavior or DS runtime test success.

## Verified source closure

- Root `AUTO_WARNING` is unique. It loads the `REBEL_AUTO` text list and hostile ship `REBEL_AUTO_WARNING`.
- `REBEL_AUTO` contains exactly nine English text references, `text_REBEL_AUTO_1` through `_9`. All describe hostile automated scouts. No choices, costs, blue requirements or embedded rewards appear in these introductions.
- Ship `REBEL_AUTO_WARNING` loads `SHIPS_AUTO`. Its exact members are `AUTO_BASIC` and `AUTO_ASSAULT`, both already supported enemy hulls.
- The ship has exactly three outcome elements: `escape`, `gotaway`, and `destroyed`. There is no `deadCrew`, surrender or recruitment closure.
- Escape attributes are exactly `timer="40" min="22" max="22"`. Its event contains the original warning text and `<ship hostile="true"/>`.
- The warning says the ship starts powering its FTL drive and will warn the fleet if it escapes. It supplies no charge duration, engine level formula or interruption rules.
- `gotaway` has original narrative followed by `<modifyPursuit amount="1"/>`. It contains no salvage.
- `destroyed` has a distinct original narrative and `<autoReward level="LOW">standard</autoReward>`. Existing DS LOW-standard payout remains an authored distribution, not recovered reward mathematics.

## What the XML does not prove

The archive provides no executable definition of escape timer units, timer-versus-hull precedence, probability sampling, charge speed, engine/piloting interruption, damage reset, crew loss precedence, or boarded-crew fate. Do not label these verified original mechanics merely because an XML attribute was imported.

A pattern comparison suggests `min/max` is an absolute hull threshold: ordinary pirates use 2..4, Rebels 3..4, forced fuel encounters 30..30, and REBEL_AUTO_BAIT uses timer2000 with 3..4. This is an inference, not engine evidence. The warning wording suggests an initiation event before actual departure. Any chosen DS timing, trigger ordering and charge policy must be documented as an intentional approximation pending executable verification.

## Implemented bounded compiler extension

Preserve the first eight root indices; append AUTO_WARNING at index8. Do not admit other escape roots merely because they share one supported tag.

Extend ImportedEventShip with raw `escape_timer`, `escape_min`, `escape_max`, `escape_node`, and `gotaway`. Absent escape has sentinel node -1 and no active timing. Compile escape narrative separately from the terminal gotaway/destroyed callbacks. Compile its hostile=true action in an existing-ship context; it must never spawn a second ship or return to the ordinary Engage stage.

For initial enablement, accept only the exact deterministic escape form required here: explicit timer1..3000 and hull min/max1..1000 with min<=max, one warning node with text and current-ship hostile=true, exactly one gotaway closure, and no chance/load/unknown attributes. Validate all other ship outcomes recursively and reject unsupported branches. Enforce duplicate/missing callback errors, finite numeric bounds and inverted range rejection. Avoid guessing defaults for missing attributes.

ImportedEventNode appends a separate pursuit integer. It cannot index the existing five-resource arrays. modifyPursuit requires exactly amount1..10, no children or text, no unknown attributes and at most one effect per node. Negative/zero amounts remain rejected until their semantics are supported.

All nine intro alternatives must remain available and preserve exactly one shared ship descriptor. Unsupported text/ships/cycles/branches must fail the entire root, as in V26.

## Runtime acceptance requirements

- Show the initial original narrative before Engage, then warn once when escape begins.
- Pause freezes relevant simulation clocks. Interrupted charge and its UI must agree.
- Engine/pilot disable and restoration affect escape according to the documented DS policy.
- Player defeat and enemy destruction/crew elimination have deliberate precedence against departure on the same update.
- Successful escape applies pursuit exactly once, with no generic salvage and no hull explosion.
- Away crew have explicit departure handling; player-controlled boarded crew cannot silently become available on the home ship.
- Other encounters retain their old behavior, including original station rewards.
- Old content policies exclude the new root; new saves retain pursuit and root uniqueness.
- Inspect actual renderer warning/countdown/blocked/departure/outcome frames, run full campaigns and cold-load tests, and build the ROM. Physical and emulator acceptance remain separate.

## Selected DS policy and compiler evidence

The runtime implementation uses a hull threshold sampled from raw min/max to latch charging. Raw timer is interpreted as seconds of operational engine/piloting time; charge pauses while those systems are disabled or ionized. These are explicit DS approximations, not verified original executable behavior. Parent integration owns runtime implementation and evidence.

The compiler appends AUTO_WARNING at root8, preserves prior roots, and emits schema3 with9 roots,66 nodes,37 choices and3 ship descriptors. The new root preserves all9 original introductions and its exact two-enemy pool. Warning, gotaway and destroyed callbacks remain distinct.

All28 Python compiler tests pass, including invalid escape bounds, unsupported chance semantics, incomplete/duplicate escape callback pairs, unsupported warning effects, malformed pursuit and complete local catalog reachability. Generated header matches the catalog exactly. This evidence does not establish runtime escape correctness or physical DS behavior.

## Integration behavior

The warning freezes the Game state and uses a synthetic Continue to return to the same encounter. Successful departure selects gotaway once; hull destruction selects destroyed. Pursuit advances the existing sector fleet-turn counter, preserving the distinct campaign travel counter. Old event policies exclude the new root and save20 preserves their exact payload layout.

Remote crew are lost with identity retained and no species death bursts. Hostile boarders and home hazards remain in the shared combat simulation. Remote fire rejects before ammunition/charge mutation; home-target bombs still launch and resolve through normal effects. Queued/in-flight attacks are cleared at the instant of departure as an explicit DS adaptation. Final evidence is recorded in V27_CHECKPOINT.md.
