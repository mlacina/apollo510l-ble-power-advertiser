# Apollo510 Lite BLE Power Advertiser

This repository provides a configurable Bluetooth Low Energy advertising reference project for the **Ambiq Apollo510 Lite platform**.

The application is intended as a clean and repeatable baseline for evaluating BLE advertising behavior on the current target hardware. A single public configuration object controls the advertising mode, interval, TX power, PHY, channel map, payload sizes, connection parameters, Periodic Advertising, connectionless CTE, and low-power mode.

The implementation uses the **AmbiqSuite Cordio BLE stack** and supports Legacy Advertising, Extended Advertising, Periodic Advertising, and connectionless Constant Tone Extension (CTE) experiments for AoA/AoD development.

> **Independent project:** This repository is an independent technical example. It is not an official Ambiq Micro or Bluetooth SIG software release and is not endorsed or maintained by either organization.

The main application flow is:

```text
FreeRTOS BLE task
        |
        v
Apollo510L radio-core boot
        |
        v
WSF / Cordio stack initialization
        |
        v
Advertiser configuration validation
        |
        +---- invalid ----> report CONFIG ERROR and do not start the radio
        |
        v
Build Cordio advertising configuration
        |
        v
Device Manager reset and GATT database hash
        |
        v
Legacy or Extended Advertising
        |
        +---- optional Periodic Advertising
        |
        `---- optional connectionless CTE
```

The project does not provide universal power-consumption figures. Power measurements must be performed on the current hardware, with the intended firmware, radio configuration, instrumentation, and test conditions.

------

## Purpose

The project is designed to provide:

- one readable, application-level BLE advertiser configuration;
- repeatable comparison of Legacy and Extended Advertising behavior;
- configurable LE 1M, LE 2M, and LE Coded PHY operation where supported by the selected advertising mode;
- Periodic Advertising and connectionless CTE experiments;
- a strict configuration-validation layer that rejects contradictory settings;
- a low-noise firmware baseline for measurements on the target hardware;
- a starting point for product-specific advertising payloads and direction-finding work.

It is a measurement and development baseline, not a Bluetooth qualification package or a complete product application.

------

## Features

- **Apollo510 Lite** BLE target
- FreeRTOS task with WSF/Cordio event dispatch
- Legacy and Extended Advertising
- Configurable advertising interval in milliseconds
- Configurable TX power from the application-level enum
- Primary PHY selection: LE 1M or LE Coded
- Secondary PHY selection: LE 1M, LE 2M, or LE Coded
- Explicit LE Coded S=2 and S=8 preferences
- Independent enable flags for primary advertising channels 37, 38, and 39
- Connectable, scannable, and non-connectable/non-scannable operation
- Configurable advertising-data and scan-response lengths
- Optional connection parameter configuration
- Optional Periodic Advertising
- Optional connectionless CTE for AoA, AoD 1 us, and AoD 2 us modes
- Deep and deeper-sleep configuration choices
- Strict validation before Cordio configuration and radio startup
- SEGGER Embedded Studio, Arm GNU Toolchain, and J-Link workflow
- Portable external dependency paths through SES Global Macros

------

## Hardware

The firmware targets the current Apollo510 Lite development hardware used by the project (**AP510DLEVB**). An on-board J-Link interface can be used for programming and debugging.

For measurement work, use suitable current-measurement equipment and document at least:

- exact board and board revision;
- power-supply voltage and connection point;
- debugger connection state;
- enabled trace and logging interfaces;
- firmware revision and build configuration;
- complete `g_advertiser_config` contents;
- scanner, sniffer, or RF instrumentation used for verification.

------

## Software Requirements

The project context and the previous validated Apollo510 Lite SES baseline use:

```text
SEGGER Embedded Studio:  8.30a
Arm GNU Toolchain:       14.2.Rel1
AmbiqSuite SDK:          5.2.0
BLE host stack:          Cordio supplied with AmbiqSuite
```

Using different tool or SDK versions may require project-file, API, radio-image, or validation changes and should be reverified.

### Target and radio image

The application runs on the Apollo510 Lite application core and boots the Apollo510 Lite BLE radio core before initializing BLE stack. The verified diagnostic output identifies the BLE radio image as type `0x02` and version `1.5.0`.

For the Apollo510 Lite EVB-based setup used by the this project, the J-Link target is:

```text
AP510DLA-CBR
```

------

# Project Portability

The SES project should not depend on machine-specific absolute paths for the AmbiqSuite SDK, Arm GNU Toolchain, or project-local source files.

The portable setup uses two SEGGER Embedded Studio **Global Macros**:

```text
AMBIQSUITE_ROOT
ARM_GCC_ROOT
```

Project-local paths should remain relative. External SDK and toolchain paths should be expressed through the macros.

------

## SES Global Macros

Open:

```text
Tools
-> Options
-> Building
-> Global Macros
```

Add:

```text
AMBIQSUITE_ROOT=<path-to-AmbiqSuite>
ARM_GCC_ROOT=<path-to-Arm-GNU-Toolchain>
```

Example:

```text
AMBIQSUITE_ROOT=D:/Ambiq/SDK/AmbiqSuite_5.2.0
ARM_GCC_ROOT=C:/Program Files (x86)/Arm GNU Toolchain arm-none-eabi/14.2 rel1
```

Do not add spaces around `=`. For example, use:

```text
AMBIQSUITE_ROOT=D:/Ambiq/SDK/AmbiqSuite_5.2.0
```

The SDK root should contain directories such as `CMSIS`, `boards`, `devices`, `mcu`, `third_party`, and `utils`. The external compiler directory is normally referenced as:

```text
$(ARM_GCC_ROOT)/bin
```

------

# Building

After configuring the SES Global Macros:

1. Open the project's `.emProject` file in SEGGER Embedded Studio.
2. Verify that the correct Apollo510 Lite target and J-Link device are selected.
3. Run **Build -> Rebuild**.
4. Confirm that the expected BLE radio image is available to the application.

No source or project path changes should normally be required when the project retains the same portable layout as the parent Apollo510 Lite project.

------

# Programming and Debugging

Connect the target using J-Link, select the exact target device, and start a debug session. A normal startup should proceed through:

```text
J-Link programming
        |
        v
