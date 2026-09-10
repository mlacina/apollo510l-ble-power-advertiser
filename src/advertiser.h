/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2026 Mariusz Łacina
 *
 * @file advertiser.h
 * @brief Public configuration interface for the Apollo510L BLE Power Advertiser.
 */

#ifndef ADVERTISER_H
#define ADVERTISER_H

#include <stdint.h>
#include <stdbool.h>

#include "wsf_os.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
    ADV_MODE_LEGACY,
    ADV_MODE_EXTENDED
} advertiser_mode_t;

typedef enum
{
    ADV_PRIMARY_PHY_1M,
    ADV_PRIMARY_PHY_CODED
} advertiser_primary_phy_t;

typedef enum
{
    ADV_SECONDARY_PHY_1M,
    ADV_SECONDARY_PHY_2M,
    ADV_SECONDARY_PHY_CODED
} advertiser_secondary_phy_t;

typedef enum
{
    ADV_CODED_S2,
    ADV_CODED_S8
} advertiser_coded_scheme_t;

typedef enum
{
    ADV_SLEEP_DEEP,
    ADV_SLEEP_DEEPER
} advertiser_sleep_mode_t;

typedef enum
{
    ADV_TX_POWER_MINUS_20_DBM = -20,
    ADV_TX_POWER_MINUS_18_DBM = -18,
    ADV_TX_POWER_MINUS_16_DBM = -16,
    ADV_TX_POWER_MINUS_14_DBM = -14,
    ADV_TX_POWER_MINUS_12_DBM = -12,
    ADV_TX_POWER_MINUS_10_DBM = -10,
    ADV_TX_POWER_MINUS_8_DBM  = -8,
    ADV_TX_POWER_MINUS_6_DBM  = -6,
    ADV_TX_POWER_MINUS_4_DBM  = -4,
    ADV_TX_POWER_MINUS_2_DBM  = -2,

    ADV_TX_POWER_0_DBM        = 0,
    
    ADV_TX_POWER_PLUS_1_DBM   = 1,
    ADV_TX_POWER_PLUS_2_DBM   = 2,
    ADV_TX_POWER_PLUS_3_DBM   = 3,
    ADV_TX_POWER_PLUS_4_DBM   = 4,
    ADV_TX_POWER_PLUS_5_DBM   = 5,
    ADV_TX_POWER_PLUS_6_DBM   = 6,
    ADV_TX_POWER_PLUS_7_DBM   = 7,
    ADV_TX_POWER_PLUS_8_DBM   = 8,
    ADV_TX_POWER_PLUS_9_DBM   = 9,
    ADV_TX_POWER_PLUS_10_DBM  = 10,
    ADV_TX_POWER_PLUS_11_DBM  = 11,
    ADV_TX_POWER_PLUS_12_DBM  = 12,
    ADV_TX_POWER_PLUS_13_DBM  = 13
} advertiser_tx_power_t;

typedef enum
{
    ADV_CTE_AOA,
    ADV_CTE_AOD_1US,
    ADV_CTE_AOD_2US
} advertiser_cte_type_t;

typedef struct
{
    bool enabled;
    uint16_t length_us;
    uint8_t count;
    advertiser_cte_type_t type;
} advertiser_cte_config_t;


typedef struct
{
    uint32_t min_interval_ms;
    uint32_t max_interval_ms;
    uint16_t latency;
    uint32_t supervision_timeout_ms;
    uint32_t update_delay_ms;
    uint8_t  update_attempts;
} advertiser_connection_config_t;


typedef struct
{
    advertiser_mode_t mode;
    uint32_t advertising_interval_ms;
    advertiser_tx_power_t tx_power;
    advertiser_primary_phy_t primary_phy;
    advertiser_secondary_phy_t secondary_phy;
    advertiser_coded_scheme_t primary_coded_scheme;
    advertiser_coded_scheme_t secondary_coded_scheme;
    bool channel_37;
    bool channel_38;
    bool channel_39;
    bool connectable;
    bool scannable;
    uint16_t advertising_data_length;
    uint16_t scan_response_length;
    advertiser_connection_config_t connection;
    advertiser_cte_config_t cte;
    uint32_t periodic_interval_ms;
    advertiser_sleep_mode_t sleep_mode;
} advertiser_config_t;

extern const advertiser_config_t g_advertiser_config;

void advertiser_handler_init(wsfHandlerId_t handler_id);
void advertiser_handler(wsfEventMask_t event, wsfMsgHdr_t *msg);
void advertiser_start(void);

#ifdef __cplusplus
}
#endif

#endif /* ADVERTISER_H */