# Changelog (firmware only)

## 26.09.06

### Fixed: battery drain while powered off
- Power and backlight GPIOs now retain their inactive levels during ESP32 deep sleep,
  preventing the probe, ADC, or display circuits from being re-enabled by floating pins.
- The backlight is explicitly switched off before entering deep sleep.
- Shutdown waits for the active-low power button to be released, preventing an immediate
  EXT0 wake-up while the button is still held. GPIO35 remains the power-on/wake input.
- Corrected the battery charging flag, which was previously inverted and prevented the
  low-battery protection from reliably detecting discharge.

## 26.07.02

### Fixed: detail screen unreachable / device reboots on button press
Root cause of "the probe detail page no longer opens on the device". `current_screen_page`
is set by the power-button task at press time, but the screen task redraws independently
every 1 s and re-reads the connected-probe list fresh each time — using the raw C++ array
index (`operator[]`, no bounds check). If the probe count drops between the button press and
the next redraw (loose probe contact, a transient sensor read miss), the detail page indexed
past the end of the now-shorter list — undefined behaviour, which crashed the device. Since
26.07.01 made fault resets *resume* instead of sleep, the crash showed up as an immediate
reboot back to the overview screen — from the button, indistinguishable from "nothing
happened, the detail page just doesn't open anymore".

- `disp::display_update()` now re-checks the current page against the freshly-read probe
  count before indexing into it, and falls back to the overview screen instead of reading
  past the end of the list.

## 26.07.01-2

### Fixed regular crashes/reboots (concurrent JSON access)
Root cause of "the firmware still crashes regularly". All the JSON builders shared **one
global `JsonDocument`**. It was mutated concurrently by three tasks — the web server
(`/api/*`), the MQTT publisher and the Opengrill publisher (both publish every second) — and
`save_settings()`/`save_probes()` re-entered it via `publish_*()` while handling a web POST.
Concurrent `clear()`/`add()`/`serializeJson()` on a single ArduinoJson memory pool corrupts
its heap → panic/reboot. Because 26.07.01 made fault resets *resume running* instead of going
to sleep, this now shows up as a visible reboot rather than the device "turning itself off".

- **Each serializer now uses its own local `JsonDocument`** → fully reentrant and thread-safe
  (the ESP32 heap allocator is itself thread-safe). No shared JSON state between tasks.
- **`Probe::push_coarse` no longer puts a 360 B array on the probes-task stack** (made static);
  it runs in a small-stacked task and could overflow it on the first coarse-buffer compaction
  during a long cook.
- **Bigger task stacks.** The alarm and probes tasks were only 1000 bytes — too tight for the
  probes task's float/`log()` thermistor math. Raised (alarm 2048, probes/battery 3072 B).

## 26.07.01-1

### Web interface — Energy & Diagnostics on the About page
- The device web UI (`/about`) now shows an **Energy** section (battery %, charging,
  measured cell voltage) and a **Diagnostics** section (last restart reason, last power-off
  reason) directly on the device, above the Authors. Reads the existing `/api/grill` fields
  added in 26.07.01; no API or behavior change.

## 26.07.01

### The device no longer turns itself off unexpectedly
Fix for "the thermometer switched itself off after ~1.5 h with the battery still over 80 %".
The firmware has no auto-off timer and no battery cutoff — the real cause was that **any**
unexpected reset (brownout from a WiFi TX current spike on an aging cell, watchdog, or a
crash) hit the boot-time "hold the button to turn on" gate and, since the button wasn't held,
went straight back into deep sleep. To the user that looks like a self-power-off.

- **Reset-reason-aware boot gate.** The "hold the button ≥2 s to turn on" gate now applies
  only to deliberate power-ons: a cold power-up (`ESP_RST_POWERON`) or a wake from our own
  deep sleep (`ESP_RST_DEEPSLEEP`). After any fault reset (brownout/panic/watchdog/…) the
  device **resumes running** instead of sleeping, so a transient fault self-recovers.
- **Protective low-battery cutoff added.** The device now runs until powered off by the
  button, and only shuts itself down to protect the cell when — while not charging — the
  fuel-gauge SoC drops to ≤5 % (plausible, non-zero reading) or the measured cell voltage
  drops to ≤3.2 V (backstop for a miscalibrated gauge). Requires ~15 s of consecutive
  confirmations so a single I2C glitch can't power the device off.
