/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2026 Mariusz Łacina
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

/*
 * Project-authored GNU ld script for SEGGER Embedded Studio.
 * The memory layout matches the Apollo510L configuration used by this project.
 */
 
 /*
 * Shared SRAM placement:
 *   0 = GROUP0, lower 1 MiB
 *   1 = GROUP1, upper 0.75 MiB
 *   2 = ALL,    full 1.75 MiB
 *
 * Default: GROUP1, preserving the current low-power IPC placement.
 * Override with:
 *   --defsym=SHARED_SRAM_SELECT=0
 *   --defsym=SHARED_SRAM_SELECT=1
 *   --defsym=SHARED_SRAM_SELECT=2
 */

/*
 * DTCM placement:
 *   0 = 128 KiB total DTCM
 *   1 = 256 KiB total DTCM
 *
 * The upper 16 KiB of the selected DTCM range is reserved for a 4 KiB heap
 * followed by a 12 KiB stack. The remaining lower range is available to the
 * application DTCM sections.
 *
 * Default: 256 KiB, preserving the current DTCM, heap, and stack layout.
 * Override with:
 *   --defsym=DTCM_SELECT=0
 *   --defsym=DTCM_SELECT=1
 */
 
PROVIDE(SHARED_SRAM_SELECT = 1);
PROVIDE(DTCM_SELECT = 0);

ENTRY(Reset_Handler)

MEMORY 
{
    FLASH     (rx)  : ORIGIN = 0x00410000, LENGTH = 2031616
    DTCM      (rwx) : ORIGIN = 0x20000000,
                      LENGTH = (DTCM_SELECT == 0 ? 0x1C000 : 0x3C000)

    /* Physical/logical Shared SRAM regions. */
    SHARED_SRAM_GROUP0 (rwx) : ORIGIN = 0x20080000, LENGTH = 0x100000
    SHARED_SRAM_GROUP1 (rwx) : ORIGIN = 0x20180000, LENGTH = 0x0C0000
    SHARED_SRAM_ALL    (rwx) : ORIGIN = 0x20080000, LENGTH = 0x1C0000

    /*
     * The only Shared SRAM region used for allocation.
     * Do not assign output sections directly to the three regions above.
     */
    SHARED_SRAM (rwx) :
        ORIGIN = (SHARED_SRAM_SELECT == 0 ? 0x20080000 :
                  SHARED_SRAM_SELECT == 1 ? 0x20180000 :
                                           0x20080000),
        LENGTH = (SHARED_SRAM_SELECT == 0 ? 0x100000 :
                  SHARED_SRAM_SELECT == 1 ? 0x0C0000 :
                                           0x1C0000)

    STACK_RAM (rw) :
        ORIGIN = (DTCM_SELECT == 0 ? 0x2001D000 : 0x2003D000),
        LENGTH = 0x03000
    HEAP_RAM  (rw) :
        ORIGIN = (DTCM_SELECT == 0 ? 0x2001C000 : 0x2003C000),
        LENGTH = 0x01000
}

ASSERT(SHARED_SRAM_SELECT >= 0 && SHARED_SRAM_SELECT <= 2,
       "SHARED_SRAM_SELECT must be 0 (GROUP0), 1 (GROUP1), or 2 (ALL)")

ASSERT(DTCM_SELECT == 0 || DTCM_SELECT == 1,
       "DTCM_SELECT must be 0 (128 KiB) or 1 (256 KiB)")

__shared_sram_selection__ = SHARED_SRAM_SELECT;
__shared_sram_start__     = ORIGIN(SHARED_SRAM);
__shared_sram_end__       = ORIGIN(SHARED_SRAM) + LENGTH(SHARED_SRAM);

__dtcm_selection__ = DTCM_SELECT;
__dtcm_start__     = ORIGIN(DTCM);
__dtcm_end__       = ORIGIN(STACK_RAM) + LENGTH(STACK_RAM);

SECTIONS
{
    .text :
    {
        . = ALIGN(4);
        __text_start__ = .;

        KEEP(*(.isr_vector))
        KEEP(*(.patch))

        *(.text .text.* .gnu.linkonce.t.*)
        *(.rodata .rodata.* .gnu.linkonce.r.*)

        . = ALIGN(4);
        _etext = .;
        __text_end__ = .;
    } > FLASH

    .ARM.extab :
    {
        . = ALIGN(4);
        *(.ARM.extab .ARM.extab.* .gnu.linkonce.armextab.*)
        . = ALIGN(4);
    } > FLASH

    .ARM.exidx :
    {
        . = ALIGN(4);
        __exidx_start = .;
        *(.ARM.exidx .ARM.exidx.* .gnu.linkonce.armexidx.*)
        __exidx_end = .;
        . = ALIGN(4);
    } > FLASH

    .dtcm_text :
    {
        . = ALIGN(4);
        _s_dtcm_text = .;
        *(.dtcm_text .dtcm_text.*)
        . = ALIGN(4);
        _e_dtcm_text = .;
    } > DTCM AT> FLASH
    _init_dtcm_text = LOADADDR(.dtcm_text);

    .data :
    {
        . = ALIGN(4);
        _sdata = .;
        *(.data .data.* .gnu.linkonce.d.*)
        . = ALIGN(4);
        _edata = .;
    } > DTCM AT> FLASH
    _init_data = LOADADDR(.data);

    .bss (NOLOAD) :
    {
        . = ALIGN(4);
        _sbss = .;
        *(.bss .bss.* .gnu.linkonce.b.*)
        *(COMMON)
        . = ALIGN(4);
        _ebss = .;
    } > DTCM

    .heap (NOLOAD) :
    {
        . = ALIGN(8);
        __heap_start__ = .;
        end = __heap_start__;
        _end = __heap_start__;
        __end = __heap_start__;

        KEEP(*(.heap .heap.*))

        . = ORIGIN(HEAP_RAM) + LENGTH(HEAP_RAM);
        __heap_end__ = .;
        __HeapLimit = .;
    } > HEAP_RAM

    .stack (NOLOAD) :
    {
        . = ALIGN(8);
        __StackLimit = .;

        KEEP(*(.stack .stack.*))

        . = ALIGN(8);
        __StackTop = .;
    } > STACK_RAM

    .shared (NOLOAD) :
    {
        . = ALIGN(4);
        KEEP(*(.resource_table .resource_table.*))
        KEEP(*(.shared .shared.*))
        . = ALIGN(4);
    } > SHARED_SRAM

    .ARM.attributes 0 :
    {
        *(.ARM.attributes)
    }
}

ASSERT(__StackTop <= ORIGIN(STACK_RAM) + LENGTH(STACK_RAM),
       "Apollo510L stack exceeds STACK_RAM")

ASSERT(ORIGIN(DTCM) >= __dtcm_start__ &&
       ORIGIN(DTCM) + LENGTH(DTCM) <= __dtcm_end__ &&
       ORIGIN(HEAP_RAM) >= __dtcm_start__ &&
       ORIGIN(HEAP_RAM) + LENGTH(HEAP_RAM) <= __dtcm_end__ &&
       ORIGIN(STACK_RAM) >= __dtcm_start__ &&
       ORIGIN(STACK_RAM) + LENGTH(STACK_RAM) <= __dtcm_end__,
       "Apollo510L DTCM regions exceed the selected powered DTCM range")
