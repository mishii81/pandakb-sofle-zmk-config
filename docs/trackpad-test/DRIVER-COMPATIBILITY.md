# Cirque driver compatibility assessment (2026-11-20)

## Current repository baseline
- `config/west.yml` pins `zmkfirmware/zmk` to `v0.3`.
- `build.yaml` uses `nice_nano_v2`, `Sofle_L nice_oled`, and `Sofle_R nice_oled`.
- Existing OLED module: `mctechnology17/zmk-nice-oled`.
- RGB remains disabled.

## Candidate assessment

### nxtkb/cirque-input-module
https://github.com/nxtkb/cirque-input-module

README documents `cirque,pinnacle2`, `data-ready-gpios`, `CONFIG_ZMK_POINTING`, `nice_nano//zmk`, relative and absolute input modes. It is actively extended for split keyboards, but its documented build targets and modern pointing configuration do **not** establish compatibility with pinned ZMK v0.3. Do not add it to production `west.yml` until tested.

### v0.3-compatible fork leads
- https://github.com/kalbasit/zmk-config/blob/main/config/west.yml — references ZMK v0.3 and a pinned `kalbasit/cirque-input-module` commit `c3e1fcfd8f9fba2e1f8f09ed664822870bce61ca`.
- https://github.com/ArtemYurov/zmk-mix/tree/zmk-v0.3 — cited as working v0.3 Cirque configuration in https://github.com/zmkfirmware/zmk/issues/3341.
- https://github.com/felixm12138/zmk-mix — documents ZMK v0.3 with a `petejohanson/cirque-input-module` dependency.

These are leads, NOT verified compatible modules for this particular shield and OLED combination. Check their actual driver binding before choosing the `compatible` name and GPIO property; old drivers may use `cirque,pinnacle` and `dr-gpios` rather than the newer `cirque,pinnacle2` and `data-ready-gpios`.

## Critical warning about current staged templates

`TM040040-split.dtsi`, `TM040040-left.overlay`, and `TM040040-right.overlay` in this docs folder are **design sketches**, not complete, validated buildable ZMK v0.3 DTS fragments. In particular, the `zmk,input-split` node topology and listener properties require confirmation against the selected ZMK version; do not copy directly into the active shield.

## Safest implementation sequence
1. Choose a v0.3-compatible Cirque driver and pin its exact revision.
2. Inspect its `dts/bindings` and example configuration to settle the actual `compatible` and DR GPIO property names.
3. Determine whether right peripheral pointing input forwarding is supported by ZMK v0.3 without a patch. If not, evaluate a separate upgraded-ZMK test branch.
4. Prepare separate `build.yaml` test targets, with both halves updated together, without modifying working targets.
5. Run GitHub Actions, inspect logs, and only then flash test UF2s.

## Confirmed physical pin choice
- User measured encoder pad below GND -> nRF P0.29. Proposed right trackpad DR: `<&gpio0 29 GPIO_ACTIVE_HIGH>`.
- Pad above GND -> P0.31 (backup).
- Right-column lower pad -> P0.09, already used in right matrix; never reassign.
- Right encoder must be disabled on test right firmware.
- Existing OLED I2C pins P0.17/P0.20 and address 0x3c are unchanged; Cirque candidate address 0x2a.


## Verification update: official ZMK v0.3 documentation

The ZMK **v0.3-specific** documentation confirms that peripheral pointing is supported with `zmk,input-split` and `zmk,input-listener`:
- https://v0-3-branch.zmk.dev/docs/development/hardware-integration/pointing
- https://v0-3-branch.zmk.dev/docs/config/split

Thus, split pointing itself does NOT require upgrading ZMK beyond v0.3. The staged shared split and listener structure matches the official v0.3 documentation at the conceptual level. Driver-specific DTS and Kconfig remain unverified.

A public configuration `kalbasit/zmk-config` explicitly pins ZMK to `3cf9006a9377f21c99e944e29e2a507d8c3e9069` (v0.3) and `cirque-input-module` to `c3e1fcfd8f9fba2e1f8f09ed664822870bce61ca`. This is evidence of a compatible *combination in another project*, not proof that this PandaKB repository will build with it:
https://github.com/kalbasit/zmk-config/blob/main/config/west.yml

Attempted to inspect the exact fork's README and binding files directly through GitHub, but the connector returned 404 for the specified repository/ref. Consequently, its binding name and Kconfig symbols are NOT confirmed. Do not silently substitute the newer `cirque,pinnacle2` syntax.

Next gate: retrieve exact driver sources, validate DTS binding and version, then enable an isolated build. No build or firmware change has been made.


## Binding source inspection completed

Fetched these **exact source files** from `petejohanson/cirque-input-module` via GitHub connector:
- `dts/bindings/input/cirque,pinnacle-i2c.yaml`: `compatible: "cirque,pinnacle"`, includes `i2c-device.yaml` and `cirque,pinnacle-common.yaml`.
- `dts/bindings/input/cirque,pinnacle-common.yaml`: defines `dr-gpios`, `sleep`, `no-taps`, `sensitivity` with enum `1x/2x/3x/4x`.
- `zephyr/module.yml`: `dts_root: .`, CMake and Kconfig integration.

Updated the staged right overlay to `cirque,pinnacle`, `dr-gpios = <&gpio0 29 GPIO_ACTIVE_HIGH>`, `sensitivity = "2x"`. This matches the inspected *repository HEAD* binding, not yet a verified pinned v0.3-compatible commit.

Important remaining checks:
- Choose an immutable revision of the module known to build with ZMK v0.3.
- Inspect `drivers/Kconfig` and ZMK v0.3 pointing config to choose correct Kconfig symbols.
- Confirm right OLED plus trackpad simultaneous operation and power use.
- Correct the separately staged `docs/TM040040-right-overlay.template.dts` legacy template before using it; it still shows newer driver syntax and is not a build input.
- Build left/right firmware; no GitHub Actions run was triggered by docs-only commits.


## Kconfig source inspection (2026-11-20)

Fetched directly from `petejohanson/cirque-input-module`:
- `drivers/Kconfig` -> `rsource "input/Kconfig"`
- `drivers/input/Kconfig` -> `menuconfig INPUT_PINNACLE`, default y, requires `GPIO`, `SPI || I2C`, `INPUT`, and `DT_HAS_CIRQUE_PINNACLE_ENABLED`.
- Optional `CONFIG_ZMK_INPUT_PINNACLE_IDLE_SLEEPER` appears only within `if ZMK_MOUSE`; its availability in the selected ZMK v0.3 version is not established.
- `zephyr/module.yml` declares `build.cmake`, `build.kconfig`, `settings.dts_root`.

A suitable test configuration is likely `CONFIG_INPUT=y`, `CONFIG_I2C=y`, `CONFIG_GPIO=y`, `CONFIG_INPUT_PINNACLE=y` for the peripheral half, but these have not been compiled. Check symbols against the selected ZMK commit before adding to `config/Sofle.conf`.

Updated `docs/TM040040-right-overlay.template.dts` to use the same older driver binding as `docs/trackpad-test/TM040040-right.overlay`, eliminating conflicting `pinnacle2`/data-ready naming.

**No driver has been pinned to a commit, no active firmware files or build.yaml changed, and no Actions build has run.**