- **Power button is debounced.** GPIO35 is input-only with no internal pull; it is now
  sampled with a short debounce so electrical noise (moisture/EMI) can no longer masquerade
  as a 2-10 s press and trigger a shutdown.
- **Shutdown/reset diagnostics.** The status API now reports `last_off_reason`
  ("button"/"low_battery"/"boot_gate"), `last_reset_reason` (e.g. "brownout"/"panic"), and
  `battery_millivolts` (measured cell voltage), so an apparent self-power-off can be
  diagnosed after the fact — and shown in the app.

## 26.06.30-3

### Connectivity / stability fixes
Fixes for "the app can no longer connect", "the web interface keeps cutting out" and an
unresponsive device — all caused by the power-saving mode being too aggressive (and on by
default).

- **`power_saving` now defaults to OFF** ("always reachable"). The device ships fully
  reachable; battery mode is opt-in via the app (Settings → power saving). Devices that
  already stored `power_saving = true` keep it, but it is now far less disruptive (below).
- **Power-saving no longer cripples the radio.** It used to drop TX power to 11 dBm, which
  made connections weak and flaky. TX power now stays at full; power-saving keeps only the
  standard modem-sleep (the Arduino default), which does not break reachability.
- **SoftAP is stopped with `softAPdisconnect()` instead of `WiFi.mode(WIFI_STA)`.** Switching
  WiFi mode at runtime tore down the network interface and killed the mDNS responder the
  app uses to find the device. Now the AP beaconing stops while STA + mDNS stay up.
- **`GET /api/probes/history` uses a fixed static buffer** instead of a 24 KB `malloc` per
  request. Under WiFi heap churn that allocation intermittently failed (503) and fragmented
  the heap, which made the web server drop connections.
- **Web server loop services sockets every 5 ms** (was 20 ms) for snappier HTTP.

## 26.06.30-2

### Power / Display fixes
- **Display no longer switches off on its own.** Power-saving used to silently override a
  "never" (0) display timeout with a 3-min backlight / 5-min screen-off default. Several
  users read this as "the device turned itself off after a few minutes". The override is
  removed: display timeouts are now governed purely by the user's explicit settings
  (`backlight_timeout_minutes` / `screen_timeout_minutes`), where `0` always means "never".
  Power-saving now affects only the Wi-Fi radio (modem sleep / reduced TX / SoftAP teardown),
  never the screen.
- **`power_saving` advertised in `GET /api/info` `capabilities`.** The Android app gates its
  power-saving toggle on this flag, so the setting was previously invisible in the app even
  though the firmware supported it. The toggle now appears automatically.

## 26.06.30-1

### History / Long cooks
- **Two-tier temperature history** for long cooks (Pulled Pork etc.). The graph window is no
  longer capped at 10 minutes:
  - *Fine tier* — last ~30 min at 10 s resolution (recent detail).
  - *Coarse tier* — the whole cook, sampled at an **adaptive interval** that starts at 60 s
    and doubles whenever the buffer fills (covers 3 h → 6 h → 12 h → 24 h → 48 h …).
  - Memory stays **fixed** (~720 B/probe, ≈5.8 KB total) regardless of cook length, and the
    sampling rate *drops* over time → battery-neutral. The probe poll/ADC cadence is
    unchanged.
- **`GET /api/probes/history` extended** (backward compatible): adds top-level
  `coarse_interval_seconds` and a per-probe `history_coarse` array. The endpoint buffer is
  now heap-allocated per request (and freed) instead of a permanently reserved static buffer.

## 26.06.30

### Battery / Power
- **`power_saving` setting** (default `true`): a real toggle between "max battery" and
  "always reachable". Persisted in NVS, exposed via `GET`/`POST /api/settings`.
- **WiFi modem-sleep actually applied**: the radio now uses `WIFI_PS_MIN_MODEM` in
  power-saving mode (previously the code still ran `setSleep(false)` despite the
  25.06.28 changelog claim). Single largest battery saving.
- **SoftAP shut down after connect** (power-saving mode): once the home network is
  joined the device switches to STA-only, stopping continuous AP beaconing. Re-running
  setup then requires a reboot/factory reset.
