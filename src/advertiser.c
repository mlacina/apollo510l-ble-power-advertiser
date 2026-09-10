/*
 * BLE Power Advertiser application.
 *
 * Project-specific modifications and new code:
 * Copyright (c) 2026 Mariusz Łacina
 *
 * Project-specific modifications are licensed under the BSD 3-Clause License.
 * See LICENSE for the applicable terms.
 *
 * This file contains code derived from or based on:
 *
 * - Arm/Packetcraft Cordio sample application code,
 *   licensed under the Apache License 2.0.
 *
 * - AmbiqSuite SDK 5.2.0 example code,
 *   licensed under the Ambiq BSD 3-Clause license.
 *
 * See THIRD_PARTY_NOTICES.md for provenance and applicable license terms.
 */

 /*
 *  Copyright (c) 2011-2019 Arm Ltd. All Rights Reserved.
 *
 *  Copyright (c) 2019 Packetcraft, Inc.
 *  
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *  
 *      http://www.apache.org/licenses/LICENSE-2.0
 *  
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
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
#include <stdbool.h>
#include <string.h>

#include "wsf_types.h"
#include "wsf_msg.h"
#include "hci_api.h"
#include "dm_api.h"
#include "att_api.h"
#include "svc_core.h"
#include "gatt/gatt_api.h"
#include "atts_main.h"
#include "adv_ext_api.h"
#include "wsf_trace.h"
#include "app_api.h"
#include "hci_drv_510L_radio.h"
#include "advertiser.h"


/* -------------------------------------------------------------------------- */
/* Configuration                                                              */
/* -------------------------------------------------------------------------- */

const advertiser_config_t g_advertiser_config =
{
    .mode = ADV_MODE_LEGACY, 

    .advertising_interval_ms = 500,

    .tx_power = ADV_TX_POWER_0_DBM,

    .primary_phy   = ADV_PRIMARY_PHY_1M, 
    .secondary_phy = ADV_SECONDARY_PHY_1M , 

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
        .min_interval_ms       = 500,
        .max_interval_ms       = 500,
        .latency               = 0,
        .supervision_timeout_ms = 6000,

        .update_delay_ms       = 100,
        .update_attempts       = 5
     },

    .cte =
    {
        .enabled   = false,
        .length_us = 80,
        .count     = 1,
        .type      = ADV_CTE_AOD_2US
    },

    .periodic_interval_ms = 0,

    .sleep_mode = ADV_SLEEP_DEEPER
};


/* -------------------------------------------------------------------------- */
/* Cordio application configuration                                           */
/* -------------------------------------------------------------------------- */

#define ADVERTISER_CONN_MAX    1U

static appExtAdvCfg_t advertiser_ext_adv_cfg =
{
    {0},       /* advertising duration */
    {0},       /* advertising interval */
    {0},       /* maximum advertising events */
    {FALSE},   /* use legacy PDU */
    {0}        /* periodic advertising interval */
};

static const appSlaveCfg_t advertiser_slave_cfg =
{
    ADVERTISER_CONN_MAX
};

static uint8_t advertiser_handle = 0;


/* -------------------------------------------------------------------------- */
/* Advertising data                                                           */
/* -------------------------------------------------------------------------- */

/*
 * Apollo510L controller currently reports a smaller maximum advertising
 * data length than this buffer allows. Cordio will enforce the controller
 * capability when the advertising data is programmed.
 */
static uint8_t advertising_data[HCI_EXT_ADV_DATA_LEN];

static uint8_t scan_response_data[31];


/* -------------------------------------------------------------------------- */
/* WSF                                                                        */
/* -------------------------------------------------------------------------- */
static wsfHandlerId_t advertiser_handler_id;

/* -------------------------------------------------------------------------- */
/* CTE                                                                        */
/* -------------------------------------------------------------------------- */
/*
 * AmbiqSuite SDK 5.2.0 implements the connectionless CTE HCI commands
 * in the Cordio HCI layer, but does not expose their declarations through
 * the public hci_api.h header used by this project.
 */
