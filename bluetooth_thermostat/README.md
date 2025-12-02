# Bluetooth - Thermostat (SHTC3)

![Technology badge](https://img.shields.io/badge/Technology-Bluetooth-green)
![License badge](https://img.shields.io/badge/License-Zlib-green)
![SDK badge](https://img.shields.io/badge/SDK-v2025.6.2-green)
[![Required board](https://img.shields.io/badge/Sparkfun-OLED%20Display-green)](https://www.sparkfun.com/products/14532)
[![Required board](https://img.shields.io/badge/Mikroe-Buzzer%202%20Click%20Board-green)](https://www.mikroe.com/buzz-2-click)
[![Required board](https://img.shields.io/badge/Sparkfun-Humidity%20Sensor-green)](https://www.sparkfun.com/products/16467)
![Build badge](https://img.shields.io/badge/Build-passing-green)
![Flash badge](https://img.shields.io/badge/Flash-223.09%20KB-blue)
![RAM badge](https://img.shields.io/badge/RAM-11.64%20KB-blue)

[![Type badge](https://img.shields.io/badge/Factory%20Automation-salmon)](https://siliconlabs-massmarket.github.io/repository-catalog/#applications-list?filter=Factory%20Automation)
[![Type badge](https://img.shields.io/badge/Process%20Automation-salmon)](https://siliconlabs-massmarket.github.io/repository-catalog/#applications-list?filter=Process%20Automation)
[![Type badge](https://img.shields.io/badge/Smart%20Agriculture-salmon)](https://siliconlabs-massmarket.github.io/repository-catalog/#applications-list?filter=Smart%20Agriculture)
[![Type badge](https://img.shields.io/badge/Smart%20Buildings-salmon)](https://siliconlabs-massmarket.github.io/repository-catalog/#applications-list?filter=Smart%20Buildings)
[![Type badge](https://img.shields.io/badge/Smart%20Hospitals-salmon)](https://siliconlabs-massmarket.github.io/repository-catalog/#applications-list?filter=Smart%20Hospitals)
[![Type badge](https://img.shields.io/badge/Smart%20HVAC-salmon)](https://siliconlabs-massmarket.github.io/repository-catalog/#applications-list?filter=Smart%20HVAC)
[![Type badge](https://img.shields.io/badge/Smart%20Metering-salmon)](https://siliconlabs-massmarket.github.io/repository-catalog/#applications-list?filter=Smart%20Metering)

## Summary

This project aims to implement a thermostat system using Silicon Labs development kits and external sensors integrated with the BLE wireless stack.  
The block diagram of this application is shown in the image below:

![overview](image/overview.png)

More detailed information can be found in the section [How it works](#how-it-works).

This code example referred to the following code examples. More detailed information can be found here:

- [OLED SSD1306 driver](https://github.com/SiliconLabs/third_party_hw_drivers_extension/tree/master/app/documentation/example/sparkfun_micro_oled_ssd1306)
- [Bluetooth security feature](https://github.com/SiliconLabs/bluetooth_stack_features/tree/master/security)
- [SHTC3 Humidity Sensor driver](https://github.com/SiliconLabs/third_party_hw_drivers_extension/tree/master/app/documentation/example/mikroe_temphum9_shtc3)
- [Buzzer driver](https://github.com/SiliconLabs/third_party_hw_drivers_extension/tree/master/app/documentation/example/mikroe_buzz2_cmt_8540s_smt)

---

## Table Of Contents

- [SDK version](#sdk-version)
- [Software Required](#software-required)
- [Hardware Required](#hardware-required)
- [Connections Required](#connections-required)
- [Setup](#setup)
  - [Create a project based on an example project](#create-a-project-based-on-an-example-project)
  - [Start with a "Bluetooth - SoC Empty" project](#start-with-a-bluetooth---soc-empty-project)
- [How It Works](#how-it-works)
  - [Application Overview](#application-overview)
  - [GATT Configurator](#gatt-configurator)
  - [Thermostat Implementation](#thermostat-implementation)
    - [Application initialization](#application-initialization)
    - [Application Workflows](#application-workflows)
    - [Algorithm workflows](#algorithm-workflows)
  - [OLED Display](#oled-display)
  - [Button](#button)
  - [Use Simplicity Connect Mobile Application](#use-simplicity-connect-mobile-application)
    - [Connect to the device](#connect-to-the-device)
    - [Read/Write characteristics](#readwrite-characteristics)
- [Report Bugs & Get Support](#report-bugs--get-support)

---

## SDK version

- [Simplicity SDK v2025.6.2](https://github.com/SiliconLabs/simplicity_sdk/releases/tag/v2025.6.2)
- [Third Party Hardware Drivers v4.4.1](https://github.com/SiliconLabs/third_party_hw_drivers_extension)

---

## Software Required

- [Simplicity Studio v5 IDE](https://www.silabs.com/developers/simplicity-studio)
- [Simplicity Connect Mobile App](https://www.silabs.com/developer-tools/simplicity-connect-mobile-app)

---

## Hardware Required

- 1x [Bluetooth Low Energy Explorer Kit](https://www.silabs.com/development-tools/wireless/bluetooth). For simplicity, Silicon Labs recommends the [BGM220-EK4314A](https://www.silabs.com/development-tools/wireless/bluetooth/bgm220-explorer-kit)
- 1x [Humidity Sensor - SHTC3 Board](https://www.sparkfun.com/products/16467)
- 1x [Buzzer 2 Click Board](https://www.mikroe.com/buzz-2-click)
- 1x [OLED Display - SSD1306](https://www.sparkfun.com/products/14532)
- 1x smartphone running the 'Simplicity Connect' mobile app

---

## Connections Required

The hardware connection is shown in the image below:

![hardware connection](image/hardware_connection.png)

The I2C connection is made from the BGM220 Bluetooth Module Explorer Kit to the Humidity Sensor board and the Micro OLED Breakout by using the Qwiic cable.

---

## Setup

To test this application, you can either create a project based on an example project or start with a "Bluetooth - SoC Empty" project based on your hardware.

> [!NOTE]
>
> - Make sure that the [Third Party Hardware Drivers extension](https://github.com/SiliconLabs/third_party_hw_drivers_extension) is installed as part of the SiSDK and the [bluetooth_applications](https://github.com/SiliconLabs/bluetooth_applications) repository is added to [Preferences > Simplicity Studio > External Repos](https://docs.silabs.com/simplicity-studio-5-users-guide/latest/ss-5-users-guide-about-the-launcher/welcome-and-device-tabs).
>
> - SDK Extension must be enabled for the project to install the required components.

### Create a project based on an example project

1. From the Launcher Home, add your product name to My Products, click on it, and click on the **EXAMPLE PROJECTS & DEMOS** tab. Find the example project filtering by "thermostat".

2. Click **Create** button on **Bluetooth - Thermostat** project. Example project creation dialog pops up -> click Create and Finish and source code should be generated.

   ![create example project](image/create_example.png)

3. Build and flash this example to your board.

### Start with a "Bluetooth - SoC Empty" project

1. Create a **Bluetooth - SoC Empty** project for your hardware using Simplicity Studio 5.

2. Copy all attached files in `inc` and `src` folders into the project root folder (overwriting the existing files).

3. Import the GATT configuration:

   - Open the .slcp file in the project.

   - Select the **CONFIGURATION TOOLS** tab and open the **Bluetooth GATT Configurator**.

   - Find the Import button and import the attached `config/btconf/gatt_configuration.btconf` file.

   - Save the GATT configuration (ctrl-s).

4. Open the .slcp file. Select the **SOFTWARE COMPONENTS** tab and install the software components:

   - [Services] → [IO Stream] → [IO Stream: USART] → default instance name: "vcom"
   - [Application] → [Utility] → [Log]
   - [Application] → [Utility] → [Assert]
   - [Application] → [Utility] → [Timer]
   - [Third Party] → [Tiny printf]
   - [Platform] → [Driver] → [Button] → [Simple Button] → default instance name: "btn0"
   - [Platform] → [Driver] → [LED] → [Simple LED] → default instance name: "led0"
   - [Platform] → [Driver] → [PWM] → [PWM] → default instance name: "mikroe"
   - [Platform] → [Driver] → [I2C] → [I2CSPM] → default instance name: "qwiic"
   - [Third Party Hardware Drivers] → [Display & LED] → [SSD1306 - Micro OLED Breakout (Sparkfun) - I2C]
   - [Third Party Hardware Drivers] → [Audio & Voice] → [CMT_8540S_SMT - Buzz 2 Click (Mikroe)]
   - [Third Party Hardware Drivers] → [Sensors] → [SHTC3 - Temp&Hum 9 Click (Mikroe)]
   - [Third Party Hardware Drivers] → [Service] → [GLIB - OLED Graphics Library]

5. Build and flash the project to your device.

> [!NOTE]
>
> A bootloader needs to be flashed to your board if the project starts from the "Bluetooth - SoC Empty" project, see [Bootloader](https://github.com/SiliconLabs/bluetooth_applications/blob/master/README.md#bootloader) for more information.

---

## How it Works

### Application Overview

![Application overview](image/application_overview.png)

### GATT Configurator

The application is based on the Bluetooth - SoC Empty example. Since the example already has the Bluetooth GATT server, advertising, and connection mechanisms, only minor changes are required.

- BLE advertiser name: **Thermostat**

- [**Service**] **Thermostat** - UUID: `9d164130-ebf2-4039-b84d-5d6bae5e5a19`

- [**Char**] **Mode** - UUID: `1dd848fb-f503-4620-b57b-b14d8ae2368f`
  - [**Read**] - Get mode value
  - [**Write**] - Set mode value (0 - heat, 1 - cool)

  - [**Char**] **Buzzer Volume** - UUID: `3af4ef21-00d9-4ead-b184-abec5949d69b`
    - [**Read**] - Get configured buzzer volume
    - [**Write**] - Set buzzer volume (0 - 10)

- [**Char**] **Setpoint (SV)** - UUID: `e0c54049-479c-4a11-aa8d-1838f88fcd98`
  - [**Read**] - Get setpoint value
  - [**Write**] - Set setpoint value (e.g.: 250 ↔ 25.0 °C; limits: -35 °C → + 120 °C ↔ -350 → 1200)

- [**Char**] **Hysteresis (HYS)** - UUID: `a6b2bc71-70da-426c-840c-281cdd578212`
  - [**Read**] - Get hysteresis value
  - [**Write**] - Set hysteresis value - hysteresis  (e.g.: 51 ↔ 5.1 °C, limits: 0 → (Upper threshold value - Lower threshold value))

- [**Char**] **Lower Threshold** - UUID: `d21dd9c4-79eb-4b02-b549-d26b289dcc3d`
  - [**Read**] - Get lower threshold value
  - [**Write**] - Set lower threshold value (-350 <= Lower threshold value < Upper threshold value)

- [**Char**] **Upper Threshold** - UUID: `a3ff7bb1-28b2-4d3c-b74c-63bdd602b2e9`
  - [**Read**] - Get upper threshold value
  - [**Write**] - Set upper threshold value - upper_threshold (Lower threshold value < Upper threshold value <= 1200)

- [**Char**] **Alarm Enabled** - UUID: `d7761e08-fe9e-4929-9149-76c858c37fe5`
  - [**Read**] - Get alarm enabled state (0 - disabled, 1 - enabled)
  - [**Write**] - Set alarm enabled state (0 - disabled, 1 - enabled)

  - [**Char**] **Measurement Interval** - UUID: `99f98ed2-1760-4712-bb98-dc61a733cdbd`
    - [**Read**] - Get configured measurement interval in s
    - [**Write**] -  Set configured measurement interval  in s (1 - 30)

- [**Char**] **Temperature (PV)** - UUID: `4085650b-5f18-48e8-b48b-81805883ee83`
  - [**Read**] - Get current averaged temperature value (e.g.: 251 ↔ 25.1 °C)
  - [**Notify**] - Get current averaged temperature value (e.g.: 251 ↔ 25.1 °C) automatically

- [**Char**] **Humidity** - UUID: `f3bf7dad-5ef4-4e6d-b3fe-46113aec0a49`
  - [**Read**] - Get current averaged humidity value (e.g.: 251 ↔ 25.1 %)
  - [**Notify**] - Get current averaged humidity value (e.g.: 251 ↔ 25.1 %) automatically

- [**Char**] **Actuator** - UUID: `d047362d-ec99-4639-a09c-30e35e066e3a`
  - [**Read**] - Get actuator status (0 - off, 1 - on)
  - [**Notify**] - Get actuator status (0 - off, 1 - on) automatically

- [**Char**] **Alarm Status** - UUID: `5c846592-ebec-4097-a8c2-df3ea2ec8926`
  - [**Read**] - Get alarm status (0 - inactive, 1 - active)
  - [**Notify**] - Get alarm status (0 - inactive, 1 - active) automatically

### Thermostat Implementation

#### Application initialization

![Application init](image/app_init.png)  

#### Application Workflows

1. Initialize the peripherals, the Bluetooth stack

2. Initialize and load the NVM3 configurations

3. Wait for the sensor to be booted and initialize the sensor with the configurations from NVM3:

4. Initialize the OLED display.

5. Start a periodic timer with a period of 5000ms, The timer callback will fire an external event to BLE stack and the event handler will display temperature and humidity data.

6. After the *sl_bt_evt_system_boot_id* event arrives, App sets up the security manager and starts advertising.

7. Handle GATT event to help the user configure the [Use Simplicity Connect Mobile Application](#use-simplicity-connect-mobile-application) and get the result from the algorithm calculation over the *EFR32 connect* mobile app

#### Algorithm workflows

![Algorithm workflows](image/algorithm_workflows.png)

### OLED Display

The following parameters are displayed on the OLED display:

- Current temperature
- Current humidity
- Set point
- Hysteresis
  
  ![OLED display](image/oled_display.png)

### Button

- When the button is released, it checks the alarm feature status, and buzzer state in accordance with the flowchart below.

  ![Flowchart Button](image/button_flowchart.png)

### Use Simplicity Connect Mobile Application

#### Connect to the device

Follow the below steps to test the example with the Simplicity Connect application:

1. Open the Simplicity Connect app on your smartphone and allow the permission requested the first time it is opened.

2. Find your device in the Bluetooth Browser, advertising as *Thermostat*, and tap Connect.

#### Read/Write characteristics

The parameters of this example application can be easily configured via BLE characteristics. Values for the characteristics are handled by the application as HEX number. Tap on the main service to see the available characteristics. Please refer [GATT Configurator](#gatt-configurator) to choose the correct characteristic.

**Read**
Push read button to request the value of a characteristic (see HEX fields).

**Write**
For setting a parameter select a characteristic and tap on its write button. Type a new value in the HEX field and push the **Send** button.

> [!TIP]
> The application handles characteristic values as hexadecimal numbers in Little Endian format. So, to write a 2-byte attribute—for example, to configure the setpoint parameter to 300 (representing 30 °C)—the value 2C01 should be entered in the HEX field.

## Report Bugs & Get Support

To report bugs in the Application Examples projects, please create a new "Issue" in the "Issues" section of [bluetooth_applications](https://github.com/SiliconLabs/bluetooth_applications) repo. Please reference the board, project, and source files associated with the bug, and reference line numbers. If you are proposing a fix, also include information on the proposed fix. Since these examples are provided as-is, there is no guarantee that these examples will be updated to fix these issues.

Questions and comments related to these examples should be made by creating a new "Issue" in the "Issues" section of [bluetooth_applications](https://github.com/SiliconLabs/bluetooth_applications) repo.

---
