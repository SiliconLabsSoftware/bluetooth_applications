# Bluetooth - BTHome v2 - People Counting (VL53L1X) #

![Type badge](https://img.shields.io/badge/Type-Virtual%20Application-green)
![Technology badge](https://img.shields.io/badge/Technology-Bluetooth-green)
![License badge](https://img.shields.io/badge/License-Zlib-green)
![SDK badge](https://img.shields.io/badge/SDK-v2024.12.2-green)
[![Required board](https://img.shields.io/badge/Sparkfun-Distance%20Sensor%20Breakout-green)](https://www.sparkfun.com/products/14722)
[![Required board](https://img.shields.io/badge/Sparkfun-Micro%20OLED%20Breakout%20(Qwiic)%20board-green)](https://www.sparkfun.com/products/14532)
![Build badge](https://img.shields.io/badge/Build-passing-green)
![Flash badge](https://img.shields.io/badge/Flash-226.28%20KB-blue)
![RAM badge](https://img.shields.io/badge/RAM-11.38%20KB-blue)

## Overview ##

This project aims to implement a people-counting application using Silicon Labs development kits integrated with the BLE wireless stack and a VL53L1X distance sensor.

This example can serve as the first step in developing other applications based on it. It can be upgraded to be a part of a people tracking system in a building, factory, or similar environment. Integrated with BLE wireless technology, the system can be easily monitored and controlled by the user.

---

## Table Of Contents ##

- [SDK version](#sdk-version)
- [Software Required](#software-required)
- [Hardware Required](#hardware-required)
- [Connections Required](#connections-required)
- [Setup](#setup)
  - [Based on an example project](#based-on-an-example-project)
  - [Start with a "Bluetooth - SoC Empty" project](#start-with-a-bluetooth---soc-empty-project)
- [How It Works](#how-it-works)
  - [GATT Database](#gatt-database)
  - [People Counting Implementation](#people-counting-algorithm)
  - [Application Initialization](#application-initialization)
  - [Testing](#testing)
    - [OLED Display](#oled-display)
    - [Button](#button)
    - [Room status notification](#room-status-notification)
    - [Reset the counting value](#reset-the-counting-value)
    - [Connect to Simplicity Connect app](#connect-to-simplicity-connect-app)
    - [Connect to Home Assistant app](#connect-to-home-assistant-app)
    - [Console View](#console-view)
- [Report Bugs & Get Support](#report-bugs--get-support)

---

## SDK version ##

- [Simplicity SDK v2024.12.2](https://github.com/SiliconLabs/simplicity_sdk)
- [Third Party Hardware Drivers v4.3.0](https://github.com/SiliconLabs/third_party_hw_drivers_extension)

---

## Software Required ##

- [Simplicity Studio v5 IDE](https://www.silabs.com/developers/simplicity-studio)
- [Home Assistant OS](https://www.home-assistant.io/)

---

## Hardware Required ##

- 1x [Bluetooth Low Energy Development Kit](https://www.silabs.com/development-tools/wireless/bluetooth). For simplicity, Silicon Labs recommends the [BGM220-EK4314A](https://www.silabs.com/development-tools/wireless/bluetooth/bgm220-explorer-kit)
- 1x [SparkFun Micro OLED Breakout (Qwiic) board](https://www.sparkfun.com/products/14532)
- 1x [SparkFun Distance Sensor Breakout - VL53L1X](https://www.sparkfun.com/products/14722)
- 1x Raspberry Pi 4 running Home Assistant OS
- 1x smartphone running Home Assistant application

---

## Connections Required ##

The I2C connection is made from the BGM220 Bluetooth Module Explorer Kit to the Distance Sensor Breakout board and the Micro OLED Breakout by using the Qwiic cable.

The hardware connection is shown in the image below:

![hardware connection](image/hardware_connection.png)

---

## Setup ##

To test this application, you can either create a project based on an example project or start with a "Bluetooth - SoC Empty" project based on your hardware.

> [!NOTE]
>
> - Make sure that the [Third Party Hardware Drivers extension](https://github.com/SiliconLabs/third_party_hw_drivers_extension) is installed as part of the SiSDK and the [bluetooth_applications](https://github.com/SiliconLabs/bluetooth_applications) repository is added to [Preferences > Simplicity Studio > External Repos](https://docs.silabs.com/simplicity-studio-5-users-guide/latest/ss-5-users-guide-about-the-launcher/welcome-and-device-tabs).
>
> - SDK Extension must be enabled for the project to install the required components.

### Based on an example project ###

1. From the Launcher Home, add your hardware to **My Products**, click on it, and click on the **EXAMPLE PROJECTS & DEMOS** tab. Find the example project filtering by **"people counting"**.

2. Click **Create** button on the **Bluetooth - People Counting (VL53L1X) with BThome v2** example. Example project creation dialog pops up -> click Create and Finish and Project should be generated.
   ![create_project](image/create_project.png)

3. Build and flash this example to the board.

### Start with a "Bluetooth - SoC Empty" project ###

1. Create a **Bluetooth - SoC Empty** project for your hardware using Simplicity Studio 5.

2. Copy all attached files in **inc** and **src** folders into the project root folder (overwriting existing).

3. Import the GATT configuration:

   - Open the .slcp file in the project.

   - Select the **CONFIGURATION TOOLS** tab and open the **Bluetooth GATT Configurator**.

   - Find the Import button and import the configuration `bluetooth_people_counting/config/btconfig/gatt_configuration.btconf` file.

   - Save the GATT configuration (ctrl-s).

4. Open the .slcp file. Select the **SOFTWARE COMPONENTS** tab and install the software components:

   - [Services] → [IO Stream] → [IO Stream: USART] → default instance name: vcom
   - [Application] → [Utility] → [Log]
   - [Platform] → [Driver] → [Button] → [Simple Button] → default instance name: btn0
   - [Platform] → [Driver] → [LED] → [Simple LED] → default instance name: led0
   - [Platform] → [Driver] → [I2C] → [I2CSPM] → default instance name: qwiic
   - [Third Party Hardware Drives] → [Sensors] → [VL53L1X - Distance Sensor Breakout (Sparkfun)]
   - [Third Party Hardware Drives] → [Display & LED] → [SSD1306 - Micro OLED Breakout (Sparkfun - I2C)]
   - [Third Party Hardware Drives] → [Services] → [GLIB - OLED Graphics Library]
   - [Third Party Hardware Drives] → [Services] → [BTHome v2]
  
5. Build and flash the project to your device.

> [!NOTE]
>
> A bootloader needs to be flashed to your board if the project starts from the "Bluetooth - SoC Empty" project, see [Bootloader](https://github.com/SiliconLabs/bluetooth_applications/blob/master/README.md#bootloader) for more information.

---

## How It Works ##

### GATT Database ###

Advertisement Packet Device name: **People Counting**

- Device name: **RFID Notify**

- **[Service] People Counting**
  - **[Char] People Entered So Far** UUID: `cf88405b-e223-4976-82aa-78c6b305b0a8`
    - [R] Get the number of people entering the room.
    - [W] Clear the number of people entering the room.
  - **[Char] People Count** UUID: `2b9837e1-5560-49e5-a8cf-2f3b0db0bd6b`
    - [R] Get the number of people in the room.
    - [W] Clear the number of people in the room.
  - **[Char] Min Distance:** UUID: `f2f7c459-e623-4970-ab36-d3a4651a694e`
    - [R] Get minimum distance that is used by counting people algorithm (mm).
    - [W] Set minimum distance that is used by counting people algorithm (mm).
  - **[Char] Max Distance:** UUID: `d0a946d7-a183-4cb7-a9cb-b9c879cdb6fa`
    - [R] Get maximum distance that is used by counting people algorithm (mm).
    - [W] Set maximum distance that is used by counting people algorithm (mm).
  - **[Char] Distance Threshold:** UUID: `0192bd9d-cb4f-49cc-b3dc-2d7facc9edcd`
    - [R] Get distance threshold that is used by counting people algorithm (mm).
    - [W] Set distance threshold that is used by counting people algorithm (mm).
  - **[Char] Timing Budget:** UUID: `01fb0e47-13c9-4369-88cc-07f58759a6a6`
    - [R] Get timing budget that is used by counting people algorithm (ms).
    - [W] Set timing budget that is used by counting people algorithm (ms).
  - **[Char] Notification Status:** UUID: `ca89196b-76e2-41a0-9e41-342f4a2ff6f1`
    - [R] Get notification status.
    - [W] Enable & disable notification status.
  - **[Char] Room capacity:** UUID: `c714d394-7e0d-4c6a-a864-1183046a244c`
    - [R] Get room capacity.
    - [W] Set room capacity.
    - [N] Get notification of room status( full or empty).

### People Counting Implementation ###

#### Application initialization ####

![Application init](image/app_init.png)  

#### Sensor initialization ####

![Sensor init](image/sensor_init.png)  

#### Sensor sampling ####

![Sensor sampling](image/sampling_workflows.png)

#### Application Workflows ####

1. Initialize the peripherals, the Bluetooth stack

2. Initialize and load the previous configurations from NVM memory.

3. Wait for the VL53L1X sensor to be booted and initialize the VL53L1X sensor with the configurations from NVM3:

   - Distance mode: LONG
   - Timing budget: 33 ms
   - Region of interest SPADs: 8x16

4. Start ranging for the VL53L1X sensor.

5. Initialize the OLED display.

6. Start a periodical timer every second. The timer callback will raise an external event to the BLE stack and the event handler will do:

   - Check if the ranging data is ready.
   - Get a new distance sample.
   - Calculate people counting algorithm with new distance sample.
   - Switch the Region of Interest (ROI) center to other zones (front or back)

7. Start a periodical timer with 1000 milliseconds for each periodic period, The timer callback will raise an external event to the BLE stack. The event handler will display people counting data which was calculated by the people counting algorithm. It also sends the data to Home Assistant server.

8. After the **sl_bt_evt_system_boot_id** event arrives, the application sets up the security manager to bond with an iOS or Android device. And then start advertising.

9. Handle GATT events to help the user configures the counting algorithm and get the result from the algorithm calculation over the **Simplicity Connect** mobile application.

### People Counting algorithm ###

The counting algorithm example relies on a list of states that have to occur in a certain order to detect if a person has crossed the specified area and in which direction this area has been crossed. These states are stored in a list and compared to two default lists of states that represent how the area is crossed in two different directions.

When no one is seen in either of the two zones, the list of states is reset.
If we consider that a person detected in the front zone equals 2, and a person detected in the back zone equals 1, the algorithm adds these values and stores the result as soon as it changes.

Eventually, if the consecutive states in the list are 0, 1, 3, 2, 0 or 0, 2, 3, 1, 0 this means a person has been detected in one direction or the other, as described in the figure below. List of status values.

![Counting Algorithm](image/people_counting_algorithm.jpg)  

#### Algorithm workflows ####

![Algorithm workflows](image/algorithm_workflows.png)

### Testing ###

#### OLED Display ####

Display the number of people standing in the detected areas and the second value represents the number of people who entered the room.
  
![OLED display](image/oled_display.png)

#### Button ####

Press the button to clear the number of people who are standing in detected areas.

#### Room status notification ####

- To receive the room's state (full or empty), the user needs to enable notification for the **Room Capacity** characteristic.

- If the number of entered people is greater than the room capacity then the device will send a "room is full" notification to the client.
  
- If the number of entered people is zero then the device will send a "room is empty" notification to the client.

#### Reset the counting value ####

- To reset the number of people who entered the room, the user has to write 0 to the **People Entered So Far** characteristic.
  
- To reset the number of people currently, the user has to write 0 to the **People Count** characteristic.

#### Connect to Simplicity Connect app ####

Follow the below steps to test the example with the Simplicity Connect application:

- Open the Simplicity Connect app on your smartphone and allow the permission requested the first time it is opened.

- Find your device in the Bluetooth Browser, advertising as *People Counting*, and tap Connect. Then you need accept the pairing request when connected for the first time.

> [!NOTE]
>
> The pairing process on Android and iOS devices is different. For more information, refer to Bluetooth security.
>
> | ![device_namae](image/device_name.png) | ![device_pairing](image/device_pairing.png)|![device_characteristics](image/device_characteristics.png)|
> | - | - | -|

#### Connect to Home Assistant app ####

The BThome integration in **Home Assistant** will detect nearby BThome devices. Discovered devices will show up in the discovered section on the integrations page in the configuration panel.

![ha1](image/ha1.png)

Open the *Home Assistant* application on your smartphone. Click [Settings] → [Devices & Services] → [BTHome], and you will see a list of nearby devices, which are sending BTHome advertisements. Find the one named "BGM220P" and click the *ADD ENTRY*. Enter the BindKey then submit, add device to your home. Now you can see the *Count* data data show in the screen.

![ha2](image/ha2.png)

#### Console View ####

You can launch Console that's integrated into Simplicity Studio or use a third-party terminal tool like Tera Term to receive the data from the USB. A screenshot of the console output is shown in the figure below.

![console_log](image/console_log.png)

---

## Report Bugs & Get Support ##

To report bugs in the Application Examples projects, please create a new "Issue" in the "Issues" section of [bluetooth_applications](https://github.com/SiliconLabsSoftware/bluetooth_applications) repo. Please reference the board, project, and source files associated with the bug, and reference line numbers. If you are proposing a fix, also include information on the proposed fix. Since these examples are provided as-is, there is no guarantee that these examples will be updated to fix these issues.

Questions and comments related to these examples should be made by creating a new "Issue" in the "Issues" section of [bluetooth_applications](https://github.com/SiliconLabsSoftware/bluetooth_applications) repo.

---
