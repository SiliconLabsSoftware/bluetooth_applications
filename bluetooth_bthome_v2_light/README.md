# Bluetooth - BTHome v2 - Light #

![Technology badge](https://img.shields.io/badge/Technology-Bluetooth-green)
![License badge](https://img.shields.io/badge/License-Zlib-green)
![SDK badge](https://img.shields.io/badge/SDK-v2025.6.2-green)
![Build badge](https://img.shields.io/badge/Build-passing-green)
![Flash badge](https://img.shields.io/badge/Flash-220.14%20KB-blue)
![RAM badge](https://img.shields.io/badge/RAM-12.38%20KB-blue)

[![Type badge](https://img.shields.io/badge/Commercial%20Lighting-salmon)](https://siliconlabs-massmarket.github.io/repository-catalog/#applications-list?filter=Commercial%20Lighting)
[![Type badge](https://img.shields.io/badge/LED%20Lighting-salmon)](https://siliconlabs-massmarket.github.io/repository-catalog/#applications-list?filter=LED%20Lighting)
[![Type badge](https://img.shields.io/badge/Smart%20Buildings-salmon)](https://siliconlabs-massmarket.github.io/repository-catalog/#applications-list?filter=Smart%20Buildings)
[![Type badge](https://img.shields.io/badge/Smart%20HVAC-salmon)](https://siliconlabs-massmarket.github.io/repository-catalog/#applications-list?filter=Smart%20HVAC)
[![Type badge](https://img.shields.io/badge/Street%20Lighting-salmon)](https://siliconlabs-massmarket.github.io/repository-catalog/#applications-list?filter=Street%20Lighting)
[![Type badge](https://img.shields.io/badge/Switches-salmon)](https://siliconlabs-massmarket.github.io/repository-catalog/#applications-list?filter=Switches)

## Overview ##

![logo](image/logo.png)

BTHome is an energy-efficient yet flexible Bluetooth format for devices to broadcast their sensor data and button presses.

Devices can run over a year on a single battery. It allows data encryption and is supported by popular home automation platforms, such as Home Assistant, out of the box.

For more information, please visit [BThome](https://bthome.io/).

This project aims to implement a BTHome v2-compatible light controller. The application provides a CLI to configure switches to control the onboard LED0; it supports press events only.

One or more switches can control a light. Each switch acts as a BTHome client that advertises a button press event to control the light. This device should use the *BTHome v2 - Switch* example for testing in this project.

The Light device acts as both a BTHome v2 Client and Server (implemented in this project as *BTHome v2 - Light*):

- As a server, it receives button events from registered devices to control the light.
- As a client, it reports the light status to Home Assistant.

![connection](image/connection2.png)

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
  - [Application Initialization](#application-initialization)
  - [BTHome v2 Events](#bthome-v2-events)
  - [Testing](#testing)
    - [Silicon Labs Devices](#silicon-labs-devices)
    - [Home Assistant app](#home-assistant-app)
- [Report Bugs & Get Support](#report-bugs--get-support)

---

## SDK version ##

- [Simplicity SDK v2025.6.2](https://github.com/SiliconLabs/simplicity_sdk/releases/tag/v2025.6.2)
- [Third Party Hardware Drivers v4.4.1](https://github.com/SiliconLabs/third_party_hw_drivers_extension)

---

## Software Required ##

- [Simplicity Studio v5 IDE](https://www.silabs.com/developers/simplicity-studio)
- [Home Assistant OS](https://www.home-assistant.io/)

---

## Hardware Required ##

- 2x [Bluetooth Low Energy Development Kit](https://www.silabs.com/development-tools/wireless/bluetooth). For simplicity, Silicon Labs recommends the [BGM220-EK4314A](https://www.silabs.com/development-tools/wireless/bluetooth/bgm220-explorer-kit)
  - 1x BGM220 running *BTHome v2 - Light*
  - 1x BGM220 running *BTHome v2 - Switch*
- 1x Raspberry Pi 4 running Home Assistant OS
- 1x Smartphone running Home Assistant application

---

## Connections Required ##

The hardware connection is shown in the image below:

![connection](image/connection1.png)

---

## Setup ##

To test this application, you can either create a project based on an example project or start with a "Bluetooth - SoC Empty" project based on your hardware.

> [!NOTE]
>
> - Make sure that the [Third Party Hardware Drivers extension](https://github.com/SiliconLabs/third_party_hw_drivers_extension) is installed as part of the SiSDK and the [bluetooth_applications](https://github.com/SiliconLabs/bluetooth_applications) repository is added to [Preferences > Simplicity Studio > External Repos](https://docs.silabs.com/simplicity-studio-5-users-guide/latest/ss-5-users-guide-about-the-launcher/welcome-and-device-tabs).
>
> - SDK Extension must be enabled for the project to install the required components.

### Based on an example project ###

1. From the Launcher Home, add your hardware to My Products, click on it, and click on the **EXAMPLE PROJECTS & DEMOS** tab. Find the example project by filtering for "bthome".

2. Click the **Create** button on the **Bluetooth - BTHome v2 - Light** example. The example project creation dialog pops up → click Create and Finish and the project should be generated.
![create_project](image/create_project.png)

3. Build and flash this example to the board.

### Start with a "Bluetooth - SoC Empty" project ###

1. Create a **Bluetooth - SoC Empty** project for your hardware using Simplicity Studio 5.

2. Copy all the .h and .c files to the project root folder (overwriting existing files).

3. Open the .slcp file. Select the **SOFTWARE COMPONENTS** tab and install the software components:

   - [Platform] → [Driver] → [Button] → [Simple Button] → default instance name: btn0.
   - [Platform] → [Driver] → [LED] → [Simple LED] → default instance name: led0.
   - [Services] → [IO Stream] → [IO Stream: USART] → default instance name: vcom
   - [Services] → [Command Line Interface] → [CLI Instance(s)] → default instance name: inst
   - [Application] → [Utility] → [Log]
   - [Third-Party Hardware Drivers] → [Services] → [BTHome v2]
   - [Third-Party Hardware Drivers] → [Services] → [BTHome v2 - Server]

4. Build and flash the project to your device.

> [!NOTE]
>
> A bootloader needs to be flashed to your board if the project starts from the "Bluetooth - SoC Empty" project, see [Bootloader](https://github.com/SiliconLabs/bluetooth_applications/blob/master/README.md#bootloader) for more information.

---

## How It Works ##

This example demonstrates a hallway lighting scenario where one or more switches control a single light. For example, in a hallway with switches at both ends, you can turn the light on and off from either location.

### Application Initialization ###

![application_initialization](image/application_init.png)

### BTHome v2 Events ###

![bthome_v2_events](image/bthome_v2_events.png)

The example implements a CLI interface that provides the following features:

- Scan the BLE network and list BTHome devices with the following parameters:
  - MAC address - Encryption (Yes/No) - Encryption Key Available (Yes/No)
- List registered devices
  - MAC address - Encryption Key
- Register key by MAC address
- Remove key by MAC address
- Add device to the interested devices list by MAC address
- Remove device from the interested devices list by MAC address
- Start/Stop light control system

### Testing ###

For testing, you need at least two Silicon Labs boards. One acts as a Switch device using the "BTHome v2 - Switch" example, and one acts as a Light device using the "BTHome v2 - Light" example.

#### Silicon Labs Devices ####

1. Open a console or terminal program (e.g., Tera Term) and connect to the device to see the logs.

2. Type `help` to see the supported commands:

   ![help](image/help.png)

3. Type `scan start` to begin scanning for BTHome v2 devices. A continuous scan will update with the latest data from devices. You can verify if the device found is *BTHome v2 - Switch*.

   ![scan_start](image/scan_start.png)

4. Type `key_register <MAC addr> <Key>` to register the *BTHome v2 - Switch* device.

   ![key](image/key.png)

5. Type `light_system start` to begin controlling the LED using the button from the *BTHome v2 - Switch* device.

   ![light_system](image/light_system.png)

6. You can also add additional buttons by following the same process.

#### Home Assistant app ####

1. The **Home Assistant** application utilizes the Bluetooth adapter on your phone/tablet to scan for BLE devices.

   ![app1](image/app1.png)

2. Open the *Home Assistant* application on your smartphone. Navigate to [Settings] → [Devices & Services] → [BTHome], and you will see a list of nearby devices that are sending BTHome advertisements. Find the device named "BTLight" and click *ADD ENTRY*. Enter the BindKey, then submit to add the device to your home.

   ![app2](image/app2.png)

3. Automations in Home Assistant allow you to automatically respond to events. You can monitor when the light turns on or off. For this example, we will create two simple automations to receive signals from devices and display them on the dashboard.

   ![app3](image/app3.png)

---

## Report Bugs & Get Support ##

To report bugs in the Application Examples projects, please create a new "Issue" in the "Issues" section of [bluetooth_applications](https://github.com/SiliconLabsSoftware/bluetooth_applications) repo. Please reference the board, project, and source files associated with the bug, and reference line numbers. If you are proposing a fix, also include information on the proposed fix. Since these examples are provided as-is, there is no guarantee that these examples will be updated to fix these issues.

Questions and comments related to these examples should be made by creating a new "Issue" in the "Issues" section of [bluetooth_applications](https://github.com/SiliconLabsSoftware/bluetooth_applications) repo.

---