extern void HciLeSetConnectionlessCteTxParamsCmd(
    uint8_t advHandle,
    uint8_t cteLen,
    uint8_t cteCnt,
    uint8_t cteType,
    uint8_t switchPatternLen,
    uint8_t *pAntennaIDs);

extern void HciLeConnectionlessCteTxEnableCmd(
    uint8_t advHandle,
    uint8_t enable);


/* -------------------------------------------------------------------------- */
/* PERIODIC ADVERTISING                                                       */
/* -------------------------------------------------------------------------- */
#define ADVERTISER_PERIODIC_DATA_SIZE       32U

static uint8_t periodic_advertising_data[ADVERTISER_PERIODIC_DATA_SIZE];

static const uint8_t periodic_adv_name[] =
{
    'P', 'e', 'r', 'i', 'o', 'd', 'i', 'c', ' ', 'A', 'd', 'v'
};


/* -------------------------------------------------------------------------- */
/* Helpers                                                                    */
/* -------------------------------------------------------------------------- */

static uint16_t advertiser_conn_interval_to_units(uint32_t interval_ms)
{
    return (uint16_t)((interval_ms * 4U + 2U) / 5U);
}


static uint16_t advertiser_supervision_timeout_to_units(uint32_t timeout_ms)
{
    /*
     * BLE supervision timeout unit = 10 ms.
     */
    return (uint16_t)(timeout_ms / 10U);
}

static uint16_t advertiser_interval_to_units(uint32_t interval_ms)
{
    if (interval_ms < 20U)
    {
        interval_ms = 20U;
    }

    if (interval_ms > 10240U)
    {
        interval_ms = 10240U;
    }

    return (uint16_t)((interval_ms * 1000U) / 625U);
}


static uint16_t advertiser_periodic_interval_to_units(uint32_t interval_ms)
{
    if (interval_ms == 0U)
    {
        return 0U;
    }

    if (interval_ms < 8U)
    {
        interval_ms = 8U;
    }

    if (interval_ms > 81918U)
    {
        interval_ms = 81918U;
    }

    return (uint16_t)((interval_ms * 4U + 2U) / 5U);
}


static uint8_t advertiser_build_channel_map(void)
{
    uint8_t channel_map = 0U;

    if (g_advertiser_config.channel_37)
    {
        channel_map |= DM_ADV_CHAN_37;
    }

    if (g_advertiser_config.channel_38)
    {
        channel_map |= DM_ADV_CHAN_38;
    }

    if (g_advertiser_config.channel_39)
    {
        channel_map |= DM_ADV_CHAN_39;
    }

    return channel_map;
}


static uint8_t advertiser_get_type(void)
{
    if (g_advertiser_config.connectable)
    {
        return DM_ADV_CONN_UNDIRECT;
    }

    if (g_advertiser_config.scannable)
    {
        return DM_ADV_SCAN_UNDIRECT;
    }

    return DM_ADV_NONCONN_UNDIRECT;
}


static uint8_t advertiser_get_primary_phy(void)
{
    switch (g_advertiser_config.primary_phy)
    {
        case ADV_PRIMARY_PHY_CODED:
            return HCI_ADV_PHY_LE_CODED;

        case ADV_PRIMARY_PHY_1M:
        default:
            return HCI_ADV_PHY_LE_1M;
    }
}


static uint8_t advertiser_get_secondary_phy(void)
{
    switch (g_advertiser_config.secondary_phy)
    {
        case ADV_SECONDARY_PHY_2M:
            return HCI_ADV_PHY_LE_2M;

        case ADV_SECONDARY_PHY_CODED:
            return HCI_ADV_PHY_LE_CODED;

        case ADV_SECONDARY_PHY_1M:
        default:
            return HCI_ADV_PHY_LE_1M;
    }
}


static uint8_t advertiser_get_primary_phy_option(void)
{
    if (g_advertiser_config.primary_phy != ADV_PRIMARY_PHY_CODED)
    {
        return HCI_PHY_OPTIONS_NONE;
    }

    if (g_advertiser_config.primary_coded_scheme == ADV_CODED_S2)
    {
        return HCI_PHY_OPTIONS_S2_REQUIRED;
    }

    return HCI_PHY_OPTIONS_S8_REQUIRED;
}


