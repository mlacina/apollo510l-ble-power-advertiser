
/*
 * Main application and low-power system configuration for the
 * Apollo510L BLE Power Advertiser.
 *
 * Project-specific modifications and new code:
 * Copyright (c) 2026 Mariusz Łacina
 *
 * Project-specific modifications are licensed under the BSD 3-Clause License.
 * See LICENSE for the applicable terms.
 *
 * This file is based on the ble_freertos_fit_lp example and related
 * RTOS support code from AmbiqSuite SDK 5.2.0, licensed under the
 * Ambiq BSD 3-Clause license.
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

/* -------------------------------------------------------------------------- */
/* Required built-ins.                                                        */
/* -------------------------------------------------------------------------- */
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>

/* -------------------------------------------------------------------------- */
/* Standard AmbiqSuite includes.                                              */
/* -------------------------------------------------------------------------- */
#include "am_mcu_apollo.h"
#include "am_bsp.h"
#include "am_util.h"

/* -------------------------------------------------------------------------- */
/* FreeRTOS include files.                                                    */
/* -------------------------------------------------------------------------- */
#include "FreeRTOS.h"
#include "task.h"

/* -------------------------------------------------------------------------- */
/* Task include files.                                                        */
/* -------------------------------------------------------------------------- */
#include "wsf_types.h"
#include "ble_task.h"
#include "advertiser.h"
#include "am_devices_510L_radio.h"

#define ALL_RETAIN 0    // 0 for min TCM retain (default), 1 for all retain
#define ENABLE_CACHE  0

/* -------------------------------------------------------------------------- */
/* Global Variables                                                           */
/* -------------------------------------------------------------------------- */

AM_SHARED_RW uint32_t g_pui32IpcShm[8192 / sizeof(uint32_t)] __attribute__((aligned(32)));

/* -------------------------------------------------------------------------- */
/* FreeRTOS debugging functions.                                              */
/* -------------------------------------------------------------------------- */
void vApplicationMallocFailedHook(void)
{
    //
    // Called if a call to pvPortMalloc() fails because there is insufficient
    // free memory available in the FreeRTOS heap.  pvPortMalloc() is called
    // internally by FreeRTOS API functions that create tasks, queues, software
    // timers, and semaphores.  The size of the FreeRTOS heap is set by the
    // configTOTAL_HEAP_SIZE configuration constant in FreeRTOSConfig.h.
    //
    while (1);
}

void vApplicationStackOverflowHook(TaskHandle_t pxTask, char *pcTaskName)
{
    (void) pcTaskName;
    (void) pxTask;

    //
    // Run time stack overflow checking is performed if
    // configconfigCHECK_FOR_STACK_OVERFLOW is defined to 1 or 2.  This hook
    // function is called if a stack overflow is detected.
    //
    while (1)
    {
        __asm("BKPT #0\n") ; // Break into the debugger
    }
}

uint32_t am_freertos_sleep(uint32_t idleTime)
{
    (void)idleTime;

#ifdef AM_DEBUG_PRINTF
    //
    // Prepare for deepsleep while SWO is still enabled
    //
    am_bsp_debug_printf_deepsleep_prepare(true);
#endif

    if (g_advertiser_config.sleep_mode == ADV_SLEEP_DEEPER)
    {
        am_hal_sysctrl_sleep(AM_HAL_SYSCTRL_SLEEP_DEEPER);
    }
    else
    {
        am_hal_sysctrl_sleep(AM_HAL_SYSCTRL_SLEEP_DEEP);
    }

#ifdef AM_DEBUG_PRINTF
    //
    // Restore from deepsleep
    //
    am_bsp_debug_printf_deepsleep_prepare(false);
#endif

    return 0;
}

void am_freertos_wakeup(uint32_t idleTime)
{
    (void)idleTime;
}

void am_timer_isr(void)
{
    uint32_t ui32Status;

    am_hal_timer_interrupt_status_get(false, &ui32Status);
    am_hal_timer_interrupt_clear(ui32Status);
}

am_devices_510L_radio_ipc_shm_t sIpcShm =
{
    .ui32IpcShmAddr = (uint32_t)g_pui32IpcShm,
    .ui32IpcShmSize = sizeof(g_pui32IpcShm)
};

