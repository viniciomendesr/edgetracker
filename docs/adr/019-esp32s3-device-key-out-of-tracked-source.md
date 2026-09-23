# ADR-019: Move the ESP32-S3 device private key out of tracked firmware source

**Date:** 2026-09-23

**Status:** Accepted

## Context

`ESP/esp32s3/esp32s3.ino` had `DEVICE_PRIVATE_KEY`, its matching `DEVICE_PUBLIC_KEY`, and the derived `DEVICE_SOURCE` string hardcoded as `static const` arrays, introduced in commit `8c5d73b` (2026-04-25). The repository is public on GitHub, so the private key has been readable by anyone since that commit — it is compromised regardless of what happens to the working tree, because git history keeps every past revision.

The key was not yet cryptographically trusted: `UNSIGNED_DEV_BYPASS` (ADR-011) is `1` on this node, so `signCanonical()` is compiled but never called, and the backend accepts `signature: "unsigned_dev"` keyed only by the `source` field. That limited the blast radius to "anyone can forge signed envelopes claiming this device's identity the day the bypass is turned off," not "anyone can forge them today." It does not make hardcoding the key in tracked source acceptable — the fix is needed before signing is ever enabled, not after.

### Options evaluated

| Option | Effort | Matches existing pattern | Leaves signing bypass alone |
|---|---|---|---|
| 1. Gitignored `secrets.h`, key supplied at build time | Small | Yes — identical to the Wi-Fi credential flow ADR-adjacent commit `d2cfe51` already established | Yes |
| 2. On-device key generation persisted in NVS (`Preferences`), mirroring the ESP8266 firmware's EEPROM flow | Larger | Partially — ESP8266 already does this, ESP32-S3 does not | Yes |

Option 2 is the better end state: it removes the key from every build artifact and every developer's disk, generating it once per physical chip instead of per checkout, matching `ESP/esp8266/esp8266.ino`'s `loadPrivateKey()`/`savePrivateKey()`/`uECC_make_key()` pattern. But it also requires rebuilding `DEVICE_SOURCE`, which is currently a compile-time literal built to avoid runtime string concatenation on a board already tight on flash (see the `MODEL_ID`-adjacent comment on arena size patches), into a runtime-built string from the generated pubkey, plus wiring `Preferences` and a reset/re-provision path equivalent to the ESP8266's `RESET` serial command. That is a firmware behavior change under time pressure before a public demo (SIICUSP), not a drop-in fix.

## Decision

Move `DEVICE_PRIVATE_KEY`, `DEVICE_PUBLIC_KEY`, and `DEVICE_SOURCE` into the existing gitignored `ESP/esp32s3/secrets.h` (option 1), alongside `WIFI_SSID`/`WIFI_PASSWORD`. `secrets.example.h` documents the three symbols with zeroed placeholders and the `openssl ecparam`/`openssl ec` commands to generate a fresh keypair, plus the reminder that a key which was ever committed must never be reused, compromised permanently by history rather than by the current file state.

Option 2 (on-device generation + NVS) is deferred as the target for a future ADR when the ESP32-S3 firmware next gets a maintenance pass, referencing this one.

## Consequences

### Positive

- No private key material in tracked source or in any future commit on this file.
- Same mental model as Wi-Fi credentials: one `secrets.h` per board, one `.gitignore` rule, one `cp secrets.example.h secrets.h` step.
- Firmware still builds with no logic changes — `signCanonical()`, `UNSIGNED_DEV_BYPASS`, and the publish path are untouched.

### Negative

- The key still lives in plaintext flash on the physical chip (no secure boot on this board), so `secrets.h` isolation fixes the repository-leak vector, not a physical-access vector. Documented as a follow-up to mainnet hardening, same as before.
- Every developer who builds this firmware now needs their own keypair; if the ESP32-S3 node in current physical use was flashed with the compromised key, it needs a fresh key and a re-flash before signing is ever enabled, independent of this source change. Tracked separately as an operational rotation task, not a code change.

### Follow-up

- On-device key generation in NVS (`Preferences`), mirroring `ESP/esp8266/esp8266.ino`, including a runtime-built `DEVICE_SOURCE` and a `RESET`-equivalent re-provision path.
- Confirm which pubkey is currently registered as `devices.public_key` for this node's sensor record, and re-run "Rotate Key" after any re-flash with a new key.

## References

- [ADR-011](011-unsigned-dev-bypass-for-unported-devices.md) — why signing is currently bypassed on this node
- [ADR-014](014-deferred-nft-minting.md), [ADR-016](016-user-paid-mint-on-mainnet.md) — device identity path B, referenced in the firmware comments this ADR touches
- `ESP/esp8266/esp8266.ino` — the on-device key generation + EEPROM persistence pattern option 2 would port
- `ESP/esp32s3/secrets.example.h` — the template this decision extends
