# Bluetooth - Optimized Energy Consuming Sensor #

![Type badge](https://img.shields.io/badge/Type-Virtual%20Application-green)
![Technology badge](https://img.shields.io/badge/Technology-Bluetooth-green)
![License badge](https://img.shields.io/badge/License-Zlib-green)
![SDK badge](https://img.shields.io/badge/SDK-v2024.12.2-green)
[![Required board](https://img.shields.io/badge/Sparkfun-Micro%20OLED%20Breakout%20(Qwiic)-green)](https://www.sparkfun.com/products/14532)
![Build badge](https://img.shields.io/badge/Build-passing-green)
![Flash badge](https://img.shields.io/badge/Flash-188.91%20KB-blue)
![RAM badge](https://img.shields.io/badge/RAM-10.54%20KB-blue)

## Overview ##

> **! THIS IS NOT A REFERENCE DESIGN !**

This example project is designed to be used within an energy-harvesting project powered by a harvester and taking sensor readings – This example code is destined to be used with various types of energy-harvest inputs, DC-DC circuity, PMICs, batteries, recharging circuits, and super-capacitors, etc. These hardware elements are not considered in this design.
This example is purely demonstrative of the lowest-power software configuration and provides valuable timing and energy measurements across different radio and MCU tasks.
The correct way to use this example is to have a known power budget in mind – this example will walk you through the various parameter settings and their tradeoffs to determine the most optimal application for the known energy budget.

For a reference design example visit here

- [https://www.silabs.com/support/training/optimize-iiot-with-wireless-asset-monitoring-and-energy-harvesting](https://www.silabs.com/support/training/optimize-iiot-with-wireless-asset-monitoring-and-energy-harvesting)

- [https://www.silabs.com/support/training/app-104-harvesting-thermal-energy-to-power-asset-monitors-in-a-factory](https://www.silabs.com/support/training/app-104-harvesting-thermal-energy-to-power-asset-monitors-in-a-factory)

**Compatible energy harvest devices:**

![pv](image/pv.png)

This application is meant to be powered by PV solar, vibration or thermal power sources.
These designs will require super-capacitor storage and power management circuits.

This project demonstrates the most energy-efficient way to take a sensor reading and transmit it.
This is achieved by putting the sensor and radio device to frequent sleep and making energy-based decisions before transmitting.

![overview](image/overview.png)

The system is composed of a sensor and a (at least one) client device. The sensor device will measure the internal temperature and then broadcast it within 1 second and then go to sleep mode. The client device will scan for the sensor device and get the temperature value from the sensor device's advertisement data then display it on the OLED screen or Simplicity Connect Application.

**Sensor:**

This device will broadcast the internal temperature in the advertisement package in 1 second and then it will go into EM2 mode in 1 second then wake up to broadcast again. The sensor device will repeat that process 2 times before going into EM4 mode for 5 seconds.

**Sensor Status Display:**

The client device will scan periodically the BLE network. Once it found the sensor device, it tries to get internal temperature value from the advertisement package to update the OLED display.

---

## Table Of Contents ##

- [SDK version](#sdk-version)
- [Software Required](#software-required)
- [Hardware Required](#hardware-required)
- [Connections Required](#connections-required)
- [Setup](#setup)
  - [Create a project based on an example project](#create-a-project-based-on-an-example-project)
  - [Start with a "Bluetooth - SoC Empty" project](#start-with-a-bluetooth---soc-empty-project)
- [How It Works](#how-it-works)
  - [Sensor](#sensor)
  - [Sensor Status Display](#sensor-status-display)
  - [Testing](#testing)
- [Report Bugs & Get Support](#report-bugs--get-support)

---

## SDK version ##

- [Simplicity SDK v2024.12.2](https://github.com/SiliconLabs/simplicity_sdk)
- [Third Party Hardware Drivers v4.3.0](https://github.com/SiliconLabs/third_party_hw_drivers_extension)

---

## Software Required ##

- [Simplicity Studio v5 IDE](https://www.silabs.com/developers/simplicity-studio)
- [Simplicity Connect Mobile App](https://www.silabs.com/developer-tools/simplicity-connect-mobile-app)

---

## Hardware Required ##

- **Sensor:**
  - 1x [SLWSTK6021A](https://www.silabs.com/development-tools/wireless/efr32xg22-wireless-starter-kit?tab=overview) EFR32xG22 Wireless Gecko Starter Kit
- 1x smartphone running the 'Simplicity Connect' mobile app

- **Sensor Status Display:**
  - 1x [Bluetooth Low Energy Development Kit](https://www.silabs.com/development-tools/wireless/bluetooth). For simplicity, Silicon Labs recommends the [BGM220-EK4314A](https://www.silabs.com/development-tools/wireless/bluetooth/bgm220-explorer-kit)
  - 1x [SparkFun Micro OLED Breakout (Qwiic)](https://www.sparkfun.com/products/14532)

---

## Connections Required ##

The hardware connection is shown in the image below:

|**Sensor** | **Sensor Status Display**|
|:-------------------------:|:-------------------------:|
|![sensor](image/sensor.png) | ![client](image/client.png)|

---

## Setup ##

To test this application, you can either create a project based on an example project or start with a "Bluetooth - SoC Empty" project based on your hardware.

> [!NOTE]
>
> - Make sure that the [Third Party Hardware Drivers extension](https://github.com/SiliconLabs/third_party_hw_drivers_extension) is installed as part of the SiSDK and the [bluetooth_applications](https://github.com/SiliconLabs/bluetooth_applications) repository is added to [Preferences > Simplicity Studio > External Repos](https://docs.silabs.com/simplicity-studio-5-users-guide/latest/ss-5-users-guide-about-the-launcher/welcome-and-device-tabs).
>
> - SDK Extension must be enabled for the project to install the required components.
nabled for the project to install the required components.

### Create a project based on an example project ###

1. From the Launcher Home, add your hardware to My Products, click on it, and click on the EXAMPLE PROJECTS & DEMOS tab. Find the example project filtering by "optimized energy".

2. Click **Create** button on both **Bluetooth - Optimized Energy Consuming Sensor** and **Bluetooth - Optimized Energy Consuming Sensor - Status Display** examples. Example project creation dialog pops up -> click Create and Finish and the projects will be generated.

   **Sensor:**

   ![create example sensor](image/create_example_sensor.png)

   **Sensor Status Display:**

   ![create example](image/create_example.png)

3. Build and flash the examples to the board.

### Start with a "Bluetooth - SoC Empty" project ###

1. Create **Bluetooth - SoC Empty** projects for your hardware using Simplicity Studio 5 (for both sensor and client devices).

2. Copy all attached files in the *inc* and *src* folders into the project root folder (overwriting existing).

   - **Sensor:**
     - [bluetooth_optimized_energy_consuming_sensor](bluetooth_optimized_energy_consuming_sensor)
   - **Sensor Status Display:**
     - [bluetooth_optimized_energy_consuming_client](bluetooth_optimized_energy_consuming_client)

3. Open the .slcp file. Select the **SOFTWARE COMPONENTS** tab and install the software components:

   - **Sensor:**
     - [Services] → [IO Stream] → [IO Stream: USART] → default instance name: **vcom**
     - [Application] → [Utility] → [Log]

   - **Sensor Status Display:**
     - [Services] → [IO Stream] → [IO Stream: USART] → default instance name: **vcom**
     - [Application] → [Utility] → [Log]
     - [Platform] → [Driver] → [I2C] → [I2CSPM] → default instance name: **qwiic**
     - [Third Party Hardware Drivers] → [Display & LED] → [SSD1306 - Micro OLED Breakout (Sparkfun) - I2C]
     - [Third Party Hardware Drivers] → [Services] → [GLIB - OLED Graphics Library]

4. Build and flash the project to your device.

> [!NOTE]
>
> - The switch application uses the bare minimum of services to avoid wasting any energy. The following software components should be installed to make the example work.
> - A bootloader needs to be flashed to your board if the project starts from the "Bluetooth - SoC Empty" project, see [Bootloader](https://github.com/SiliconLabs/bluetooth_applications/blob/master/README.md#bootloader) for more information.

---

## How It Works ##

### Sensor ###

![Application overview](image/sensor_overview.png)

The energy-harvesting, battery-less applications have very finite amounts of energy available (~200..300uJ), therefore the application shall be optimized to transmit only the minimal required information within the shortest feasible time slot.

In these types of energy-harvesting devices, the products will spend the majority of their lifetime in deep sleep EM4.

After a power-on reset the application reads the internal temperature of the microcontroller and starts to transmit it, once the first advertisement is done the application logic sends the microcontroller into EM2 sleep mode for 1 second and repeats this transmit cycle twice.

The device goes into EM4 deep sleep mode right after transmitting the third message and stays in it until the BURTC (after 5 seconds) triggers the microcontroller to wake up.

The microcontroller wakes up and after the initialization process it begins to read the internal temperature and starts to transmit the messages in accordance with the figure above.

The payload size, TX power and the advertisement time significantly influence the required energy to transmit the switch status successfully.

> [!NOTE]
>
> The default advertisement time is 100 ms, decreasing this value can help to optimize the application to meet the available energy budget.

In your implementation, you can vary the level and duration of sleep, and modify payload size and number of transmissions based on available energy or known energy budget.

More information about Bluetooth energy optimization is available [here](https://www.silabs.com/documents/public/application-notes/an1366-bluetooth-use-case-based-low-power-optimization.pdf).

**Bluetooth Stack Initialization:**

The device is configured as:

- Non-connectable mode
- TX power is configured to 0 dB
- Advertisement duration is configured to 100 ms

A configuration switch is added to reduce the number of primary channels.

**Application runtime:**

![sensor device runtime](image/runtime.png)

**Advertisement Packet:**

The AdvData field in the advertisement packet is as table below:

| DeviceName | Internal Temperature |
|-----|-----|
| BG22_SE | 4 byte |

- Internal Temperature is in Celsius unit.
- Device is not connectable. It sends [manufacturer specific advertisement](https://github.com/SiliconLabs/bluetooth_stack_features/tree/master/advertising/advertising_manufacturer_specific_data) packets.

**Performance Measurements:**

> **The results of the performance measurements below are not officially specified values!**

![performance measurement](image/energy_harvesting_app_performance.png)

<table>
    <thead>
        <tr>
            <th>Device</th>
            <th>Action</th>
            <th>Measured</th>
            <th>Unit</th>
        </tr>
    </thead>
    <tbody>
        <tr>
            <td rowspan=13>xG22</td>
            <td>LFXO Power On -> TX -> EM2</td>
            <td>248</td>
            <td>ms</td>
        </tr>
        <tr>
            <td>LFRCO Power On -> TX -> EM2</td>
            <td>124</td>
            <td>ms</td>
        </tr>
        <tr>
            <td>LFXO EM4 wakeup Power On -> TX -> EM2</td>
            <td>238</td>
            <td>ms</td>
        </tr>
        <tr>
            <td>LFRCO EM4 wakeup Power On -> TX -> EM2</td>
            <td>100</td>
            <td>ms</td>
        </tr>
        <tr>
            <td>EM2 wakeup -> TX -> EM2 sleep</td>
            <td>2.064</td>
            <td>ms</td>
        </tr>
        <tr>
            <td>EM2 wakeup -> TX -> EM4 sleep</td>
            <td>4.17</td>
            <td>ms</td>
        </tr>
        <tr>
            <td>Power On -> Secure boot done</td>
            <td>19.8</td>
            <td>ms</td>
        </tr>
        <tr>
            <td>EM4 wakeup -> Secure boot done</td>
            <td>11.8</td>
            <td>ms</td>
        </tr>
        <tr>
            <td>LFXO EM2 current</td>
            <td>1.49</td>
            <td>uA</td>
        </tr>
        <tr>
            <td>LFRCO EM2 current</td>
            <td>1.79</td>
            <td>uA</td>
        </tr>
        <tr>
            <td>EM4 current</td>
            <td>0.24</td>
            <td>uA</td>
        </tr>
        <tr>
            <td>Secure boot average current</td>
            <td>2.799</td>
            <td>uA</td>
        </tr>
        <tr>
            <td>EM4 secure boot average current</td>
            <td>2.962</td>
            <td>uA</td>
        </tr>
    </tbody>
</table>

### Sensor status display ###

![Application overview](image/client_overview.png)

**Display:**

The client device will display no data if there is no result after scanning for a period of time. Otherwise, it will display the internal temperature that is broadcasted by the sensor device.

![Display](image/display.png)

### Testing ###

**Sensor:**

Follow the below steps to test the Sensor with the Simplicity Connect application:

- Open the Simplicity Connect app on your smartphone and allow the permission requested the first time it is opened.

- Find your device in the Bluetooth Browser, advertising as *BG22_SE*.

   ![sensor Simplicity connect](image/efr_connect.png)

- To see how the sensor device optimizes energy consumption, you can open the Energy Profier tool on SimplictyStudio to monitor the consumed current and power. You can easily see that the device consumes less current when it is in EM2 and EM4 mode.

   ![energy monitor](image/energy_monitor.png)

**Sensor Status Display:**

The client device will display the temperature value that is broadcasted by the sensor device. If there is no device named **BG22_SE** found within 5 seconds client device will display "no data".

![client configuration efr](image/client_result.GIF)

---

## Report Bugs & Get Support ##

To report bugs in the Application Examples projects, please create a new "Issue" in the "Issues" section of [bluetooth_applications](https://github.com/SiliconLabs/bluetooth_applications) repo. Please reference the board, project, and source files associated with the bug, and reference line numbers. If you are proposing a fix, also include information on the proposed fix. Since these examples are provided as-is, there is no guarantee that these examples will be updated to fix these issues.

Questions and comments related to these examples should be made by creating a new "Issue" in the "Issues" section of [bluetooth_applications](https://github.com/SiliconLabs/bluetooth_applications) repo.

---
