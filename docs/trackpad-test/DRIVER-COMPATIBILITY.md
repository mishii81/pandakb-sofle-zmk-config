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