application startup
        |
        v
FreeRTOS BLE task
        |
        v
HciDrvRadioBoot(1)
        |
        v
WSF / Cordio initialization
        |
        v
advertiser_handler_init()
        |
        v
advertiser_start()
        |
        v
DmDevReset()
```

Trace output is useful during functional validation, but SWO, UART logging, breakpoints, and an attached debugger can affect low-power behavior. Disable unnecessary diagnostics and disconnect measurement-disturbing interfaces before collecting final hardware results.

------

# Quick Start

1. Install AmbiqSuite SDK 5.2.0, Arm GNU Toolchain 14.2.Rel1, and SEGGER Embedded Studio.
2. Configure `AMBIQSUITE_ROOT` and `ARM_GCC_ROOT` in SES.
3. Open the `.emProject` file and verify the J-Link target.
4. Edit `g_advertiser_config` in `advertiser.c`.
5. Rebuild and program the board.
6. Check the startup trace for configuration errors and successful radio-core boot.
7. Verify the over-the-air behavior with a BLE scanner, sniffer, or suitable RF instrument.
8. Disable diagnostic interfaces as appropriate, then perform measurements on the current hardware.

------

# Advertiser Configuration

The public configuration is a single constant object:

```c
const advertiser_config_t g_advertiser_config =
{
    /* application-selected settings */
};
```

The configuration uses human-readable millisecond and microsecond values. Helper functions convert these values into BLE/Cordio units before the controller is programmed.

## Configuration reference

| Field | Values / range | Meaning and rules |
|---|---|---|
| `mode` | `ADV_MODE_LEGACY`, `ADV_MODE_EXTENDED` | Selects Legacy or Extended advertising PDUs. Periodic Advertising requires Extended mode. |
| `advertising_interval_ms` | `20..10240` ms | Interval for the primary advertising set. Converted to 0.625 ms units. |
| `tx_power` | enum values from `-20` to `+13` dBm | Requested radio TX-power level. The actual supported/effective level remains controller- and board-dependent. |
| `primary_phy` | `ADV_PRIMARY_PHY_1M`, `ADV_PRIMARY_PHY_CODED` | PHY used for the primary Extended Advertising transmission. Legacy Advertising requires LE 1M. |
| `secondary_phy` | `ADV_SECONDARY_PHY_1M`, `ADV_SECONDARY_PHY_2M`, `ADV_SECONDARY_PHY_CODED` | PHY for secondary-channel Extended Advertising packets. Not used to change Legacy primary advertising. |
| `primary_coded_scheme` | `ADV_CODED_S2`, `ADV_CODED_S8` | Coded PHY preference for the primary PHY. Relevant only when `primary_phy` is Coded. |
| `secondary_coded_scheme` | `ADV_CODED_S2`, `ADV_CODED_S8` | Coded PHY preference for the secondary PHY. Relevant only when `secondary_phy` is Coded. |
| `channel_37` | `true` / `false` | Enables primary advertising channel 37. |
| `channel_38` | `true` / `false` | Enables primary advertising channel 38. |
| `channel_39` | `true` / `false` | Enables primary advertising channel 39. At least one primary channel must be enabled. |
| `connectable` | `true` / `false` | Allows a scanner/central to initiate a connection where the selected advertising mode permits it. |
| `scannable` | `true` / `false` | Allows scan requests and scan responses where the selected mode permits it. |
| `advertising_data_length` | application buffer; mode/controller limits apply | Requested advertising-data length. Legacy mode is limited to 31 bytes. The runtime controller capability is authoritative for Extended Advertising. |
| `scan_response_length` | buffer up to 31 bytes | Requested scan-response length. It is used only when `scannable == true`; Legacy scan response is limited to 31 bytes. |
| `connection` | nested structure | Used and validated only when `connectable == true`. |
| `periodic_interval_ms` | `0`, or `8..81918` ms | `0` disables Periodic Advertising. A nonzero value enables it and requires Extended, non-connectable, non-scannable operation. Converted to 1.25 ms units. |
| `cte` | nested structure | Optional connectionless CTE. Requires enabled Periodic Advertising. |
| `sleep_mode` | `ADV_SLEEP_DEEP`, `ADV_SLEEP_DEEPER` | Selects the intended platform low-power policy between BLE events. The application idle path must actually consume this setting for it to affect hardware behavior. |

## TX power

The application-level TX-power enum exposes:

```text
-20, -18, -16, -14, -12, -10, -8, -6, -4, -2 dBm
  0 dBm
 +1 through +13 dBm