static uint8_t advertiser_get_secondary_phy_option(void)
{
    if (g_advertiser_config.secondary_phy != ADV_SECONDARY_PHY_CODED)
    {
        return HCI_PHY_OPTIONS_NONE;
    }

    if (g_advertiser_config.secondary_coded_scheme == ADV_CODED_S2)
    {
        return HCI_PHY_OPTIONS_S2_REQUIRED;
    }

    return HCI_PHY_OPTIONS_S8_REQUIRED;
}


static uint16_t advertiser_build_data(uint16_t requested_length)
{
    uint16_t i;

    if (requested_length < 3U)
    {
        requested_length = 3U;
    }

    if (requested_length > sizeof(advertising_data))
    {
        requested_length = sizeof(advertising_data);
    }

    advertising_data[0] = (uint8_t)(requested_length - 1U);
    advertising_data[1] = DM_ADV_TYPE_MANUFACTURER;

    for (i = 2U; i < requested_length; i++)
    {
        advertising_data[i] = (uint8_t)(i - 2U);
    }

    return requested_length;
}


static uint8_t advertiser_build_scan_response(uint16_t requested_length)
{
    uint8_t length;
    uint8_t i;

    if (requested_length < 3U)
    {
        requested_length = 3U;
    }

    if (requested_length > sizeof(scan_response_data))
    {
        requested_length = sizeof(scan_response_data);
    }

    length = (uint8_t)requested_length;

    scan_response_data[0] = length - 1U;
    scan_response_data[1] = DM_ADV_TYPE_MANUFACTURER;

    for (i = 2U; i < length; i++)
    {
        scan_response_data[i] = (uint8_t)(0x80U + i - 2U);
    }

    return length;
}

static uint8_t advertiser_cte_length_to_units(uint16_t length_us)
{
    return (uint8_t)(length_us / 8U);
}

static uint8_t advertiser_get_cte_type(void)
{
    switch (g_advertiser_config.cte.type)
    {
        case ADV_CTE_AOD_1US:
            return HCI_CTE_TYPE_REQ_AOD_1_US;

        case ADV_CTE_AOD_2US:
            return HCI_CTE_TYPE_REQ_AOD_2_US;

        case ADV_CTE_AOA:
        default:
            return HCI_CTE_TYPE_REQ_AOA;
    }
}

static uint8_t cte_antenna_ids[HCI_MIN_NUM_ANTENNA_IDS] = {0, 1};

static void advertiser_configure_cte(void)
{
    uint8_t cte_length;

    if (!g_advertiser_config.cte.enabled)
    {
        return;
    }

    if (g_advertiser_config.mode != ADV_MODE_EXTENDED)
    {
        return;
    }

    cte_length =
        advertiser_cte_length_to_units(
        g_advertiser_config.cte.length_us);

    HciLeSetConnectionlessCteTxParamsCmd(
        advertiser_handle,
        cte_length,
        g_advertiser_config.cte.count,
        advertiser_get_cte_type(),
        HCI_MIN_NUM_ANTENNA_IDS,
        cte_antenna_ids);

    HciLeConnectionlessCteTxEnableCmd(
        advertiser_handle,
        TRUE);
}

