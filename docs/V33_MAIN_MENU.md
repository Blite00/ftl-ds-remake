# V33 main menu

The ROM starts at a title menu with the locally extracted Kestrel image. It validates the existing campaign file and its backup without starting simulation. Continue is enabled only when the existing loader accepts a checkpoint. A missing or invalid save, or unavailable storage, leaves Continue disabled; New Run remains available.

- Up/Down selects; A activates. Touch activates the same large rows.
- Continue loads the already validated checkpoint into play.
- New Run starts a seeded Kestrel campaign. If a saved or live run exists, a confirmation defaults to Cancel. Starting a new run does not delete or write save files. A later explicit map SAVE replaces the checkpoint under the existing backup rules.
- Controls shows a concise map/combat button guide; A/B or the bottom button returns.
- Sound cycles OFF/LOW/MED/HIGH for the current session. This setting is not persisted and the menu is silent.
- Start on the normal map opens the menu with Resume Run. B resumes the same live campaign. Combat Start retains Actions.
- From victory/defeat, A/Start or the bottom button returns to the main menu and rechecks storage for a valid checkpoint.

The main loop bypasses campaign stepping, UI timers and gameplay audio while the menu is active. The menu state machine itself cannot write campaign state or storage. Campaign save format and migration rules are unchanged. The current loader reports validity as a boolean, so the menu cannot distinguish missing saves from storage failures; it displays that limitation explicitly.

`main_menu_smoke` covers disabled Continue, input paths, confirmation/cancel, no implicit save replacement, missing storage, primary corruption with valid backup and framebuffer boundaries. Strict host and UBSan checks passed. Physical DS input, appearance, audio and storage validation remain pending.