```

The value is passed to the Apollo controller through the vendor-specific TX-power command. Treat it as a requested setting; board losses, antenna implementation, calibration, controller firmware, regulatory limits, and device capability determine the actual radiated result.

## LE Coded S=2 and S=8

When a selected PHY is `*_CODED`, the corresponding coded-scheme field selects:

- `ADV_CODED_S2`: S=2 coding preference, nominal LE Coded 500 kbit/s;
- `ADV_CODED_S8`: S=8 coding preference, nominal LE Coded 125 kbit/s and greater coding redundancy.

The coded-scheme fields have no effect when their associated PHY is LE 1M or LE 2M. LE 2M is available only as a secondary PHY in this configuration interface.

## Advertising channels

`channel_37`, `channel_38`, and `channel_39` form the primary advertising channel map. Any one-, two-, or three-channel combination is accepted, but disabling all three channels is rejected.

These flags select primary advertising channels only. Extended and Periodic Advertising may also use secondary/data channels selected by the controller; the primary channel map does not force Periodic Advertising onto channels 37, 38, or 39.

## Payload and scan response

The current data builder creates deterministic manufacturer-specific test data. Requested lengths below three bytes are raised to the minimum representation used by the builder.

Important limits:

- Legacy advertising data: maximum 31 bytes;
- Legacy scan response: maximum 31 bytes;
- current scan-response buffer: 31 bytes;
- actual Extended Advertising maximum: reported/enforced by the controller at runtime.

Do not document or depend on a fixed Apollo510 Lite Extended Advertising maximum unless it has been queried and verified with the exact controller firmware in use. A local buffer being large enough does not prove that the controller accepts the same length.

When `scannable == false`, `scan_response_length` is inactive. For measurement comparisons, change only one payload parameter at a time and verify that the controller accepted the data.

------

# Legacy, Extended, and Periodic Advertising

## Legacy Advertising

Legacy mode uses legacy advertising PDUs and requires:

```c
.mode        = ADV_MODE_LEGACY,
.primary_phy = ADV_PRIMARY_PHY_1M,
```

Advertising data and scan response are each limited to 31 bytes. Legacy `ADV_IND` operation may be both connectable and scannable. Periodic Advertising and connectionless CTE are not available in Legacy mode.

## Extended Advertising

Extended mode enables primary/secondary PHY selection and larger advertising data subject to controller capability.

An Extended Advertising set must not be both connectable and scannable in this project:

```text
connectable=true,  scannable=false  -> valid
connectable=false, scannable=true   -> valid
connectable=false, scannable=false  -> valid
connectable=true,  scannable=true   -> rejected
```

The primary Extended Advertising PDU can point to an auxiliary packet on a secondary/data channel. The selected `secondary_phy` applies to that secondary-channel transmission.

## Periodic Advertising

Periodic Advertising is enabled by setting `periodic_interval_ms` to a nonzero value. It requires:

```c
.mode        = ADV_MODE_EXTENDED,
.connectable = false,
.scannable   = false,
```

The application-level periodic interval range is `8..81918` ms because the interface uses integer milliseconds. The BLE controller uses 1.25 ms units. For example:

```text
500 ms  -> 400 units
1000 ms -> 800 units
```

Set:

```c
.periodic_interval_ms = 0,
```

to disable Periodic Advertising.

Periodic packets use secondary/data channels selected by the controller. A short payload may appear as a single periodic transmission per event; larger data can require chained auxiliary packets.

------

# Connectable and Scannable Rules

The effective advertising type is selected from the two flags:

| `connectable` | `scannable` | Legacy | Extended | Periodic |
|---:|---:|---|---|---|
| `false` | `false` | Non-connectable, non-scannable | Valid | Required |
| `true` | `false` | Connectable | Valid | Invalid |
| `false` | `true` | Scannable | Valid | Invalid |
| `true` | `true` | Supported by legacy `ADV_IND` use | Rejected | Invalid |

Connection parameters are inactive when `connectable == false`. Scan-response data is inactive when `scannable == false`.

------

# Connection Parameters

The nested connection configuration is:

```c
.connection =
{
    .min_interval_ms         = 500,
    .max_interval_ms         = 500,
    .latency                 = 0,
    .supervision_timeout_ms  = 6000,
    .update_delay_ms         = 100,
    .update_attempts         = 5
},
```

It is used and validated only for connectable advertising.

| Field | Accepted value | Notes |
|---|---:|---|
| `min_interval_ms` | `8..4000` ms | Integer-millisecond application representation; converted to 1.25 ms BLE units. |
| `max_interval_ms` | `8..4000` ms | Must be greater than or equal to `min_interval_ms`. |
| `latency` | `0..499` events | Peripheral/slave latency. |
| `supervision_timeout_ms` | `100..32000` ms | Must be a multiple of 10 ms. |
| `update_delay_ms` | application policy | Delay before the automatic connection parameter update; zero disables it. |
| `update_attempts` | `uint8_t` policy value | Must be zero when `update_delay_ms == 0`. |

The validator also requires the strict BLE relationship:

```text
supervision_timeout
    > 2 x (1 + latency) x effective_max_connection_interval
