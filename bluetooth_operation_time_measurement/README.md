# Bluetooth - Operation Time Measurement (BMA400) #

![Type badge](https://img.shields.io/badge/Type-Virtual%20Application-green)
![Technology badge](https://img.shields.io/badge/Technology-Bluetooth-green)
![License badge](https://img.shields.io/badge/License-Zlib-green)
![SDK badge](https://img.shields.io/badge/SDK-v2024.12.2-green)
[![Required board](https://img.shields.io/badge/Mikroe-ACCEL%205%20CLICK-green)](https://www.mikroe.com/accel-5-click)
[![Required board](https://img.shields.io/badge/Adafruit-IS31FL3741%2013x9%20PWM%20RGB%20LED%20Matrix%20Driver-green)](https://www.adafruit.com/product/5201)
![Build badge](https://img.shields.io/badge/Build-passing-green)
![Flash badge](https://img.shields.io/badge/Flash-218.54%20KB-blue)
![RAM badge](https://img.shields.io/badge/RAM-11.24%20KB-blue)

## Overview ##

This project exemplifies how to develop an application, which uses the Silicon Labs Explorer Kit and Mikroe ACCEL 5 CLICK board and Adafruit IS31FL3741 components to monitor the operation time of machineries and display the information graphically. The application will use BLE for wireless communication between the devices.

**Sensor** device:
The sensor device is a Silicon Labs Explorer Kit, which is connected to a MikroE ACCEL 5 CLICK sensor board. This device periodically measures the operation time level and advertises the processed time value to the client devices.

The advertisement package contains a measurement counter value and the processed operation time in second.

**Client** device:
The client device is a Silicon Labs Explorer Kit, which is connected to an Adafruit IS31FL3741 RGB LED Matrix Display.

This device provides two operational modes.

If the BTN0 is pressed during the initialization sequence, then the application boots into the configuration mode. In this operational mode, the device sends normal advertisement packages and is connectable to other BLE devices. Users can connect to the client via the Simplicity Connect mobile application. This mode provides BLE characteristics to configure the operation time threshold.

If the BTN0 is released during the initialization phase, the application starts scanning the BLE network. The application periodically scans the BLE network looking for a sensor device. The client device subscribes to the operation time characteristic to get notifications about changes in the measured operation time.

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
  - [Sensor Overview](#sensor-overview)
  - [Client](#client)
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

- 2x [Bluetooth Low Energy Development Kit](https://www.silabs.com/development-tools/wireless/bluetooth). One is for the Sensor, another is for the Client. For example, [xG24-EK2703A](https://www.silabs.com/development-tools/wireless/efr32xg24-explorer-kit?tab=overview)
- 1x [Adafruit IS31FL3741 13x9 PWM RGB LED Matrix Driver - STEMMA QT / Qwiic](https://www.adafruit.com/product/5201)
- 1x [ACCEL 5 CLICK](https://www.mikroe.com/accel-5-click)
- 1x smartphone running the 'Simplicity Connect' mobile app

---

## Connections Required ##

The hardware connection is shown in the image below:

![hardware overview](image/overview.png)

**Sensor:**

The ACCEL 5 CLICK board can be easily connected to the EFR32xG24 Explorer Kit via a mikroBUS

**Client:**

Adafruit IS31FL3741 13x9 PWM RGB LED Matrix board can be easily connected to the EFR32xG24 Explorer Kit by using a Qwiic cable.

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

1. From the Launcher Home, add your hardware to My Products, click on it, and click on the **EXAMPLE PROJECTS & DEMOS** tab. Find the example project filtering by "bma400".

2. Click **Create** button on both **Bluetooth - Operation Time Measurement (BMA400) - Client** and **Bluetooth - Operation Time Measurement (BMA400) - Sensor** examples. Example project creation dialog pops up -> click Create and Finish and Project should be generated.
![create_project](image/create_project.png)

3. Build and flash this example to the board.

### Start with a "Bluetooth - SoC Empty" project ###

1. Create a **Bluetooth - SoC Empty** project for your hardware using Simplicity Studio 5.

2. copy all the .h and .c files to the following directory of the project root folder (overwriting existing file).

   - **Sensor** device: [bluetooth_operation_time_measurement_sensor](bluetooth_operation_time_measurement_sensor)

   - **Client** device: [bluetooth_operation_time_measurement_client](bluetooth_operation_time_measurement_client)

3. Open the .slcp file. Select the **SOFTWARE COMPONENTS** tab and install the software components:

   - **Sensor** device:
     - [Application] → [Utility] → [Timer]
     - [Platform] → [Driver] → [Button] → [Simple Button] → default instance name: btn0
     - [Services] → [IO Stream] → [IO Stream: USART] → default instance name: vcom
     - [Application] → [Utility] → [Log]
     - [Platform] → [Driver]→ [I2C] → [I2CSPM] → default instance name: mikroe
     - [Third Party Hardware Drivers] → [Sensors] → [BMA400 - ACCEL 5 CLICK (Mikroe) - I2C]

   - **Client** device:
     - [Application] → [Utility] → [Timer]
     - [Services] → [IO Stream] → [IO Stream: USART] → default instance name: vcom
     - [Application] → [Utility] → [Log]
     - [Platform] → [Driver] → [Button] → [Simple Button] → default instance name: btn0
     - [Platform] → [Driver] → [I2C] → [I2CSPM] → default instance name: qwiic
     - [Third Party Hardware Drivers] → [Display & LED] → [IS31FL3741 - 13x9 PWM RGB LED Matrix (Adafruit) - I2C]
       ![rgb_led_config](image/rgb_led_config.png)
     - [Third Party Hardware Drivers] → [Services] → [GLIB - OLED Graphics Library]

4. Import the GATT configuration:

   - Open the .slcp file in the project again.
   - Select the CONFIGURATION TOOLS tab and open the "Bluetooth GATT Configurator".
   - Find the Import button and import the  gatt_configuration.btconf file.
     - **Sensor** device: [gatt_configuration.btconf](bluetooth_operation_time_measurement_sensor/config/btconf/gatt_configuration.btconf)
     - **Client** device: [gatt_configuration.btconf](bluetooth_operation_time_measurement_client/config/btconf/gatt_configuration.btconf)
   - Save the GATT configuration (ctrl-s).

5. Build and flash this example to the board.

> [!NOTE]
>
> - A bootloader needs to be flashed to your board if the project starts from the "Bluetooth - SoC Empty" project, see [Bootloader](https://github.com/SiliconLabs/bluetooth_applications/blob/master/README.md#bootloader) for more information.

---

## How it Works ##

### Sensor Overview ###

![sensor_overview](image/sensor_overview.png)

#### Sensor GATT Database ####

- Advertisement Packet
- Device name: **bma400_sensor**
- [Service] **Operation Time Sensor**
  - [Char] Operation Time
    - [R/N] Get operation time so far in seconds
    - [W] Reset the operation time counter (valid value: 1)

#### Sensor Implementation ####

**Application initialization:**

![sensor_init](image/sensor_init.png)

**Application runtime:**

The BMA400 accelerometer sensor detects the activity and application logic changes in its state in accordance with the figure below.

- Idle state: the device is turned off, the operation time is not measured
- Operation state: the device is turned on, the operation time measurement is active

![sensor_runtime](image/sensor_runtime.png)

**Advertisement Packet:**

AdvData field in the advertisement packet is as table below:

| DeviceName | SampleCounter | Operation time |
|-----|-----|-----|
| bma400_sensor | 4 byte | 4 byte |

- SampleCounter: Device increases the counter value for each new measurement
- Operation time: Operation time in second
- Device is not connectable. It sends [manufacturer specific advertisement](https://github.com/SiliconLabs/bluetooth_stack_features/tree/master/advertising/advertising_manufacturer_specific_data) packets.

### Client ###

#### Client Overview ###

![client_overview](image/client_overview.png)

#### Client GATT Database ###

Advertisement Packet
Device name: bma400_client

GATT Database

- [Service] Operation Time Measurement Client
  - [Char] Operation Time Threshold
    - [R] Get the configured operation time threshold value
    - [W] Set operation time threshold value

#### Client Implementation ####

**Application initialization:**

![client_init](image/client_init.png)

**Runtime - Configuration Mode:**

![config_mode](image/client_config.png)

**Runtime - Normal Mode:**

*Client normal mode:*

![client_event](image/client_normal.png)

*BLE Notification Event:*

This function processes the measured values, checks the values against the configured thresholds.

The measured operation time is displayed on the connected LED matrix display.

![client_ble_notify](image/client_ble_notify.png)

**Display:**

If the Client is in the configuration mode, the RGB LED matrix will show a threshold value.

If the Client is in normal mode, the OLED will show the operation time received from the sensor device.
If the measured operation time reaches the configured threshold, then besides showing the measured operation time, the background of the text blinks in red.

### Testing ###

**Sensor:**

Follow the below steps to test the example with the Simplicity Connect application:

1. Open the Simplicity Connect app on your smartphone and allow the permission requested the first time it is opened.

2. Find your device in the Bluetooth Browser, advertising as *bma400_sensor*, and tap Connect.

3. After Connect to device name bma400_sensor, you should see the operation time service UUID. Please have a look at the red highlighted area below in the result pictures.

   ![app_phone](image/app_phone.png)

4. After flashing the code to the sensor board, a similar output from the serial terminal is shown as below.

   ![sensor_log ](image/sensor_log.png)

**Client:**

- After the firmware is configured, the device starts in normal mode. In this state, it starts scanning advertising devices.

- Open your terminal emulator and connect to your client device over its serial port. Set the baud rate to 115200. A similar output is expected as below.

  ![client_log](image/client_log.png)

> [!NOTE]
>
> Button PB0 should be pressed during startup (power-on or reset) to run the client in Configuration Mode. The terminal will display the below text.
>
> ![client_log_config](image/client_log_config.png)

---

## Report Bugs & Get Support ##

To report bugs in the Application Examples projects, please create a new "Issue" in the "Issues" section of [bluetooth_applications](https://github.com/SiliconLabs/bluetooth_applications) repo. Please reference the board, project, and source files associated with the bug, and reference line numbers. If you are proposing a fix, also include information on the proposed fix. Since these examples are provided as-is, there is no guarantee that these examples will be updated to fix these issues.

Questions and comments related to these examples should be made by creating a new "Issue" in the "Issues" section of [bluetooth_applications](https://github.com/SiliconLabs/bluetooth_applications) repo.

---