/* -------------------------------------------------------------------------- */
/* Advertising setup                                                          */
/* -------------------------------------------------------------------------- */
static void advertiser_configure(void)
{
    uint16_t advertising_length;
    uint8_t channel_map;
    bool use_legacy_pdu;

    advertising_length =
        advertiser_build_data(g_advertiser_config.advertising_data_length);

    AppExtAdvSetData(
        advertiser_handle,
        APP_ADV_DATA_DISCOVERABLE,
        advertising_length,
        advertising_data,
        sizeof(advertising_data));

    AppExtAdvSetData(
        advertiser_handle,
        APP_ADV_DATA_CONNECTABLE,
        advertising_length,
        advertising_data,
        sizeof(advertising_data));

    if (g_advertiser_config.scannable)
    {
        uint8_t scan_length =
            advertiser_build_scan_response(
            g_advertiser_config.scan_response_length);

        AppExtAdvSetData(
            advertiser_handle,
            APP_SCAN_DATA_DISCOVERABLE,
            scan_length,
            scan_response_data,
            sizeof(scan_response_data));

        AppExtAdvSetData(
            advertiser_handle,
            APP_SCAN_DATA_CONNECTABLE,
            scan_length,
            scan_response_data,
            sizeof(scan_response_data));
    }

    channel_map = advertiser_build_channel_map();

    if (channel_map == 0U)
    {
        return;
    }

    DmAdvSetChannelMap(advertiser_handle, channel_map);

    HciVscUpdateTxpwrLevel(
        (txPowerLevel_t)g_advertiser_config.tx_power);

    use_legacy_pdu =
        (g_advertiser_config.mode == ADV_MODE_LEGACY);

    DmAdvUseLegacyPdu(
        advertiser_handle,
        use_legacy_pdu);

    if (g_advertiser_config.mode == ADV_MODE_EXTENDED)
    {
        DmAdvSetPhyParamV2(
            advertiser_handle,
            advertiser_get_primary_phy(),
            0,
            advertiser_get_secondary_phy(),
            advertiser_get_primary_phy_option(),
            advertiser_get_secondary_phy_option());
    }

    AppExtSetAdvType(
        advertiser_handle,
        advertiser_get_type());

    AppSetBondable(FALSE);
}

static void advertiser_start_advertising(void)
{
    if (g_advertiser_config.connectable)
    {
        AppExtAdvStart(
            1,
            &advertiser_handle,
            APP_MODE_AUTO_INIT);
    }
    else
    {
        AppExtAdvStart(
            1,
            &advertiser_handle,
            APP_MODE_DISCOVERABLE);
    }
}

static void advertiser_start_periodic_advertising(void)
{
    memset(
        periodic_advertising_data,
        0,
        sizeof(periodic_advertising_data));

    AppPerAdvSetData(
        advertiser_handle,
        0,
        periodic_advertising_data,
        sizeof(periodic_advertising_data));

    AppPerAdvSetAdValue(
        advertiser_handle,
        DM_ADV_TYPE_LOCAL_NAME,
        sizeof(periodic_adv_name),
        (uint8_t *)periodic_adv_name);

    AppPerAdvStart(advertiser_handle);
}

/* -------------------------------------------------------------------------- */
/* DM callback                                                                */
/* -------------------------------------------------------------------------- */

static void advertiser_dm_callback(dmEvt_t *event)
{
    dmEvt_t *message;
    uint16_t length;

    length = DmSizeOfEvt(event);

    message = WsfMsgAlloc(length);

    if (message != NULL)
    {
        memcpy(message, event, length);

        WsfMsgSend(advertiser_handler_id,message);
    }
}

static void advertiser_att_callback(attEvt_t *event)
{
    attEvt_t *message;

    message = WsfMsgAlloc(sizeof(attEvt_t) + event->valueLen);

    if (message != NULL)
    {
        memcpy(message, event, sizeof(attEvt_t));

        message->pValue = (uint8_t *)(message + 1);

        if (event->valueLen != 0U)
        {
            memcpy(message->pValue,
                   event->pValue,
                   event->valueLen);
        }

        WsfMsgSend(advertiser_handler_id, message);
    }
}

/* -------------------------------------------------------------------------- */
/* WSF handler                                                                */
/* -------------------------------------------------------------------------- */