```

The relationship is checked using the interval after conversion to BLE units. Equality is not sufficient.

------

# Sleep Modes

The public configuration exposes:

```c
ADV_SLEEP_DEEP
ADV_SLEEP_DEEPER
```

These values are intended to select the platform idle/sleep policy used between BLE events. `ADV_SLEEP_DEEPER` is the measurement-oriented baseline in the current configuration.

The enum alone does not guarantee a particular power state. Confirm that the FreeRTOS idle path or platform power-management layer reads `g_advertiser_config.sleep_mode` and enters the requested mode. Trace output, debugger activity, clocks, peripherals, board jumpers, and the selected power rail can all change the observed result.

------

# Connectionless CTE and AoA/AoD

The nested CTE configuration is:

```c
.cte =
{
    .enabled   = true,
    .length_us = 80,
    .count     = 1,
    .type      = ADV_CTE_AOA
},
```

Available types are:

```text
ADV_CTE_AOA      -> AoA CTE
ADV_CTE_AOD_1US  -> AoD with 1 us switching slots
ADV_CTE_AOD_2US  -> AoD with 2 us switching slots
```

Validation rules:

- CTE requires enabled Periodic Advertising;
- `length_us` must be `16..160` us in 8 us steps;
- `count` must be `1..16`;
- `type` must be one of the three values above.

The user-facing CTE length is converted to HCI units by dividing by 8. CTE configuration and enable commands are issued after the advertising set has started.

AoA and AoD names describe the direction-finding method, but successful CTE transmission does not by itself validate antenna switching, antenna geometry, receiver IQ sampling, or angle estimation. AoD modes require a real and correctly integrated antenna switching pattern. The application must provide antenna IDs that are valid for the target RF design and controller implementation.

------

# Configuration Validation

The project uses strict validation: invalid settings are reported and advertising does not start. Invalid values are not silently repaired as normal operating behavior. Conversion helpers may retain defensive clamping, but a correctly integrated startup path calls the validator first.

Recommended placement:

```text
advertiser_handler_init()
        |
        v
