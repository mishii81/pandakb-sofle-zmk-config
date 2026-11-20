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
