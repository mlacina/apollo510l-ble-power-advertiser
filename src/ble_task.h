/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2026 Mariusz Łacina
 *
 * BLE task interface for the Apollo510L BLE Power Advertiser.
 */

#ifndef BLE_TASK_H
#define BLE_TASK_H

#ifdef __cplusplus
extern "C" {
#endif

void ble_task(void *context);

#ifdef __cplusplus
}
#endif

#endif /* BLE_TASK_H */