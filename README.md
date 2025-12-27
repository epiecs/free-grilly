# Free-Grilly: Community Firmware for Grilleye Max

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT) 

**Reviving your Grilleye Max thermometer with this community-driven firmware!**

This project provides alternative firmware for the Grilleye Max thermometer. After the original manufacturer discontinued software support, this firmware was developed through reverse engineering to keep these capable devices functional.

---

<img width="823" height="564" alt="image" src="https://github.com/user-attachments/assets/9a72935c-6b91-450a-b318-8b493ba91b6c" />

---

**⚠️ IMPORTANT DISCLAIMER ⚠️**

* This firmware is unofficial and NOT affiliated with or endorsed by the original manufacturer (GrillEye® by G&C).
* Flashing custom firmware onto your device carries risks. You could potentially damage or "brick" your Grilleye Max.
* You proceed entirely **at your own risk**. The authors and contributors are not responsible for any damage to your device.
* Flashing custom firmware will void any remaining manufacturer warranty.

---

- [Free-Grilly: Community Firmware for Grilleye Max](#free-grilly-community-firmware-for-grilleye-max)
  - [Features](#features)
  - [Apps and integrations](#apps-and-integrations)
  - [API documentation](#api-documentation)
  - [Todo](#todo)
  - [Installation](#installation)
  - [First Use \& WiFi Setup](#first-use--wifi-setup)
  - [Usage](#usage)
    - [Button](#button)
  - [Updating Firmware (OTA)](#updating-firmware-ota)
  - [Supported probes](#supported-probes)
  - [Home assistant support](#home-assistant-support)
  - [Contributing](#contributing)

## Features

* **On-Device Temperature Display:** Shows current probe temperatures directly on the Grilleye Max screen.
* **Audible Alerts:** The device beeps to notify you when temperatures go outside a set range or when food is nearing its target temperature.
* **Web Interface Control:** All device management and configuration is handled through an easy-to-use web interface.
* **Dual Web Access:** Access the web interface via:
    * A local Access Point (AP) mode (`http://192.168.200.10`) for initial setup.
    * Your home WiFi network (once configured) using the device's local IP address.
* **Probe Flexibility:**
    * **Custom Probe Configuration:** Manually configure support for various NTC thermistor probes by entering their specific resistance (kΩ at reference temperature), reference temperature (°C), and Beta coefficient value.
    * **Pre-configured Probes:** Includes ready-to-use settings for popular probes like the Ikea Fantast.
* **Over-the-Air (OTA) Updates:** Easily update the firmware wirelessly through the web interface once the initial flashing is done.
* **Local REST API:** Provides a RESTful API endpoint on the device for integration with custom scripts, home automation systems, or other applications.
* * **MQTT support:** All data (grill status/probes/settings) are sent to a mqtt topic. You can also configure probes and settings via mqtt.
  * [Mqtt documentation](docs/mqtt.md)
* **Battery Management:** Includes functional battery monitoring and management based on the device's hardware.
* **Button Functionality:** The side button works for powering the device on/off and performing a factory reset (via long 10 seconds press).
* **Persistent Settings:** All your configuration settings are saved directly on the device's non-volatile memory.

## Apps and integrations

- An Iphone app created by @rogiernl: https://testflight.apple.com/join/wYTte6sP

## API documentation

- Api documentation is include in the [openapi.yaml file](docs/openapi.yaml)
- You can also view this [online](https://editor-next.swagger.io/?url=https://raw.githubusercontent.com/epiecs/free-grilly/refs/heads/master/docs/openapi.yaml)

## Todo

- Fully work out guide and readme
  - add documentation for the home assistant integration
- source code
  - clean up of existing code and classes

## Installation

For installation please refer to the [Flashing guide](docs/how_to_flash.md)

## First Use & WiFi Setup

1.  **Power On:** Turn on your Grilleye Max by pressing the side button. Once you hear the beeps you can release the button.
2.  **AP Mode:** The device will initially boot into Access Point (AP) mode. It will create its own WiFi network. *(FreeGrilly_xxxxxx)*
3.  If you can not see this network you can hold the side button for 2 seconds until you hear 2 beeps and then release the button. The screen will now show you the name of the wifi ap on your free-grilly
4.  **Connect to AP:** Using your phone or computer, connect to this new WiFi network.
5.  **Access Web Interface:** Open a web browser and navigate to `http://192.168.200.10`.
6.  **Configure WiFi:** Find the "Settings" or "WiFi Configuration" page in the web interface. Select your home WiFi network (SSID), enter the password, and save.
7.  **Reconnect:** The Grilleye Max should now connect to your own WiFi network. Its IP address will now be assigned by your router  (The local ap will keep on working as well).

## Usage

- **Temperatures:** Temperature readings from connected probes are displayed on the device's screen and updated in the web interface.
- **Web Interface:** Access the web interface using the IP address assigned by your router to view temperatures, change settings, etc.
- **Probe Settings:** Within the web interface settings, you can:
    - Set minimum and target temperature alerts for each probe.
    - Configure the type of probe connected (e.g., adjust settings to work with Ikea Fantast probes).

### Button

- Hold the button for 2 seconds to boot the device. You will hear 2 beeps
- If the device is booted hold the button for 2 seconds until you hear 2 beeps and then release the button to view a status screen with wifi details etc
- Tap the button to return to the temperature view
- Hold the button for 3 seconds until you hear 3 beeps to turn the device off
- Hold the button for 10 seconds until you hear 3 long beeps to factory reset

## Updating Firmware (OTA)

Once Free-Grilly is installed, you can update to newer versions wirelessly:

  1. Download the latest `free-grilly-yyyy-mm-dd.bin` from Releases.
  2. Access the web interface.
  3. Go to the 'Update' page.
  4. Upload the downloaded `.bin` file.
  5. Wait for the device to update.
  6. Once the update is done you will have to boot the device by holding the button.

## Supported probes

Free-Grilly supports more probes that the included Grilleye Iris probes. Support for the following probes is built-in:

| Probe type     | ref C | ref kOhm | ref beta | notes                                          |
|----------------|-------|----------|----------|------------------------------------------------|
| Grilleye Iris  | 25    | 100      | 4250     | The best probe you can use. Fast and accurate  |
| Ikea fantast   | 25    | 230      | 4250     | Slow. Do not use for quick measurements        |
| Maverick ET733 | 25    | 200      | 4250     |                                                |
| Weber Igrill   | 25    | 100      | 3830     |                                                |

Apart from that you can also add your own `custom` **NTC** probes if you know the 3 most important values
-   Reference temperature in Celcius
-   Reference beta value
-   Reference resistance in kOhm

If you want you can use our [handy calculator spreadsheet](docs/probe_calculator.xlsx) if you want to calculate the values for your own probes. 

## Home assistant support
You can connect your free-grilly to your current home assistant installation when using the mqtt broker. For this you will need to manually create the needed entities. @woutercoppens provided us/you with a decent starting point. This can also be seen in issue #5:

```yaml
# Probe 1 (Defines Anchors)
- name: "Free Grilly Probe 1 Temperature"
  unique_id: "free_grilly_probe_1_temp"
  <<: &probe_temp_defaults
    state_topic: "free-grilly/7107d787-30c7-4d20-9b1b-db4cfd14fca4/grill" #Adjust
    unit_of_measurement: "°C"
    device_class: temperature
    state_class: measurement
    payload_available: "online"
    payload_not_available: "offline"
    device: &device_info
      identifiers: "free_grilly_7107d787" #Adjust
      name: "Free Grilly Thermometer"
      manufacturer: "Free-Grilly"
      sw_version: "25.08.27"
  value_template: "{{ value_json.probes[0].temperature }}"
  availability_template: "{{ 'online' if value_json.probes[0].connected == true else 'offline' }}"

- name: "Free Grilly Probe 1 Target"
  unique_id: "free_grilly_probe_1_target"
  <<: &probe_target_defaults
    state_topic: "free-grilly/7107d787-30c7-4d20-9b1b-db4cfd14fca4/grill" #Adjust
    unit_of_measurement: "°C"
    device_class: temperature
    icon: mdi:target
    payload_available: "online"
    payload_not_available: "offline"
    device: *device_info
  value_template: "{{ value_json.probes[0].target_temperature }}"
  availability_template: "{{ 'online' if value_json.probes[0].connected == true else 'offline' }}"

# Probe 2
- name: "Free Grilly Probe 2 Temperature"
  unique_id: "free_grilly_probe_2_temp"
  <<: *probe_temp_defaults
  value_template: "{{ value_json.probes[1].temperature }}"
  availability_template: "{{ 'online' if value_json.probes[1].connected == true else 'offline' }}"
- name: "Free Grilly Probe 2 Target"
  unique_id: "free_grilly_probe_2_target"
  <<: *probe_target_defaults
  value_template: "{{ value_json.probes[1].target_temperature }}"
  availability_template: "{{ 'online' if value_json.probes[1].connected == true else 'offline' }}"

# Probe 3
- name: "Free Grilly Probe 3 Temperature"
  unique_id: "free_grilly_probe_3_temp"
  <<: *probe_temp_defaults
  value_template: "{{ value_json.probes[2].temperature }}"
  availability_template: "{{ 'online' if value_json.probes[2].connected == true else 'offline' }}"
- name: "Free Grilly Probe 3 Target"
  unique_id: "free_grilly_probe_3_target"
  <<: *probe_target_defaults
  value_template: "{{ value_json.probes[2].target_temperature }}"
  availability_template: "{{ 'online' if value_json.probes[2].connected == true else 'offline' }}"

# Probe 4
- name: "Free Grilly Probe 4 Temperature"
  unique_id: "free_grilly_probe_4_temp"
  <<: *probe_temp_defaults
  value_template: "{{ value_json.probes[3].temperature }}"
  availability_template: "{{ 'online' if value_json.probes[3].connected == true else 'offline' }}"
- name: "Free Grilly Probe 4 Target"
  unique_id: "free_grilly_probe_4_target"
  <<: *probe_target_defaults
  value_template: "{{ value_json.probes[3].target_temperature }}"
  availability_template: "{{ 'online' if value_json.probes[3].connected == true else 'offline' }}"

# Probe 5
- name: "Free Grilly Probe 5 Temperature"
  unique_id: "free_grilly_probe_5_temp"
  <<: *probe_temp_defaults
  value_template: "{{ value_json.probes[4].temperature }}"
  availability_template: "{{ 'online' if value_json.probes[4].connected == true else 'offline' }}"
- name: "Free Grilly Probe 5 Target"
  unique_id: "free_grilly_probe_5_target"
  <<: *probe_target_defaults
  value_template: "{{ value_json.probes[4].target_temperature }}"
  availability_template: "{{ 'online' if value_json.probes[4].connected == true else 'offline' }}"

# Probe 6
- name: "Free Grilly Probe 6 Temperature"
  unique_id: "free_grilly_probe_6_temp"
  <<: *probe_temp_defaults
  value_template: "{{ value_json.probes[5].temperature }}"
  availability_template: "{{ 'online' if value_json.probes[5].connected == true else 'offline' }}"
- name: "Free Grilly Probe 6 Target"
  unique_id: "free_grilly_probe_6_target"
  <<: *probe_target_defaults
  value_template: "{{ value_json.probes[5].target_temperature }}"
  availability_template: "{{ 'online' if value_json.probes[5].connected == true else 'offline' }}"

# Probe 7
- name: "Free Grilly Probe 7 Temperature"
  unique_id: "free_grilly_probe_7_temp"
  <<: *probe_temp_defaults
  value_template: "{{ value_json.probes[6].temperature }}"
  availability_template: "{{ 'online' if value_json.probes[6].connected == true else 'offline' }}"
- name: "Free Grilly Probe 7 Target"
  unique_id: "free_grilly_probe_7_target"
  <<: *probe_target_defaults
  value_template: "{{ value_json.probes[6].target_temperature }}"
  availability_template: "{{ 'online' if value_json.probes[6].connected == true else 'offline' }}"

# Probe 8
- name: "Free Grilly Probe 8 Temperature"
  unique_id: "free_grilly_probe_8_temp"
  <<: *probe_temp_defaults
  value_template: "{{ value_json.probes[7].temperature }}"
  availability_template: "{{ 'online' if value_json.probes[7].connected == true else 'offline' }}"
- name: "Free Grilly Probe 8 Target"
  unique_id: "free_grilly_probe_8_target"
  <<: *probe_target_defaults
  value_template: "{{ value_json.probes[7].target_temperature }}"
  availability_template: "{{ 'online' if value_json.probes[7].connected == true else 'offline' }}"

# Battery
- name: "Free Grilly Battery"
  unique_id: "free_grilly_battery"
  state_topic: "free-grilly/7107d787-30c7-4d20-9b1b-db4cfd14fca4/grill" #Adjust
  unit_of_measurement: "%"
  device_class: battery
  state_class: measurement
  payload_available: "online"
  payload_not_available: "offline"
  availability_template: "{{ 'online' if value_json.wifi_connected else 'offline' }}"
  value_template: "{{ value_json.battery_percentage }}"
  device: *device_info
```

## Contributing

We welcome contributions to help improve Free-Grilly! Whether it's fixing bugs, adding features, improving documentation, or testing, your input is valuable.

Here's how you can contribute:

* **Reporting Issues:** If you find a bug or have an idea for a new feature, please check the existing [Issues](https://github.com/epiecs/free-grilly/issues) first. If it hasn't been reported, please open a new issue, providing as much detail as possible.
  **Submitting Changes (Pull Requests):** If you'd like to contribute code or documentation changes:
    1.  Fork the repository.
    2.  Create a new branch for your changes (`git checkout -b feature/your-feature-name`).
    3.  Make your changes and commit them with clear messages.
    4.  Push your branch to your fork (`git push origin feature/your-feature-name`).
    5.  Open a [Pull Request](https://github.com/epiecs/free-grilly/pulls) against the main branch of this repository.

Thank you for considering contributing to Free-Grilly!