int main(void)
{
    am_util_delay_ms(3000);

    am_hal_pwrctrl_mcu_memory_config_t McuMemCfg =
    {
         //
         //  In order to demonstrate the lowest possible power, this example enables the ROM automatic power down feature.
         //  This should not be used in general for most applications.
         //
        .eROMMode       = AM_HAL_PWRCTRL_ROM_AUTO,
        .eRetainDTCM    = AM_HAL_PWRCTRL_MEMRETCFG_TCMPWDSLP_RETAIN,
        .eNVMCfg        = AM_HAL_PWRCTRL_NVM,
        .bKeepNVMOnInDeepSleep     = false,

#if ALL_RETAIN

        .eDTCMCfg       = AM_HAL_PWRCTRL_DTCM256K,

#else 

        .eDTCMCfg       = AM_HAL_PWRCTRL_DTCM128K,

#endif // ALL_RETAIN     
    };



    am_hal_pwrctrl_sram_memcfg_t SRAMMemCfg =
    {
#if ALL_RETAIN

        .eSRAMCfg         = AM_HAL_PWRCTRL_SRAM_1P75M,
        .eSRAMRetain      = AM_HAL_PWRCTRL_SRAM_1P75M,

#else // ALL_RETAIN

        .eSRAMCfg         = AM_HAL_PWRCTRL_SRAM_0P75M,
        .eSRAMRetain      = AM_HAL_PWRCTRL_SRAM_0P75M,

#endif // ALL_RETAIN
        
        .eActiveWithMCU   = AM_HAL_PWRCTRL_SRAM_NONE,
        .eActiveWithGFX   = AM_HAL_PWRCTRL_SRAM_NONE,
        .eActiveWithDISP  = AM_HAL_PWRCTRL_SRAM_NONE, 
    };

    //
    // Initialize the printf interface for UART output.
    //
    am_bsp_debug_printf_enable();

    //
    // Print the banner.
    //
    am_util_stdio_terminal_clear();
    am_util_stdio_printf("FreeRTOS BLE Power Advertiser\n");

#ifndef AM_DEBUG_PRINTF
    //
    // To minimize power during the run, disable the UART.
    //
    am_bsp_debug_printf_disable();
#endif

    //
    // Configure the board for low power.
    //
    am_bsp_low_power_init();
    am_hal_rtc_osc_select(AM_HAL_RTC_OSC_LFRC); // Use LFRC instead of XT

    //
    // Disable RTC Oscillator
    //
    am_hal_rtc_osc_disable();

    //
    // Disable all peripherals including Crypto
    //
    for (am_hal_pwrctrl_periph_e periph = AM_HAL_PWRCTRL_PERIPH_IOSFD0;
        periph < AM_HAL_PWRCTRL_PERIPH_MAX; periph ++)
    {
        if (periph == AM_HAL_PWRCTRL_PERIPH_NETAOL)
        {
            // Keep the RSS power on
            break;
        }
        am_hal_pwrctrl_periph_disable(periph);
    }

#ifndef WSF_TRACE_ENABLED
    //
    // Disable Debug Subsystem
    //
    MCUCTRL->DBGCTRL = 0;
#endif

    am_hal_pwrctrl_mcu_memory_config(&McuMemCfg);

    //
    //
    // MRAM0 LP Setting affects the Latency to wakeup from Sleep. Hence, it should be configured in the application.
    //
    //
    // Configure the MRAM for low power mode.  Note that using MRAM LP mode has been shown to interact with
    // the Apollo5 RevA SW workaround for LP/HP mode transitions.  Thus, it is not recommended for general use
    // in customer applications.
    MCUCTRL->MRAMCRYPTOPWRCTRL_b.MRAM0LPREN = 1;
    MCUCTRL->MRAMCRYPTOPWRCTRL_b.MRAM0SLPEN = 0;
    MCUCTRL->MRAMCRYPTOPWRCTRL_b.MRAM0PWRCTRL = 0;


#if ENABLE_CACHE
    //
    // Enable caches
    //
    am_hal_cachectrl_icache_enable();
    am_hal_cachectrl_dcache_enable(true);

    //
    // Set up the attributes.
    //
    am_hal_mpu_attr_configure(&sMPUAttr, 1);
    //
    // Clear the MPU regions.
    //
    am_hal_mpu_region_clear();
    //
    // Set up the regions.
    //
    am_hal_mpu_region_configure(&sMPUCfg, 1);
    //
    // Invalidate and clear DCACHE, this is required by CM55 TRF.
    //
    am_hal_cachectrl_dcache_invalidate(NULL, true);
    //
    // MPU enable
    //
    am_hal_mpu_enable(true, true);

#else
    //
    // Disable caches
    //
    am_hal_cachectrl_icache_disable();
    am_hal_cachectrl_dcache_disable();

    //
    // Set the CPDLPSTATE configuration for deepersleep mode
    //
    am_hal_pwrctrl_pwrmodctl_cpdlp_t sDeeperSleepCpdlpConfig =
    {
         .eRlpConfig = AM_HAL_PWRCTRL_RLP_OFF,
         .eElpConfig = AM_HAL_PWRCTRL_ELP_OFF,
         .eClpConfig = AM_HAL_PWRCTRL_CLP_OFF
    };

    am_hal_pwrctrl_pwrmodctl_cpdlp_config(sDeeperSleepCpdlpConfig);

    //
    // Power off caches
    //
    am_hal_cachectrl_caches_power_control(false);
#endif // ENABLE_CACHE


    //
    // Initialize the IPC share memory
    //
    am_devices_510L_radio_ipc_shm_init(&sIpcShm);

    //
    // Disable SRAM
    //
    am_hal_pwrctrl_sram_config(&SRAMMemCfg);

    //
    // Prepare BLE Task.
    //
    xTaskCreate(ble_task, "BLE", 512, NULL, 3, NULL);

    //
    // Start the scheduler.
    //
    vTaskStartScheduler();

    //
    // We shouldn't ever get here.
    //
    while (1)
    {
    }
}

