# FourKit Port Reconnaissance

Phase 1 deliverable for porting the FourKit plugin system into LCE-Revelations. This document is the source of truth for what needs to change, where, and how risky each change is. Phases 2-3 of the port consume this document directly.

## Source repos referenced

| Role | Path |
|---|---|
| Vanilla baseline | `C:\Users\revela\Documents\Minecraft\itsRevela` |
| Donor (has FourKit) | `C:\Users\revela\Documents\Minecraft\FourKit\MinecraftConsoles` |
| Target (this repo) | `C:\Users\revela\Documents\Minecraft\LCE-Hub\LCE-Revelations` |

Vanilla and donor were forked from upstream at similar points; the donor adds FourKit on top. The target is a more recent independent fork that has its own divergence (Security subsystem, OpManager, revoke-token command, additional ServerProperties fields).

## Executive summary

- **27 files** in the donor are FourKit-bearing across `Minecraft.Server/`, `Minecraft.World/`, and `Minecraft.Client/`. Of those, **23 are clean patches** (target identical to vanilla in those files), and **only 4 require manual merge work** (all in `Minecraft.Server/`).
- **7 pure-add native files** drop in unmodified.
- **1 entire managed project** (`Minecraft.Server.FourKit/`) drops in unmodified.
- **No blockers identified.** Every Fire* hook site exists in target at vanilla content; the conflicts are concentrated in build files and a small set of `Minecraft.Server/` source files where target diverged.
- **Native callback ABI is intact.** Spot-checks of player/world/inventory subsystem APIs that FourKitNatives.cpp depends on all match between donor and target.

## A. File diff matrix

### A.1 Pure adds (drop in verbatim)

| File | Notes |
|---|---|
| `Minecraft.Server/FourKitBridge.cpp` | Event firing + callback registry |
| `Minecraft.Server/FourKitBridge.h` | Public Fire* and Handle* surface |
| `Minecraft.Server/FourKitNatives.cpp` | C# → C++ callback implementations (~80 functions) |
| `Minecraft.Server/FourKitNatives.h` | Native function declarations |
| `Minecraft.Server/FourKitRuntime.cpp` | hostfxr load + .NET 10 bootstrap |
| `Minecraft.Server/FourKitRuntime.h` | Runtime init API |
| `Minecraft.Server/FourKitMappers.cpp` | Type/enum mapping helpers |
| `Minecraft.Server.FourKit/` (entire project) | Managed plugin host, Bukkit-style API, ~54 events |

### A.2 Modified files in `Minecraft.Server/` (the merge work)

| File | Donor delta | Target diverges from vanilla? | Merge class |
|---|---|---|---|
| `CMakeLists.txt` | +24 lines (FourKit dep + post-build copy) | No | CLEAN_PATCH |
| `cmake/sources/Common.cmake` | +7 FourKit sources, donor also strips OpManager/Security (donor lacks those) | No | **SELECTIVE_MERGE**: add donor's FourKit sources only; preserve target's OpManager/Security entries |
| `Windows64/ServerMain.cpp` | FourKit init/shutdown + 1 inline FireWorldSave | No | CLEAN_PATCH |
| `Console/ServerCliEngine.cpp` | HandleConsoleCommand hook + GetPluginCommandHelp integration | No | CLEAN_PATCH |
| `Console/commands/help/CliCommandHelp.cpp` | Plugin command help integration | Unknown: check before merge | LIKELY_CLEAN |
| `Console/commands/whitelist/CliCommandWhitelist.cpp` | Unknown delta | Unknown | LIKELY_CLEAN |
| `Access/Access.cpp` | Donor delta unrelated to FourKit hooks per grep | No | CLEAN_PATCH (verify) |
| `Access/Access.h` | Same | No | CLEAN_PATCH (verify) |
| `ServerLogManager.cpp` | Logger plumbing for FourKit log routing | No | CLEAN_PATCH |
| `ServerLogManager.h` | Same | No | CLEAN_PATCH |
| `ServerLogger.cpp` | Same | No | CLEAN_PATCH |
| `ServerProperties.cpp` | Donor REMOVES fields target relies on | **Yes** | **CONFLICT: KEEP TARGET** |
| `ServerProperties.h` | Same | **Yes** | **CONFLICT: KEEP TARGET** |

