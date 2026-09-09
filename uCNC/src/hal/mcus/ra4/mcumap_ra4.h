/*
	Name: mcumap_ra4.h
	Description: Contains all MCU and PIN definitions for Renesas RA4 series (RA4M1) to run uCNC.

	Copyright: Copyright (c) Joao Martins
	Author: Joao Martins
	Date: 09/09/2026

	uCNC is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version. Please see <http://www.gnu.org/licenses/>

	uCNC is distributed WITHOUT ANY WARRANTY;
	Also without the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
	See the GNU General Public License for more details.
*/

#ifndef MCUMAP_RA4_H
#define MCUMAP_RA4_H

#ifdef __cplusplus
extern "C"
{
#endif

/*
	Generates all the interface definitions.
	This creates a middle HAL layer between the board IO pins and the MCU functionalities
*/

/* ======================================================================== */
/* Section 2: Device/SDK includes                                           */
/* ======================================================================== */
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

/* CMSIS core for Cortex-M4 with FPU (RA4M1) */
#ifndef __FPU_PRESENT
#define __FPU_PRESENT 1
#endif
#ifndef __NVIC_PRIO_BITS
#define __NVIC_PRIO_BITS 4
#endif
/* IRQn_Type: defined here for standalone C builds. When the Renesas FSP is
 * active (Arduino.h included first, _RENESAS_RA_ defined), bsp_arm_exceptions.h
 * provides the same enum. The BSP_ARM_EXCEPTIONS_H guard avoids duplication. */
#ifndef BSP_ARM_EXCEPTIONS_H
typedef enum IRQn
{
    Reset_IRQn            = -15,
    NonMaskableInt_IRQn   = -14,
    HardFault_IRQn        = -13,
    MemoryManagement_IRQn = -12,
    BusFault_IRQn         = -11,
    UsageFault_IRQn       = -10,
    SecureFault_IRQn      = -9,
    SVCall_IRQn           = -5,
    DebugMonitor_IRQn     = -4,
    PendSV_IRQn           = -2,
    SysTick_IRQn          = -1,
} IRQn_Type;
#endif
#include "core_cm4.h"

/* RA4M1 register layout types (matching FSP/CMSIS device header layout).
 * These are self-contained so this file compiles standalone. If the Renesas
 * FSP device header is already included (via Arduino.h), the #ifndef guards
 * prevent redefinition. */
#ifndef R7FA4M1AB_H

/* R_PORT0_Type: GPIO port registers (16 bytes, stride 0x20 per port) */
typedef struct
{
    volatile uint16_t PODR;  /* +0x00: Port Output Data */
    volatile uint16_t PDR;   /* +0x02: Port Direction */
    volatile uint16_t EIDR;  /* +0x04: Event Input Data (unused) */
    volatile const uint16_t PIDR;  /* +0x06: Port Input Data */
    volatile uint16_t PORR;  /* +0x08: Port Output Reset (write 1 to clear) */
    volatile uint16_t POSR;  /* +0x0A: Port Output Set (write 1 to set) */
    volatile uint16_t EORR;  /* +0x0C: Event Output Reset */
    volatile uint16_t EOSR;  /* +0x0E: Event Output Set */
} R_PORT0_Type;

/* R_PFS_PORT_PIN_Type: Per-pin PFS register (4 bytes, 16 per port = 64 per port) */
typedef struct
{
    union
    {
        volatile uint32_t PmnPFS;
        struct
        {
            volatile uint32_t PODR  : 1;
            volatile uint32_t PIDR  : 1;
            volatile uint32_t PDR   : 1;
            uint32_t             : 1;
            volatile uint32_t PCR   : 1;
            volatile uint32_t PIM   : 1;
            volatile uint32_t NCODR : 1;
            uint32_t             : 3;
            volatile uint32_t DSCR  : 2;
            volatile uint32_t EOFR  : 2;
            volatile uint32_t ISEL  : 1;
            volatile uint32_t ASEL  : 1;
            volatile uint32_t PMR   : 1;
            uint32_t             : 7;
            volatile uint32_t PSEL  : 5;
            uint32_t             : 3;
        } PmnPFS_b;
    };
} R_PFS_PORT_PIN_Type;

typedef struct { R_PFS_PORT_PIN_Type PIN[16]; } R_PFS_PORT_Type;
typedef struct { R_PFS_PORT_Type PORT[15]; } R_PFS_Type;

/* R_PMISC_Type: PFS write protection register */
typedef struct
{
    uint8_t RESERVED[3];
    volatile uint8_t PWPR;
    uint8_t RESERVED1[1];
    volatile uint8_t PWPRS;
} R_PMISC_Type;

/* Port and PFS base addresses (RA4M1 hardware fixed) */
#define R_PORT0_BASE  0x40040000UL
#define R_PFS_BASE    0x40040800UL
#define R_PMISC_BASE  0x40040D00UL

/* Individual port pointers (matching FSP/CMSIS naming) */
#define R_PORT0       ((R_PORT0_Type *) R_PORT0_BASE)
#define R_PORT1       ((R_PORT0_Type *)(R_PORT0_BASE + 0x20UL))
#define R_PORT2       ((R_PORT0_Type *)(R_PORT0_BASE + 0x40UL))
#define R_PORT3       ((R_PORT0_Type *)(R_PORT0_BASE + 0x60UL))
#define R_PORT4       ((R_PORT0_Type *)(R_PORT0_BASE + 0x80UL))
#define R_PORT5       ((R_PORT0_Type *)(R_PORT0_BASE + 0xA0UL))
#define R_PORT6       ((R_PORT0_Type *)(R_PORT0_BASE + 0xC0UL))
#define R_PORT7       ((R_PORT0_Type *)(R_PORT0_BASE + 0xE0UL))
#define R_PORT8       ((R_PORT0_Type *)(R_PORT0_BASE + 0x100UL))
#define R_PORT9       ((R_PORT0_Type *)(R_PORT0_BASE + 0x120UL))
#define R_PORT10      ((R_PORT0_Type *)(R_PORT0_BASE + 0x140UL))
#define R_PORT11      ((R_PORT0_Type *)(R_PORT0_BASE + 0x160UL))
#define R_PORT12      ((R_PORT0_Type *)(R_PORT0_BASE + 0x180UL))
#define R_PORT13      ((R_PORT0_Type *)(R_PORT0_BASE + 0x1A0UL))
#define R_PORT14      ((R_PORT0_Type *)(R_PORT0_BASE + 0x1C0UL))

#define R_PFS         ((R_PFS_Type *) R_PFS_BASE)
#define R_PMISC       ((R_PMISC_Type *) R_PMISC_BASE)

/* R_GPT0_Type: General PWM Timer registers (32-bit width, 0x78 bytes) */
/* All GPT channels (0-7 on RA4M1) use the same register layout, stride 0x100 */
typedef struct
{
    volatile uint32_t GTWP;       /* +0x00: Write Protection */
    volatile uint32_t GTSTR;      /* +0x04: Software Start */
    volatile uint32_t GTSTP;      /* +0x08: Software Stop */
    volatile uint32_t GTCLR;      /* +0x0C: Software Clear */
    volatile uint32_t GTSSR;      /* +0x10: Start Source Select */
    volatile uint32_t GTPSR;      /* +0x14: Stop Source Select */
    volatile uint32_t GTCSR;      /* +0x18: Clear Source Select */
    volatile uint32_t GTUPSR;     /* +0x1C: Up Count Source Select */
    volatile uint32_t GTDNSR;     /* +0x20: Down Count Source Select */
    volatile uint32_t GTICASR;    /* +0x24: Input Capture Source Select A */
    volatile uint32_t GTICBSR;    /* +0x28: Input Capture Source Select B */
    volatile uint32_t GTCR;       /* +0x2C: Control (CST, MD[2:0], TPCS[3:0]) */
    volatile uint32_t GTUDDTYC;   /* +0x30: Up/Down Duty */
    volatile uint32_t GTIOR;      /* +0x34: I/O Control */
    volatile uint32_t GTINTAD;    /* +0x38: Interrupt Adjustment */
    volatile uint32_t GTST;       /* +0x3C: Status */
    volatile uint32_t GTBER;      /* +0x40: Buffer Enable */
    volatile uint32_t GTITC;      /* +0x44: Interrupt Skipping Setting */
    volatile uint32_t GTCNT;      /* +0x48: Counter */
    volatile uint32_t GTCCR[6];   /* +0x4C-0x60: Compare Capture A-F */
    volatile uint32_t GTPR;       /* +0x64: Cycle Period */
    volatile uint32_t GTPBR;      /* +0x68: Cycle Period Buffer */
    volatile uint32_t GTPDBR;     /* +0x6C: Cycle Period Double Buffer */
} R_GPT0_Type;

/* GPT channel base addresses (stride 0x100 per channel) */
#define R_GPT0_BASE  0x40078000UL
#define R_GPT1_BASE  0x40078100UL
#define R_GPT2_BASE  0x40078200UL
#define R_GPT3_BASE  0x40078300UL
#define R_GPT4_BASE  0x40078400UL
#define R_GPT5_BASE  0x40078500UL
#define R_GPT6_BASE  0x40078600UL
#define R_GPT7_BASE  0x40078700UL

#define R_GPT0    ((R_GPT0_Type *) R_GPT0_BASE)
#define R_GPT1    ((R_GPT0_Type *) R_GPT1_BASE)
#define R_GPT2    ((R_GPT0_Type *) R_GPT2_BASE)
#define R_GPT3    ((R_GPT0_Type *) R_GPT3_BASE)
#define R_GPT4    ((R_GPT0_Type *) R_GPT4_BASE)
#define R_GPT5    ((R_GPT0_Type *) R_GPT5_BASE)
#define R_GPT6    ((R_GPT0_Type *) R_GPT6_BASE)
#define R_GPT7    ((R_GPT0_Type *) R_GPT7_BASE)

/* R_SCI0_Type: Serial Communications Interface (SCI) registers */
/* All SCI channels on RA4M1 use the same register layout (0x34 bytes) */
typedef struct
{
    volatile uint8_t  SMR;        /* +0x00: Serial Mode */
    volatile uint8_t  BRR;        /* +0x01: Bit Rate */
    volatile uint8_t  SCR;        /* +0x02: Serial Control */
    volatile uint8_t  TDR;        /* +0x03: Transmit Data */
    volatile uint8_t  SSR;        /* +0x04: Serial Status */
    volatile const uint8_t  RDR;  /* +0x05: Receive Data */
    volatile uint8_t  SCMR;       /* +0x06: Smart Card Mode */
    volatile uint8_t  SEMR;       /* +0x07: Serial Extended Mode */
    volatile uint8_t  SNFR;       /* +0x08: Noise Filter */
    volatile uint8_t  SIMR1;      /* +0x09: I2C Mode 1 */
    volatile uint8_t  SIMR2;      /* +0x0A: I2C Mode 2 */
    volatile uint8_t  SIMR3;      /* +0x0B: I2C Mode 3 */
    volatile const uint8_t  SISR; /* +0x0C: I2C Status */
    volatile uint8_t  SPMR;       /* +0x0D: SPI Mode */
    volatile uint16_t TDRHL;      /* +0x0E: Transmit 9-bit / FIFO */
    volatile const uint16_t RDRHL; /* +0x10: Receive 9-bit / FIFO */
    volatile uint8_t  MDDR;       /* +0x12: Modulation Duty */
    volatile uint8_t  DCCR;       /* +0x13: Data Compare Control */
    volatile uint16_t FCR;        /* +0x14: FIFO Control */
    volatile const uint16_t FDR;  /* +0x16: FIFO Data Count */
    volatile const uint16_t LSR;  /* +0x18: Line Status */
    volatile uint16_t CDR;        /* +0x1A: Compare Match Data */
    volatile uint8_t  SPTR;       /* +0x1C: Serial Port */
    volatile uint8_t  ACTR;       /* +0x1D: Adjustment Timing */
} R_SCI0_Type;

/* SCI base addresses (stride 0x20 per channel) */
#define R_SCI0_BASE  0x40070000UL
#define R_SCI1_BASE  0x40070020UL
#define R_SCI2_BASE  0x40070040UL
#define R_SCI3_BASE  0x40070060UL

#define R_SCI0    ((R_SCI0_Type *) R_SCI0_BASE)
#define R_SCI1    ((R_SCI0_Type *) R_SCI1_BASE)
#define R_SCI2    ((R_SCI0_Type *) R_SCI2_BASE)
#define R_SCI3    ((R_SCI0_Type *) R_SCI3_BASE)

/* R_MSTP_Type: Module Stop Control registers */
typedef struct
{
    volatile uint32_t MSTPCRA;    /* +0x00 */
    volatile uint32_t MSTPCRB;    /* +0x04 */
    volatile uint32_t MSTPCRC;    /* +0x08 */
    volatile uint32_t MSTPCRD;    /* +0x0C */
    volatile uint32_t MSTPCRE;    /* +0x10 */
} R_MSTP_Type;

#define R_MSTP    ((R_MSTP_Type *)(0x40047000UL - 4UL))

#define R_MSTP_SCI1       (1UL << 30)  /* MSTPCRB bit 30 */
#define R_MSTP_GPT_LOWER  (1UL << 5)   /* MSTPCRD bit 5 (GPT2-7) */
#define R_MSTP_GPT_HIGHER (1UL << 6)   /* MSTPCRD bit 6 (GPT0-1) */
#define R_MSTP_GPT(n)     (1UL << (31 - (n))) /* MSTPCRE bit 31-n (GPT0..GPT9) */

#endif /* R7FA4M1AB_H */

/* BSP-compatible constants (used in PFS pin access) */
#define BSP_IO_PRV_8BIT_MASK  0xFF
#define BSP_IO_PFS_PDR_OUTPUT 4U

/* ======================================================================== */
/* Section 3: Clock and step-rate constants                                 */
/* ======================================================================== */

#ifndef F_CPU
#define F_CPU SystemCoreClock
#warning "F_CPU not defined as a constant. Cycle-accurate delays may be wrong"
#endif

#ifndef F_STEP_MAX
#define F_STEP_MAX 100000
#endif

#ifndef F_STEP_MIN
#define F_STEP_MIN 4
#endif

/* Peripheral clock defaults (RA4M1 with HOCO 48 MHz, no dividers) */
#ifndef MCU_PCLKB
#define MCU_PCLKB 48000000UL
#endif
#ifndef MCU_PCLKD
#define MCU_PCLKD 48000000UL
#endif

/* ======================================================================== */
/* Section 4: Delay constants                                               */
/* ======================================================================== */

#ifndef MCU_CYCLES_PER_LOOP
#define MCU_CYCLES_PER_LOOP 4
#endif

#ifndef MCU_CYCLES_LOOP_OVERHEAD
#define MCU_CYCLES_LOOP_OVERHEAD 1
#endif

#define mcu_delay_loop(X) \
	do \
	{ \
		asm volatile("" ::: "memory"); \
		register uint16_t __count = (X); \
		__asm__ volatile( \
			"1: sub %[cnt], %[cnt], #1\n" \
			"   cmp %[cnt], #0\n" \
			"   bne 1b\n" \
			"   nop\n" \
			: [cnt] "+r"(__count) \
			: \
			: "cc"); \
		asm volatile("" ::: "memory"); \
	} while (0)

/* ======================================================================== */
/* Section 5: NVIC/IRQ priority table                                       */
/* ======================================================================== */
/* Lower numeric value = higher priority on Cortex-M4 with 4-bit priority   */
/* Recommended ordering: input/ITP > RTC > servo/oneshot > comms            */

#define NVIC_INPUT_IRQ_Pri 1
#define NVIC_SPI_IRQ_Pri 3
#define NVIC_UART_IRQ_Pri 4
#define NVIC_ITP_IRQ_Pri 5
#define NVIC_ONESHOT_IRQ_Pri 6
#define NVIC_SERVO_IRQ_Pri 6
#define NVIC_RTC_IRQ_Pri 8
#define NVIC_I2C_IRQ_Pri 9
#define NVIC_USB_IRQ_Pri 10

/* ======================================================================== */
/* Section 6: ROM/flash string macros                                       */
/* ======================================================================== */
/* RA4M1 is von-Neumann (ARM), use RAM defaults from mcu.h                  */
/* These are provided as defaults in mcu.h, no override needed              */

/* ======================================================================== */
/* Section 7: Byte/bit operations                                           */
/* ======================================================================== */
/* Provided by mcu.h defaults (SETBIT, CLEARBIT, CHECKBIT, TOGGLEBIT,       */
/* SETFLAG, CLEARFLAG, CHECKFLAG, TOGGLEFLAG)                               */

/* ======================================================================== */
/* Section 8: Token-pasting helpers and RA4M1 register access               */
/* ======================================================================== */

/* Generic token helpers */
#define __helper_ex__(left, mid, right) left##mid##right
#define __helper__(left, mid, right)    __helper_ex__(left, mid, right)

#ifndef __indirect__
#define __indirect__ex__(X, Y) DIO##X##_##Y
#define __indirect__(X, Y)    __indirect__ex__(X, Y)
#endif

/* RA4M1 GPIO register access via FSP/BSP register structs */

/* PFS-based pin access for configuration (PmnPFS register) */
/* Uses BSP_IO_PRV_8BIT_MASK to mask pin index to 0-15 range */
#define __ra_pfs_pin__(X) R_PFS->PORT[__indirect__(X, PORT)].PIN[__indirect__(X, BIT) & BSP_IO_PRV_8BIT_MASK]

/* Fast GPIO atomic register access via R_PORT at computed base address */
/* R_PORT0_BASE is provided by CMSIS device header (FSP) or defined inline (bare-metal) */
#define __ra_port_ex__(X) (R_PORT##X)
#define __ra_port__(X) __ra_port_ex__(X)

/* PFS write protection unlock helper (call before PFS register writes) */
#define mcu_unlock_pfs() do { R_PMISC->PWPR = 0; R_PMISC->PWPR = (1U << 6); } while(0)

/* ======================================================================== */
/* Section 9: DIO pin table                                                 */
/* ======================================================================== */
/* Every canonical friendly pin gets a conditional block that defines the   */
/* friendly name, DIO<n> number, and PORT/BIT aliases.                      */
/* Boardmap definitions of <PIN>_PORT and <PIN>_BIT activate each block.    */

/* --- STEP0..STEP7 (DIO1..DIO8) --- */
#if (defined(STEP0_PORT) && defined(STEP0_BIT))
#define STEP0 1
#define DIO1 1
#define DIO1_PORT STEP0_PORT
#define DIO1_BIT STEP0_BIT
#endif
#if (defined(STEP1_PORT) && defined(STEP1_BIT))
#define STEP1 2
#define DIO2 2
#define DIO2_PORT STEP1_PORT
#define DIO2_BIT STEP1_BIT
#endif
#if (defined(STEP2_PORT) && defined(STEP2_BIT))
#define STEP2 3
#define DIO3 3
#define DIO3_PORT STEP2_PORT
#define DIO3_BIT STEP2_BIT
#endif
#if (defined(STEP3_PORT) && defined(STEP3_BIT))
#define STEP3 4
#define DIO4 4
#define DIO4_PORT STEP3_PORT
#define DIO4_BIT STEP3_BIT
#endif
#if (defined(STEP4_PORT) && defined(STEP4_BIT))
#define STEP4 5
#define DIO5 5
#define DIO5_PORT STEP4_PORT
#define DIO5_BIT STEP4_BIT
#endif
#if (defined(STEP5_PORT) && defined(STEP5_BIT))
#define STEP5 6
#define DIO6 6
#define DIO6_PORT STEP5_PORT
#define DIO6_BIT STEP5_BIT
#endif
#if (defined(STEP6_PORT) && defined(STEP6_BIT))
#define STEP6 7
#define DIO7 7
#define DIO7_PORT STEP6_PORT
#define DIO7_BIT STEP6_BIT
#endif
#if (defined(STEP7_PORT) && defined(STEP7_BIT))
#define STEP7 8
#define DIO8 8
#define DIO8_PORT STEP7_PORT
#define DIO8_BIT STEP7_BIT
#endif

/* --- DIR0..DIR7 (DIO9..DIO16) --- */
#if (defined(DIR0_PORT) && defined(DIR0_BIT))
#define DIR0 9
#define DIO9 9
#define DIO9_PORT DIR0_PORT
#define DIO9_BIT DIR0_BIT
#endif
#if (defined(DIR1_PORT) && defined(DIR1_BIT))
#define DIR1 10
#define DIO10 10
#define DIO10_PORT DIR1_PORT
#define DIO10_BIT DIR1_BIT
#endif
#if (defined(DIR2_PORT) && defined(DIR2_BIT))
#define DIR2 11
#define DIO11 11
#define DIO11_PORT DIR2_PORT
#define DIO11_BIT DIR2_BIT
#endif
#if (defined(DIR3_PORT) && defined(DIR3_BIT))
#define DIR3 12
#define DIO12 12
#define DIO12_PORT DIR3_PORT
#define DIO12_BIT DIR3_BIT
#endif
#if (defined(DIR4_PORT) && defined(DIR4_BIT))
#define DIR4 13
#define DIO13 13
#define DIO13_PORT DIR4_PORT
#define DIO13_BIT DIR4_BIT
#endif
#if (defined(DIR5_PORT) && defined(DIR5_BIT))
#define DIR5 14
#define DIO14 14
#define DIO14_PORT DIR5_PORT
#define DIO14_BIT DIR5_BIT
#endif
#if (defined(DIR6_PORT) && defined(DIR6_BIT))
#define DIR6 15
#define DIO15 15
#define DIO15_PORT DIR6_PORT
#define DIO15_BIT DIR6_BIT
#endif
#if (defined(DIR7_PORT) && defined(DIR7_BIT))
#define DIR7 16
#define DIO16 16
#define DIO16_PORT DIR7_PORT
#define DIO16_BIT DIR7_BIT
#endif

/* --- STEP0_EN..STEP7_EN (DIO17..DIO24) --- */
#if (defined(STEP0_EN_PORT) && defined(STEP0_EN_BIT))
#define STEP0_EN 17
#define DIO17 17
#define DIO17_PORT STEP0_EN_PORT
#define DIO17_BIT STEP0_EN_BIT
#endif
#if (defined(STEP1_EN_PORT) && defined(STEP1_EN_BIT))
#define STEP1_EN 18
#define DIO18 18
#define DIO18_PORT STEP1_EN_PORT
#define DIO18_BIT STEP1_EN_BIT
#endif
#if (defined(STEP2_EN_PORT) && defined(STEP2_EN_BIT))
#define STEP2_EN 19
#define DIO19 19
#define DIO19_PORT STEP2_EN_PORT
#define DIO19_BIT STEP2_EN_BIT
#endif
#if (defined(STEP3_EN_PORT) && defined(STEP3_EN_BIT))
#define STEP3_EN 20
#define DIO20 20
#define DIO20_PORT STEP3_EN_PORT
#define DIO20_BIT STEP3_EN_BIT
#endif
#if (defined(STEP4_EN_PORT) && defined(STEP4_EN_BIT))
#define STEP4_EN 21
#define DIO21 21
#define DIO21_PORT STEP4_EN_PORT
#define DIO21_BIT STEP4_EN_BIT
#endif
#if (defined(STEP5_EN_PORT) && defined(STEP5_EN_BIT))
#define STEP5_EN 22
#define DIO22 22
#define DIO22_PORT STEP5_EN_PORT
#define DIO22_BIT STEP5_EN_BIT
#endif
#if (defined(STEP6_EN_PORT) && defined(STEP6_EN_BIT))
#define STEP6_EN 23
#define DIO23 23
#define DIO23_PORT STEP6_EN_PORT
#define DIO23_BIT STEP6_EN_BIT
#endif
#if (defined(STEP7_EN_PORT) && defined(STEP7_EN_BIT))
#define STEP7_EN 24
#define DIO24 24
#define DIO24_PORT STEP7_EN_PORT
#define DIO24_BIT STEP7_EN_BIT
#endif

/* --- PWM0..PWM15 (DIO25..DIO40) --- */
#if (defined(PWM0_PORT) && defined(PWM0_BIT))
#define PWM0 25
#define DIO25 25
#define DIO25_PORT PWM0_PORT
#define DIO25_BIT PWM0_BIT
#endif
#if (defined(PWM1_PORT) && defined(PWM1_BIT))
#define PWM1 26
#define DIO26 26
#define DIO26_PORT PWM1_PORT
#define DIO26_BIT PWM1_BIT
#endif
#if (defined(PWM2_PORT) && defined(PWM2_BIT))
#define PWM2 27
#define DIO27 27
#define DIO27_PORT PWM2_PORT
#define DIO27_BIT PWM2_BIT
#endif
#if (defined(PWM3_PORT) && defined(PWM3_BIT))
#define PWM3 28
#define DIO28 28
#define DIO28_PORT PWM3_PORT
#define DIO28_BIT PWM3_BIT
#endif
#if (defined(PWM4_PORT) && defined(PWM4_BIT))
#define PWM4 29
#define DIO29 29
#define DIO29_PORT PWM4_PORT
#define DIO29_BIT PWM4_BIT
#endif
#if (defined(PWM5_PORT) && defined(PWM5_BIT))
#define PWM5 30
#define DIO30 30
#define DIO30_PORT PWM5_PORT
#define DIO30_BIT PWM5_BIT
#endif
#if (defined(PWM6_PORT) && defined(PWM6_BIT))
#define PWM6 31
#define DIO31 31
#define DIO31_PORT PWM6_PORT
#define DIO31_BIT PWM6_BIT
#endif
#if (defined(PWM7_PORT) && defined(PWM7_BIT))
#define PWM7 32
#define DIO32 32
#define DIO32_PORT PWM7_PORT
#define DIO32_BIT PWM7_BIT
#endif
#if (defined(PWM8_PORT) && defined(PWM8_BIT))
#define PWM8 33
#define DIO33 33
#define DIO33_PORT PWM8_PORT
#define DIO33_BIT PWM8_BIT
#endif
#if (defined(PWM9_PORT) && defined(PWM9_BIT))
#define PWM9 34
#define DIO34 34
#define DIO34_PORT PWM9_PORT
#define DIO34_BIT PWM9_BIT
#endif
#if (defined(PWM10_PORT) && defined(PWM10_BIT))
#define PWM10 35
#define DIO35 35
#define DIO35_PORT PWM10_PORT
#define DIO35_BIT PWM10_BIT
#endif
#if (defined(PWM11_PORT) && defined(PWM11_BIT))
#define PWM11 36
#define DIO36 36
#define DIO36_PORT PWM11_PORT
#define DIO36_BIT PWM11_BIT
#endif
#if (defined(PWM12_PORT) && defined(PWM12_BIT))
#define PWM12 37
#define DIO37 37
#define DIO37_PORT PWM12_PORT
#define DIO37_BIT PWM12_BIT
#endif
#if (defined(PWM13_PORT) && defined(PWM13_BIT))
#define PWM13 38
#define DIO38 38
#define DIO38_PORT PWM13_PORT
#define DIO38_BIT PWM13_BIT
#endif
#if (defined(PWM14_PORT) && defined(PWM14_BIT))
#define PWM14 39
#define DIO39 39
#define DIO39_PORT PWM14_PORT
#define DIO39_BIT PWM14_BIT
#endif
#if (defined(PWM15_PORT) && defined(PWM15_BIT))
#define PWM15 40
#define DIO40 40
#define DIO40_PORT PWM15_PORT
#define DIO40_BIT PWM15_BIT
#endif

/* --- SERVO0..SERVO5 (DIO41..DIO46) --- */
#if (defined(SERVO0_PORT) && defined(SERVO0_BIT))
#define SERVO0 41
#define DIO41 41
#define DIO41_PORT SERVO0_PORT
#define DIO41_BIT SERVO0_BIT
#endif
#if (defined(SERVO1_PORT) && defined(SERVO1_BIT))
#define SERVO1 42
#define DIO42 42
#define DIO42_PORT SERVO1_PORT
#define DIO42_BIT SERVO1_BIT
#endif
#if (defined(SERVO2_PORT) && defined(SERVO2_BIT))
#define SERVO2 43
#define DIO43 43
#define DIO43_PORT SERVO2_PORT
#define DIO43_BIT SERVO2_BIT
#endif
#if (defined(SERVO3_PORT) && defined(SERVO3_BIT))
#define SERVO3 44
#define DIO44 44
#define DIO44_PORT SERVO3_PORT
#define DIO44_BIT SERVO3_BIT
#endif
#if (defined(SERVO4_PORT) && defined(SERVO4_BIT))
#define SERVO4 45
#define DIO45 45
#define DIO45_PORT SERVO4_PORT
#define DIO45_BIT SERVO4_BIT
#endif
#if (defined(SERVO5_PORT) && defined(SERVO5_BIT))
#define SERVO5 46
#define DIO46 46
#define DIO46_PORT SERVO5_PORT
#define DIO46_BIT SERVO5_BIT
#endif

/* --- DOUT0..DOUT49 (DIO47..DIO96) --- */
#if (defined(DOUT0_PORT) && defined(DOUT0_BIT))
#define DOUT0 47
#define DIO47 47
#define DIO47_PORT DOUT0_PORT
#define DIO47_BIT DOUT0_BIT
#endif
#if (defined(DOUT1_PORT) && defined(DOUT1_BIT))
#define DOUT1 48
#define DIO48 48
#define DIO48_PORT DOUT1_PORT
#define DIO48_BIT DOUT1_BIT
#endif
#if (defined(DOUT2_PORT) && defined(DOUT2_BIT))
#define DOUT2 49
#define DIO49 49
#define DIO49_PORT DOUT2_PORT
#define DIO49_BIT DOUT2_BIT
#endif
#if (defined(DOUT3_PORT) && defined(DOUT3_BIT))
#define DOUT3 50
#define DIO50 50
#define DIO50_PORT DOUT3_PORT
#define DIO50_BIT DOUT3_BIT
#endif
#if (defined(DOUT4_PORT) && defined(DOUT4_BIT))
#define DOUT4 51
#define DIO51 51
#define DIO51_PORT DOUT4_PORT
#define DIO51_BIT DOUT4_BIT
#endif
#if (defined(DOUT5_PORT) && defined(DOUT5_BIT))
#define DOUT5 52
#define DIO52 52
#define DIO52_PORT DOUT5_PORT
#define DIO52_BIT DOUT5_BIT
#endif
#if (defined(DOUT6_PORT) && defined(DOUT6_BIT))
#define DOUT6 53
#define DIO53 53
#define DIO53_PORT DOUT6_PORT
#define DIO53_BIT DOUT6_BIT
#endif
#if (defined(DOUT7_PORT) && defined(DOUT7_BIT))
#define DOUT7 54
#define DIO54 54
#define DIO54_PORT DOUT7_PORT
#define DIO54_BIT DOUT7_BIT
#endif
#if (defined(DOUT8_PORT) && defined(DOUT8_BIT))
#define DOUT8 55
#define DIO55 55
#define DIO55_PORT DOUT8_PORT
#define DIO55_BIT DOUT8_BIT
#endif
#if (defined(DOUT9_PORT) && defined(DOUT9_BIT))
#define DOUT9 56
#define DIO56 56
#define DIO56_PORT DOUT9_PORT
#define DIO56_BIT DOUT9_BIT
#endif
#if (defined(DOUT10_PORT) && defined(DOUT10_BIT))
#define DOUT10 57
#define DIO57 57
#define DIO57_PORT DOUT10_PORT
#define DIO57_BIT DOUT10_BIT
#endif
#if (defined(DOUT11_PORT) && defined(DOUT11_BIT))
#define DOUT11 58
#define DIO58 58
#define DIO58_PORT DOUT11_PORT
#define DIO58_BIT DOUT11_BIT
#endif
#if (defined(DOUT12_PORT) && defined(DOUT12_BIT))
#define DOUT12 59
#define DIO59 59
#define DIO59_PORT DOUT12_PORT
#define DIO59_BIT DOUT12_BIT
#endif
#if (defined(DOUT13_PORT) && defined(DOUT13_BIT))
#define DOUT13 60
#define DIO60 60
#define DIO60_PORT DOUT13_PORT
#define DIO60_BIT DOUT13_BIT
#endif
#if (defined(DOUT14_PORT) && defined(DOUT14_BIT))
#define DOUT14 61
#define DIO61 61
#define DIO61_PORT DOUT14_PORT
#define DIO61_BIT DOUT14_BIT
#endif
#if (defined(DOUT15_PORT) && defined(DOUT15_BIT))
#define DOUT15 62
#define DIO62 62
#define DIO62_PORT DOUT15_PORT
#define DIO62_BIT DOUT15_BIT
#endif
#if (defined(DOUT16_PORT) && defined(DOUT16_BIT))
#define DOUT16 63
#define DIO63 63
#define DIO63_PORT DOUT16_PORT
#define DIO63_BIT DOUT16_BIT
#endif
#if (defined(DOUT17_PORT) && defined(DOUT17_BIT))
#define DOUT17 64
#define DIO64 64
#define DIO64_PORT DOUT17_PORT
#define DIO64_BIT DOUT17_BIT
#endif
#if (defined(DOUT18_PORT) && defined(DOUT18_BIT))
#define DOUT18 65
#define DIO65 65
#define DIO65_PORT DOUT18_PORT
#define DIO65_BIT DOUT18_BIT
#endif
#if (defined(DOUT19_PORT) && defined(DOUT19_BIT))
#define DOUT19 66
#define DIO66 66
#define DIO66_PORT DOUT19_PORT
#define DIO66_BIT DOUT19_BIT
#endif
#if (defined(DOUT20_PORT) && defined(DOUT20_BIT))
#define DOUT20 67
#define DIO67 67
#define DIO67_PORT DOUT20_PORT
#define DIO67_BIT DOUT20_BIT
#endif
#if (defined(DOUT21_PORT) && defined(DOUT21_BIT))
#define DOUT21 68
#define DIO68 68
#define DIO68_PORT DOUT21_PORT
#define DIO68_BIT DOUT21_BIT
#endif
#if (defined(DOUT22_PORT) && defined(DOUT22_BIT))
#define DOUT22 69
#define DIO69 69
#define DIO69_PORT DOUT22_PORT
#define DIO69_BIT DOUT22_BIT
#endif
#if (defined(DOUT23_PORT) && defined(DOUT23_BIT))
#define DOUT23 70
#define DIO70 70
#define DIO70_PORT DOUT23_PORT
#define DIO70_BIT DOUT23_BIT
#endif
#if (defined(DOUT24_PORT) && defined(DOUT24_BIT))
#define DOUT24 71
#define DIO71 71
#define DIO71_PORT DOUT24_PORT
#define DIO71_BIT DOUT24_BIT
#endif
#if (defined(DOUT25_PORT) && defined(DOUT25_BIT))
#define DOUT25 72
#define DIO72 72
#define DIO72_PORT DOUT25_PORT
#define DIO72_BIT DOUT25_BIT
#endif
#if (defined(DOUT26_PORT) && defined(DOUT26_BIT))
#define DOUT26 73
#define DIO73 73
#define DIO73_PORT DOUT26_PORT
#define DIO73_BIT DOUT26_BIT
#endif
#if (defined(DOUT27_PORT) && defined(DOUT27_BIT))
#define DOUT27 74
#define DIO74 74
#define DIO74_PORT DOUT27_PORT
#define DIO74_BIT DOUT27_BIT
#endif
#if (defined(DOUT28_PORT) && defined(DOUT28_BIT))
#define DOUT28 75
#define DIO75 75
#define DIO75_PORT DOUT28_PORT
#define DIO75_BIT DOUT28_BIT
#endif
#if (defined(DOUT29_PORT) && defined(DOUT29_BIT))
#define DOUT29 76
#define DIO76 76
#define DIO76_PORT DOUT29_PORT
#define DIO76_BIT DOUT29_BIT
#endif
#if (defined(DOUT30_PORT) && defined(DOUT30_BIT))
#define DOUT30 77
#define DIO77 77
#define DIO77_PORT DOUT30_PORT
#define DIO77_BIT DOUT30_BIT
#endif
#if (defined(DOUT31_PORT) && defined(DOUT31_BIT))
#define DOUT31 78
#define DIO78 78
#define DIO78_PORT DOUT31_PORT
#define DIO78_BIT DOUT31_BIT
#endif
#if (defined(DOUT32_PORT) && defined(DOUT32_BIT))
#define DOUT32 79
#define DIO79 79
#define DIO79_PORT DOUT32_PORT
#define DIO79_BIT DOUT32_BIT
#endif
#if (defined(DOUT33_PORT) && defined(DOUT33_BIT))
#define DOUT33 80
#define DIO80 80
#define DIO80_PORT DOUT33_PORT
#define DIO80_BIT DOUT33_BIT
#endif
#if (defined(DOUT34_PORT) && defined(DOUT34_BIT))
#define DOUT34 81
#define DIO81 81
#define DIO81_PORT DOUT34_PORT
#define DIO81_BIT DOUT34_BIT
#endif
#if (defined(DOUT35_PORT) && defined(DOUT35_BIT))
#define DOUT35 82
#define DIO82 82
#define DIO82_PORT DOUT35_PORT
#define DIO82_BIT DOUT35_BIT
#endif
#if (defined(DOUT36_PORT) && defined(DOUT36_BIT))
#define DOUT36 83
#define DIO83 83
#define DIO83_PORT DOUT36_PORT
#define DIO83_BIT DOUT36_BIT
#endif
#if (defined(DOUT37_PORT) && defined(DOUT37_BIT))
#define DOUT37 84
#define DIO84 84
#define DIO84_PORT DOUT37_PORT
#define DIO84_BIT DOUT37_BIT
#endif
#if (defined(DOUT38_PORT) && defined(DOUT38_BIT))
#define DOUT38 85
#define DIO85 85
#define DIO85_PORT DOUT38_PORT
#define DIO85_BIT DOUT38_BIT
#endif
#if (defined(DOUT39_PORT) && defined(DOUT39_BIT))
#define DOUT39 86
#define DIO86 86
#define DIO86_PORT DOUT39_PORT
#define DIO86_BIT DOUT39_BIT
#endif
#if (defined(DOUT40_PORT) && defined(DOUT40_BIT))
#define DOUT40 87
#define DIO87 87
#define DIO87_PORT DOUT40_PORT
#define DIO87_BIT DOUT40_BIT
#endif
#if (defined(DOUT41_PORT) && defined(DOUT41_BIT))
#define DOUT41 88
#define DIO88 88
#define DIO88_PORT DOUT41_PORT
#define DIO88_BIT DOUT41_BIT
#endif
#if (defined(DOUT42_PORT) && defined(DOUT42_BIT))
#define DOUT42 89
#define DIO89 89
#define DIO89_PORT DOUT42_PORT
#define DIO89_BIT DOUT42_BIT
#endif
#if (defined(DOUT43_PORT) && defined(DOUT43_BIT))
#define DOUT43 90
#define DIO90 90
#define DIO90_PORT DOUT43_PORT
#define DIO90_BIT DOUT43_BIT
#endif
#if (defined(DOUT44_PORT) && defined(DOUT44_BIT))
#define DOUT44 91
#define DIO91 91
#define DIO91_PORT DOUT44_PORT
#define DIO91_BIT DOUT44_BIT
#endif
#if (defined(DOUT45_PORT) && defined(DOUT45_BIT))
#define DOUT45 92
#define DIO92 92
#define DIO92_PORT DOUT45_PORT
#define DIO92_BIT DOUT45_BIT
#endif
#if (defined(DOUT46_PORT) && defined(DOUT46_BIT))
#define DOUT46 93
#define DIO93 93
#define DIO93_PORT DOUT46_PORT
#define DIO93_BIT DOUT46_BIT
#endif
#if (defined(DOUT47_PORT) && defined(DOUT47_BIT))
#define DOUT47 94
#define DIO94 94
#define DIO94_PORT DOUT47_PORT
#define DIO94_BIT DOUT47_BIT
#endif
#if (defined(DOUT48_PORT) && defined(DOUT48_BIT))
#define DOUT48 95
#define DIO95 95
#define DIO95_PORT DOUT48_PORT
#define DIO95_BIT DOUT48_BIT
#endif
#if (defined(DOUT49_PORT) && defined(DOUT49_BIT))
#define DOUT49 96
#define DIO96 96
#define DIO96_PORT DOUT49_PORT
#define DIO96_BIT DOUT49_BIT
#endif

/* --- Canonical gap: DIO97..DIO99 unused --- */
/* --- LIMIT_X..LIMIT_C (DIO100..DIO108) --- */
#if (defined(LIMIT_X_PORT) && defined(LIMIT_X_BIT))
#define LIMIT_X 100
#define DIO100 100
#define DIO100_PORT LIMIT_X_PORT
#define DIO100_BIT LIMIT_X_BIT
#endif
#if (defined(LIMIT_Y_PORT) && defined(LIMIT_Y_BIT))
#define LIMIT_Y 101
#define DIO101 101
#define DIO101_PORT LIMIT_Y_PORT
#define DIO101_BIT LIMIT_Y_BIT
#endif
#if (defined(LIMIT_Z_PORT) && defined(LIMIT_Z_BIT))
#define LIMIT_Z 102
#define DIO102 102
#define DIO102_PORT LIMIT_Z_PORT
#define DIO102_BIT LIMIT_Z_BIT
#endif
#if (defined(LIMIT_X2_PORT) && defined(LIMIT_X2_BIT))
#define LIMIT_X2 103
#define DIO103 103
#define DIO103_PORT LIMIT_X2_PORT
#define DIO103_BIT LIMIT_X2_BIT
#endif
#if (defined(LIMIT_Y2_PORT) && defined(LIMIT_Y2_BIT))
#define LIMIT_Y2 104
#define DIO104 104
#define DIO104_PORT LIMIT_Y2_PORT
#define DIO104_BIT LIMIT_Y2_BIT
#endif
#if (defined(LIMIT_Z2_PORT) && defined(LIMIT_Z2_BIT))
#define LIMIT_Z2 105
#define DIO105 105
#define DIO105_PORT LIMIT_Z2_PORT
#define DIO105_BIT LIMIT_Z2_BIT
#endif
#if (defined(LIMIT_A_PORT) && defined(LIMIT_A_BIT))
#define LIMIT_A 106
#define DIO106 106
#define DIO106_PORT LIMIT_A_PORT
#define DIO106_BIT LIMIT_A_BIT
#endif
#if (defined(LIMIT_B_PORT) && defined(LIMIT_B_BIT))
#define LIMIT_B 107
#define DIO107 107
#define DIO107_PORT LIMIT_B_PORT
#define DIO107_BIT LIMIT_B_BIT
#endif
#if (defined(LIMIT_C_PORT) && defined(LIMIT_C_BIT))
#define LIMIT_C 108
#define DIO108 108
#define DIO108_PORT LIMIT_C_PORT
#define DIO108_BIT LIMIT_C_BIT
#endif

/* --- PROBE (DIO109) --- */
#if (defined(PROBE_PORT) && defined(PROBE_BIT))
#define PROBE 109
#define DIO109 109
#define DIO109_PORT PROBE_PORT
#define DIO109_BIT PROBE_BIT
#endif

/* --- ESTOP, SAFETY_DOOR, FHOLD, CS_RES (DIO110..DIO113) --- */
#if (defined(ESTOP_PORT) && defined(ESTOP_BIT))
#define ESTOP 110
#define DIO110 110
#define DIO110_PORT ESTOP_PORT
#define DIO110_BIT ESTOP_BIT
#endif
#if (defined(SAFETY_DOOR_PORT) && defined(SAFETY_DOOR_BIT))
#define SAFETY_DOOR 111
#define DIO111 111
#define DIO111_PORT SAFETY_DOOR_PORT
#define DIO111_BIT SAFETY_DOOR_BIT
#endif
#if (defined(FHOLD_PORT) && defined(FHOLD_BIT))
#define FHOLD 112
#define DIO112 112
#define DIO112_PORT FHOLD_PORT
#define DIO112_BIT FHOLD_BIT
#endif
#if (defined(CS_RES_PORT) && defined(CS_RES_BIT))
#define CS_RES 113
#define DIO113 113
#define DIO113_PORT CS_RES_PORT
#define DIO113_BIT CS_RES_BIT
#endif

/* --- ANALOG0..ANALOG15 (DIO114..DIO129) --- */
#if (defined(ANALOG0_PORT) && defined(ANALOG0_BIT))
#define ANALOG0 114
#define DIO114 114
#define DIO114_PORT ANALOG0_PORT
#define DIO114_BIT ANALOG0_BIT
#endif
#if (defined(ANALOG1_PORT) && defined(ANALOG1_BIT))
#define ANALOG1 115
#define DIO115 115
#define DIO115_PORT ANALOG1_PORT
#define DIO115_BIT ANALOG1_BIT
#endif
#if (defined(ANALOG2_PORT) && defined(ANALOG2_BIT))
#define ANALOG2 116
#define DIO116 116
#define DIO116_PORT ANALOG2_PORT
#define DIO116_BIT ANALOG2_BIT
#endif
#if (defined(ANALOG3_PORT) && defined(ANALOG3_BIT))
#define ANALOG3 117
#define DIO117 117
#define DIO117_PORT ANALOG3_PORT
#define DIO117_BIT ANALOG3_BIT
#endif
#if (defined(ANALOG4_PORT) && defined(ANALOG4_BIT))
#define ANALOG4 118
#define DIO118 118
#define DIO118_PORT ANALOG4_PORT
#define DIO118_BIT ANALOG4_BIT
#endif
#if (defined(ANALOG5_PORT) && defined(ANALOG5_BIT))
#define ANALOG5 119
#define DIO119 119
#define DIO119_PORT ANALOG5_PORT
#define DIO119_BIT ANALOG5_BIT
#endif
#if (defined(ANALOG6_PORT) && defined(ANALOG6_BIT))
#define ANALOG6 120
#define DIO120 120
#define DIO120_PORT ANALOG6_PORT
#define DIO120_BIT ANALOG6_BIT
#endif
#if (defined(ANALOG7_PORT) && defined(ANALOG7_BIT))
#define ANALOG7 121
#define DIO121 121
#define DIO121_PORT ANALOG7_PORT
#define DIO121_BIT ANALOG7_BIT
#endif
#if (defined(ANALOG8_PORT) && defined(ANALOG8_BIT))
#define ANALOG8 122
#define DIO122 122
#define DIO122_PORT ANALOG8_PORT
#define DIO122_BIT ANALOG8_BIT
#endif
#if (defined(ANALOG9_PORT) && defined(ANALOG9_BIT))
#define ANALOG9 123
#define DIO123 123
#define DIO123_PORT ANALOG9_PORT
#define DIO123_BIT ANALOG9_BIT
#endif
#if (defined(ANALOG10_PORT) && defined(ANALOG10_BIT))
#define ANALOG10 124
#define DIO124 124
#define DIO124_PORT ANALOG10_PORT
#define DIO124_BIT ANALOG10_BIT
#endif
#if (defined(ANALOG11_PORT) && defined(ANALOG11_BIT))
#define ANALOG11 125
#define DIO125 125
#define DIO125_PORT ANALOG11_PORT
#define DIO125_BIT ANALOG11_BIT
#endif
#if (defined(ANALOG12_PORT) && defined(ANALOG12_BIT))
#define ANALOG12 126
#define DIO126 126
#define DIO126_PORT ANALOG12_PORT
#define DIO126_BIT ANALOG12_BIT
#endif
#if (defined(ANALOG13_PORT) && defined(ANALOG13_BIT))
#define ANALOG13 127
#define DIO127 127
#define DIO127_PORT ANALOG13_PORT
#define DIO127_BIT ANALOG13_BIT
#endif
#if (defined(ANALOG14_PORT) && defined(ANALOG14_BIT))
#define ANALOG14 128
#define DIO128 128
#define DIO128_PORT ANALOG14_PORT
#define DIO128_BIT ANALOG14_BIT
#endif
#if (defined(ANALOG15_PORT) && defined(ANALOG15_BIT))
#define ANALOG15 129
#define DIO129 129
#define DIO129_PORT ANALOG15_PORT
#define DIO129_BIT ANALOG15_BIT
#endif

/* --- DIN0..DIN49 (DIO130..DIO179) --- */
#if (defined(DIN0_PORT) && defined(DIN0_BIT))
#define DIN0 130
#define DIO130 130
#define DIO130_PORT DIN0_PORT
#define DIO130_BIT DIN0_BIT
#endif
#if (defined(DIN1_PORT) && defined(DIN1_BIT))
#define DIN1 131
#define DIO131 131
#define DIO131_PORT DIN1_PORT
#define DIO131_BIT DIN1_BIT
#endif
#if (defined(DIN2_PORT) && defined(DIN2_BIT))
#define DIN2 132
#define DIO132 132
#define DIO132_PORT DIN2_PORT
#define DIO132_BIT DIN2_BIT
#endif
#if (defined(DIN3_PORT) && defined(DIN3_BIT))
#define DIN3 133
#define DIO133 133
#define DIO133_PORT DIN3_PORT
#define DIO133_BIT DIN3_BIT
#endif
#if (defined(DIN4_PORT) && defined(DIN4_BIT))
#define DIN4 134
#define DIO134 134
#define DIO134_PORT DIN4_PORT
#define DIO134_BIT DIN4_BIT
#endif
#if (defined(DIN5_PORT) && defined(DIN5_BIT))
#define DIN5 135
#define DIO135 135
#define DIO135_PORT DIN5_PORT
#define DIO135_BIT DIN5_BIT
#endif
#if (defined(DIN6_PORT) && defined(DIN6_BIT))
#define DIN6 136
#define DIO136 136
#define DIO136_PORT DIN6_PORT
#define DIO136_BIT DIN6_BIT
#endif
#if (defined(DIN7_PORT) && defined(DIN7_BIT))
#define DIN7 137
#define DIO137 137
#define DIO137_PORT DIN7_PORT
#define DIO137_BIT DIN7_BIT
#endif
#if (defined(DIN8_PORT) && defined(DIN8_BIT))
#define DIN8 138
#define DIO138 138
#define DIO138_PORT DIN8_PORT
#define DIO138_BIT DIN8_BIT
#endif
#if (defined(DIN9_PORT) && defined(DIN9_BIT))
#define DIN9 139
#define DIO139 139
#define DIO139_PORT DIN9_PORT
#define DIO139_BIT DIN9_BIT
#endif
#if (defined(DIN10_PORT) && defined(DIN10_BIT))
#define DIN10 140
#define DIO140 140
#define DIO140_PORT DIN10_PORT
#define DIO140_BIT DIN10_BIT
#endif
#if (defined(DIN11_PORT) && defined(DIN11_BIT))
#define DIN11 141
#define DIO141 141
#define DIO141_PORT DIN11_PORT
#define DIO141_BIT DIN11_BIT
#endif
#if (defined(DIN12_PORT) && defined(DIN12_BIT))
#define DIN12 142
#define DIO142 142
#define DIO142_PORT DIN12_PORT
#define DIO142_BIT DIN12_BIT
#endif
#if (defined(DIN13_PORT) && defined(DIN13_BIT))
#define DIN13 143
#define DIO143 143
#define DIO143_PORT DIN13_PORT
#define DIO143_BIT DIN13_BIT
#endif
#if (defined(DIN14_PORT) && defined(DIN14_BIT))
#define DIN14 144
#define DIO144 144
#define DIO144_PORT DIN14_PORT
#define DIO144_BIT DIN14_BIT
#endif
#if (defined(DIN15_PORT) && defined(DIN15_BIT))
#define DIN15 145
#define DIO145 145
#define DIO145_PORT DIN15_PORT
#define DIO145_BIT DIN15_BIT
#endif
#if (defined(DIN16_PORT) && defined(DIN16_BIT))
#define DIN16 146
#define DIO146 146
#define DIO146_PORT DIN16_PORT
#define DIO146_BIT DIN16_BIT
#endif
#if (defined(DIN17_PORT) && defined(DIN17_BIT))
#define DIN17 147
#define DIO147 147
#define DIO147_PORT DIN17_PORT
#define DIO147_BIT DIN17_BIT
#endif
#if (defined(DIN18_PORT) && defined(DIN18_BIT))
#define DIN18 148
#define DIO148 148
#define DIO148_PORT DIN18_PORT
#define DIO148_BIT DIN18_BIT
#endif
#if (defined(DIN19_PORT) && defined(DIN19_BIT))
#define DIN19 149
#define DIO149 149
#define DIO149_PORT DIN19_PORT
#define DIO149_BIT DIN19_BIT
#endif
#if (defined(DIN20_PORT) && defined(DIN20_BIT))
#define DIN20 150
#define DIO150 150
#define DIO150_PORT DIN20_PORT
#define DIO150_BIT DIN20_BIT
#endif
#if (defined(DIN21_PORT) && defined(DIN21_BIT))
#define DIN21 151
#define DIO151 151
#define DIO151_PORT DIN21_PORT
#define DIO151_BIT DIN21_BIT
#endif
#if (defined(DIN22_PORT) && defined(DIN22_BIT))
#define DIN22 152
#define DIO152 152
#define DIO152_PORT DIN22_PORT
#define DIO152_BIT DIN22_BIT
#endif
#if (defined(DIN23_PORT) && defined(DIN23_BIT))
#define DIN23 153
#define DIO153 153
#define DIO153_PORT DIN23_PORT
#define DIO153_BIT DIN23_BIT
#endif
#if (defined(DIN24_PORT) && defined(DIN24_BIT))
#define DIN24 154
#define DIO154 154
#define DIO154_PORT DIN24_PORT
#define DIO154_BIT DIN24_BIT
#endif
#if (defined(DIN25_PORT) && defined(DIN25_BIT))
#define DIN25 155
#define DIO155 155
#define DIO155_PORT DIN25_PORT
#define DIO155_BIT DIN25_BIT
#endif
#if (defined(DIN26_PORT) && defined(DIN26_BIT))
#define DIN26 156
#define DIO156 156
#define DIO156_PORT DIN26_PORT
#define DIO156_BIT DIN26_BIT
#endif
#if (defined(DIN27_PORT) && defined(DIN27_BIT))
#define DIN27 157
#define DIO157 157
#define DIO157_PORT DIN27_PORT
#define DIO157_BIT DIN27_BIT
#endif
#if (defined(DIN28_PORT) && defined(DIN28_BIT))
#define DIN28 158
#define DIO158 158
#define DIO158_PORT DIN28_PORT
#define DIO158_BIT DIN28_BIT
#endif
#if (defined(DIN29_PORT) && defined(DIN29_BIT))
#define DIN29 159
#define DIO159 159
#define DIO159_PORT DIN29_PORT
#define DIO159_BIT DIN29_BIT
#endif
#if (defined(DIN30_PORT) && defined(DIN30_BIT))
#define DIN30 160
#define DIO160 160
#define DIO160_PORT DIN30_PORT
#define DIO160_BIT DIN30_BIT
#endif
#if (defined(DIN31_PORT) && defined(DIN31_BIT))
#define DIN31 161
#define DIO161 161
#define DIO161_PORT DIN31_PORT
#define DIO161_BIT DIN31_BIT
#endif
#if (defined(DIN32_PORT) && defined(DIN32_BIT))
#define DIN32 162
#define DIO162 162
#define DIO162_PORT DIN32_PORT
#define DIO162_BIT DIN32_BIT
#endif
#if (defined(DIN33_PORT) && defined(DIN33_BIT))
#define DIN33 163
#define DIO163 163
#define DIO163_PORT DIN33_PORT
#define DIO163_BIT DIN33_BIT
#endif
#if (defined(DIN34_PORT) && defined(DIN34_BIT))
#define DIN34 164
#define DIO164 164
#define DIO164_PORT DIN34_PORT
#define DIO164_BIT DIN34_BIT
#endif
#if (defined(DIN35_PORT) && defined(DIN35_BIT))
#define DIN35 165
#define DIO165 165
#define DIO165_PORT DIN35_PORT
#define DIO165_BIT DIN35_BIT
#endif
#if (defined(DIN36_PORT) && defined(DIN36_BIT))
#define DIN36 166
#define DIO166 166
#define DIO166_PORT DIN36_PORT
#define DIO166_BIT DIN36_BIT
#endif
#if (defined(DIN37_PORT) && defined(DIN37_BIT))
#define DIN37 167
#define DIO167 167
#define DIO167_PORT DIN37_PORT
#define DIO167_BIT DIN37_BIT
#endif
#if (defined(DIN38_PORT) && defined(DIN38_BIT))
#define DIN38 168
#define DIO168 168
#define DIO168_PORT DIN38_PORT
#define DIO168_BIT DIN38_BIT
#endif
#if (defined(DIN39_PORT) && defined(DIN39_BIT))
#define DIN39 169
#define DIO169 169
#define DIO169_PORT DIN39_PORT
#define DIO169_BIT DIN39_BIT
#endif
#if (defined(DIN40_PORT) && defined(DIN40_BIT))
#define DIN40 170
#define DIO170 170
#define DIO170_PORT DIN40_PORT
#define DIO170_BIT DIN40_BIT
#endif
#if (defined(DIN41_PORT) && defined(DIN41_BIT))
#define DIN41 171
#define DIO171 171
#define DIO171_PORT DIN41_PORT
#define DIO171_BIT DIN41_BIT
#endif
#if (defined(DIN42_PORT) && defined(DIN42_BIT))
#define DIN42 172
#define DIO172 172
#define DIO172_PORT DIN42_PORT
#define DIO172_BIT DIN42_BIT
#endif
#if (defined(DIN43_PORT) && defined(DIN43_BIT))
#define DIN43 173
#define DIO173 173
#define DIO173_PORT DIN43_PORT
#define DIO173_BIT DIN43_BIT
#endif
#if (defined(DIN44_PORT) && defined(DIN44_BIT))
#define DIN44 174
#define DIO174 174
#define DIO174_PORT DIN44_PORT
#define DIO174_BIT DIN44_BIT
#endif
#if (defined(DIN45_PORT) && defined(DIN45_BIT))
#define DIN45 175
#define DIO175 175
#define DIO175_PORT DIN45_PORT
#define DIO175_BIT DIN45_BIT
#endif
#if (defined(DIN46_PORT) && defined(DIN46_BIT))
#define DIN46 176
#define DIO176 176
#define DIO176_PORT DIN46_PORT
#define DIO176_BIT DIN46_BIT
#endif
#if (defined(DIN47_PORT) && defined(DIN47_BIT))
#define DIN47 177
#define DIO177 177
#define DIO177_PORT DIN47_PORT
#define DIO177_BIT DIN47_BIT
#endif
#if (defined(DIN48_PORT) && defined(DIN48_BIT))
#define DIN48 178
#define DIO178 178
#define DIO178_PORT DIN48_PORT
#define DIO178_BIT DIN48_BIT
#endif
#if (defined(DIN49_PORT) && defined(DIN49_BIT))
#define DIN49 179
#define DIO179 179
#define DIO179_PORT DIN49_PORT
#define DIO179_BIT DIN49_BIT
#endif

/* --- Canonical gap: DIO180..DIO199 unused --- */
/* --- COM pins (DIO200..DIO215) --- */
#if (defined(TX_PORT) && defined(TX_BIT))
#define TX 200
#define DIO200 200
#define DIO200_PORT TX_PORT
#define DIO200_BIT TX_BIT
#endif
#if (defined(RX_PORT) && defined(RX_BIT))
#define RX 201
#define DIO201 201
#define DIO201_PORT RX_PORT
#define DIO201_BIT RX_BIT
#endif
#if (defined(USB_DM_PORT) && defined(USB_DM_BIT))
#define USB_DM 202
#define DIO202 202
#define DIO202_PORT USB_DM_PORT
#define DIO202_BIT USB_DM_BIT
#endif
#if (defined(USB_DP_PORT) && defined(USB_DP_BIT))
#define USB_DP 203
#define DIO203 203
#define DIO203_PORT USB_DP_PORT
#define DIO203_BIT USB_DP_BIT
#endif
#if (defined(SPI_CLK_PORT) && defined(SPI_CLK_BIT))
#define SPI_CLK 204
#define DIO204 204
#define DIO204_PORT SPI_CLK_PORT
#define DIO204_BIT SPI_CLK_BIT
#endif
#if (defined(SPI_SDI_PORT) && defined(SPI_SDI_BIT))
#define SPI_SDI 205
#define DIO205 205
#define DIO205_PORT SPI_SDI_PORT
#define DIO205_BIT SPI_SDI_BIT
#endif
#if (defined(SPI_SDO_PORT) && defined(SPI_SDO_BIT))
#define SPI_SDO 206
#define DIO206 206
#define DIO206_PORT SPI_SDO_PORT
#define DIO206_BIT SPI_SDO_BIT
#endif
#if (defined(SPI_CS_PORT) && defined(SPI_CS_BIT))
#define SPI_CS 207
#define DIO207 207
#define DIO207_PORT SPI_CS_PORT
#define DIO207_BIT SPI_CS_BIT
#endif
#if (defined(I2C_CLK_PORT) && defined(I2C_CLK_BIT))
#define I2C_CLK 208
#define DIO208 208
#define DIO208_PORT I2C_CLK_PORT
#define DIO208_BIT I2C_CLK_BIT
#endif
#if (defined(I2C_DATA_PORT) && defined(I2C_DATA_BIT))
#define I2C_DATA 209
#define DIO209 209
#define DIO209_PORT I2C_DATA_PORT
#define DIO209_BIT I2C_DATA_BIT
#endif
#if (defined(TX2_PORT) && defined(TX2_BIT))
#define TX2 210
#define DIO210 210
#define DIO210_PORT TX2_PORT
#define DIO210_BIT TX2_BIT
#endif
#if (defined(RX2_PORT) && defined(RX2_BIT))
#define RX2 211
#define DIO211 211
#define DIO211_PORT RX2_PORT
#define DIO211_BIT RX2_BIT
#endif
#if (defined(SPI2_CLK_PORT) && defined(SPI2_CLK_BIT))
#define SPI2_CLK 212
#define DIO212 212
#define DIO212_PORT SPI2_CLK_PORT
#define DIO212_BIT SPI2_CLK_BIT
#endif
#if (defined(SPI2_SDI_PORT) && defined(SPI2_SDI_BIT))
#define SPI2_SDI 213
#define DIO213 213
#define DIO213_PORT SPI2_SDI_PORT
#define DIO213_BIT SPI2_SDI_BIT
#endif
#if (defined(SPI2_SDO_PORT) && defined(SPI2_SDO_BIT))
#define SPI2_SDO 214
#define DIO214 214
#define DIO214_PORT SPI2_SDO_PORT
#define DIO214_BIT SPI2_SDO_BIT
#endif
#if (defined(SPI2_CS_PORT) && defined(SPI2_CS_BIT))
#define SPI2_CS 215
#define DIO215 215
#define DIO215_PORT SPI2_CS_PORT
#define DIO215_BIT SPI2_CS_BIT
#endif

/* ======================================================================== */
/* Section 10: Feature flags                                                */
/* ======================================================================== */

#if (defined(TX) && defined(RX))
#define MCU_HAS_UART
#endif
#if (defined(TX2) && defined(RX2))
#define MCU_HAS_UART2
#endif
#if (defined(USB_DP) && defined(USB_DM))
#define MCU_HAS_USB
#endif
#if (defined(SPI_CLK) && defined(SPI_SDI) && defined(SPI_SDO))
#define MCU_HAS_SPI
#ifndef SPI_MODE
#define SPI_MODE 0
#endif
#ifndef SPI_FREQ
#define SPI_FREQ 1000000UL
#endif
#endif
#if (defined(SPI2_CLK) && defined(SPI2_SDI) && defined(SPI2_SDO))
#define MCU_HAS_SPI2
#ifndef SPI2_MODE
#define SPI2_MODE 0
#endif
#ifndef SPI2_FREQ
#define SPI2_FREQ 1000000UL
#endif
#endif
#if (defined(I2C_CLK) && defined(I2C_DATA))
#define MCU_HAS_I2C
#endif
#if defined(ONESHOT_TIMER)
#define MCU_HAS_ONESHOT_TIMER
#endif
#ifndef MCU_HAS_FLASHUPDATE
#define MCU_HAS_FLASHUPDATE
#endif

/* ======================================================================== */
/* Section 11: Peripheral configuration blocks                              */
/* ======================================================================== */

/* --- UART1 (SCI1) Configuration --- */
#ifdef MCU_HAS_UART
/* SCI1 IRQ numbers from RA4M1 vector table (fixed NVIC slots) */
#define SCI1_RXI_IRQn  ((IRQn_Type)22)
#define SCI1_TXI_IRQn  ((IRQn_Type)23)
#define SCI1_TEI_IRQn  ((IRQn_Type)24)
#define SCI1_ERI_IRQn  ((IRQn_Type)25)
/* Register access macros */
#define COM_UART    R_SCI1
#define MCU_SERIAL_ISR  SCI1_RXI_IRQHandler
#define COM_OUTREG  (COM_UART)->TDR
#define COM_INREG   (COM_UART)->RDR
/* Baud calculation: BRR = PCLKB / (16 * BAUDRATE) - 1, for async UART */
#define MCU_BAUD_CALC(baud)  ((uint8_t)(((MCU_PCLKB) / (16UL * (uint32_t)(baud))) - 1UL))
#ifndef BAUDRATE
#define BAUDRATE 115200UL
#endif
#endif /* MCU_HAS_UART */

/* --- Timer allocation: ITP, ONESHOT, PWM --- */

/* ITP timer (step pulse generation) - uses GPT counter overflow ISR */
/* ITP timer number comes from boardmap (default GPT320 = GPT0) */
#ifndef ITP_TIMER
#define ITP_TIMER 0
#endif
/* ISR name for ITP: GPTn_IRQHandler (we register in mcu_ra4.c) */
#define MCU_ITP_ISR     __helper__(GPT, ITP_TIMER, _IRQHandler)
/* ITP timer register pointer */
#define ITP_TIMER_REG   __helper__(R_GPT, ITP_TIMER, )
/* Internal ITP event number for ICU/ELC routing */
#define ITP_ELC_EVENT   __helper__(ELC_EVENT_GPT, ITP_TIMER, _COUNTER_OVERFLOW)
/* Allocate NVIC slot from programmable pool (avoiding SCI/ICU fixed slots) */
#define ITP_IRQn        ((IRQn_Type)14)
/* ITP timer clock = PCLKD (typically 48 MHz / divider from TPCS) */
#define ITP_TIMER_CLOCK MCU_PCLKD
/* Clock enable: module stop clear bit for this GPT channel */
#define ITP_CLOCK_ENABLE()  (R_MSTP->MSTPCRE &= ~(R_MSTP_GPT(ITP_TIMER)))

/* ONESHOT timer (single-shot timeout) */
#ifdef MCU_HAS_ONESHOT_TIMER
#ifndef ONESHOT_TIMER
#define ONESHOT_TIMER 3
#endif
/* ISR name for ONESHOT: GPTn_IRQHandler */
#define MCU_ONESHOT_ISR   __helper__(GPT, ONESHOT_TIMER, _IRQHandler)
/* ONESHOT timer register pointer */
#define ONESHOT_TIMER_REG __helper__(R_GPT, ONESHOT_TIMER, )
/* Allocate NVIC slot */
#define ONESHOT_IRQn      ((IRQn_Type)15)
/* Clock enable */
#define ONESHOT_CLOCK_ENABLE()  (R_MSTP->MSTPCRE &= ~(R_MSTP_GPT(ONESHOT_TIMER)))
#endif /* MCU_HAS_ONESHOT_TIMER */

/* PWM timer (GPT164 = GPT4 from boardmap PWM0_TIMER=4) */
/* Note: PWM uses hardware output compare, no ISR needed */

/* --- External interrupt (IRQ) allocation --- */
/* RA4M1 has 16 ICU external IRQ pins (IRQ0..IRQ15) on NVIC slots 0..15 */
/* Input-change pins (limits, controls, probe) use these external IRQs */
/* Pin-to-IRQ mapping handled in mcu_ra4.c via ICU registers */

/* ======================================================================== */
/* ======================================================================== */
/* All macros use __indirect__(X, PORT) for port number and                */
/* __indirect__(X, BIT) for pin bit, resolved via the DIO<n>_PORT/BIT       */
/* aliases defined in the DIO pin table above.                              */

/* Configuration via PFS register (requires PFS unlock via mcu_unlock_pfs() */
/* or R_BSP_PinAccessEnable() before use)                                   */
#define mcu_config_output(X) \
	do { \
		__ra_pfs_pin__(X).PmnPFS = BSP_IO_PFS_PDR_OUTPUT; \
	} while(0)

#define mcu_config_input(X) \
	do { \
		__ra_pfs_pin__(X).PmnPFS = 0; \
	} while(0)

#define mcu_config_pullup(X) \
	do { \
		__ra_pfs_pin__(X).PmnPFS_b.PCR = 1; \
		__ra_pfs_pin__(X).PmnPFS_b.PODR = 1; \
	} while(0)

/* Alternate function config (ISR, PWM, etc.) - set PMR.
 * PSEL should be set separately by the caller for peripheral functions.
 * For input ISR (external interrupt), set ISEL=1 with PMR=1. */
#define mcu_config_altfunc(X) \
	do { \
		__ra_pfs_pin__(X).PmnPFS_b.PMR = 1; \
	} while(0)

#define mcu_config_input_isr(X) \
	do { \
		__ra_pfs_pin__(X).PmnPFS = 0; \
		__ra_pfs_pin__(X).PmnPFS_b.ISEL = 1; \
		__ra_pfs_pin__(X).PmnPFS_b.PMR = 1; \
	} while(0)

/* PWM config is handled in mcu_ra4.c (GPT timer setup) */
#define mcu_config_pwm(X, freq)

/* PWM duty cycle via GPT GTCCR (PWM0 = GPT4, channel A = GTCCR[0]) */
#ifndef mcu_set_pwm
#define mcu_set_pwm(diopin, pwmvalue) \
	do { \
		(void)(diopin); \
		R_GPT4->GTCCR[0] = ((uint32_t)(pwmvalue) * R_GPT4->GTPR) / 255UL; \
	} while(0)
#endif
#ifndef mcu_get_pwm
#define mcu_get_pwm(diopin) \
	((uint8_t)((R_GPT4->GTCCR[0] * 255UL) / R_GPT4->GTPR))
#endif

/* Fast atomic GPIO operations via R_PORT atomic set/reset registers */
#define mcu_set_output(X) \
	(__ra_port__(__indirect__(X, PORT))->POSR = (1UL << __indirect__(X, BIT)))

#define mcu_clear_output(X) \
	(__ra_port__(__indirect__(X, PORT))->PORR = (1UL << __indirect__(X, BIT)))

#define mcu_get_input(X) \
	(CHECKBIT(__ra_port__(__indirect__(X, PORT))->PIDR, __indirect__(X, BIT)))

#define mcu_get_output(X) \
	(CHECKBIT(__ra_port__(__indirect__(X, PORT))->PODR, __indirect__(X, BIT)))

#define mcu_toggle_output(X) \
	(TOGGLEBIT(__ra_port__(__indirect__(X, PORT))->PODR, __indirect__(X, BIT)))

/* Analog config - set ASEL bit in PFS register */
#define mcu_config_analog(X) \
	do { \
		__ra_pfs_pin__(X).PmnPFS_b.ASEL = 1; \
	} while(0)

/* Probe ISR control (shared with input ISR config) */
#if defined(PROBE_PORT) && defined(PROBE_BIT)
#define mcu_enable_probe_isr()   (mcu_config_input_isr(PROBE))
#define mcu_disable_probe_isr()  do {} while(0)
#else
#define mcu_enable_probe_isr()
#define mcu_disable_probe_isr()
#endif

/* ======================================================================== */
/* Section 13: Time/ISR macros                                              */
/* ======================================================================== */
/* Standard CMSIS intrinsics for Cortex-M4 */
#define mcu_enable_global_isr   __enable_irq
#define mcu_disable_global_isr  __disable_irq
#define mcu_get_global_isr()    (__get_PRIMASK() == 0u)
#define mcu_in_isr_context()    (__get_IPSR() != 0)

/* Microsecond free-run counter via SysTick (1ms tick assumed) */
#define mcu_free_micros() \
	((uint32_t)((((SysTick->LOAD + 1) - SysTick->VAL) * 1000UL) / (SysTick->LOAD + 1)))

/* ======================================================================== */
/* Section 14: Stream plumbing                                              */
/* ======================================================================== */
/* Primary stream is UART1 (SCI1) on UNO R4 Minima (P100 RX, P101 TX).     */
/* mcu_getc/mcu_putc overrides use buffered UART I/O from mcu_ra4.c.       */

#ifdef __cplusplus
}
#endif

#endif