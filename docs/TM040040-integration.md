# TM040040-2024-303 right-hand trackpad integration

Status: preparation only; no firmware behavior changed.

## Target
- PandaKB Sofle RGB MX, nice!nano v2 split BLE
- Left: existing OLED + EC11 encoder (central)
- Right: existing OLED + Cirque TM040040-2024-303 (peripheral)
- RGB remains disabled
- TM040040 shipped SPI, convert to I2C after verifying R1 on the physical module
- Preserve the current working main firmware until a pair of test builds passes

## Existing configuration observed
- ZMK revision v0.3 in config/west.yml
- build.yaml: Sofle_L nice_oled, Sofle_R nice_oled, settings_reset
- config/Sofle.conf: CONFIG_ZMK_RGB_UNDERGLOW=n, CONFIG_WS2812_STRIP=n
- config/Sofle.keymap: left and right encoder sensor bindings
- OLED module: mctechnology17/zmk-nice-oled

## Integration checklist
- [ ] Inspect actual TM040040 PCB, confirm R1 and I2C mode conversion.
- [ ] Obtain PandaKB Sofle_R shield pin mappings and verify available SDA/SCL, DR/interrupt GPIO, and electrical compatibility with right OLED. Do not assume pin numbers.
- [ ] Verify OLED and trackpad I2C addresses, pullups, voltage, and simultaneous bus use.
- [ ] Choose driver compatible with pinned ZMK v0.3 (e.g. appropriate out-of-tree Cirque module), or upgrade ZMK and validate OLED module compatibility. Zephyr 4.1 upstream Cirque driver is NOT automatically available in v0.3.
- [ ] Confirm split-peripheral input event forwarding to central; build matching left and right images.
- [ ] Add right-only devicetree overlay and appropriate Kconfig, keeping existing build targets intact; create separately named test variants.
- [ ] Replace right encoder bindings only after confirming input mapping; retain left encoder.
- [ ] Build both halves in GitHub Actions and inspect logs before flashing.
- [ ] Test cursor movement, tap click, scroll, BLE reconnection, OLED status, and idle/sleep power behavior on real hardware.

## References
- https://zmk.dev/blog/2025/12/09/zephyr-4-1
- https://github.com/nxtkb/cirque-input-module
- https://github.com/Frosthaven/zmk-config-corne-eclipse/blob/main/WIRING_TRACKPAD.md

## Important
This document is a safe starting point, not a validated wiring diagram or functioning trackpad firmware.


## Repository pin audit (2026-11-20)

Source files inspected in this repository:
- `boards/shields/Sofle/Sofle.dtsi`
- `boards/shields/Sofle/Sofle_R.overlay`
- `boards/shields/Sofle/Sofle_L.overlay`
- `boards/shields/Sofle/Kconfig.defconfig`

Confirmed existing OLED bus:
- `i2c0_default`: SDA `P0.17`, SCL `P0.20`.
- `&pro_micro_i2c` is enabled and contains `ssd1306@3c` (128x32).
- OLED is selected via `zephyr,display = &oled` in shared shield DTSI.
- Cirque module example uses `glidepoint@2a` (address `0x2a`), so address collision with `0x3c` is not expected, subject to actual device confirmation.

**Pin conflict warning**: shared `Sofle.dtsi` assigns right encoder A/B to `&pro_micro 20` / `&pro_micro 21`. These are *Pro Micro header pin identifiers*, NOT nRF `P0.20` / `P0.21`; do not conflate the numbering systems. Right overlay currently enables `&right_encoder`. Right keyboard matrix uses `&pro_micro 10,16,14,15,18,19` for columns and `&pro_micro 5,6,7,8,9` for rows. Select DR only after mapping physical header pads to nRF GPIO and checking actual accessibility.

Potential external module:
- https://github.com/nxtkb/cirque-input-module
- Its README documents `cirque,pinnacle2`, `reg = <0x2a>`, `data-ready-gpios`, `CONFIG_ZMK_POINTING`, and a split-oriented input path.
- Example DR GPIO `P0.22` is NOT a verified free PandaKB pin.
- Do not assume module's current main branch is compatible with the repository's ZMK v0.3. Verify at a pinned commit and run test builds.

Split design:
- Right = peripheral trackpad input source.
- Left = central; both sides need coordinated firmware with input-split routing and input listener on central (see https://zmk.dev/docs/hardware-integration/pointing).
- Keep right OLED, left OLED and left EC11 unchanged where possible; disable right EC11 only in the dedicated test variant.
- Both left and right test firmware images must be flashed together for split pointer forwarding.

Remaining blockers before code enablement:
1. Identify a physical accessible DR GPIO on the right without matrix/OLED collision.
2. Check OLED and trackpad bus pullups, connector pinout, 3.3 V supply, and R1 I2C conversion on delivered unit.
3. Confirm driver + ZMK v0.3 compatibility, including ZMK input-split availability and OLED build.
4. Create isolated test build targets and verify GitHub Actions success.

No trackpad functionality is enabled by this documentation update.


## Corrected measured encoder pad mapping (user continuity test, 2026-11-20)

The earlier visual inference D4/P0.22 and D5/P0.24 was **incorrect** and must not be used.

Measured pad connections (right PCB, user-confirmed):
- Pad immediately above GND: nice!nano D17 (A2), nRF P0.31.
- Pad immediately below GND: nice!nano D16 (A1), nRF P0.29.
- Lower of two pads in right column: nice!nano D10, nRF P0.09.
- Ground pad: GND.

IMPORTANT: Existing `Sofle.dtsi` uses `&pro_micro 10` as a right-hand matrix column, so D10/P0.09 is **not** a safe DR candidate unless verified otherwise. The pin identifiers `&pro_micro 20/21` in the encoder definition are not the physical D20/D21 labels; check the board's `pro_micro` mapping before changing.

Next: inspect actual nRF board/pro_micro mappings and check whether D16/P0.29 or D17/P0.31 is occupied by the matrix or another device. Do not enable DR in firmware until verified.


## Verified ZMK connector mapping and DR selection (2026-11-20)

Checked the official ZMK nice!nano `arduino_pro_micro_pins.dtsi` mapping:
- `&pro_micro 20` -> `&gpio0 29` (P0.29), right encoder A.
- `&pro_micro 21` -> `&gpio0 31` (P0.31), right encoder B.
- `&pro_micro 10` -> `&gpio0 9` (P0.09), right matrix column.

These match the user's continuity measurements by nRF GPIO. Note that the **D16/D17 labels in the photo** are not the same as the ZMK `&pro_micro 16/17` identifiers. Use the nRF P0.xx values as the reliable reference.

**Proposed DR: P0.29, `<&gpio0 29 GPIO_ACTIVE_HIGH>`**, from the measured pad below GND; alternate P0.31. Disable `&right_encoder` in a right-only test overlay before claiming the pad. P0.09 must remain in the matrix.

References:
- https://github.com/zmkfirmware/zmk/blob/main/app/module/boards/nicekeyboards/nice_nano/arduino_pro_micro_pins.dtsi
- https://github.com/nxtkb/cirque-input-module

Compatibility remains untested: the current cirque module README uses newer `nice_nano//zmk` build syntax and ZMK pointing features, while this repository pins ZMK v0.3 and builds `nice_nano_v2`. Do not add its main branch to the working firmware without an isolated test and compatibility assessment.
