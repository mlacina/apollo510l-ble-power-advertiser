# Third-Party Notices

This repository contains source code derived from or based on third-party
components, including AmbiqSuite SDK, Arm/Packetcraft Cordio and FreeRTOS.

Project-authored portions of the repository are licensed separately under the
repository [LICENSE](LICENSE).

## Ambiq Micro, Inc. - AmbiqSuite SDK 5.2.0

Source package:

- AmbiqSuite SDK 5.2.0
- Official source/download page: https://contentportal.ambiq.com

Included Ambiq-derived files:

```text
src/main.c
src/ble_task.c
```

### Project-specific modifications

`main.c` is derived from the `ble_freertos_fit_lp` example and related FreeRTOS
application support code from AmbiqSuite SDK 5.2.0. It has been substantially
modified for the Apollo510L BLE Power Advertiser project, including low-power
memory configuration, BLE radio IPC setup, configurable sleep behavior, cache
and retention configuration, and project-specific FreeRTOS task startup.

`ble_task.c` is derived from the BLE radio-task implementation provided with
AmbiqSuite SDK 5.2.0 and has been substantially modified for the Apollo510L BLE
Power Advertiser project. Project-specific changes include WSF buffer
configuration, Apollo510L radio integration, advertiser handler integration,
and application-specific task flow.

These Ambiq-derived files retain the original Ambiq copyright and BSD 3-Clause
license notices contained in the corresponding AmbiqSuite SDK source files.

The general Ambiq Software License Terms applicable to other SDK components are
not redistributed with this repository.

### Ambiq License Notice

Copyright (c) 2026, Ambiq Micro, Inc.
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice,
   this list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

3. Neither the name of the copyright holder nor the names of its contributors
   may be used to endorse or promote products derived from this software
   without specific prior written permission.

Third party software included in this distribution is subject to the
additional license terms as defined in the /docs/licenses directory.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
POSSIBILITY OF SUCH DAMAGE.

## Arm / Packetcraft Cordio

Portions of `src/advertiser.c`, including application-framework event callback,
message-dispatch and initialization patterns, are derived from Arm/Packetcraft
Cordio sample application code distributed with AmbiqSuite SDK 5.2.0.

The remaining project-specific portions of `src/advertiser.c` are licensed
under the repository BSD 3-Clause License.

Copyright (c) 2011-2019 Arm Ltd. All Rights Reserved.
Copyright (c) 2019 Packetcraft, Inc.

License: Apache License 2.0

The full Apache License 2.0 text is provided in:

```text
LICENSES/Apache-2.0.txt
```

## FreeRTOS

`FreeRTOSConfig.h` is based on the FreeRTOS configuration distributed with
AmbiqSuite SDK 5.2.0 and retains the original FreeRTOS MIT license notice.

The project-specific modifications to `FreeRTOSConfig.h` are also licensed
under the MIT License.

FreeRTOS V202212.00

Copyright (C) 2020 Amazon.com, Inc. or its affiliates.

License: MIT

## Project-authored Apollo510L linker script

`apollo510l_ses.ld` is a project-authored GNU ld linker script.

The Apollo510L memory map and baseline Cortex-M section organization were
informed by the linker configuration used in AmbiqSuite SDK 5.2.0. The script
contains substantial project-specific implementation and is not an
AmbiqSuite-derived source file.

The linker script is licensed under the repository BSD 3-Clause License.

## External Development Tools

The following tools are required or recommended for building and debugging the
project but are not distributed with this repository:

- AmbiqSuite SDK 5.2.0
- Arm GNU Toolchain
- SEGGER Embedded Studio
- SEGGER J-Link software

Each external tool is distributed under its own license terms.