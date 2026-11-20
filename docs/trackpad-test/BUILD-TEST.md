# Test-build instructions (draft PR #1)

Branch: `feature/cirque-tm040040-prep`
Draft PR: https://github.com/mishii81/pandakb-sofle-zmk-config/pull/1

## Active firmware changes in this branch
- `config/west.yml`: pins `geeksville/cirque-input-module` to commit `ba53fe81bad6690c5999b9e0c113988c1246b4d1`.
- `boards/shields/Sofle/Sofle.dtsi`: shared input split and central listener.
- `boards/shields/Sofle/Sofle_L.overlay`: enables listener on left central.
- `boards/shields/Sofle/Sofle_R.overlay`: right I2C `cirque,pinnacle` at `0x2a`, DR=P0.29, right encoder disabled.
- `config/Sofle.conf`: enables ZMK pointing, Zephyr input, I2C, GPIO.
- `build.yaml` unchanged: existing left/right OLED and reset targets. The draft PR is for CI testing only.

## How to inspect build
Open the draft PR's Checks tab or Actions > Build ZMK firmware, select the latest run on this branch. Verify:
1. `west update` fetches the pinned cirque module.
2. `Sofle_L nice_oled` compiles and includes `zmk,input-listener`.
3. `Sofle_R nice_oled` compiles and includes `cirque,pinnacle`, `zmk,input-split`.
4. `settings_reset` still compiles.
5. No devicetree or Kconfig warnings, and output UF2 files for both halves.

## Safety
- Draft PR: do NOT merge until validated.
- Do NOT flash test firmware before physical TM040040 I2C conversion and wiring are verified.
- Build success only verifies compilation, not physical I2C communication, touch response, split BLE, or OLED coexistence.
- Existing `main` release workflow is gated to `main`, so branch pushes must not publish a new latest release.

## Troubleshooting starting points
- If `CONFIG_ZMK_POINTING` unavailable, inspect exact ZMK v0.3 revision and log.
- If driver fails, inspect `geeksville/cirque-input-module` CMake/Kconfig compatibility and pin an older compatible commit.
- If split binding fails, inspect the generated `zephyr.dts` in the Actions build logs.
- If right encoder sensor reference causes warnings, adjust `sensors` and `sensor-bindings` in the test branch only.
