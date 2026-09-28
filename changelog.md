# Changelog (firmware only)

## 2026-09-28
Tested on a real Grilleye Max. Install it from Settings > Firmware updates with the `-ota.bin` file.

### New
- Reach the grill by name: `http://grilly-plus-xxxxxxxx.local` (mDNS), on your home WiFi and on its own hotspot. The name is on the device's Info screen and the About page, and your router shows it too
- Mute a sounding alarm from the web app (a bar on every page) or over MQTT (`<prefix>/<uuid>/config/mute`)
- The card of the probe that triggered an alarm is highlighted, and `/api/grill` reports `alarm_sounding` and a per-probe `alarm`
- Calibration offset per probe (-10 to +10 °C) in the probe editor, for probes that read a little high or low
- Diagnostics on the About page: why the grill last restarted or switched off, and the battery voltage (`last_reset_reason`, `last_off_reason`, `battery_millivolts` in `/api/grill`)
- Show/hide button on every password field
- The About page shows the hotspot name and address as well as the WiFi address
- Favicon and iPhone home screen icon
- Probe cards are shown two per row on tablets and desktops, so up to 8 probes always fill whole rows

### Fixes
- A failed battery reading is detected and ignored instead of showing 65535 %, so the low battery warning works again
- The early warning beep ("degrees before") no longer pauses probe readings for 2.4 seconds or cuts off other beeps
- The power button is debounced, so electrical noise can't cut a long press short or fake a press
- Data shared between the firmware's tasks (settings, probe names, IP address) is locked while it's changed, preventing rare crashes
- More stack for the probes, alarm and battery tasks
- After a wrong current admin password, correcting it retries the change without typing the new password again

### Other
- Every pull request is built and tested automatically (GitHub Actions)

## 2026-09-27
First Grilly+ release, a fork of Free-Grilly 2026-04-18. Settings are kept when upgrading from Free-Grilly (same settings storage), and the first update can be installed through Free-Grilly's existing /update page.

### New web app
- New web app, built for phones and desktops alike, replacing the Bootstrap pages
    - One dashboard with a card per connected probe: temperature, target or range, progress, "to go" / ready / in range / too low / too high, and how long the probe has been connected
    - Tap a probe card to edit its name, alarm (off, target or range) and probe type, with large steppers; reference values are only shown for custom probes
    - Settings save themselves; network changes (WiFi and hotspot) are collected and applied together with one button after a confirmation
    - WiFi scan in the Network card, sorted by signal
    - Battery (with charging) and WiFi signal indicators with icons, and a banner when the grill can't be reached
    - Dark and light theme following your device, floating navigation bar on phones
    - About view with firmware and connection details and credits to Free-Grilly
    - One compressed page of about 19 KB instead of about 311 KB, cached by the browser until the next firmware update; frees about 290 KB of flash
    - Old page urls (/probes, /settings, /about, /update) redirect into the app
- Save errors are shown instead of "saved successfully", and the entered values are kept

### Firmware updates and security
- Firmware updates are installed from the Settings view (Firmware updates card) with upload progress; the grill restarts by itself and the page reloads. ElegantOTA is removed
- Optional admin password protects firmware updates; changing or removing it needs the current one, and it can't be changed over MQTT
- Only real OTA firmware files are accepted, a -full.bin is refused
- New and factory-reset devices get a generated hotspot password, shown on the device's Info screen; upgraded devices keep their current hotspot password
- Passwords are never returned by the api or published over MQTT
- Web pages on other sites can no longer change settings or install firmware (json-only posts, no CORS for writes, required update header)

### Fixes
- The grill boots straight back up after an update, a factory reset or a crash instead of shutting down
- The backlight switches off at shutdown and the power rails stay off while asleep
- Fixed a crash when a probe is unplugged while its details page is shown on the device
- Empty probe sockets no longer flicker between connected and disconnected or show "nan"
- Alarms no longer replay when a probe is saved (for example after renaming it or an MQTT reconnect)
- "Beep when ready" and "Beep outside target" are respected
- Range alarms of 4 degrees or less re-arm again; a minimum of 0 or below means target mode
- The overview for 5 to 8 probes on the device shows the configured unit instead of always Celsius
- Disabling beeps no longer wipes the saved volume
- The Cucaracha alarm setting is saved
- The settings page no longer saves the DHCP address as a static IP; an incomplete static IP falls back to DHCP
- Switching from a static IP back to DHCP works without a reboot
- The gateway is used as DNS fallback for a static IP
- Restart the display timeouts when settings are saved, so a new brightness stays visible
- Missing keys in api and MQTT updates keep their current value instead of being stored as "null" or 0; values are validated before anything is saved
- The api, MQTT and Opengrill tasks no longer share one json document, which could garble messages or crash the device
- WiFi scan lists at most 20 networks so the response always fits

### MQTT and Opengrill
- Changing the MQTT broker, port, topic or credentials takes effect right away and subscribes to the new config topics
- Reconnects back off from 5 seconds to a minute and stop when MQTT or Opengrill is disabled
- Rejected MQTT config messages are reported on `<prefix>/<uuid>/error`
- Saving probes publishes the probes topic
- Opengrill connects on the Opengrill port instead of the MQTT port

### Api
- `/api/grill` returns `connected_seconds` per probe
- `/api/settings` returns `<name>_password_set` instead of passwords
- New `POST /api/update` for firmware updates

### Other
- Renamed to Grilly+: new defaults for fresh installs (name `Grilly+`, mqtt prefix `grilly-plus`, AP name `GrillyPlus_xxxxxx`), MQTT and Opengrill client ids start with `grilly-plus-`
- Firmware release files are named `grilly-plus-yyyy-mm-dd-*.bin`
- The build platform version is pinned so builds are reproducible
- The release script works on Linux, macOS and Windows and stops on errors
- Development tools: a dev server with mock data, and unit tests for the web app and the build script

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