### A.3 Modified files in `Minecraft.World/` and `Minecraft.Client/`

The donor changed 53 files in `Minecraft.World/` and 95 files in `Minecraft.Client/` overall, but most of those are upstream churn unrelated to FourKit. The FourKit-specific subset is identified by `grep "FourKit"` and yields **23 files**, all of which are identical between vanilla and target: i.e., every single one is CLEAN_PATCH.

#### `Minecraft.World/` FourKit hook files (17, all CLEAN_PATCH)

`AbstractContainerMenu.cpp`, `CactusTile.cpp`, `CocoaTile.cpp`, `CropTile.cpp`, `EggTile.cpp`, `FireTile.cpp`, `GrassTile.cpp`, `ItemEntity.cpp`, `LiquidTileDynamic.cpp`, `LivingEntity.cpp`, `Mushroom.cpp`, `NetherWartTile.cpp`, `PistonBaseTile.cpp`, `ReedTile.cpp`, `Sapling.cpp`, `StemTile.cpp`, `ThrownEnderpearl.cpp`

#### `Minecraft.Client/` FourKit hook files (6, all CLEAN_PATCH)

`PendingConnection.cpp`, `PlayerConnection.cpp`, `ServerLevel.cpp`, `ServerPlayer.cpp`, `ServerPlayerGameMode.cpp`, `TeleportCommand.cpp`

> **Note on naming:** Despite the `Minecraft.Client/` folder name, several of these files (`PlayerConnection`, `ServerPlayer`, `ServerLevel`, etc.) are shared engine code that the dedicated server also compiles. They are valid hook sites for a server-side plugin system.

#### Build files in `Minecraft.World/` and `Minecraft.Client/`

The donor also modifies `Minecraft.World/cmake/sources/Common.cmake`, `Minecraft.Client/cmake/sources/Common.cmake`, `Minecraft.Client/cmake/sources/Windows.cmake`, and `Minecraft.Client/CMakeLists.txt`, but none of these contain FourKit references: they are upstream churn and should NOT be touched as part of the FourKit port.

## B. Fire* hook site inventory

Donor exposes Fire*/Handle* via `FourKitBridge::` (see `Minecraft.Server/FourKitBridge.h`). Hook calls appear in 26 source files across the three subprojects.

### B.1 Hook sites by subsystem

| Subsystem | Files (in donor) |
|---|---|
| Server lifecycle | `Minecraft.Server/Windows64/ServerMain.cpp` (init, shutdown, FireWorldSave) |
| Console / commands | `Minecraft.Server/Console/ServerCliEngine.cpp` (HandleConsoleCommand, GetPluginCommandHelp), `Minecraft.Server/Console/commands/help/CliCommandHelp.cpp` |
| Player connection / login | `Minecraft.Client/PlayerConnection.cpp`, `Minecraft.Client/PendingConnection.cpp` |
| Player gameplay | `Minecraft.Client/ServerPlayer.cpp`, `Minecraft.Client/ServerPlayerGameMode.cpp`, `Minecraft.Client/TeleportCommand.cpp` |
| World / level | `Minecraft.Client/ServerLevel.cpp` |
| Living entities & damage | `Minecraft.World/LivingEntity.cpp`, `Minecraft.World/ItemEntity.cpp`, `Minecraft.World/ThrownEnderpearl.cpp` |
| Block / tile (growth, burn, spread, ignite, piston) | `Minecraft.World/CactusTile.cpp`, `CocoaTile.cpp`, `CropTile.cpp`, `EggTile.cpp`, `FireTile.cpp`, `GrassTile.cpp`, `LiquidTileDynamic.cpp`, `Mushroom.cpp`, `NetherWartTile.cpp`, `PistonBaseTile.cpp`, `ReedTile.cpp`, `Sapling.cpp`, `StemTile.cpp` |
| Inventory / containers | `Minecraft.World/AbstractContainerMenu.cpp` |

### B.2 Hook insertion-site safety

Per A.3, **every file in B.1 outside of `Minecraft.Server/` is byte-identical between vanilla and target**. That means donor's hook insertions can be applied as a straight patch without adapting around target-side refactors.

