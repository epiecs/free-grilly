# Changelog (firmware only)

## 2025-05-14

- firmware version bump to 2025-05-16
- current info screen (wifi/version/..) is now shown after pressing and holding the button for 1 second
- fixed the bug where an alarm sounds when booting or connecting a new probe for probes without a target temperature

## 2025-05-18

- added support for Maverick ET733 probes (web/backend/api)
- added support for setting a port for the mqtt broker
- Added the mqtt broker url and port to the api + api spec

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