void advertiser_handler(wsfEventMask_t event_mask,wsfMsgHdr_t *message)
{
    (void)event_mask;

    if (message == NULL)
    {
        return;
    }

    APP_TRACE_INFO1("advertiser got evt %d", message->event);

    if ((message->event >= ATT_CBACK_START) &&
       (message->event <= ATT_CBACK_END))
    {
        AppServerProcAttMsg(message);
    }
    else if ((message->event >= DM_CBACK_START) &&
        (message->event <= DM_CBACK_END))
    {
        AppSlaveProcDmMsg((dmEvt_t *)message);
    }


    switch (message->event)
    {
        case DM_RESET_CMPL_IND:

            attsCsfSetHashUpdateStatus(TRUE);
            AttsCalculateDbHash();  
            break;

        case ATTS_DB_HASH_CALC_CMPL_IND:

            advertiser_configure();
            advertiser_start_advertising();
            if (g_advertiser_config.periodic_interval_ms != 0U)
            {
                advertiser_start_periodic_advertising();
            }
            break;

        case DM_CONN_CLOSE_IND:

            advertiser_start_advertising();
            break;

        case DM_ADV_SET_START_IND:

            if (g_advertiser_config.cte.enabled)
            {
                advertiser_configure_cte();
            }
            break;

        case DM_VENDOR_SPEC_CMD_CMPL_IND:
        case DM_VENDOR_SPEC_IND:

            break;

        default:
            break;
    }
}


/* -------------------------------------------------------------------------- */
/* Handler initialization                                                     */
/* -------------------------------------------------------------------------- */

static appUpdateCfg_t advertiser_update_cfg =
{
    0,      /* connection idle period - disable automatic update */
    24,     /* minimum connection interval */
    40,     /* maximum connection interval */
    0,      /* connection latency */
    400,    /* supervision timeout */
    0       /* update attempts */
};

static void advertiser_build_connection_config(void)
{
    advertiser_update_cfg.idlePeriod =
        g_advertiser_config.connection.update_delay_ms;

    advertiser_update_cfg.connIntervalMin =
        advertiser_conn_interval_to_units(
            g_advertiser_config.connection.min_interval_ms);

    advertiser_update_cfg.connIntervalMax =
        advertiser_conn_interval_to_units(
            g_advertiser_config.connection.max_interval_ms);

    advertiser_update_cfg.connLatency =
        g_advertiser_config.connection.latency;

    advertiser_update_cfg.supTimeout =
        advertiser_supervision_timeout_to_units(
            g_advertiser_config.connection.supervision_timeout_ms);

    advertiser_update_cfg.maxAttempts =
        g_advertiser_config.connection.update_attempts;
}