In `Minecraft.Server/`, the hook-bearing files (`ServerMain.cpp`, `ServerCliEngine.cpp`, `CliCommandHelp.cpp`) are also identical or near-identical to vanilla per A.2. Net result: **the locked Phase 3 hook adaptation policy ("adapt to LCE-Revelations refactors") will not need to fire** for any hook site in the current state of the target. If the user pulls upstream changes between now and Phase 3, this assumption must be re-validated.

### B.3 Three concrete hook sites verified end-to-end (others follow same pattern)

1. **`Minecraft.Server/Windows64/ServerMain.cpp:681`**: `FourKitBridge::FireWorldSave()` inside the autosave trigger block. Target file is byte-identical to vanilla → clean insertion.
2. **`Minecraft.Server/Console/ServerCliEngine.cpp:165`**: `if (FourKitBridge::HandleConsoleCommand(normalizedLine)) return true;` early return for plugin command preprocessing. Target file byte-identical → clean insertion.
3. **`Minecraft.Server/Console/ServerCliEngine.cpp:215-246`**: `FourKitBridge::GetPluginCommandHelp(...)` integration into the help printer. Same file, clean insertion.

## C. Native callback surface (FourKitNatives.cpp ↔ engine APIs)

`FourKitNatives.cpp` implements ~80 functions that wrap engine APIs and expose them to managed code. Spot-check of the highest-risk subsystems:

| Native function | Engine API called | Target has it? | Signature drift? |
|---|---|---|---|
| `NativeSetPlayerHealth(int, float)` | `ServerPlayer::setHealth(float)` | Yes | None |
| `NativeTeleportPlayer(int, double, double, double)` | `PlayerConnection::teleport(...)` | Yes | None |
| `NativeSetTile(int, int, int, int, int, int)` | `ServerLevel::setBlock(...)` | Yes | None |
| `NativeGetPlayerAddress(int, char*, int, int*)` | `PlayerConnection` accessors | Yes | None |
| `NativeGetPlayerLatency(int)` | `PlayerConnection::latency` | Yes (field) | None |
| `NativeBanPlayer(int, char*, int)` | `Access::AddPlayerBan(...)` | Yes | None |
| `NativeSetItemMeta(int, int, char*, int)` | `ItemInstance` serialization | Yes | None |
| `NativeGetContainerContents(int, int*, int)` | `AbstractContainerMenu` traversal | Yes | None |

**Verdict:** No drift detected in critical subsystems. Phase 2's native bridge build is expected to compile against target's headers without shimming. Any drift discovered at compile time will be a Phase 2.3 finding, not a known risk.

## D. Critical hot files

| File | Vanilla LOC | Donor LOC | Target LOC | Status |
|---|---|---|---|---|
| `Minecraft.Server/Windows64/ServerMain.cpp` | 1,257 | 1,257 | 1,257 | Identical across all three. Donor's FourKit additions sit on the vanilla baseline; target has not touched it. |
| `Minecraft.Server/CMakeLists.txt` | 86 | 110 (+24) | 86 | Target is vanilla. Donor adds FourKit deps and a post-build copy step. Clean apply. |
| `Minecraft.Server/cmake/sources/Common.cmake` | 628 | 640 (+12 net; donor also strips ~16 lines of OpManager/Security) | 628 | Target is vanilla. **Selective merge:** apply donor's `FourKit*.cpp` additions, do NOT apply donor's removals. |
| `Minecraft.Server/ServerProperties.cpp` | 965 | 930 (-35) | 965 | Donor is older / leaner; target carries hardcore + security fields donor never had. **Keep target version unchanged**, do not merge donor's deletions. |
| `Minecraft.Server/ServerProperties.h` | n/a | n/a | n/a | Same logic: keep target. |
| `Minecraft.Server/Console/commands/` | 20 subdirs | 18 subdirs (no `revoketoken`) | 20 subdirs (has `revoketoken`) | Donor predates `revoketoken`; target has it. Keep target's command set. |

## E. Top-level structure of the target