- **Reduced WiFi TX power** (`WIFI_POWER_11dBm`) in power-saving mode.
- **Display auto-off in power-saving mode**: when the user leaves the timeouts at 0,
  power-saving applies a default 3-min backlight / 5-min screen timeout. Explicit user
  settings always take precedence. (The raw default stays 0; the earlier 25.06.28 "3 min
  default" claim was reverted in the code and is intentionally *not* reintroduced.)
- **Lower task wakeup rates**: webserver loop `delay(2)`→`delay(20)`; MQTT/Opengrill
  tasks idle at 1 s when no broker is configured (were 50 ms); battery poll 1 s→5 s;
  probe poll 500 ms→1 s in power-saving mode. Removed a stray `delay(10)` from the
  display render path.

### Bug fixes
- **`POST /api/settings` no longer wipes config**: settings are now merged — only fields
  present in the request body are updated. Previously a partial update (e.g. from the
  Android app) reset `local_ap_*`, `wifi_*` IP config, `mqtt_*`, brightness and timeouts
  to empty/zero, which could break the setup AP and blank the display.

### Docs
- Corrected the setup AP SSID format in `docs/android_app.md` to `FreeGrilly_<mac6>`
  (last 6 hex of the Wi-Fi MAC), matching `generate_hostname()`.
- Documented `power_saving` in `docs/openapi.yaml` and `docs/android_app.md`.

> **Note on 25.06.28 below:** the "Battery / Power" items in that entry were partly
> aspirational — `WIFI_PS_MIN_MODEM` was not actually in the source and the backlight
> default had been reverted to 0. The Unreleased entry above reflects what the firmware
> now really does.

## 25.06.28

### Battery / Power
- **WiFi Modem-Sleep**: switched from `setSleep(false)` to `setSleep(WIFI_PS_MIN_MODEM)` — single largest battery saving (~2–3×), STA connection remains stable
- **Dynamic Frequency Scaling (DFS)**: CPU scales between 80–240 MHz automatically (`esp_pm_configure`); peripherals unaffected (`light_sleep_enable=false`)
- **Default backlight timeout**: changed from 0 (always on) to 3 minutes for new installs; button or alarm wake re-enables it
- **Webserver idle delay**: `delay(1)` → `delay(2)` to ease CPU load in webserver task

### New Features
- **Temperature history ringbuffer**: each probe stores up to 60 samples (sampled every 10 s, ~10 min of history) in RAM as `int16_t` (celsius×10, ~1 KB total)
- **ETA / remaining-time estimation**: `Probe::seconds_to_target()` uses linear regression over the last 20 history samples; displayed on LCD and in web API
- **mDNS discovery**: device advertises as `free-grilly-<uuid8>.local` via `_http._tcp` and `_free-grilly._tcp` with TXT records (uuid, name, fw)
- **Web alarm banner + Mute button**: sticky red banner when any probe alarms; browser Notification API; `POST /api/alarm/mute` silences buzzer
- **Gzip web assets**: CSS (~130 KB) and JS compressed in Flash via `tools/generate_web_assets.py`; served with `Content-Encoding: gzip`

### Web UI (html_source/index.html)
- Added Canvas sparkline graph per probe card (no external library, pure `<canvas>`)
- Added ETA display per probe card (`eta_seconds` from `/api/grill`)
- Alarm banner with mute button
- Browser Notification support for background alerts
- Fixed `base_url` hardcode — now uses relative URLs (works from any network)

### API
- `GET /api/grill` extended: adds `alarm_active`, `mdns_hostname`; per-probe `alarm` + `eta_seconds`
- `GET /api/probes/history` — returns history ringbuffer for all 8 probes
- `POST /api/alarm/mute` — mutes active alarm
- `GET /api/info` — lightweight device identity + capabilities array
- `docs/openapi.yaml` updated to v25.6.28

### LCD (Display.cpp)
- Detail screen: shows ETA (`in H:MM`) on line 2; probe label blinks (inverted) when alarm active
- Info screen: shows mDNS short hostname (e.g. `a1b2c3d4.local`)

### Build / CI
- New `tools/generate_web_assets.py`: regenerates all `lib/Website/*.h` from `html_source/` (gzip for CSS/JS)
- New `.github/workflows/release.yml`: tag-triggered CI builds OTA + Full flash bins, runs size gate (< 2 MB), attaches both as GitHub Release assets
- `build_firmware_release.sh` kept as local fallback

### Documentation
- `docs/android_app.md`: full Android app API guide (provisioning, NSD discovery, REST reference, architecture proposal with Kotlin/Compose/Retrofit)
- `docs/openapi.yaml`: new paths `/probes/history`, `/alarm/mute`, `/info`; Grill schema extended

## 2026-04-18
- Cleaned up MQTT code
- Cleaned up network code
- Applied tweaks and fixes to wifi init in main to drastically improve connection to wifi on cold boot
- Updated mqtt connect/reconnect behavior with better handling
- Added Opengrill settings to
    - Web settings
    - Api
    - Api documentation
    - Config storage management
- Added opengrill handlers in jsonutilities
- Added opengrill task runner in main loop
- Cleaned up mqtt reconnect code in main loop
- Main loop blocks less for mqtt/opengrill receive loop
- Fixed issue with dns lookups failing
- cleaned up wifi/dns code
- added extra checks that mqtt/opengrill can only connect if wifi connected
- mqtt data is now sent properly when updating probes
- opengrill update is sent when changing the grill name
- opengrill update is sent when probes are updates
- for opengrill we correctly send nullptr for probes that have no minimum temperature

## 2026-03-30
- Fix settings not being able to be updated - #25
- Added easter egg in settings

## 2026-03-22
- Fix display update lock on screen timeout - PR #24 - Thanks @ctrochalakis!
- Fix typo in probe 8 calibration settings loading - PR #23 - Thanks @ctrochalakis!

## 2025-12-27
- Mqtt settings now get applied when changed via api/mqtt/web interface without needing a reload (Enhancement for issue #22)

## 2025-12-22
- Added the option to set the brightness for the backlight of the screen (Enhancement for issue #10)
- The mqtt code now first checks for retained messages from external systems in case of disconnects
- Fixed openapi spec to correctly show probes under the grill endpoint
- Merged Display beta branch (full release notes available under releases)

## 2025-08-26
- Updated the web interface to show the mqtt topics when setting up mqtt
- Updated the documentation with a guide on how to use mqtt

## 2025-08-05
- bugfix for issue #16 due to incorrect buffer sizes

## 2025-08-04 - Deprecated due to bug
- Enhancement for issue #14 to be able to set names for probes
- Fixed but with labels not showing correctly on the main page

## 2025-07-27
- Fix for issue #12 where you could only set custom values for probe 1
- Added support for i-grill probes (Thanks @Robbie1983)

## 2025-07-17
- Fix for issue #7 where Fahrenheit was not displayed on the lcd

## 2025-06-14
- Added the option to set the timeout for the backlight and/or the screen (Enhancement for issue #2)
- Web interface
    - You can now toggle to only view connected probes in the webinterface - Thanks @Bardesss!
    - Added inputs for mqtt and backlight enhancements
- MQTT
    - Fixed bug with string literal used in the wrong location
    - Fixed bug where we forgot to cast to the correct data type
    - Added the option to set a mqtt username and password (Enhancement for issue #4)
- API
    - Added documentation and fields for mqtt user/pass and screen/backlight enhancements

## 2025-06-01
- Fixed bug where stacksize was not big enough to run the factory reset
- Added a config flag that checks if Free-Grilly has internet access

## 2025-05-30

- Internal code refactoring
    - Json data is now in a seperate class for re-use
    - Tweaked heap/stack usage for a more responsive experience
    - And many more
- Added mqtt support
    - free grilly can be configured to use a mqtt broker
- Sending mqtt data
    - send the grill status (wifi/temps) every second `<prefix>/<uuid>/grill`
    - sends a message when probes have been changed `<prefix>/<uuid>/probes`
    - sends a message when settings have been changed `<prefix>/<uuid>/settings`
    - On bootup a message with the retain flag will be published to `probes` and `settings`
- Configurable via mqtt
    - probes configuration can be changed via `<prefix>/<uuid>/config/probes`
    - settings configuration can be changed via `<prefix>/<uuid>/config/settings`
    - Probes and settings will check for a retained mqtt message and apply this if found. Afterwards the retained message will be cleared.
- Mqtt data is sent in json and follows the `openapi` spec
- Web ui has been updated to allow configuring the mqtt broker via the web ui
- API has been updated to allow configuring the mqtt broker via api
- Updated the openapi spec to include the new mqtt data (mqtt topic)

## 2025-05-18

- added support for Maverick ET733 probes (web/backend/api)
- added support for setting a port for the mqtt broker
- Added the mqtt broker url and port to the api + api spec

## 2025-05-16

- firmware version bump to 2025-05-16
- current info screen (wifi/version/..) is now shown after pressing and holding the button for 1 second
- fixed the bug where an alarm sounds when booting or connecting a new probe for probes without a target temperature
