# V29 surrender implementation checklist

Goal: a source-backed surrender offer can interrupt a live encounter, let the player accept or refuse, preserve boarding identities, apply its exact supported consequence once, and never become a destruction/escape reward.

## Dependency order

- [x] Core atomic ceasefire and negotiated crew exchange, with separate surrender outcome.
- [x] Strict original surrender descriptor/closure compilation, retaining all branches and weights.
- [ ] Generic original REBEL dependencies: larger weighted text lists, reward families, crew reward capacity, weapon reward/overflow UI, chance-based escape defaults and overlapping trigger policy.
- [ ] Campaign offer state, once-only trigger, frozen choices, accept/refuse callbacks and source ownership.
- [ ] Touch/button offer presentation, reward preview, boarding consequences and rejection feedback.
- [ ] Persistence policy and old-save compatibility for new content rules.
- [ ] Earned surrender campaigns, cold loads, affected renderer inspection and ROM checkpoint.
- [ ] Emulator input/audio/storage and physical DS acceptance.

## Source scope

The small ENGI_UNLOCK_2FAKE closure belongs to an unfinished quest. Compiling it is useful importer evidence but does not justify placing it in the random encounter pool. Keep the nine current roots unchanged until a complete context-faithful addition is supported.

Generic REBEL is a coherent standalone target. Its rare crew and weapon reward branches, weighted surrender text list, random reward family and optional escape behavior are required dependencies; do not discard branches to make it compile.

Weighted text lists now support 32 entries without removing duplicates. The generic encounter remains disabled while loot, crew acquisition, chance-based escape and campaign offer handling are incomplete.

## Core contract and authored adaptation

Acceptance is a negotiated ceasefire, represented separately from victory, defeat and escape. Both hulls remain intact. Living friendly boarders return to legal home positions; living boarders owned by this enemy return to its legal positions. Existing carried intruders belong to earlier encounters and remain hostile. Preserve identities, names, health, species and skills; do not resurrect dead crew. Cancel queued attacks and further combat clocks. Preflight capacity and all record ownership before mutation.

This automatic exchange, destination fallback, projectile cancellation and treatment of pending remote deaths are explicit DS adaptations. The XML specifies hostility changes and consequences; it does not establish these engine behaviors. Record pending home death effects without moving their damage to the remote ship. The source-driven offer is not playable until campaign and UI integration are complete.