static bool advertiser_validate_config(const advertiser_config_t *cfg)
{
    /* Advertising interval */
    if ((cfg->advertising_interval_ms < 20U) ||
        (cfg->advertising_interval_ms > 10240U))
    {
        APP_TRACE_INFO0(
            "CONFIG ERROR: advertising interval must be 20..10240 ms");
        return false;
    }

    /* At least one primary advertising channel */
    if (!cfg->channel_37 &&
        !cfg->channel_38 &&
        !cfg->channel_39)
    {
        APP_TRACE_INFO0(
            "CONFIG ERROR: enable at least one advertising channel");
        return false;
    }

    if (cfg->advertising_data_length > sizeof(advertising_data))
    {
        APP_TRACE_INFO0(
            "CONFIG ERROR: advertising data length exceeds application buffer");
        return false;
    }

    if (cfg->scan_response_length > sizeof(scan_response_data))
    {
        APP_TRACE_INFO0(
            "CONFIG ERROR: scan response length exceeds application buffer");
        return false;
    }

    /* Legacy advertising */
    if (cfg->mode == ADV_MODE_LEGACY)
    {
        if (cfg->primary_phy != ADV_PRIMARY_PHY_1M)
        {
            APP_TRACE_INFO0(
                "CONFIG ERROR: Legacy advertising requires LE 1M PHY");
            return false;
        }

        if (cfg->advertising_data_length > 31U)
        {
            APP_TRACE_INFO0(
                "CONFIG ERROR: Legacy advertising data max is 31 bytes");
            return false;
        }

        if (cfg->scan_response_length > 31U)
        {
            APP_TRACE_INFO0(
                "CONFIG ERROR: Legacy scan response max is 31 bytes");
            return false;
        }
    }

    /* Extended advertising cannot be connectable and scannable */
    if ((cfg->mode == ADV_MODE_EXTENDED) &&
        cfg->connectable &&
        cfg->scannable)
    {
        APP_TRACE_INFO0(
            "CONFIG ERROR: Extended advertising cannot be both "
            "connectable and scannable");
        return false;
    }

    /* Periodic Advertising */
    if (cfg->periodic_interval_ms != 0U)
    {
        if (cfg->mode != ADV_MODE_EXTENDED)
        {
            APP_TRACE_INFO0(
                "CONFIG ERROR: Periodic advertising requires Extended mode");
            return false;
        }

        if (cfg->connectable || cfg->scannable)
        {
            APP_TRACE_INFO0(
                "CONFIG ERROR: Periodic advertising must be "
                "non-connectable and non-scannable");
            return false;
        }

        if ((cfg->periodic_interval_ms < 8U) ||
            (cfg->periodic_interval_ms > 81918U))
        {
            APP_TRACE_INFO0(
                "CONFIG ERROR: periodic interval out of range");
            return false;
        }
    }

    /* Connectionless CTE */
    if (cfg->cte.enabled)
    {
        if (cfg->periodic_interval_ms == 0U)
        {
            APP_TRACE_INFO0(
                "CONFIG ERROR: Connectionless CTE requires "
                "Periodic Advertising");
            return false;
        }

        if ((cfg->cte.length_us < 16U) ||
            (cfg->cte.length_us > 160U) ||
            ((cfg->cte.length_us % 8U) != 0U))
        {
            APP_TRACE_INFO0(
                "CONFIG ERROR: CTE length must be 16..160 us "
                "in 8 us steps");
            return false;
        }

        if ((cfg->cte.count < 1U) ||
            (cfg->cte.count > 16U))
        {
            APP_TRACE_INFO0(
                "CONFIG ERROR: CTE count must be 1..16");
            return false;
        }

        if ((cfg->cte.type != ADV_CTE_AOA) &&
            (cfg->cte.type != ADV_CTE_AOD_1US) &&
            (cfg->cte.type != ADV_CTE_AOD_2US))
        {
            APP_TRACE_INFO0(
                "CONFIG ERROR: invalid CTE type");
            return false;
        }
    }

    /* Connection parameters */
    if (cfg->connectable)
    {
        uint16_t min_interval_units;
        uint16_t max_interval_units;
        uint64_t supervision_timeout_us;
        uint64_t required_timeout_us;

        /*
         * BLE connection interval:
         * 0x0006 .. 0x0C80 in 1.25 ms units
         * = 7.5 ms .. 4000 ms.
         *
         * Configuration uses integer milliseconds, therefore 8 ms is the
         * smallest user-facing value representable here.
         */
        if ((cfg->connection.min_interval_ms < 8U) ||
            (cfg->connection.min_interval_ms > 4000U))
        {
            APP_TRACE_INFO0(
                "CONFIG ERROR: minimum connection interval must be 8..4000 ms");
            return false;
        }

        if ((cfg->connection.max_interval_ms < 8U) ||
            (cfg->connection.max_interval_ms > 4000U))
        {
            APP_TRACE_INFO0(
                "CONFIG ERROR: maximum connection interval must be 8..4000 ms");
            return false;
        }

        if (cfg->connection.min_interval_ms >
            cfg->connection.max_interval_ms)
        {
            APP_TRACE_INFO0(
                "CONFIG ERROR: minimum connection interval exceeds maximum");
            return false;
        }

        /*
         * Connection latency:
         * 0 .. 499 connection events.
         */
        if (cfg->connection.latency > 499U)
        {
            APP_TRACE_INFO0(
                "CONFIG ERROR: connection latency must be 0..499");
            return false;
        }

        /*
         * Supervision timeout:
         * 0x000A .. 0x0C80 in 10 ms units
         * = 100 ms .. 32000 ms.
         */
        if ((cfg->connection.supervision_timeout_ms < 100U) ||
            (cfg->connection.supervision_timeout_ms > 32000U))
        {
            APP_TRACE_INFO0(
                "CONFIG ERROR: supervision timeout must be 100..32000 ms");
            return false;
        }

        /*
         * Avoid silently truncating the value in
         * advertiser_supervision_timeout_to_units().
         */
        if ((cfg->connection.supervision_timeout_ms % 10U) != 0U)
        {
            APP_TRACE_INFO0(
                "CONFIG ERROR: supervision timeout must be a multiple of 10 ms");
            return false;
        }

        /*
         * Validate the supervision timeout against the effective connection
         * interval after conversion to BLE units.
         *
         * Requirement:
         *
         * supervision_timeout >
         *     2 * (1 + latency) * max_connection_interval
         */
        min_interval_units =
            advertiser_conn_interval_to_units(
                cfg->connection.min_interval_ms);

        max_interval_units =
            advertiser_conn_interval_to_units(
                cfg->connection.max_interval_ms);

        if (min_interval_units > max_interval_units)
        {
            APP_TRACE_INFO0(
                "CONFIG ERROR: invalid converted connection interval range");
            return false;
        }

        supervision_timeout_us =
            (uint64_t)cfg->connection.supervision_timeout_ms * 1000ULL;

        required_timeout_us =
            2ULL *
            ((uint64_t)cfg->connection.latency + 1ULL) *
            (uint64_t)max_interval_units *
            1250ULL;

        if (supervision_timeout_us <= required_timeout_us)
        {
            APP_TRACE_INFO0(
                "CONFIG ERROR: supervision timeout too short for "
                "connection interval and latency");
            return false;
        }

        /*
         * Cordio application policy:
         * update_delay_ms == 0 disables automatic connection parameter update.
         * In that case update_attempts should also be zero.
         */
        if ((cfg->connection.update_delay_ms == 0U) &&
            (cfg->connection.update_attempts != 0U))
        {
            APP_TRACE_INFO0(
                "CONFIG ERROR: update_attempts must be 0 when "
                "connection update is disabled");
            return false;
        }
    }

    return true;
}