advertiser_validate_config(&g_advertiser_config)
        |
        +---- false ----> print CONFIG ERROR, mark invalid, return
        |
        v
build Cordio configuration
        |
        v
advertiser_start()
        |
        +---- invalid ----> return without DmDevReset()
        |
        v
radio startup
```

The validator checks:

- advertising interval range;
- at least one enabled primary advertising channel;
- Legacy PHY and payload limits;
- Extended connectable/scannable exclusivity;
- Periodic mode, interval, connectability, and scannability dependencies;
- CTE dependency, length, count, and type;
- connection interval ranges and ordering;
- connection latency;
- supervision-timeout range, granularity, and relationship to interval/latency;
- connection-update policy consistency.

When `connectable == false`, connection fields are deliberately ignored. When CTE is disabled, CTE detail fields are inactive. This allows one shared configuration structure without forcing unused settings to be meaningful.

For validation testing, start from one known-good baseline and change only one field per test. Check both the Boolean result and the expected `CONFIG ERROR`; also verify that `DmDevReset()` is not reached after rejection.

------

# Usage Examples

## Current known-good Extended + Periodic + AoA baseline

```c
const advertiser_config_t g_advertiser_config =
{
    .mode = ADV_MODE_EXTENDED,

    .advertising_interval_ms = 200,
    .tx_power = ADV_TX_POWER_0_DBM,

    .primary_phy   = ADV_PRIMARY_PHY_1M,
    .secondary_phy = ADV_SECONDARY_PHY_1M,

    .primary_coded_scheme   = ADV_CODED_S8,
    .secondary_coded_scheme = ADV_CODED_S8,

    .channel_37 = true,
    .channel_38 = true,
    .channel_39 = true,

    .connectable = false,
    .scannable   = false,

    .advertising_data_length = 31,
    .scan_response_length    = 20,

    .connection =
    {
        .min_interval_ms        = 500,
        .max_interval_ms        = 500,
        .latency                = 0,
        .supervision_timeout_ms = 6000,
        .update_delay_ms        = 100,
        .update_attempts        = 5
    },

    .cte =
    {
        .enabled   = true,
        .length_us = 80,
        .count     = 1,
        .type      = ADV_CTE_AOA
    },

    .periodic_interval_ms = 500,
    .sleep_mode = ADV_SLEEP_DEEPER
};
```

This configuration is valid. Because it is non-connectable, the `connection` block is present but inactive. Because it is non-scannable, `scan_response_length` is also inactive. The coded-scheme settings are inactive while both selected PHYs are LE 1M.

## Legacy connectable and scannable advertising

```c
.mode = ADV_MODE_LEGACY,
.advertising_interval_ms = 200,
.primary_phy = ADV_PRIMARY_PHY_1M,
.connectable = true,
.scannable = true,
.advertising_data_length = 31,
.scan_response_length = 20,
.periodic_interval_ms = 0,
.cte.enabled = false,
```

Use a valid `connection` block because this example is connectable.

## Extended scannable advertising on LE 2M secondary PHY

```c
.mode = ADV_MODE_EXTENDED,
.primary_phy = ADV_PRIMARY_PHY_1M,
.secondary_phy = ADV_SECONDARY_PHY_2M,
.connectable = false,
.scannable = true,
.periodic_interval_ms = 0,
.cte.enabled = false,
```

## Extended non-connectable advertising on LE Coded PHY

```c
.mode = ADV_MODE_EXTENDED,
.primary_phy = ADV_PRIMARY_PHY_CODED,
.secondary_phy = ADV_SECONDARY_PHY_CODED,
.primary_coded_scheme = ADV_CODED_S8,
.secondary_coded_scheme = ADV_CODED_S8,
.connectable = false,
.scannable = false,
.periodic_interval_ms = 0,
.cte.enabled = false,
```

Use S=2 instead when the experiment requires the faster LE Coded preference.

------

# Current Verified Status

The following project behavior has been verified in the development work represented by this baseline:

```text
[PASS] Project compilation with the established SES/GCC setup
[PASS] Apollo510L BLE radio-core boot and Cordio initialization
[PASS] Extended Advertising startup
[PASS] Non-connectable advertising visible to a BLE scanner
[PASS] Configured primary advertising interval behavior
[PASS] Periodic Advertising startup event
[PASS] Periodic Advertising visible as non-connectable
[PASS] Periodic interval behavior at test settings
[PASS] Connectionless CTE enable/disable changes periodic RF-event duration
[PASS] CTE duration scales with configured 40, 80, and 160 us lengths
[PASS] Current baseline accepted by the defined configuration rules
```

These checks confirm the advertiser flow, Periodic Advertising operation, and generation of a CTE-like extension with the expected duration scaling. They do **not** by themselves prove correct IQ samples, antenna switching, AoA/AoD accuracy, RF output power, Bluetooth qualification, or final product power consumption.

------

# Known Limitations and Notes

- The actual maximum Extended Advertising data length is controller-dependent and should be queried or observed at runtime.
- The scan-response buffer is currently limited to 31 bytes.
- Payload builders generate deterministic test content, not product-ready advertising structures.
- Integer-millisecond configuration cannot represent every BLE interval exactly; values are converted to controller units.
- The application-level minimum connection interval is 8 ms because the public interface uses whole milliseconds, although the BLE unit can represent 7.5 ms.
- Periodic Advertising and connectionless CTE require Extended, non-connectable, non-scannable operation in this project.
- CTE duration observations do not validate antenna switching or angle calculation.
- AoD operation requires hardware-specific antenna control and a valid switching pattern.
- Requested TX power must be verified on the target hardware if absolute RF output matters.
- Debuggers, logging, trace, unused peripherals, clocks, and board-level circuitry can alter measurement behavior.
- No quantitative current-consumption result should be treated as portable across boards, firmware revisions, radio images, tool settings, or laboratory setups.

------

# Measurement Guidance

Before comparing advertiser configurations:

1. Use the current target board and record its revision.
2. Rebuild from a known source revision.
3. Record the complete advertiser configuration.
4. Confirm the selected mode and interval over the air.
5. Confirm Periodic Advertising and CTE separately when enabled.
6. Remove or disable unnecessary trace and debug activity.
7. Keep supply, cabling, instrument settings, payload, and environment constant.
8. Change one advertiser parameter at a time.

This README intentionally contains no current-consumption results or cross-board current comparisons.

------

# Development Strategy

Validate the project in layers:

```text
SES / build system
        |
        v
