/**
 * Copyright (c) 2012 by Lukas Heidemann <lukasheidemann@gmail.com>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once
#include <apic.h>
#include <stdint.h>

// LAPIC register indices
#define LAPIC_REG_ID            0x020   //< LAPIC ID
#define LAPIC_REG_VERSION       0x030   //< LAPIC version
#define LAPIC_REG_TPR           0x080   //< Task Priority Register
#define LAPIC_REG_EOI           0x0B0   //< End of Interrupt
#define LAPIC_REG_LDR           0x0D0   //< Logical Destination Register
#define LAPIC_REG_SVR           0x0F0   //< Spurious Vector Register
#define LAPIC_REG_ICR_LOW       0x300   //< Interrupt Command Register (lower DWORD)
#define LAPIC_REG_ICR_HIGH      0x310   //< Interrupt Command Register (upper DWORD)
#define LAPIC_REG_TIMER         0x320   //< Local: Timer
#define LAPIC_REG_PCINT         0x340   //< Local: Performance Counter
#define LAPIC_REG_LINT0         0x350   //< Local: LINT0 Pin (Normal)
#define LAPIC_REG_LINT1         0x360   //< Local: LINT1 Pin (NMI)
#define LAPIC_REG_ERRINT        0x370   //< Local: Error Interrupt
#define LAPIC_REG_TIMER_INIT    0x380   //< Timer: Initial Count
#define LAPIC_REG_TIMER_CUR     0x390   //< Timer: Current Count
#define LAPIC_REG_TIMER_DIV     0x3E0   //< Timer: Divisor

// Interrupt Command Register structure
#define LAPIC_ICR_VECTOR        0
#define LAPIC_ICR_DVL_MODE      8
#define LAPIC_ICR_DEST_MODE     11
#define LAPIC_ICR_DLV_STATUS    12
#define LAPIC_ICR_LEVEL         14
#define LAPIC_ICR_TRIGGER       15
#define LAPIC_ICR_SHORTHAND     18
#define LAPIC_ICR_DEST          56

// LAPIC timer LVT structure
#define LAPIC_TIMER_VECTOR      0
#define LAPIC_TIMER_MASK        16
#define LAPIC_TIMER_TRIGGER     17

// Default value for registers
#define LAPIC_TPR               0x0
#define LAPIC_PCINT             (1 << 16)
#define LAPIC_SVR               0x20 | (1 << 8)
#define LAPIC_LINT0             (0b111 << 8) | (1 << 15)
#define LAPIC_LINT1             (0b100 << 8)
#define LAPIC_ERRINT            (1 << 16)
#define LAPIC_LDR               (1 << (lapic_id() % 8))

// INIT IPI
#define LAPIC_IPI_INIT(id)                             ( \
        (APIC_DELIVERY_INIT    << LAPIC_ICR_DVL_MODE)   | \
        (APIC_SHORT_NONE       << LAPIC_ICR_SHORTHAND)  | \
        (APIC_TRIGGER_EDGE     << LAPIC_ICR_TRIGGER)    | \
        (APIC_LEVEL_ASSERT     << LAPIC_ICR_LEVEL)      | \
        (APIC_MODE_PHYSICAL    << LAPIC_ICR_DEST_MODE)  | \
        ((uint64_t) id         << LAPIC_ICR_DEST)       )

// STARTUP IPI
#define LAPIC_IPI_STARTUP(id, entry_point)             ( \
        (APIC_DELIVERY_STARTUP << LAPIC_ICR_DVL_MODE)   | \
        (APIC_SHORT_NONE       << LAPIC_ICR_SHORTHAND)  | \
        (APIC_TRIGGER_EDGE     << LAPIC_ICR_TRIGGER)    | \
        (APIC_LEVEL_ASSERT     << LAPIC_ICR_LEVEL)      | \
        (APIC_MODE_PHYSICAL    << LAPIC_ICR_DEST_MODE)  | \
        ((uint64_t) id         << LAPIC_ICR_DEST)       | \
        ((entry_point >> 12)   << LAPIC_ICR_VECTOR)     )

/**
 * Reads a register of the CPU's LAPIC, given its <index>.
 *
 * Uses the LAPIC address stored in the info tables.
 * Behavior is undefined when called for an invalid register.
 *
 * @param index the index of the register
 * @return value of the register
 */
uint32_t lapic_register_read(uint16_t index);

/**
 * Writes a <value> to a register of the CPU's LAPIC, given its <index>.
 *
 * Returns the previous value of the register, as reading the value is
 * required nevertheless, because some LAPIC implementations only allow a
 * register to be written directly after it has been read.
 *
 * Uses the LAPIC address stored in the info tables.
 * Behavior is undefined when called for an invalid register.
 *
 * @param index the index of the register to write
 * @param value the new value of the register
 * @return the old value of the register
 */
uint32_t lapic_register_write(uint16_t index, uint32_t value);

/**
 * Enables the CPU's LAPIC and configures it to reasonable defaults.
 */
void lapic_setup(void);

/**
 * Returns the APIC id of the CPU's LAPIC.
 *
 * @return APIC id
 */
uint8_t lapic_id(void);

/**
 * Signals an EOI to the CPU's LAPIC.
 */
void lapic_eoi(void);

/**
 * Sends an IPI by writing the given <icr> value into the LAPIC's
 * Interrupt Control Register.
 *
 * @param icr the value to be written to the ICR
 */
void lapic_ipi(uint64_t icr);

/**
 * Updates the LAPIC's timer configuration.
 *
 * Writes the initial count, the interrupt vector, the interrupt mask and defines, whether
 * the timer should restart itself periodically.
 *
 * @param init_count the timer's initial counter value
 * @param vector the vector the timer should raise an interrupt on
 * @param mask true, when no interrupts should be raised, false otherwise
 * @param periodic whether the timer should fire periodically
 */
void lapic_timer_update(uint32_t init_count, uint8_t vector, bool mask, bool periodic);

/**
 * Calibrates the timer using the PIT and writes the results to the info tables.
 */
void lapic_timer_calibrate(void);

/**
 * Waits for the specified time (in micro seconds) using the LAPIC timer.
 *
 * Will reset the LAPIC timer and will temporarily change an IDT entry. Make sure
 * all other interrupts are masked.
 *
 * @param time the time to wait in micro seconds
 */
void lapic_timer_wait(uint64_t time);
