/*
 * BLE task and Cordio/WSF stack initialization for the
 * Apollo510L BLE Power Advertiser.
 *
 * Project-specific modifications and new code:
 * Copyright (c) 2026 Mariusz Łacina
 *
 * Project-specific modifications are licensed under the BSD 3-Clause License.
 * See LICENSE for the applicable terms.
 *
 * This file is based on the radio task implementation from
 * AmbiqSuite SDK 5.2.0, licensed under the Ambiq BSD 3-Clause license.
 *
 * See THIRD_PARTY_NOTICES.md for provenance and applicable license terms.
 */

//*****************************************************************************
//
// Copyright (c) 2026, Ambiq Micro, Inc.
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice,
// this list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright
// notice, this list of conditions and the following disclaimer in the
// documentation and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its
// contributors may be used to endorse or promote products derived from this
// software without specific prior written permission.
//
// Third party software included in this distribution is subject to the
// additional license terms as defined in the /docs/licenses directory.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
//
// This is part of revision release_sdk5p2p0-66487dd10 of the AmbiqSuite Development Package.
//
//*****************************************************************************

#include <stdint.h>

#include "am_util.h"
#include "FreeRTOS.h"
#include "task.h"

#include "wsf_types.h"
#include "wsf_buf.h"
#include "wsf_timer.h"
#include "wsf_os.h"
#include "hci_handler.h"
#include "hci_core.h"
#include "hci_drv.h"
#include "hci_drv_apollo.h"
#include "hci_drv_510L_radio.h"
#include "dm_handler.h"
#include "l2c_handler.h"
#include "l2c_api.h"
#include "att_handler.h"
#include "att_api.h"
#include "smp_handler.h"
#include "smp_api.h"
#include "app_api.h"
#include "advertiser.h"
#include "ble_task.h"

/* -------------------------------------------------------------------------- */
/* WSF buffer pools                                                           */
/* -------------------------------------------------------------------------- */
#define BLE_WSF_POOL_COUNT       4U
#define BLE_WSF_POOL_16_SIZE     16U
#define BLE_WSF_POOL_16_COUNT    8U
#define BLE_WSF_POOL_32_SIZE     32U
#define BLE_WSF_POOL_32_COUNT    4U
#define BLE_WSF_POOL_64_SIZE     64U
#define BLE_WSF_POOL_64_COUNT    6U
#define BLE_WSF_POOL_280_SIZE    280U
#define BLE_WSF_POOL_280_COUNT   14U


static uint32_t ble_wsf_memory[
    (
        BLE_WSF_POOL_COUNT * 16U +
        BLE_WSF_POOL_16_SIZE  * BLE_WSF_POOL_16_COUNT +
        BLE_WSF_POOL_32_SIZE  * BLE_WSF_POOL_32_COUNT +
        BLE_WSF_POOL_64_SIZE  * BLE_WSF_POOL_64_COUNT +
        BLE_WSF_POOL_280_SIZE * BLE_WSF_POOL_280_COUNT
    ) / sizeof(uint32_t)
];


static const wsfBufPoolDesc_t ble_wsf_pools[BLE_WSF_POOL_COUNT] =
{
    { BLE_WSF_POOL_16_SIZE,  BLE_WSF_POOL_16_COUNT  },
    { BLE_WSF_POOL_32_SIZE,  BLE_WSF_POOL_32_COUNT  },
    { BLE_WSF_POOL_64_SIZE,  BLE_WSF_POOL_64_COUNT  },
    { BLE_WSF_POOL_280_SIZE, BLE_WSF_POOL_280_COUNT }
};


/* -------------------------------------------------------------------------- */
/* Cordio/WSF stack initialization                                            */
/* -------------------------------------------------------------------------- */
static void ble_stack_init(void)
{
    wsfHandlerId_t handler_id;
    uint16_t required_memory;

    WsfOsInit();
    WsfTimerInit();

    required_memory = WsfBufInit(
        sizeof(ble_wsf_memory),
        (uint8_t *)ble_wsf_memory,
        BLE_WSF_POOL_COUNT,
        (wsfBufPoolDesc_t *)ble_wsf_pools);

    if (required_memory > sizeof(ble_wsf_memory))
    {
        am_util_debug_printf(
            "BLE WSF buffer memory short by %u bytes\r\n",
            required_memory - sizeof(ble_wsf_memory));

        configASSERT(0);
    }

    SecInit();
    SecAesInit();
    SecCmacInit();
    SecEccInit();

    handler_id = WsfOsSetNextHandler(HciHandler);
    HciHandlerInit(handler_id);

    handler_id = WsfOsSetNextHandler(DmHandler);

    DmDevVsInit(0);
    DmExtAdvInit();
    DmPhyInit();
    DmConnInit();
    DmExtConnSlaveInit();
    DmSecInit();
    DmSecLescInit();
    DmPrivInit();

    DmHandlerInit(handler_id);

    handler_id = WsfOsSetNextHandler(L2cSlaveHandler);

    L2cSlaveHandlerInit(handler_id);
    L2cInit();
    L2cSlaveInit();

    handler_id = WsfOsSetNextHandler(AttHandler);

    AttHandlerInit(handler_id);
    AttsInit();
    AttsIndInit();
    AttcInit();

    handler_id = WsfOsSetNextHandler(SmpHandler);

    SmpHandlerInit(handler_id);
    SmprInit();
    SmprScInit();

    HciSetMaxRxAclLen(251);

    handler_id = WsfOsSetNextHandler(AppHandler);
    AppHandlerInit(handler_id);

    handler_id = WsfOsSetNextHandler(advertiser_handler);
    advertiser_handler_init(handler_id);

    handler_id = WsfOsSetNextHandler(HciDrvHandler);
    HciDrvHandlerInit(handler_id);
}


void ble_task(void *context)
{
    (void)context;

    HciDrvRadioBoot(1);

    ble_stack_init();

    advertiser_start();

    for (;;)
    {
        wsfOsDispatcher();
    }
}