startup and BLE radio image
        |
        v
WSF / Cordio initialization
        |
        v
configuration validation
        |
        v
Legacy or Extended Advertising
        |
        v
Periodic Advertising
        |
        v
connectionless CTE
        |
        v
RF and hardware measurements
```

Keep a known-good baseline and introduce one configuration change at a time. A BLE scanner confirms discoverability and basic advertising properties; use a sniffer or suitable RF equipment for protocol details that a general-purpose scanner does not expose.

------

# License and Third-Party Components

Project-authored source code is licensed under the BSD 3-Clause License.
See [LICENSE](LICENSE) for the authoritative terms.

The project uses Ambiq-originated software, including AmbiqSuite components,
the Cordio BLE stack integration, Apollo510 startup/runtime support, and the
Apollo510L radio-core image. These components retain their original copyright
and license terms and are not relicensed by this project.

See [THIRD_PARTY_NOTICES.md](THIRD_PARTY_NOTICES.md) for third-party components,
provenance information, and applicable license notices.

The AmbiqSuite SDK is not distributed with this repository. Obtain the SDK and
any required radio image through the appropriate Ambiq distribution channel and
configure `AMBIQSUITE_ROOT` to point to the local installation.

Bluetooth product names and trademarks are the property of Bluetooth SIG, Inc.
Ambiq and Apollo product names are trademarks of Ambiq Micro, Inc. Other product
names and trademarks are the property of their respective owners.