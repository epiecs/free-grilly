# Changelog (firmware only)
## TBD
- Added the option to set the brightness for the backlight of the screen (Enhancement for issue #10)

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