# Third-Party Notices

This repository contains source code derived from or based on third-party
components, including AmbiqSuite SDK, Arm/Packetcraft Cordio and FreeRTOS.

Project-authored portions of the repository are licensed separately under the
repository [LICENSE](LICENSE).

## Ambiq Micro, Inc. - AmbiqSuite SDK 5.2.0

Source package:

- AmbiqSuite SDK 5.2.0
- Official source/download page: https://contentportal.ambiq.com

Included Ambiq-originated or Ambiq-derived files:

```text
src/main.c
src/ble_task.c
src/ble_task.h
portions of src/advertiser.c
```

### Project-specific modifications

`main.c` is based on the `ble_freertos_fit_lp` example and related RTOS
support code from AmbiqSuite SDK 5.2.0.

`ble_task.c` and `ble_task.h` are based on the radio task implementation
and interface from AmbiqSuite SDK 5.2.0.

Portions of `advertiser.c` related to Extended Advertising, Periodic
Advertising and connectionless CTE are based on AmbiqSuite SDK 5.2.0
example code.

The Ambiq-originated files remain subject to the applicable Ambiq copyright
and license terms. Original notices present in those files must be retained.

Ambiq-originated source files that contain the Ambiq BSD 3-Clause notice
remain subject to those permissive terms, reproduced below.

The general Ambiq Software License Terms applicable to other SDK components
are not redistributed with this repository.

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

Portions of `advertiser.c` are derived from Arm/Packetcraft Cordio sample
application code.

Copyright (c) 2011-2019 Arm Ltd. All Rights Reserved.
Copyright (c) 2019 Packetcraft, Inc.

License: Apache License 2.0
The full license text is provided in `LICENSES/Apache-2.0.txt`.

## FreeRTOS

`FreeRTOSConfig.h` is based on the FreeRTOS configuration supplied with
AmbiqSuite SDK 5.2.0 and retains the original FreeRTOS license notice.

FreeRTOS V202212.00
Copyright (C) 2020 Amazon.com, Inc. or its affiliates.

License: MIT

## External Development Tools

The following tools are required or recommended for building and debugging the
project but are not distributed with this repository:

- AmbiqSuite SDK 5.2.0
- Arm GNU Toolchain
- SEGGER Embedded Studio
- SEGGER J-Link software

Each external tool is distributed under its own license terms.
