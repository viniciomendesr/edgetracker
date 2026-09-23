# Timeline — Sparked Sense

Chronological record of the project's evolution. Architectural decisions are documented separately in [ADRs](adr/README.md).

Format inspired by [Keep a Changelog](https://keepachangelog.com/).

---

## Phase 1 — Foundation and UI (30-31 Oct 2025)

**Contributors:** Vinicio Mendes, Nicolas Gabriel, Figma Bot

### Added
- Initial commit with Figma Make import (93 files, ~19,500 lines): React + Vite frontend, Radix UI components, Tailwind styling, Recharts, Supabase Edge Function with KV store
- Supabase project setup (`djzexivvddzzduetmkel`): `kv_store` table, Edge Function `server` (Hono + Deno), Supabase Auth
- Vercel deployment as `sparkedsensemvpv1` at `sparkedsensemvp.vercel.app` (Vite SPA)
- Environment variables on Vercel: `VITE_SUPABASE_URL`, `VITE_SUPABASE_ANON_KEY`

### Changed
- Nicolas Gabriel: Supabase integration fixes, build/deploy adjustments with pnpm (2 PRs merged from `nicolas` branch)
- Content adjustments, favicon, README cleanup

> Stack decisions documented in [ADR-001](adr/001-stack-and-infrastructure.md)

---

## Phase 2 — Refinement and documentation (Nov-Dec 2025)

**Contributors:** Vinicio Mendes, Pedro Goularte

### Added
- Pedro Goularte: mission statement in README
- Pedro Goularte: `sparked-three` Vercel project as Solana integration test environment (Token + Wallet Address page, `SERVER_SECRET_KEY_BASE58` for devnet NFT minting)
- Pitch video link in README

### Fixed
- Typography and heading capitalization on HomePage

---

## Phase 3 — Architecture refactoring (09 Mar 2026)

**Contributor:** Vinicio Mendes (with AI assistance — Claude)

### Removed
- 2,186 lines of duplicated/unused code: `kv_store.tsx`, `deviceRegistry.ts`, `redis.ts`, `solanaService.ts`, `supabaseClient.ts`, and legacy SQL migration from `src/supabase/`

### Changed
- API route standardization: removed versioning from paths (`/server/v1/*` → `/server/*`)
- Updated `api.ts` (frontend) and `kv_store.ts` (backend) to match new routes
- Redeployed Edge Function `server`

> Backend unification documented in [ADR-002](adr/002-unified-backend-edge-functions.md)

---

## Phase 4 — IoT hardware integration + DePIN (09-10 Mar 2026)

**Contributor:** Vinicio Mendes (with AI assistance — Claude)

### Discovered
- Next.js API routes in `app/api/` were dead code — project uses `vite build`, `next` is not a dependency
- `sparked-three.vercel.app` was a test page, not the application backend
- No functional backend existed for IoT device communication

### Added (database — 10 Mar)
- PostgreSQL table `devices`: `publicKey` PK, `macAddress`, `nftAddress` UNIQUE, `txSignature`, `lastTsSeen`, `revoked`, `challenge`, `ownerAddress`, `claimToken` UNIQUE, `is_mock`, `mock_sensor_type`, `mock_private_key`
- PostgreSQL table `sensor_readings`: `id` UUID PK, `nft_address`, `timestamp`, `data` JSONB
- RLS disabled on both tables; `GRANT ALL` to `anon`, `authenticated`, `service_role`

### Added (Edge Function — 10 Mar)
- `@noble/curves` via esm.sh for secp256k1 verification (replacing `npm:elliptic` which caused BOOT_ERROR on Deno)
- `POST /server/register-device`: two-step challenge-response with secp256k1 signature verification
- `POST /server/sensor-data`: reading ingestion with cryptographic verification, 55s rate limit via `lastTsSeen`, dual write to PostgreSQL + KV store
- Bridge between KV store and PostgreSQL: `sensor-data` route resolves sensor by `claimToken` and writes in dashboard-compatible format

### Added (ESP8266 firmware — 10 Mar)
- `ESP/ESP.ino`: DHT11 sensor on pin D2, secp256k1 key generation/persistence in EEPROM, challenge-response registration, cryptographic signing of canonical JSON payloads, 60s send interval
- Endpoints pointed to Supabase Edge Function with `Authorization: Bearer <anon_key>`
- WiFi networks tested: `firetheboxv2`, iPhone hotspot, `MVISIA_2.4GHz` (Inova USP). `eduroam` incompatible (WPA2-Enterprise not supported by ESP8266)

### Removed
- Legacy tables (`devices`, `sensor_readings`, `datasets`, `audit_logs`) with incompatible schema (snake_case vs camelCase)

### Verified
- End-to-end test successful: DHT11 → ESP8266 → WiFi → HTTPS → Edge Function → secp256k1 verification → PostgreSQL + KV store → dashboard with live chart. Sensor status changed from "Inactive" to "Active"

> Signature verification documented in [ADR-003](adr/003-secp256k1-signature-verification.md)
> Storage architecture documented in [ADR-004](adr/004-dual-layer-storage.md)

---

## Phase 5 — Blockchain identity reset (10 Mar 2026)

**Contributor:** Vinicio Mendes (with AI assistance — Claude)

### Changed
- New Solana devnet wallet via Phantom Wallet: `6RuAxerE8GsMziM4c77ZzakfMAiebSfTE3LX4S1EyMNn` (10 SOL airdrop)
- Removed old Solana secret key (previously owned by Pedro Goularte)

### Removed
- Full database cleanup: `DELETE FROM` on `sensor_readings`, `devices`, `kv_store_4a89e1c9`, and Supabase Auth users

### Added
- Fresh device registration after ESP8266 EEPROM reset: new NFT address `b1f7dbeb...`, claim token `ed52b6ee...`
- First post-reset reading: 24.7C / 83% humidity — HTTP 200

### Fixed (frontend reactivity)
- `sensor-detail.tsx`: added 15s polling for real sensors via `readingAPI.list()` with immediate call on mount
- `dashboard.tsx`: sparkline differentiation between mock (local 2s) and real (API 15s) sensors; global polling at 30s as Realtime fallback
- `api.ts`: added `Cache-Control: no-cache, no-store` header to prevent stale responses

> Identity reset documented in [ADR-005](adr/005-blockchain-identity-reset.md)

---

## Phase 6 — CORS fix and Edge Function stability (10 Mar 2026)

**Contributor:** Vinicio Mendes (with AI assistance — Claude)

### Fixed
- Homepage "Unable to Load Featured Sensors" error: `Cache-Control` header added in Phase 5 was not whitelisted in CORS `Access-Control-Allow-Headers`, causing browser preflight rejection
- Added `Cache-Control` to CORS whitelist in 4 locations in Edge Function
- Redeployed Edge Function `server` (v9)

### Changed
- `.gitignore`: added `.vscode`; removed `.claude/worktrees` and `.vscode` from git tracking

> CORS decision documented in [ADR-006](adr/006-cors-header-whitelisting.md)

---

## Phase 7 — Documentation restructure and blockchain planning (10 Mar 2026)

**Contributor:** Vinicio Mendes (with AI assistance — Claude)

### Changed
- Project documentation restructured into `docs/` directory: `timeline.md` (Keep a Changelog format) and `docs/adr/` (Nygard ADR format) with index table
- Legacy Next.js routes (`app/`) moved to `_reference/` with migration status README — these were dead code (project uses Vite, not Next.js)
- Root `TIMELINE.md` replaced by `docs/timeline.md`
- Timestamps (HH:MM) added to all ADR decision dates and index

### Removed
- Dead frontend code: `src/lib/websocket.ts` (61 lines, never used), `src/lib/supabaseClient.ts` (3 lines, duplicate of Edge Function client)
- Dead backend stubs from `supabase/functions/server/lib/`: `deviceRegistry.ts` (204 lines), `redis.ts` (148 lines), `solanaService.ts` (168 lines), `supabaseClient.ts` (28 lines) — all unused legacy stubs with incompatible interfaces

### Added
- ADR-007: Fix Merkle tree and define on-chain schema before blockchain integration — establishes priority order: fix Merkle tree → define NFT metadata schema → define anchoring format → implement Solana integration. Defers MQTT migration and send interval reduction
- ADR-008: Solana devnet over testnet — standardizes on devnet for development and academic validation, skipping testnet entirely (devnet → mainnet path)

> Blockchain planning documented in [ADR-007](adr/007-merkle-tree-before-blockchain.md) and [ADR-008](adr/008-solana-devnet-over-testnet.md)

---

## Phase 8 — Binary Merkle tree with inclusion proofs (10 Mar 2026)

**Contributor:** Vinicio Mendes (with AI assistance — Claude)

### Changed (backend — Edge Function)
- Replaced linear hash (`SHA-256(concat(all_hashes))`) with proper binary Merkle tree: pair-wise hashing, odd leaves duplicated, domain-separated leaf nodes (`SHA-256(readingHash)`)
- Empty tree root is now deterministic: `SHA-256('') = e3b0c44...` (was `crypto.randomUUID()`)
- Deterministic sort before tree construction: readings sorted by timestamp ascending, then by ID as tiebreaker
- New module `supabase/functions/server/lib/merkle.ts`: `buildTree()`, `generateProof()`, `verifyProof()`, `sha256Hex()`
- Replaced all 6 callsites of old `calculateMerkleRoot` in `index.ts`
- Enhanced `GET /sensors/:id/hourly-merkle` and public equivalent to return `leafCount` and `leaves[]`
- Enhanced `POST /verify/merkle` with inclusion proof mode (`leafHash + proof + merkleRoot`)
- New endpoint `GET /sensors/:id/merkle-proof/:leafIndex` (authenticated)
- New endpoint `GET /public/sensors/:sensorId/merkle-proof/:leafIndex` (public)

### Added (frontend)
- `src/lib/merkle.ts`: browser-side verification via Web Crypto API (`verifyMerkleProof()`, `verifyMerkleRoot()`)
- `src/lib/types.ts`: `MerkleProofStep`, `MerkleProofData` types
- `src/lib/api.ts`: `merkleAPI.getProof()`, `publicAPI.getPublicMerkleProof()` methods

### Changed (frontend)
- `sensor-detail.tsx`: replaced fake `handleVerifyMerkle` (setTimeout + toast) with real client-side tree reconstruction and root comparison
- `public-sensor-detail.tsx`: same real verification for public sensor view
- `audit.tsx`: replaced 3 fake verifications (Quick Verify, Merkle root input, single hash input) with real cryptographic verification; updated "How to Verify Independently" instructions to describe binary Merkle tree algorithm

### Fixed
- Removed Deno-only JSR packages (`@jsr/std__crypto`, `@jsr/supabase__supabase-js`) and `hono` from root `package.json` — these caused `yarn install` failure on Vercel (JSR registry unreachable from npm)

> Merkle tree decision documented in [ADR-007](adr/007-merkle-tree-before-blockchain.md)

---

## Phase 9 — Bug fixes and KV store pagination (16 Mar 2026)

**Contributor:** Vinicio Mendes (with AI assistance — Claude)

### Fixed (frontend)
- SVG sparkline rendering error (`<polyline> attribute points: Expected number`): replaced percentage-based coordinates (`"0%,100%"`) with numeric values and `viewBox="0 0 100 100"`; added `vectorEffect="non-scaling-stroke"` for consistent stroke width
- Division-by-zero guard: sparkline now requires `liveData.length > 1` (was `> 0`), preventing `NaN` when only one data point exists

### Fixed (backend — Edge Function)
- `GET /sensors/:id` returning 500: Merkle tree computation and dataset aggregation wrapped in individual try-catch blocks so a failure in either no longer crashes the entire endpoint (returns `hourlyMerkleRoot: null` gracefully)
- Same defensive handling applied to `GET /public/sensors/featured`
- **KV store 1000-row ceiling:** `getByPrefix()` was hitting Supabase JS client's default 1000-row limit, silently truncating readings, stats, and Merkle trees. Replaced single query with paginated `.range()` loop in 1000-row pages until all rows are fetched

### Removed
- Stale git worktrees (`fervent-shamir`, `recursing-curran`) — cleaned up after verifying no pending changes

---

## Phase 10 — Schema normalization, readings migration to PostgreSQL, WiFi geolocation (17 Mar 2026)

**Contributor:** Vinicio Mendes (with AI assistance — Claude)

### Fixed (database — schema normalization)
- **Production `devices` table renamed from camelCase to snake_case:** `publicKey` → `public_key`, `macAddress` → `mac_address`, `nftAddress` → `nft_address`, `txSignature` → `tx_signature`, `lastTsSeen` → `last_ts_seen`, `claimToken` → `claim_token`, `ownerAddress` → `owner_address`
- Added `id UUID` primary key to `devices` (was using `publicKey` as PK)
- Added missing columns to `devices` in production: `name`, `type`, `description`, `visibility`, `mode`, `status`, `owner_wallet`, `owner_user_id`, `thumbnail_url`, `created_at`, `updated_at`
- Local migration `002_fix_schema_gaps.sql`: creates `kv_store_4a89e1c9` table, adds IoT columns (`challenge`, `nft_address`, `tx_signature`, `last_ts_seen`, `revoked`), geolocation columns (`location`, `latitude`, `longitude`, `location_accuracy`), and composite index `idx_sensor_readings_nft_ts`
- All Edge Function queries updated to use snake_case column names (8 callsites in `register-device`, `sensor-data`, and `resolveNftAddress`)

### Changed (backend — readings migration from KV to PostgreSQL)
- **Eliminated O(n) KV store scans for readings:** all 15 `kv.getByPrefix('reading:...')` callsites replaced with `getSensorReadings()` helper that queries `sensor_readings` PostgreSQL table directly via index scan
- New `countSensorReadings()` helper uses `SELECT count` for stats/featured (avoids fetching all rows)
- Mock sensors continue reading from KV store (fallback path preserved)
- `POST /sensor-data` no longer writes readings to KV store — only sensor metadata updates remain
- **Cleaned 9,482 stale reading entries from `kv_store`** — freed ~39 MB of storage (95% of KV usage)
- Readings count in featured/stats now shows correct total (was capped at 1,000 by Supabase JS client default)

### Added (WiFi geolocation — ADR-009)
- `POST /server/device-location` endpoint: receives WiFi AP scan (BSSIDs + RSSI), queries Mylnikov API per BSSID, computes RSSI-weighted centroid, reverse geocodes via Nominatim, stores location in `devices` table + KV sensor metadata
- ESP8266 firmware: `scanAndReportLocation()` scans nearby WiFi networks on boot, sends top 5 APs (BSSID + RSSI) to backend — fire-and-forget, non-blocking
- Frontend: location display with MapPin icon in sensor-card, sensor-detail, and public-sensor-detail; coordinate display with accuracy when available
- Geolocation columns added to `devices`: `location` (text), `latitude`, `longitude`, `location_accuracy` (numeric)

### Known issue
- **Mylnikov API has poor coverage in Brazil** — the 5 BSSIDs scanned by the ESP8266 returned HTTP 422 (not found). Needs alternative provider (Apple WiFi DB via Cloudflare Worker is available in `wifi-geolocate-worker/` but not yet deployed)

### Verified
- 32/32 local test suite passing
- Production sensor (DHT11) continues receiving data normally (28.8°C @ 22:31 UTC)
- Readings count in production correctly shows 9,735 (was stuck at 1,000)

---

## Phase 11 — Frontend styling polish and sentence case (20 Mar 2026)

**Contributor:** Vinicio Mendes (with AI assistance — Claude)

### Changed (frontend)
- **SensorCard:** centered Stored/Readings metrics in their boxes; added `formatDataSize` utility (`src/lib/format.ts`) for human-readable byte formatting
- **Home page featured sensors:** redesigned section with responsive grid layout (1-col mobile, 2-col tablet, 3-col desktop), ready for multiple sensors; replaced inline Tailwind v4 responsive classes with plain CSS `@media` queries to work around specificity issues
- **Public sensors page:** tightened header padding, added sensor count badge, adaptive grid layout based on sensor count (centered single sensor, 2-col for 2, 3-col for 3+), subtler info card styling
- **Sentence case:** converted ~30 UI strings across home, public sensors, and sensor card from Title Case to sentence case (e.g., "Featured Public Sensors" → "Featured public sensors", "View Details" → "View details")

### Fixed
- Tailwind CSS v4 specificity bug: `flex-col md:flex-row` and `grid-cols-3` responsive variants lost specificity in generated CSS — workaround uses `.featured-spotlight` and `.featured-metrics` CSS classes with `@media` queries

---

## Phase 12 — README rewrite to reflect current stack and features (22 Apr 2026)

**Contributor:** Vinicio Mendes (with AI assistance — Claude)

### Changed
- `README.md` rewritten to match current state of the project after Phases 8–11:
  - Stack corrected from "Next.js + Tailwind" to Vite + React + TypeScript (Next.js code was moved to `_reference/` back in Phase 4)
  - Framing broadened from environmental-data product to hardware-agnostic DePIN infrastructure platform (aligned with `CLAUDE.md`), mentioning retail analytics / TinyML as an example application being built on top
  - Added sections for binary Merkle tree with client-side inclusion proofs (Phase 8) and WiFi-based geolocation via Apple WiFi DB / Cloudflare Worker (Phase 10)
  - Public API table rebuilt with the real endpoints exposed by the Edge Function today (including `/public/sensors/:sensorId/merkle-proof/:leafIndex`, `/server/device-location`, `/verify/merkle`)
  - Storage description corrected: "In-memory Redis-like layer" replaced by Supabase KV table (`kv_store_4a89e1c9`) used only for sensor metadata and datasets; readings live in PostgreSQL (Phase 10)
  - Added references to `docs/timeline.md`, `docs/adr/`, and `CLAUDE.md`

### Fixed
- Broken documentation links removed: `BACKEND_INTEGRATION_GUIDE.md` and `API_QUICK_REFERENCE.md` (did not exist)
- Placeholder URLs replaced: `github.com/your-username/SparkedSense.git` → `github.com/viniciomendesr/sparkedsense.git`

> PR [sparkedsense#10](https://github.com/viniciomendesr/sparkedsense/pull/10) — squash merged into `main` (`f0d60e6`)

---

## Phase 13 — Sensor-agnostic ingestion envelope (in progress, target 2026-04-24)

**Contributor:** Vinicio Mendes (with AI assistance — Claude)

### Scope
- Adopt CloudEvents-compatible envelope with SenML-compatible payloads for the ingestion layer, per [ADR-010](adr/010-sensor-agnostic-ingestion-envelope.md).
- Single endpoint `POST /reading` supersedes the shape implied by `POST /sensor-data`; legacy endpoint becomes an adapter that wraps DHT11 payloads into `io.sparkedsense.sensor.environmental` envelopes.
- New platform-blessed event types (`sensor.environmental`, `inference.classification`, `inference.transcription`, …) under reverse-DNS namespace `io.sparkedsense.*`.
- Frontend type-dispatched renderer framework; first three renderers needed for the 2026-04-24 demo (environmental SenML, acoustic classification from ESP32-S3, transcription from MacBook MEC gateway).

### Rationale for ordering vs. other pending work
- Hard demo deadline (2 days) imposes a freeze on refactors with wide blast radius. Items pending from [ADR-007](adr/007-merkle-tree-before-blockchain.md) (NFT metadata schema, anchoring transaction format, real Solana devnet integration) are orthogonal to the envelope shape and deferred.
- Backend modularization (`supabase/functions/server/index.ts` at 1900+ lines) is a soft prerequisite that was intentionally **not** done first: risk of regressions in working code outweighs the cleanup benefit at this horizon. Instead, ADR-010 code ships as new modules (`ingest.ts`, `schemas/`) alongside the monolith; extraction of legacy handlers follows post-demo.
- WiFi geolocation with Brazil coverage (pending in Phase 10) is already resolved: backend actively uses `GEOLOCATE_WORKER_URL` pointing at the Cloudflare Worker (Apple WiFi DB), not Mylnikov.

### Conscious bets
- The adapter preserves production DHT11 — no firmware change required for the existing sensor to keep reporting during and after the migration.
- `sensor_readings_compat` view projects SenML records back into the legacy shape so the current frontend keeps working while renderers for new modalities are added.
- The author's concurrent undergraduate research (heterogeneous Edge AI inference aggregation) composes on top of this layer without blurring platform vs. research boundaries.

### Status
- [x] ADR written and merged, status **Accepted** (this phase marks the transition from Proposed).
- [x] `docs/event-types/` with JSON Schemas for eight platform-blessed types (`sensor.environmental`, `sensor.generic`, `inference.classification`, `inference.regression`, `inference.detection`, `inference.transcription`, `inference.semantic_summary`, `raw.audio`).
- [x] Migration `003_readings_envelope.sql` creating `readings` table + `sensor_readings_compat` view.
- [x] Backend `POST /reading` with envelope shape validation, secp256k1 verification over canonical JSON (recursive key sort), and lightweight typed-payload validation.
- [x] Backend adapter: `POST /sensor-data` dual-writes SenML envelopes into `readings` so the live DHT11 enters the new feed without a firmware change.
- [x] Frontend type-dispatched renderer framework in `src/components/renderers/` (environmental SenML, classification, transcription, generic fallback).
- [x] Page `/demo-claro` consuming the new `/public/readings-v2/:sensorId` feed; verified end-to-end with real hardware (11 envelopes rendered live from the production DHT11).
- [ ] ESP32-S3 acoustic client emits envelopes directly (not wrapped); MacBook whisper gateway emits envelopes — pending firmware/client code for the demo.

> Implementation order and risks documented in [ADR-010](adr/010-sensor-agnostic-ingestion-envelope.md). This phase is intentionally open until the demo stabilizes, at which point items 8–9 of the ADR's implementation order (deprecation window, quickstart documentation) move to a future phase.

---

## Phase 14 — Pre-demo UX polish and security hardening (22 Apr 2026)

**Contributor:** Vinicio Mendes (with AI assistance — Claude)

### Added (sensor-agnostic envelope — ADR-010 continuation)
- [ADR-011](adr/011-unsigned-dev-bypass-for-unported-devices.md): `signature: "unsigned_dev"` bypass accepted in `POST /server/reading` for devices whose firmware has not yet ported the secp256k1 signing pipeline. Device identity still enforced via the `source` → registered `public_key` lookup. `TODO(ADR-011)` anchor in the code marks the removal trigger (Node 2 ESP32-S3 port landing with 100 consecutive valid signatures).

### Added (sensor UX)
- **Edit title/description from the owner view** ([src/components/edit-sensor-dialog.tsx](../src/components/edit-sensor-dialog.tsx)): pencil icon next to the header, dialog with validation, `sensorAPI.update` call. Backend allowlist restricts mutable fields to `['name', 'description', 'visibility']` — `location`/`latitude`/`longitude` explicitly rejected server-side because they are firmware-derived signals (preserves ADR-003 trust model).
- **Location label includes neighborhood** ([supabase/functions/server/index.ts](../supabase/functions/server/index.ts) `buildLocationText`): reverse-geocode prioritises `suburb → neighbourhood → quarter → city_district → residential` before the city. Existing sensor `36c1d3d2-...` self-updated from `São Paulo, São Paulo, Brasil` to `Pinheiros, São Paulo, São Paulo, Brasil` via the new `POST /sensors/:id/refresh-location` endpoint (owner-only, re-geocodes from stored lat/lng only — does not accept client-supplied coordinates).
- **Chart redesign in crypto style** ([src/components/sensor-chart.tsx](../src/components/sensor-chart.tsx)): shared `SensorChart` component with stats header (current value, absolute delta, percentage delta, trend-coloured), Min/Max/Avg/Points badges, gradient area fill, dashed reference line at the mean, and a brush for zooming the historical range. Modes: `live` (last 60 points, no range selector) and `historical` (1H/6H/1D/1W/All selector + brush). Replaces the previous plain `LineChart` in both owner and public sensor detail pages.
- **Historical Data Feed section** shown below Real-Time Data Feed on both pages; owner view flattened the tab layout so `Live Data` and `Historical Data` live side by side inside `Live Stream`, matching the public layout.
- **`Last Updated` syncs with the latest reading** on the public page (was tied to `sensor.updatedAt` which lags the ingestion stream).

### Changed (backend)
- `getSensorReadings` now paginates via `.range(offset, offset + 999)` up to the requested limit. Supabase PostgREST silently caps responses at 1000 rows per default config; the historical chart was hitting that cap and showing an incomplete `Max` value. Pagination lets the 50,000-point historical query return the real dataset.
- `POST /sensor-data` dual-writes each reading into the new `readings` table as an `io.sparkedsense.sensor.environmental` envelope. The live DHT11 now populates both the legacy `sensor_readings` path and the ADR-010 feed consumed by `/demo-claro`.
- `sensorAPI.update` allowlist added: `['name', 'description', 'visibility']`. Any other field in the request body is silently discarded server-side.

### Fixed (deployment)
- **Vercel SPA 404 on refresh**: added `vercel.json` with a catch-all rewrite to `/index.html`. Previously, refreshing on any client-side route (e.g. `/public-sensors`) bypassed React Router and hit Vercel's file-system 404.
- **`.DS_Store` untracked from git** and added to both the project `.gitignore` and the machine-global `~/.gitignore_global`.

### Security (migration `004_security_hardening.sql`)
Supabase advisor reported 5 ERROR + 4 WARN; migration 004 drove it to 0 ERROR + 1 WARN + 2 INFO.

- Enabled RLS on `devices`, `sensor_readings`, `sensor_metrics`.
- Added owner-only SELECT policy on `devices` — `mac_address` (PII) is no longer exposed to anon or to other owners via direct PostgREST.
- Recreated `sensor_readings_compat` view with `WITH (security_invoker = true)` so the view respects the caller's RLS instead of the creator's.
- Replaced the `USING (true)` policy on `kv_store_4a89e1c9` with `TO service_role` scope.
- Pinned `search_path = public, pg_catalog` on `update_updated_at_column` and `update_sensor_metrics` to prevent schema-shadowing attacks.
- Edge function keeps working because `SERVICE_ROLE` bypasses RLS. PostgREST calls from anon/authenticated now correctly return empty arrays for `devices` and `sensor_readings`.
- Remaining WARN (`auth_leaked_password_protection`) must be toggled in the Supabase Auth dashboard — not actionable via SQL.

---

## Phase 15 — Solana anchoring for dataset Merkle roots (22 Apr 2026)

**Contributor:** Vinicio Mendes (with AI assistance — Claude)

### Added ([ADR-017](adr/017-solana-memo-anchoring.md), numbered 012 at the time)
- **Real Solana devnet anchoring** for dataset Merkle roots via the Memo Program. Replaces the `setTimeout(3000)` simulation in `POST /server/datasets/:id/anchor`.
- **Dedicated server keypair** `36QSgfod6aZQTn57dshDdhAxfNaVtpvQzHUQWJUaVUYy` (devnet, funded with 6 SOL). Secret stored exclusively in Supabase secret `SOLANA_SERVER_SECRET_KEY_BASE58`, isolated from personal wallets.
- **Manual transaction construction** in [`supabase/functions/server/lib/solana.ts`](../supabase/functions/server/lib/solana.ts) — legacy tx format, ed25519 sign via `@noble/curves`, raw JSON-RPC. `@solana/web3.js` was rejected because it triggers `WORKER_RESOURCE_LIMIT` in Supabase Edge Runtime even with lazy imports.
- **Self-describing memo payload**: `sparked-sense://dataset/<id>?root=<merkleRoot>&n=<count>&from=<iso>&to=<iso>` — under 566 bytes, readable directly in Solana Explorer's log.
- **Diagnostic endpoint** `GET /server/public/anchor-info` returns `{ enabled, publicKey, cluster, balanceSol, explorerAddressUrl }` so third parties can audit the anchoring wallet.
- **Dataset schema** gains `anchorTxSignature`, `anchorExplorerUrl`, `anchorCluster`, `anchorMemo`, `anchoredAt` ([src/lib/types.ts](../src/lib/types.ts)).

### Changed (UX)
- **Removed the "Verify Data Integrity" paste-your-own-hash panel** from owner and public sensor detail pages. The panel was cryptographically correct but UX-zero — nobody arrives at the page with a Merkle root in the clipboard. The audit page retains a technical Merkle-proof view for advanced users.
- **Added "View onchain anchor" button** per anchored dataset, linking directly to `explorer.solana.com/tx/<sig>?cluster=devnet`. Trust delegates to Solana Explorer rather than to an input field.
- **Removed the placeholder "View Sensor NFT on Solana Explorer"** button in the owner view — it pointed at a bare `explorer.solana.com/` URL because device-identity NFTs are still simulated. Comment in code references ADR-007/ADR-017 (numbered 012 at the time) for context.

### Fallback + resilience
- If `SOLANA_SERVER_SECRET_KEY_BASE58` is unset or the anchor tx fails (RPC flake, wallet unfunded, devnet reset), the handler falls back to the legacy simulated flow. The dataset still publishes; the UI surfaces the absence of an anchor tx gracefully (button only renders when `anchorExplorerUrl` is truthy).

### Verified
- Test tx [`2HNdwW5qLZNszUmVq43uXfwbLtXqQS3VPbVTjSiVu1bfCzWFanXXSMXwkdgVqueVsuWV8tBrbfz5pzjCobb4Dx7W`](https://explorer.solana.com/tx/2HNdwW5qLZNszUmVq43uXfwbLtXqQS3VPbVTjSiVu1bfCzWFanXXSMXwkdgVqueVsuWV8tBrbfz5pzjCobb4Dx7W?cluster=devnet) submitted and confirmed at slot 457407445. Memo log shows the URI with `root=e3b0c44298fc1c14...`. Fee: 5000 lamports.
- `GET /server/public/anchor-info` returns 200 with current balance (6 SOL at deploy time).
- Existing `/server/sensor-data` and other endpoints continue to work (lazy import prevents the heavy Solana module from loading on every cold start).

### Still deferred from ADR-007 (post-demo)
- NFT minting for device identity — `sensor.nftAddress` remains `devnet_sim_<hex>` until a Metaplex-capable runtime replaces the Deno edge function for this specific task.
- Anchoring via a custom Solana program (PDA) — not needed; Memo Program gives integrity proof at negligible cost and maximum tooling support.

---

## Current status

**Implemented:** End-to-end DePIN flow with secp256k1 cryptographic authentication, simulated digital identity (nftAddress), real-time dashboard with crypto-style live + historical charts (Min/Max/Avg/Points, gradient fill, timeframe selector, brush zoom), binary Merkle tree with inclusion proofs for dataset integrity verification (client-side and server-side), PostgreSQL as canonical storage for sensor readings (KV store retained only for sensor metadata and datasets), Solana devnet wallet under project control, homepage with Featured Public Sensors, WiFi-based geolocation with neighborhood label via Apple WiFi DB through Cloudflare Worker, owner-editable sensor name/description, RLS hardening across `devices`/`sensor_readings`/`sensor_metrics` and security_invoker view. Structured documentation with ADR index and timeline in `docs/`.

**In progress (Phase 13, target 2026-04-24):** Sensor-agnostic ingestion envelope per [ADR-010](adr/010-sensor-agnostic-ingestion-envelope.md) — CloudEvents + SenML adoption, `POST /reading` endpoint live with dual-write from legacy `/sensor-data`, renderer framework for heterogeneous modalities (environmental telemetry, ML classification, transcription). Remaining: ESP32-S3 acoustic client + MacBook whisper gateway emitting envelopes directly. During the demo window, Node 2 events use the [ADR-011](adr/011-unsigned-dev-bypass-for-unported-devices.md) `unsigned_dev` bypass until the signing pipeline is ported.

**Next steps (from ADR-007, mostly closed):**
1. ~~Fix Merkle tree implementation~~ — done (Phase 8)
2. ~~Deploy WiFi geolocation provider with Brazil coverage~~ — done (Apple WiFi DB via Cloudflare Worker, env `GEOLOCATE_WORKER_URL` in use)
3. NFT metadata schema for device identity — deferred (needs Metaplex-capable runtime; `sensor.nftAddress` stays simulated)
4. ~~Anchoring transaction format~~ — done: Memo Program via [ADR-017](adr/017-solana-memo-anchoring.md)
5. ~~Real Solana devnet integration (dataset anchoring)~~ — done (Phase 15); NFT minting still deferred

**Also pending (post-demo):** ESP32-S3 secp256k1 signing pipeline port (removes [ADR-011](adr/011-unsigned-dev-bypass-for-unported-devices.md) bypass), firmware resilience (WiFi reconnection, watchdog, HTTPS timeout — see audit), backend modularization (`index.ts` split), open source documentation, ESP8266 migration from legacy `POST /sensor-data` to native envelope emission (see ADR-010 item 8 + new [ADR-015](adr/015-unify-ingestion-on-adr-010.md)), server-side downsampling (LTTB) for charts past ~30k readings per fetch (free-tier edge memory is the binding constraint, not CPU), Supabase Auth `leaked_password_protection` toggle via dashboard.

### Phase 17 — Repository hygiene (17 Aug 2026)

#### ADR-012 numbering collision resolved

Two ADRs carried the number 012: `unsigned_dev` sensor mode (2026-04-23) and Solana Memo Program anchoring (2026-04-22). Anchoring was renumbered to [ADR-017](adr/017-solana-memo-anchoring.md); the sensor mode keeps 012.

The date order argued for the opposite move, since anchoring is a day older. Reference count decided it: all 12 `ADR-012` mentions in source code refer to the sensor mode, spread across `esp32s3.ino`, `index.ts`, `api.ts` and four frontend components. Anchoring was cited only from documentation, in four places, all updated. Renumbering the cheaper side left the deployed firmware comment and the backend untouched.

ADR-017 carries a `Renumbered` line in its header so pre-2026-08-17 citations of "ADR-012" for anchoring still resolve. The index in `docs/adr/README.md` is ordered by number, so 017 now sits last despite its April date.

#### Evaluation instruments versioned

`docs/ground-truth/` entered the repository: the acoustic ground-truth protocol plus two evaluation scripts, `ground-truth.py` (frame-level capture from Serial with known labels, then confusion matrix, precision/recall/F1 and threshold sweep) and `verificacao-terceiro.py` (seven architecture metrics computed as an independent third party, calling no operator endpoint).

They were written outside version control and existed on one machine only, which is the wrong place for the instruments that produce the numbers cited in academic text. The protocol also records why the 2.995 production inferences cannot be reported as classifier performance: `PUBLISH_THRESHOLD_CLARO = 0.30f` has priority over the argmax, `noise` has no publication path at all, and no ground-truth label exists anywhere in the database.

Neither script has been executed yet. No results are committed.

**First execution (17 Aug 2026, later the same day).** `verificacao-terceiro.py` ran against production and surfaced three bugs in itself, all fixed in the same commit that records this: the dataset's `sensorId` (a sensor UUID) was fed to `sensor_readings.nft_address` (hex), returning zero rows, so the "reconstructed" root was the empty-tree hash — the resolution is `sensor.claimToken → devices.claim_token` (`index.ts:79`); the legacy leaf canonical used `unit: ""` and `Z`-suffixed timestamps where production (`pgReadingToKvFormat`, `index.ts:169`) uses the sensor type's unit (`°C`) and PostgREST's `+00:00` serialization, plus JS number formatting (`24`, not `24.0` — 3,984 of the 38,780 readings have integer values); and the timestamp mutation replaced a millisecond digit with `9` unconditionally, a null mutation whenever the digit already was `9` (19 of 200 draws), which counted legitimate signature passes as missed detections.

Results after the fixes: anchored root `6b0c14c7…` reproduced exactly from the 38,780 raw legacy rows (`confere: True`); 100% of sampled envelopes (4,956) verify by a third party with zero operator calls; 800/800 mutations rejected. The measured canonical envelope is **561 bytes** — the "393 bytes" previously cited in the SIICUSP abstract draft does not match and needs correction there. The M6 onboarding metric (8 s median, n=3) is unusable: `devices.created_at` postdates the first reading on two of three nodes, so the delta is negative for one node and 36 days for another.

#### Dead Python dependency removed

`requirements.txt` (flask, redis, python-dotenv, gunicorn, all unpinned) was deleted in `4e1be3a`.

It entered the repo in `497cc3c` "Transfer backend" (30 Oct 2025), the bulk import that also brought the Next.js `app/api/` route handlers. [ADR-002](adr/002-unified-backend-edge-functions.md) replaced those handlers with Supabase Edge Functions, but the Python manifest survived that cleanup. No `.py` file exists anywhere in the repo, so the file pinned nothing and described no runnable service. The `redis` references that remain are the TypeScript `ioredis` client, a separate dependency.

Anyone carrying a local `.venv` built from it can delete it; nothing in the project reads from it.

### Phase 16 — Post-demo refactor: deferred minting + ingestion unification (25 Apr 2026)

The 2026-04-24 Claro demo exposed a structural friction: every physical device had to mint an NFT *before* it could publish, which is wrong as a default for a DePIN platform aiming at commodity hardware. The `unsigned_dev` mode introduced for the demo (ADR-012) turned out to be the right shape for the *general* case, not a one-off patch.

Two new ADRs formalise the direction:

- [ADR-014](adr/014-deferred-nft-minting.md) — **Deferred NFT minting**. All physical devices register in mode `unverified` (renamed from `unsigned_dev`) and start publishing immediately. Mint becomes an explicit user action via a "Mint NFT" button, paid by the server wallet on devnet. Sensor transitions `unverified → real` when mint confirms onchain. Datasets keep mixed-signature provenance metadata so auditors see the attestation composition explicitly. Supersedes ADR-012 (mode renaming) and partially supersedes ADR-003 (mint-first registration is no longer mandatory).
- [ADR-015](adr/015-unify-ingestion-on-adr-010.md) — **Unify ingestion on `/server/reading`** (ADR-010 envelope). The legacy `/server/sensor-data` endpoint becomes deprecated; the ESP8266 + DHT11 sensor (Nó #1 prod) migrates to publish CloudEvents envelopes with `io.sparkedsense.sensor.environmental` SenML records. Hard cutover with `sensor_readings` frozen as a read-only history; backend continues querying both tables for that sensor during the transition window. Existing NFT identity is preserved (transport change only).

#### Code changes shipped in this phase

1. **Mode rename** — `unsigned_dev` → `unverified` across TypeScript types, backend mode checks, and UI labels. Wire-protocol marker `signature: "unsigned_dev"` in the envelope is unchanged (it's an event-level property, separate from sensor-level attestation status). Backwards-compat shim `isUnverifiedMode(mode)` accepts both values on read so existing KV records continue to work without an immediate migration.
2. **Mint endpoint + UI** — `POST /server/sensors/:id/mint` accepts owner-authenticated calls, validates the sensor is in `unverified` mode, generates simulated `nft_address`/`claim_token`/`tx_signature` (same pattern as Step 2 of `/server/register-device`), and persists `mintedAt`. Sensor detail page renders a "Mint NFT" button when `mode === 'unverified'`; clicking promotes the sensor to `real` mode. Idempotent: if the device row already has an `nft_address`, the existing identity is reused.
3. **Dataset signature-composition metadata** — `POST /server/datasets` now stamps each dataset with `mintStatus` (sensor mode at creation) and, for `unverified` sources, `signatureComposition: { verified, unsigned }` computed via two parallel HEAD counts on the `readings` table. Dataset cards on the sensor detail page render an "Unverified Source" badge with a tooltip listing the breakdown.
4. **ESP8266 (Nó #1) firmware migrated to ADR-010** — `ESP/ESP.ino` now publishes CloudEvents envelopes to `/server/reading` instead of the legacy `/server/sensor-data` shape. Same secp256k1 keypair, same NFT identity, same DHT11 readings — only the transport changes. Top-level keys are inserted in alphabetical order so `serializeJson` output equals canonical JSON without a runtime sort; SenML records use `n/u/v` order. Signature is hex secp256k1 over canonical JSON of the envelope minus `signature`. Legacy `/server/sensor-data` continues to accept writes (with `X-Sparked-Deprecation` response header and a `console.warn`) for ~7 days as rollback safety per ADR-015.
5. **One-shot KV migration** — `POST /server/admin/migrate-unverified-mode` (gated by `X-Admin-Migration` header) rewrites legacy `mode: 'unsigned_dev'` KV rows to `'unverified'`. Idempotent. Ran successfully on 2026-04-25: scanned 2 sensor records, 1 candidate, 1 updated.

#### Mainnet mint flow

[ADR-016](adr/016-user-paid-mint-on-mainnet.md) is now `Proposed`, deferring the user-paid mint flow (Solana wallet adapter, mint goes directly to user's wallet, server keeps anchoring role only) until mainnet deployment is on the table. Devnet stays server-paid as ADR-014 specifies.

#### Still pending in Phase 16

- **Drop the `isUnverifiedMode()` backwards-compat shim** in `index.ts` once we're confident the KV migration is complete and no other code path references the legacy value. Cheap insurance for now; remove in a future cleanup pass.

### Phase 16 closure — ADR-015 cutover (27 Apr 2026)

The legacy ingestion endpoint was retired and the read-side union shipped, closing ADR-015.

#### Read-side union (`a1a9a2c`)
- `getSensorReadings` and `countSensorReadings` real-mode branches now resolve `nft_address` (legacy) **and** `device_id` (envelope) in parallel, query `sensor_readings` and `readings` independently, then merge DESC by timestamp and slice to `limit`. Union is correct under `top-N-DESC(A ∪ B) = top-N-DESC(top-N-DESC(A) ∪ top-N-DESC(B))`.
- This fixes a class of bugs where sensors created as `mode: "real"` but publishing envelopes (e.g. Nó #2 acoustic with the `unsigned_dev` bypass) returned `{readings: []}` while `lastReading` was populated. Validated in production: Nó #2 audit page now surfaces all 98 envelopes; Nó #1 audit/dashboard see the union of ~65500 legacy + ~1000 envelope rows.
- Cost: ~+400ms TTFB on `/public/readings/:id` (extra device-id resolution + envelope-side count). Acceptable; the kv.getByPrefix scan in every public handler remains the larger latency contributor.

#### Nó #1 firmware flash + observation
- Earliest envelope in `readings` for Nó #1: **2026-04-26T19:32:42** — flash happened ~32h after the firmware commit (`4bf8734`).
- Latest envelope at cutover time: 2026-04-27T13:19. ~17h of continuous envelope-only traffic with valid hex secp256k1 signatures (1000+ rows sampled, zero `signature: ''` matches).
- Event type: `io.sparkedsense.sensor.environmental` — ADR-015 conformant.

#### Cutover (`/server/sensor-data` → 410 Gone)
- Endpoint retired 2026-04-27. Handler returns `{ error: 'This endpoint was removed on 2026-04-27...', code: 'gone' }` with `X-Sparked-Deprecation` header. Handler body (signature verification + dual-write mirror + KV bookkeeping) was deleted per the no-removed-code-shims rule; git history preserves the prior implementation.
- Deviation from ADR-015 step 5: the 7-day buffer was shortened to ~17h because the conditions motivating that buffer don't apply (single legacy device, single operator, hard-cutover firmware with no runtime fallback, no third-party consumers of the endpoint). Documented in ADR-015's Cutover note section.
- Rollback path: redeploy the prior Edge Function version if any regression surfaces.

#### What this leaves behind
- `sensor_readings` is now strictly read-only history, queried via the legacy side of the real-mode union read.
- Pre-cutover envelope rows produced by the legacy `/server/sensor-data` dual-write mirror remain in `readings` and continue surfacing via the envelope side of the union.
- Open follow-ups (not blocking ADR-015 closure):
  - Drop `isUnverifiedMode()` shim after KV migration converges.
  - Tackle the `kv.getByPrefix('sensor:')` scan that dominates TTFB on public handlers (separate ADR candidate).
  - `/public/sensors/featured` at ~3.2s — iterates per public sensor with Merkle build; refactor target.

### Phase 16 addendum — ESP32-S3 (Nó #2) build configuration for on-device KWS (17 Aug 2026)

A flash session for `ESP/esp32s3/esp32s3.ino` surfaced four failures in sequence, all of them Arduino IDE **build configuration** — no firmware source change was involved. Recording the working configuration here because none of it is inferable from the codebase, and the failure modes are misleading.

Board is a Waveshare ESP32-S3 **N16R8** (16MB quad flash, 8MB **octal** PSRAM), selected in the IDE as `ESP32S3 Dev Module`, arduino-esp32 core `3.3.11`.

#### Working configuration

| Tools menu | Value | Why |
|---|---|---|
| Port | `/dev/cu.wchusbserial…` | see serial port note below |
| Flash Size | `16MB (128Mb)` | matches N16R8; esptool confirms `Detected flash size: 16MB` |
| Partition Scheme | `16M Flash (3MB APP/9.9MB FATFS)` | sketch is ~2.29MB; see below |
| PSRAM | `OPI PSRAM` | N16R8 PSRAM is octal — `QSPI PSRAM` will not initialise |
| Flash Mode | `QIO` | flash is quad; only the PSRAM is octal. These are independent |

#### 1. Serial port — duplicate drivers

Both ESP nodes use WCH USB-serial bridges, and **two macOS drivers claim them simultaneously**, producing two device nodes for one physical board:

- Nó #1 (ESP8266, CH340 `1A86:7523`) → `/dev/cu.usbserial-110` (Apple `AppleUSBCHCOM`) and `/dev/cu.wchusbserial110` (WCH `cn.wch.CH34xVCPDriver`)
- Nó #2 (ESP32-S3, CH343 `1A86:55D3`) → `/dev/cu.usbmodem59090879581` (Apple CDC-ACM) and `/dev/cu.wchusbserial59090879581` (WCH)

The CH343 enumerates as USB CDC-ACM, so the Apple-side node is named `usbmodem…` and looks like an ESP32-S3 native-USB port. It is not — `ioreg` shows a single USB device and zero Espressif VID `303A` devices.

**Always select the `wchusbserial…` node.** On the ESP32-S3 the Apple driver connects and identifies the chip correctly, then fails mid-upload with `Failed to write to target RAM (result was 0107: Checksum error)` — it looks like it works right up until it doesn't. Do not uninstall the WCH driver.

#### 2. `text section exceeds available space in board`

```
Sketch uses 2288117 bytes (174%) of program storage space. Maximum is 1310720 bytes.
```

`1310720` = `0x140000`, the app slot of `default.csv` (the 4MB scheme). The IDE was treating a 16MB board as 4MB. `app3M_fat9M_16MB.csv` gives `0x300000` = 3145728 bytes → ~73% used.

Do **not** pick `16M Flash (2MB APP/12.5MB FATFS)` — 2MB is smaller than the sketch and fails identically. `nvs` sits at `0x9000`/`0x5000` in both schemes, so switching does not lose NVS/Preferences data.

**Side effect worth knowing:** while this error persists nothing is flashed, so the board keeps running the *previous* firmware. Symptom during this session was the Serial Monitor showing a Wi-Fi SSID that had already been changed in the source — easily mistaken for a config bug in the firmware.

#### 3. `Failed to allocate TFLite arena (524288 bytes)` + `Failed to run DSP process (-1002)`

Root cause was `PSRAM=disabled` in the build FQBN. With PSRAM off, `MALLOC_CAP_SPIRAM` does not exist in the heap allocator, the patched allocator falls through to internal DRAM, and a 512KB arena cannot fit in the ~250KB of free DRAM. `-1002` is `EIDSP_OUT_OF_MEM` — same cause, not a separate fault.

Verified allocation chain (all correct; only the PSRAM menu setting was wrong):

```
tflite_micro.h:121      ei_aligned_calloc(16, arena_size)
ei_aligned_malloc.h:81  ei_calloc(size + hdr_size, 1)
espressif/ei_classifier_porting.cpp:123  heap_caps_malloc(total, MALLOC_CAP_SPIRAM)
```

The board's own boot line is the fastest check: `PSRAM livre: 8384768 bytes` = working, `0 bytes` = still disabled.

**Note on GPIO:** enabling OPI PSRAM reserves GPIO 33–37. The I²S mic is on GPIO 4/5/6 (`I2S_SCK_PIN`, `I2S_WS_PIN`, `I2S_SD_PIN`), so there is no conflict — but any future pin assignment must avoid that range.

#### 4. Edge Impulse library patches are not in version control

The KWS build depends on two hand-edits inside `~/Documents/Arduino/libraries/Audio_Classification_-_Keyword_Spotting_-_Demov1_inferencing/`, already noted in the `.ino` header:

- `src/tflite-model/tflite_learn_972566_32.h` — `arena_size` raised to `524288` (default `209478`)
- `src/edge-impulse-sdk/porting/espressif/ei_classifier_porting.cpp` — `ei_malloc`/`ei_calloc` route allocations `>= 2048` bytes to PSRAM

These live outside the repo and are silently lost on re-importing the Edge Impulse `.zip`. The `espressif` porting file is the one that actually compiles for this target (confirmed via object sizes: espressif `24764` bytes vs. arduino `6088` bytes, the latter being an empty translation unit) — patching `porting/arduino/` instead would have no effect.

**Follow-up:** vendor these two patches into the repo as a `.patch` file (or check in the modified library) so the build is reproducible on another machine.

### Phase 15 addendum — persist `verify_jwt=false` for device ingestion (24 Apr 2026)

- A redeploy of the `server` Edge Function (rate-limit tuning for the 2026-04-24 Claro demo) silently re-enabled Supabase's gateway-level JWT verification, breaking every device call with `UNAUTHORIZED_NO_AUTH_HEADER`. Root cause: the original deploy used `--no-verify-jwt` as a CLI flag, not a persisted project setting.
- Fix: created [`supabase/config.toml`](../supabase/config.toml) with `[functions.server] verify_jwt = false`. Supabase CLI reads this on every deploy, so the flag is no longer deploy-time discipline. Redeployed and validated via `POST /server/register-device` returning a 64-hex `challenge` without an Authorization header.
- Rationale captured in [ADR-013](adr/013-edge-function-jwt-disabled-for-device-ingestion.md): IoT devices authenticate by secp256k1 possession (ADR-003) or the ADR-011 `unsigned_dev` marker, not by Supabase JWT. Enabling the gateway check blocks legitimate device traffic before the handler runs.

### Phase 15 addendum — parallel pagination (22 Apr 2026)

- `getSensorReadings` (backend) now does a HEAD count first, then fires all pages of 1000 in parallel with `Promise.all` instead of serial. Reduces 50-page fetch from ~15-18s (which was hitting `WORKER_RESOURCE_LIMIT` HTTP 546 on the Supabase free tier) to ~3s on the wall clock. Frontend historical chart bumped from 10k → 25k rows.
- Binding constraint at this point is **memory on the free-tier edge runtime** (~256MB), not wall clock. Experimentally, 30k rows works; 50k still trips `WORKER_RESOURCE_LIMIT`. Real fix for sensors with >30k useful historical rows is LTTB downsampling server-side — listed under post-demo pending.

### Phase 10 addendum — device location stops caching (17 Aug 2026)

- `POST /server/device-location` no longer short-circuits on a 24h dedup cache. Every boot resolves the location from the WiFi scan. Rationale and the accepted costs are in [ADR-018](adr/018-remove-device-location-dedup-cache.md), which amends the cache clause of [ADR-009](adr/009-wifi-geolocation-for-sensors.md).
- The cache keyed on `nft_address`, which does not change when a device moves. Both nodes are development boards carried between lab, office and demo venues, so the dashboard was showing addresses the device had left days earlier. The response said `{"success": true, "cached": true}` — indistinguishable from a fresh lookup unless the caller reads the flag, which none did.
- Cost accepted for now: one worker request per boot, with no endpoint-level protection against a reboot loop, and `devices.updated_at` churning from centroid jitter. Safe only while location stays out of the anchored envelope — it is not in the ADR-010 reading or the [ADR-017](adr/017-solana-memo-anchoring.md) Merkle leaves, so jitter moves a database row and not an on-chain hash.
- Follow-up recorded in ADR-018: a `location_bssids` column plus set-similarity comparison, so the worker call is gated on the scan actually changing rather than on elapsed time.
- Nominatim reverse-geocoding calls from `POST /server/sensors/:id/refresh-location` now carry a contact address in the `User-Agent` (`edge-tracker/1.0 (contatoviniciom@gmail.com)`). Nominatim's usage policy requires an identifiable contact; requests without one are subject to blocking.

### Security fix — ESP32-S3 device private key removed from tracked source (23 Sep 2026)

- `DEVICE_PRIVATE_KEY` had been hardcoded in `ESP/esp32s3/esp32s3.ino` since commit `8c5d73b` (25 Apr 2026). The repository is public, so the key was compromised from that commit onward. Signing was never live for this node (`UNSIGNED_DEV_BYPASS=1`, [ADR-011](adr/011-unsigned-dev-bypass-for-unported-devices.md)), so nothing was signing envelopes with a forgeable key in production — but the key had to come out before that bypass is ever removed.
- `DEVICE_PRIVATE_KEY`, `DEVICE_PUBLIC_KEY` and `DEVICE_SOURCE` moved into the existing gitignored `ESP/esp32s3/secrets.h`, alongside the Wi-Fi credentials. `secrets.example.h` documents the three symbols and the `openssl` commands to generate a fresh keypair. Decision and the on-device-generation alternative considered are in [ADR-019](adr/019-esp32s3-device-key-out-of-tracked-source.md).
- Also found in history: Wi-Fi SSID/password committed in firmware files across several networks, up to commit `d2cfe51` (17 Aug 2026), which moved them to the same gitignored `secrets.h` pattern. Both leaks need manual rotation by the project owner — tracked outside this repo, not a code change.
- **Follow-up:** confirm whether the physical Node 2 board is currently flashed with the compromised key; if so, generate a new keypair, re-flash, and run "Rotate Key" on the sensor detail page before signing is ever enabled.