static bool advertiser_config_valid = false;

void advertiser_handler_init(wsfHandlerId_t handler_id)
{
    bool use_legacy_pdu;

    advertiser_handler_id = handler_id;

    advertiser_config_valid =
        advertiser_validate_config(&g_advertiser_config);

    if (!advertiser_config_valid)
    {
        APP_TRACE_INFO0(
            "CONFIG ERROR: advertiser configuration rejected");
        return;
    }

    use_legacy_pdu =
        (g_advertiser_config.mode == ADV_MODE_LEGACY);

    advertiser_ext_adv_cfg.useLegacyPdu[0] =
        use_legacy_pdu;

    advertiser_ext_adv_cfg.advInterval[0] =
        advertiser_interval_to_units(
        g_advertiser_config.advertising_interval_ms);

    advertiser_ext_adv_cfg.perAdvInterval[0] = 
        advertiser_periodic_interval_to_units(
        g_advertiser_config.periodic_interval_ms);

    pAppExtAdvCfg =
        &advertiser_ext_adv_cfg;

    pAppSlaveCfg =
        (appSlaveCfg_t *)&advertiser_slave_cfg;

    pAppUpdateCfg =
        (appUpdateCfg_t *)&advertiser_update_cfg;


    advertiser_build_connection_config();

    AppSlaveInit();
    AppServerInit();
}



/* -------------------------------------------------------------------------- */
/* Start                                                                      */
/* -------------------------------------------------------------------------- */

void advertiser_start(void)
{

    if (!advertiser_config_valid)
    {
        return;
    }

    DmRegister(advertiser_dm_callback);

    DmConnRegister(
        DM_CLIENT_ID_APP,
        advertiser_dm_callback);

    AttRegister(advertiser_att_callback);
    AttConnRegister(AppServerConnCback);

    SvcCoreGattCbackRegister(GattReadCback, GattWriteCback);
    SvcCoreAddGroup();
    
    DmDevReset();
}