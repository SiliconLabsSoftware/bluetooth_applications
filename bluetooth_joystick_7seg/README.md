# Bluetooth - Joystick 7seg #

![Type badge](https://img.shields.io/badge/Type-Virtual%20Application-green)
![Technology badge](https://img.shields.io/badge/Technology-Bluetooth-green)
![License badge](https://img.shields.io/badge/License-Zlib-green)
![SDK badge](https://img.shields.io/badge/SDK-v2024.12.2-green)
[![Required board](https://img.shields.io/badge/Mikroe-UT-green)](https://www.mikroe.com/ut-s-7-seg-r-click)
[![Required board](https://img.shields.io/badge/Sparkfun-Qwiic%20Joystick%20Board-green)](https://www.sparkfun.com/products/15168)
![Build badge](https://img.shields.io/badge/Build-passing-green)
![Flash badge](https://img.shields.io/badge/Flash-213.66%20KB-blue)
![RAM badge](https://img.shields.io/badge/RAM-10.98%20KB-blue)

## Description ##

This project shows an example of Bluetooth using Silicon Labs development kits and external sensors integrated with the BLE wireless stack. This project uses a Sparkfun Qwiic Joystick board and a UT-L 7-SEG R click board.

UT-L 7-SEG R click carries two SMD ultra-thin 7-segment LED displays and the MAX6969 constant-current LED driver from Maxim Integrated. It communicates with the target microcontroller over the SPI interface.

For more information about the MAX6969 constant-current LED driver, see the [specification page](https://datasheets.maximintegrated.com/en/ds/MAX6969.pdf).

The SparkFun Qwiic Joystick combines the convenience of the Qwiic connection system and an analog joystick that is similar to the analog joysticks on PS2 (PlayStation 2) controllers. Directional movements are simply measured with two 10 kΩ potentiometers, connected with a gimbal mechanism that separates the horizontal and vertical movements. This joystick also has a select button that is actuated when the joystick is pressed down. With the pre-installed firmware, the ATtiny85 acts as an intermediary (microcontroller) for the analog and digital inputs from the joystick. This allows the Qwiic Joystick to report its position over I2C.

For more information about the SparkFun Qwiic Joystick, see the [specification page](https://learn.sparkfun.com/tutorials/qwiic-joystick-hookup-guide).

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

- 1x [Bluetooth Low Energy Explorer Kit](https://www.silabs.com/development-tools/wireless/bluetooth). For simplicity, Silicon Labs recommends the [BGM220-EK4314A](https://www.silabs.com/development-tools/wireless/bluetooth/bgm220-explorer-kit)
- 1x [SparkFun Qwiic Joystick Board](https://www.sparkfun.com/products/15168)
- 1x [UT-S 7-SEG R click Board](https://www.mikroe.com/ut-s-7-seg-r-click)
- 1x smartphone running the 'Simplicity Connect' mobile app

---

## Connections Required ##

Attach the UT-L 7-SEG R Click board to the Explorer kit. Make sure that the 45-degree corner of the NFC board matches the 45-degree white line of the Silicon Labs Explorer kit

The Sparkfun Qwiic Joystick board can be easily connected by using a Qwiic cable.

![connection](image/connection.png)

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

1. From the Launcher Home, add your hardware to My Products, click on it, and click on the **EXAMPLE PROJECTS & DEMOS** tab. Find the example project filtering by "joystick_7seg".

2. Click **Create** button on the **Bluetooth - Joystick 7seg** example. Example project creation dialog pops up -> click Create and Finish and Project should be generated.
![create project](image/create_project.png)

3. Build and flash this example to the board.

### Start with a "Bluetooth - SoC Empty" project ###

1. Create a **Bluetooth - SoC Empty** project for your hardware using Simplicity Studio 5.

2. Copy the file `src/app.c` into the project root folder (overwriting the existing file).

3. Open the .slcp file again. Install and configure the following components:

   - [Services] → [IO Stream] → [IO Stream: USART] → default instance name: vcom
   - [Application] → [Utility] → [Log]
   - [Platform] → [Driver] → [Button] → [Simple Button] → default instance name: btn0
   - [Platform] → [Driver] → [I2C] → [I2CSPM] → default instance name: qwiic
   - [Third Party Hardware Drivers] → [Display & LED] → [MAX6969 - UT-M 7-SEG R Click (Mikroe)]
   - [Third Party Hardware Drivers] → [Human Machine Interface] → [Qwiic Joystick (Sparkfun)]

4. Import the GATT configuration:

   - Open the .slcp file in the project again.
   - Select the CONFIGURATION TOOLS tab and open the "Bluetooth GATT Configurator".
   - Click on the Import button and import the attached `config/btconf/gatt_configuration.btconf` file.
   - Save the GATT configuration (ctrl-s).
     ![gatt config](image/import_gatt_configuaration.png)

5. Build and flash this example to the board.

> [!NOTE]
>
> - A bootloader needs to be flashed to your board if the project starts from the "Bluetooth - SoC Empty" project, see [Bootloader](https://github.com/SiliconLabs/bluetooth_applications/blob/master/README.md#bootloader) for more information.

---

## How It Works ##

The application reads a current joystick position every 100ms and prints it on the 7-segment LED display. For simplicity, reading is the current vertical position from joystick and only look at the MSB and get an 8-bit reading (for 256 positions). Each time the joystick moves up to the highest position (255), the joystick data will be increased by 1. On the contrary, each time the joystick moves down to the lowest position (0), the joystick data will be decreased by 1. When the Bluetooth connection is opened, the joystick data can be seen via Bluetooth by reading it manually or it can be also automatically updated using Notification.

Follow the below steps to test the example with the Simplicity Connect application:

1. Open the Simplicity Connect app on your smartphone and allow the permission requested the first time it is opened.
2. Find your device in the Bluetooth Browser, advertising as *joystick_7seg*, and tap Connect. Then you need accept the pairing request when connected for the first time.
3. Find the Unknown Service in the GATT database.
4. Try to read, write, re-read the characteristics, and check the value.
5. Enable Notification on this service. Try to move up/down the joystick then check the value on the 7-segment LED display and on the Simplicity Connect app.
6. Try to use the BTN0 button to turn on/off the 7-segment LED display at any time.

   ![Simplicity connect](image/efr_connect.png)

7. You can launch the Console that is integrated in Simplicity Studio or can use a third-party terminal tool like Tera Term to receive the logs from the virtual COM port.

   ![console](image/console.png)

---

## Report Bugs & Get Support ##

To report bugs in the Application Examples projects, please create a new "Issue" in the "Issues" section of [bluetooth_applications](https://github.com/SiliconLabs/bluetooth_applications) repo. Please reference the board, project, and source files associated with the bug, and reference line numbers. If you are proposing a fix, also include information on the proposed fix. Since these examples are provided as-is, there is no guarantee that these examples will be updated to fix these issues.

Questions and comments related to these examples should be made by creating a new "Issue" in the "Issues" section of [bluetooth_applications](https://github.com/SiliconLabs/bluetooth_applications) repo.

---
