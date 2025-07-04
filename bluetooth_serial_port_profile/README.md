# Bluetooth - Serial Port Profile (SPP) #

![Type badge](https://img.shields.io/badge/Type-Virtual%20Application-green)
![Technology badge](https://img.shields.io/badge/Technology-Bluetooth-green)
![License badge](https://img.shields.io/badge/License-Zlib-green)
![SDK badge](https://img.shields.io/badge/SDK-v2024.12.2-green)
![Build badge](https://img.shields.io/badge/Build-passing-green)
![Flash badge](https://img.shields.io/badge/Flash-196.2%20KB-blue)
![RAM badge](https://img.shields.io/badge/RAM-10.55%20KB-blue)

## Overview ##

This example provides a simple template for SPP-like communication (also know as wire replacement), where Bluetooth serves as a transport channel for serial communication between two devices. To keep the code as short and simple as possible, the features are minimal. Users are expected to customize the code as needed to match their project requirements.

Although the serial communication is symmetric between the two devices, still there is a server and a client in the communication due to the nature of Bluetooth technology. The associated sample code is a single application that implements both server and client roles.

We have implemented a **client-side device**, named [web-bluetooth-spp-application](https://github.com/SiliconLabsSoftware/web-bluetooth-spp-application) to exemplify how to communicate with Silicon Labs wireless micro-controllers via the Web Bluetooth API using the BLE protocol.

---

## Table of Contents ##

- [Description](#description)
  - [SPP Server](#spp-server)
  - [SPP Client](#spp-client)
  - [Power management](#power-management)
  - [Known Issues](#known-issues)
- [Overview](#overview)
- [SDK version](#sdk-version)
- [Hardware Required](#hardware-required)
- [Software Required](#software-required)
- [Connections Required](#connections-required)
- [Setup](#setup)
  - [Create a project based on an example project](#create-a-project-based-on-an-example-project)
  - [Start with a "Bluetooth - SoC Empty" project](#start-with-a-bluetooth---soc-empty-project)
- [How It Works](#how-it-works)
  - [Test with 2 EFR boards](#test-with-2-efr-boards)
  - [Test with Web App](#test-with-web-app)
- [Report Bugs & Get Support](#report-bugs--get-support)

---

## Description ##

### SPP Server ###

Because Bluetooth Low Energy does not have a standard SPP service, it needs to be implemented as a custom service. The custom service is as minimal as possible. Only one characteristic is used for both incoming and outgoing data. The service is defined in the *gatt_configuration.btconf* file associated with this document and shown below.

```xml
 <!--SPP Service-->
  <service advertise="true" name="SPP Service" requirement="mandatory" sourceId="" type="primary" uuid="4880c12c-fdcb-4077-8920-a450d7f9b907">
    <informativeText/>
    <!--SPP Data-->
    <characteristic const="false" id="spp_data" name="SPP Data" sourceId="" uuid="fec26ec4-6d71-4442-9f81-55bc21d658d6">
      <informativeText/>
      <value length="20" type="hex" variable_length="true">00</value>
      <properties>
        <write_no_response authenticated="false" bonded="false" encrypted="false"/>
        <notify authenticated="false" bonded="false" encrypted="false"/>
      </properties>
    </characteristic>
  </service>
```

At the boot event, the server is put into advertisement mode. This example uses advertising packets that are automatically filled by the stack. In the custom SPP service definition (see above), *advertise* is set to *true*, meaning that the stack will automatically add the 128-bit UUID in advertisement packets. The SPP client will use this information to recognize the SPP server among other BLE peripherals.

For incoming data (data sent by the SPP client and written to UART in the SPP server), unacknowledged write transfers (*write_no_response*) are used, which provides better performance than normal acknowledged writes because several write operations can fit into one connection interval.

For outgoing data (data received from UART and sent to SPP client), notifications with the command `sl_bt_gatt_server_send_notification` are used. Notifications are unacknowledged, which again allows several notifications to fit into one connection interval.
Note that data transfers are unacknowledged at the GATT level. This means that at **application level**, no acknowledgments occur. However, at the lower protocol layers, each packet is still acknowledged and retransmissions are used when needed to ensure that all packets are delivered.
The core of the SPP example implementation is a 256-byte FIFO buffer (data[] in send_spp_data function) used to manage outgoing data. Data is received from UART and pushed to the SPP client using notifications. Incoming data from client raises the `sl_bt_evt_gatt_server_attribute_value` event. The received data is then copied to UART.

### SPP client ###

In terms of incoming/outgoing UART data, the SPP client works the same way as the SPP server. A similar 256-byte FIFO buffer is used with the following differences:

Data is received over the air by notifications(`sl_bt_evt_gatt_characteristic_value` event). Data is sent by calling `sl_bt_gatt_write_characteristic_value_without_response`.

The client is more complex than the server because it needs to detect the SPP server by looking at the advertisement packets. Additionally, the client needs to do service discovery after connecting to the client to get the information about the remote GATT database.

To use the SPP service, the client needs to know the characteristic handle values. The handles are discovered dynamically so that hard-coded values are not needed. This is essential if the SPP server needs to be ported to some other BLE module and the handle values are not known in advance.

At startup, the client starts discovery by calling `sl_bt_scanner_start`. For each received advertisement packet, the stack will raise event `sl_bt_evt_scanner_scan_report`. To recognize the SPP server, the client scans through each advertisement packet and searches for the 128-bit service UUID that is assigned for the custom SPP service.

Scanning advertisement packets is done with the function `process_scan_response`. Advertising packets include one or more advertising data elements that are encoded as defined in the BT specification. Each advertising element begins with a length byte that indicates the length of that field, which makes scanning through all elements in a while-loop easy.

The second byte in the advertising element is the AD type. The predefined types are listed in [Blutetooth SIG website](https://www.bluetooth.com/specifications/assigned-numbers/Generic-Access-Profile).

In this use case, types 0x06 and 0x07 that indicate incomplete/complete list of 128-bit service values are relevant. If this AD type is found, the AD payload is compared against the known 128-bit UUID of the SPP service to check if there is a match.

After finding a match in the advertising data, the client opens a connection by calling `sl_bt_connection_open`. When the connection is opened, the next task is to discover services and figure out the handle value that corresponds to the SPP_data characteristic (see XML definition of our custom service attached).

The service discovery is implemented as a simple state machine and the sequence of operations after connection is opened and summarized below:

1) Call `sl_bt_gatt_discover_primary_services_by_uuid` to start service discovery

2) Call `sl_bt_gatt_discover_characteristics` to find the characteristics in the SPP service (in FIND_SERVICE state)

3) Call `sl_bt_gatt_set_characteristic_notification` to enable notifications for spp_data characteristic (in FIND_CHAR state)

Note that in step one above, only services that match the specific UUID are relevant for service discovery. Another option is to call `sl_bt_gatt_discover_primary_services` and return list of all services in the remote GATT database.

After each procedure in the above sequence is completed, the stack will raise event `sl_bt_evt_gatt_procedure_completed`. The client uses a variable *main_state* to keep track of the current state. The `sl_bt_evt_gatt_procedure_completed` event will trigger the state machine to move on to the next logical state.

When notifications are enabled, the application is in transparent SPP mode, which is indicated by writing string "SPP Mode ON" to UART. After this point, any data that is received from UART is sent to server using non-acknowledged write transfer. Similarly, all data received via notifications (event: `sl_bt_evt_gatt_characteristic_value`) is copied to the local UART.

Note that on the server application the "SPP Mode ON" string is printed to the console. On the server side, this is done when the remote client has enabled notifications for the spp_data characteristic.

Data is handled transparently, meaning that the transmitted values can be either ASCII strings or binary data, or a mixture of these.

### SPP web bluetooth application ###

You can check out the source code here: [web-bluetooth-spp-application](https://github.com/SiliconLabsSoftware/web-bluetooth-spp-application) and follow the instructions to run it locally. Alternatively, you can use the live demo online at [https://siliconlabssoftware.github.io/web-bluetooth-spp-application/](https://siliconlabssoftware.github.io/web-bluetooth-spp-application/). A detailed guide is also provided in the [Test with Web App](#test-with-web-app) section, which describes how to use the live demo with the server-side of this application example.

### Power management ###

USART peripheral is not accessible in EM2 sleep mode. For this reason, both the client and the server applications disable sleeping (EM2 mode) temporarily when the SPP mode is active. **SPP mode** in this context means that the client and server are connected and that the client has enabled notifications for the SPP_data characteristics.

When SPP mode is entered, the code calls `sl_power_manager_add_em_requirement(SL_POWER_MANAGER_EM1)` to temporarily disable sleeping. When connection is closed (or client disables notifications), `sl_power_manager_remove_em_requirement(SL_POWER_MANAGER_EM1)` is called to re-enable sleeping.

### Known issues ###

This example implementation does not guarantee 100% reliable transfer. For incoming data, the driver uses a FIFO buffer whose size is defined using symbol `SL_IOSTREAM_USART_VCOM_RX_BUFFER_SIZE` (default value is 32).

To get a more reliable operation, increase the `SL_IOSTREAM_USART_VCOM_RX_BUFFER_SIZE` value. However, even with a large FIFO buffer, some data may get lost if the data rate is very high. If the FIFO buffer in RAM becomes full, the driver will simply drop the bytes that do not fit.

---

## SDK version ##

- [Simplicity SDK v2024.12.2](https://github.com/SiliconLabs/simplicity_sdk)

---

## Software Required ##

- [Simplicity Studio v5 IDE](https://www.silabs.com/developers/simplicity-studio)
- [Web Bluetooth SPP Application](https://github.com/SiliconLabsSoftware/web-bluetooth-spp-application)

---

## Hardware Required ##

- 2x [Bluetooth Low Energy Development Kit](https://www.silabs.com/development-tools/wireless/bluetooth). For simplicity, Silicon Labs recommends the [BGM220-EK4314A](https://www.silabs.com/development-tools/wireless/bluetooth/bgm220-explorer-kit)

---

## Connections Required ##

- Connect the Bluetooth Development Kits to the PC through a compatible-cable. For example, a micro USB cable for the BGM220 Bluetooth Module Explorer Kit.

---

## Setup ##

To test this application, you can either create a project based on an example project or start with a "Bluetooth - SoC Empty" project based on your hardware.

> [!NOTE]
>
> Make sure that the [bluetooth_applications](https://github.com/SiliconLabs/bluetooth_applications) repository is added to [Preferences > Simplicity Studio > External Repos](https://docs.silabs.com/simplicity-studio-5-users-guide/latest/ss-5-users-guide-about-the-launcher/welcome-and-device-tabs).

### Create a project based on an example project ###

1. From the Launcher Home, add your hardware to My Products, click on it, and click on the **EXAMPLE PROJECTS & DEMOS** tab. Find the example project filtering by "spp".

2. Click **Create** button on the **Bluetooth - Serial Port Profile (SPP) - Server** and **Bluetooth - Serial Port Profile (SPP) - Client** examples. Example project creation dialog pops up -> click Create and Finish and Project should be generated.
![create_project](image/create_project.png)

3. Build and flash this example to the board.

### Start with a "Bluetooth - SoC Empty" project ###

1. Create a **Bluetooth - SoC Empty** project for your hardware using Simplicity Studio 5.

2. Copy all attached files in the *inc* and *src* folders into the project root folder (overwriting existing file).

   - With **Server** device: [bt_spp_server](bt_spp_server)
   - With **client** device: [bt_spp_client](bt_spp_client)

3. Import the GATT configuration:

   - Open the .slcp file in the project.

   - Select the **CONFIGURATION TOOLS** tab and open the **Bluetooth GATT Configurator**.

   - Find the Import button and import the attached files:
     - With **Server** device: `bt_spp_server/config/btconf/gatt_configuration.btconf`.
     - With **Client** device: `bt_spp_client/config/btconf/gatt_configuration.btconf`.

   - Save the GATT configuration (ctrl-s).

4. Open the .slcp file. Select the **SOFTWARE COMPONENTS tab** and install the software components:

   - [Services] → [IO Stream] → [IO Stream: USART] → default instance name: vcom

   - [Platform] → [Board] → [Board Control] → enable *Virtual COM UART*

   - [Application] → [Utility] → [Log]

5. Build and flash the project to your device.

> [!NOTE]
>
> A bootloader needs to be flashed to your board if the project starts from the "Bluetooth - SoC Empty" project, see [Bootloader](https://github.com/SiliconLabs/bluetooth_applications/blob/master/README.md#bootloader) for more information.

---

## How It Works ##

### Test with 2 EFR boards ###

1. Flash one radio board with the client code and another one with the server code.

2. Open two instances of your favorite terminal program and connect to both kits via the virtual COM port (find the JLink CDC UART ports). Use the following UART settings: **baud rate 115200, 8N1, no flow control**.

3. Press the reset button on both kits.

4. The two kits automatically find each other and set up a connection. Logs are visible from the terminal programs.

5. Once the connection is set up and SPP mode is on, type anything to either of the terminal programs and see it appearing on the other terminal.

   ![spp_terminal](image/spp_terminal.gif)

> [!NOTE]
>
> Make sure that you are using the same baud rate and flow control settings in your starter kit and radio board or module firmware as well as your terminal program. For WSTK, this can be checked in Debug Adapters->Launch Console->Admin view, by typing "serial vcom".
>
> ![v3_launch_console](image/v3_launch_console.png)
> ![v3_wstk_config](image/v3_wstk_config.png)

### Test with Web App ###

1. Flash one radio board with the server code. Open a terminal program(e.g., Tera Term) and connect to server device to see the logs.

2. Follow the guideline from [web-bluetooth-spp-application](https://github.com/SiliconLabsSoftware/web-bluetooth-spp-application) to run the Web Application locally. Or you can use the live demo online here for testing purpose - [https://siliconlabssoftware.github.io/web-bluetooth-spp-application/](https://siliconlabssoftware.github.io/web-bluetooth-spp-application/)

3. From the web browser, press the connect button and choose the server device named: **SPP Exam**. After sucessfully connected, you will be able to send message between the web app client and server device.

   |![Web app](image/web_app.png) |![Server console](image/console_server.png)|
   |:-:|:-:|
   | **Web App** | **Server Device** |

---

## Report Bugs & Get Support ##

To report bugs in the Application Examples projects, please create a new "Issue" in the "Issues" section of [bluetooth_applications](https://github.com/SiliconLabs/bluetooth_applications) repo. Please reference the board, project, and source files associated with the bug, and reference line numbers. If you are proposing a fix, also include information on the proposed fix. Since these examples are provided as-is, there is no guarantee that these examples will be updated to fix these issues.

Questions and comments related to these examples should be made by creating a new "Issue" in the "Issues" section of [bluetooth_applications](https://github.com/SiliconLabs/bluetooth_applications) repo.

---
