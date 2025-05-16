# Free-Grilly: Community Firmware for Grilleye Max

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT) 

**Reviving your Grilleye Max thermometer with this community-driven firmware!**

This project provides alternative firmware for the Grilleye Max thermometer. After the original manufacturer discontinued software support, this firmware was developed through reverse engineering to keep these capable devices functional.

---

**⚠️ IMPORTANT DISCLAIMER ⚠️**

* This firmware is unofficial and NOT affiliated with or endorsed by the original manufacturer (GrillEye® by G&C).
* Flashing custom firmware onto your device carries risks. You could potentially damage or "brick" your Grilleye Max.
* You proceed entirely **at your own risk**. The authors and contributors are not responsible for any damage to your device.
* Flashing custom firmware will void any remaining manufacturer warranty.

---

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
* **Battery Management:** Includes functional battery monitoring and management based on the device's hardware.
* **Button Functionality:** The side button works for powering the device on/off and performing a factory reset (via long 10 seconds press).
* **Persistent Settings:** All your configuration settings are saved directly on the device's non-volatile memory.

## Prerequisites

Before you begin, ensure you have the following:

* **Hardware:**
    * A Grilleye Max device.
    * A USB cable to connect the Grilleye Max to your computer.
* **Software:**
    * A computer running Windows, macOS, or Linux.
    * [Python](https://www.python.org/downloads/) installed.
    * [esptool.py](https://github.com/espressif/esptool): Install it via pip: `pip install esptool`
        * You can also download premade binaries for Windows/Linux/Mac: https://github.com/espressif/esptool/releases (for intel macs use https://github.com/espressif/esptool/releases/tag/v4.5.1)
    * [CP210x USB to UART Bridge Drivers](https://www.silabs.com/developers/usb-to-uart-bridge-vcp-drivers) installed. Download and install the drivers appropriate for your operating system.
* **Firmware File:**
    * Download the latest `free-grilly.bin` file from the [Releases page](https://github.com/epiecs/free-grilly/releases) of this repository.

## Installation (Initial Flashing via USB)

This process will replace the original firmware on your Grilleye Max.

> **Watch out!:** For the first flash you will need to use the `merged` variant of the firmware. This contains extra data which is not included in the normal updates (bootloader, partitions,...).

1.  **Connect the Device:** Connect your Grilleye Max to your computer using the USB cable.
2.  **Find Your Serial Port:**
    * **Windows:** Open Device Manager (search `devmgmt.msc`). Look under "Ports (COM & LPT)". You should see an entry like "Silicon Labs CP210x USB to UART Bridge (COMx)". Note the `COMx` number (e.g., `COM3`).
    * **macOS:** Open Terminal and run `ls /dev/cu.*`. Look for something like `/dev/cu.usbserial-XXXX` or `/dev/cu.SLAB_USBtoUART`.
    * **Linux:** Open Terminal and run `ls /dev/ttyUSB*`. Look for something like `/dev/ttyUSB0`.
3.  **Flash the Firmware:**
    * Open your Terminal or Command Prompt.
    * Navigate to the directory where you downloaded the `free-grilly.bin` file.
    * Run the `esptool.py` command, replacing `<YOUR_SERIAL_PORT>` with the port name you found in step 2:

        ```bash
        esptool.py --port <YOUR_SERIAL_PORT> write_flash 0x0 free-grilly.bin
        ```
        *Example for Linux:* `esptool.py --port /dev/ttyUSB0 write_flash 0x0 free-grilly-merged-yyyy-mm-dd.bin`
        *Example for Windows:* `esptool.py --port COM3 write_flash 0x0 free-grilly-merged-yyyy-mm-dd.bin`
        *Example for macOS:* `esptool.py --port /dev/cu.SLAB_USBtoUART write_flash 0x0 free-grilly-merged-yyyy-mm-dd.bin`

5.  **Wait:** The flashing process will take a minute or two. Do not disconnect the device. `esptool.py` will indicate when it's complete.
6.  **Reboot:** Once flashing is successful, disconnect and reconnect the USB cable, or power cycle the device. To start the device hold the power button until the Grilleye beeps.

## First Use & WiFi Setup

1.  **Power On:** Turn on your Grilleye Max by pressing the side button.
2.  **AP Mode:** The device will initially boot into Access Point (AP) mode. It will create its own WiFi network. *(FreeGrilly_xxxxxx)*
3.  **Connect to AP:** Using your phone or computer, connect to this new WiFi network.
4.  **Access Web Interface:** Open a web browser and navigate to `http://192.168.200.10`.
5.  **Configure WiFi:** Find the "Settings" or "WiFi Configuration" page in the web interface. Select your home WiFi network (SSID), enter the password, and save.
6.  **Reconnect:** The Grilleye Max should now connect to your own WiFi network. Its IP address will now be assigned by your router  (The local ap will keep on working as well).

## Usage

* **Temperatures:** Temperature readings from connected probes are displayed on the device's screen and updated in the web interface.
* **Web Interface:** Access the web interface using the IP address assigned by your router to view temperatures, change settings, etc.
* **Probe Settings:** Within the web interface settings, you can:
    * Set minimum and target temperature alerts for each probe.
    * Configure the type of probe connected (e.g., adjust settings to work with Ikea Fantast probes).

## Updating Firmware (OTA)

Once Free-Grilly is installed, you can update to newer versions wirelessly:

  1. Download the latest `free-grilly-yyyy-mm-dd.bin` from Releases.
  2. Access the web interface.
  3. Go to the 'Update' page.
  4. Upload the downloaded `.bin` file.
  5. Wait for the device to update and reboot.

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