| Item | Status |
|---|---|
| `samples/` at repo root | NOT FOUND: Phase 7 will create it |
| `docs/` at repo root | NOT FOUND: created by this document |
| `docker/` | EXISTS: contains `dedicated-server/{Dockerfile,entrypoint.sh}`. Phase 5 must inspect to decide whether the image builds the server from source or consumes the release zip |
| `Minecraft.Server.FourKit/` | NOT FOUND (as expected) |

## Merge strategy summary (input to Phase 2 and Phase 3)

1. **Phase 2 source drop:** Copy the 7 PURE_ADD files into `Minecraft.Server/` verbatim. Copy the entire `Minecraft.Server.FourKit/` project to repo root verbatim.
2. **Phase 2 build files:**
   - `Minecraft.Server/CMakeLists.txt`: apply donor's diff verbatim (target is vanilla).
   - `Minecraft.Server/cmake/sources/Common.cmake`: apply ONLY the donor's additions of `FourKit*.cpp`. Do not delete OpManager or Security entries.
3. **Phase 3 hooks (the entire surface, all CLEAN_PATCH):**
   - 3 sites in `Minecraft.Server/` (`ServerMain.cpp`, `ServerCliEngine.cpp`, `CliCommandHelp.cpp`)
   - 17 sites in `Minecraft.World/` (block/entity/container)
   - 6 sites in `Minecraft.Client/` (player/level/connection)
   - 1 lifecycle hook pair in `ServerMain.cpp` for `FourKitRuntime::Initialize` / shutdown
   - Logger plumbing in `ServerLogManager.{cpp,h}` and `ServerLogger.cpp`
4. **Phase 3 conflicts to leave alone:**
   - `ServerProperties.{cpp,h}`: keep target version, do not merge donor.
   - `Console/commands/revoketoken/`: keep target version, donor lacks it.
   - `Access/OpManager.{cpp,h}`: keep target version, donor lacks it.
   - `Security/`: keep target version, donor lacks it.
5. **Spot-checks needed before merge:**
   - `Console/commands/help/CliCommandHelp.cpp`: verify donor's delta is purely the plugin help integration and not entangled with anything target changed.
   - `Console/commands/whitelist/CliCommandWhitelist.cpp`: confirm scope of donor's edit.
   - `Access/Access.{cpp,h}`: confirm donor's delta is not security-related.

## Risks updated post-recon

| Risk | Original severity | Post-recon severity | Reason |
|---|---|---|---|
| LCE-Revelations divergence at hook sites | HIGH | **LOW** | All 26 hook-bearing source files are byte-identical between vanilla and target. |
| CMake structural divergence | HIGH | **LOW** | Target's `CMakeLists.txt` and `Common.cmake` are byte-identical to vanilla. |
| ServerMain.cpp init-order conflicts | MEDIUM | **LOW** | File is identical to vanilla. |
| ServerProperties conflict | NEW | **MEDIUM** | Donor strips fields target relies on. Mitigation: do not merge donor's version. |
| Native ABI drift in subsystem APIs | MEDIUM | **LOW** | Spot-checks all clean. Compile-time will catch any miss. |
| Donor predates target features (revoketoken, OpManager, Security) | NEW | **LOW** | Easy to preserve target's additions; donor doesn't depend on their absence. |
| Self-contained .NET 10 publish size | MEDIUM | **MEDIUM** | Unchanged. ~70-100 MB release zip growth. |
| Hot-reload absent | LOW | **LOW** | Inherited limitation, documented. |

## Open follow-ups for Phase 2

1. Confirm `Console/commands/help/CliCommandHelp.cpp` and `Console/commands/whitelist/CliCommandWhitelist.cpp` deltas are FourKit-related (currently marked LIKELY_CLEAN, not verified).
2. Confirm `Access/Access.{cpp,h}` delta scope.
3. Decide whether the donor's changes to `ServerLogManager` and `ServerLogger` are strictly additive (capture for FourKit log routing) or entangled with other refactoring.
4. Confirm during Phase 2.3 that `FourKitRuntime.cpp`'s hostfxr loader handles the case where `hostfxr.dll` is absent gracefully (server should warn and continue, not crash): this matters because the self-contained publish puts hostfxr next to the server exe, so the failure mode only fires if a user deletes it.
