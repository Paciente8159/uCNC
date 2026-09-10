/*
	Name: mcu_ra4.c
	Description: Contains all the function declarations necessary to interact with the MCU.
		This provides a opaque interface between the uCNC and the MCU unit used to power the uCNC.

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

#include "../../../cnc.h"

#if (MCU == MCU_RA4M1)

#include <math.h>

#ifdef MCU_HAS_USB
#include <tusb_ucnc.h>
#endif

/* ======================================================================== */
/* ICU register definitions (Interrupt Controller Unit)                     */
/* ======================================================================== */
#define R_ICU_IELSR(irq)  (*((volatile uint32_t *)(0x40006300UL + ((uint32_t)(irq) * 4UL))))
#define ICU_IELS_MASK     0x1FFUL
#define ICU_IELS_SET(irq, event)  (R_ICU_IELSR(irq) = (R_ICU_IELSR(irq) & ~ICU_IELS_MASK) | (uint32_t)(event))

/* ELC event numbers for RA4M1 (from bsp_elc.h) */
#define ELC_EVENT_ICU_IRQ0      1
#define ELC_EVENT_GPT0_OVERFLOW 93
#define ELC_EVENT_GPT1_OVERFLOW 101
#define ELC_EVENT_GPTn_OVERFLOW(n)  (ELC_EVENT_GPT0_OVERFLOW + ((n) * 8))

/* SCI event numbers for the instance selected as UART1 (UART_PORT).
 * RA4M1 instantiates SCI0, SCI1, SCI2 and SCI9; the ICU event numbers are
 * not contiguous across instances, hence the explicit per-instance mapping
 * (RXI + 1/2/3 = TXI/TEI/ERI). */
#ifdef MCU_HAS_UART
#if (UART_PORT == 0)
#define MCU_SCI_RXI_EVENT 152
#elif (UART_PORT == 1)
#define MCU_SCI_RXI_EVENT 158
#elif (UART_PORT == 2)
#define MCU_SCI_RXI_EVENT 163
#else /* UART_PORT == 9 */
#define MCU_SCI_RXI_EVENT 168
#endif
#define ELC_EVENT_SCI_RXI  (MCU_SCI_RXI_EVENT)
#define ELC_EVENT_SCI_TXI  (MCU_SCI_RXI_EVENT + 1)
#define ELC_EVENT_SCI_TEI  (MCU_SCI_RXI_EVENT + 2)
#define ELC_EVENT_SCI_ERI  (MCU_SCI_RXI_EVENT + 3)
#endif /* MCU_HAS_UART */

#ifdef MCU_HAS_UART2
#if (UART2_PORT == 0)
#define MCU_SCI2_RXI_EVENT 152
#elif (UART2_PORT == 1)
#define MCU_SCI2_RXI_EVENT 158
#elif (UART2_PORT == 2)
#define MCU_SCI2_RXI_EVENT 163
#else /* UART2_PORT == 9 */
#define MCU_SCI2_RXI_EVENT 168
#endif
#define ELC_EVENT_SCI2_RXI  (MCU_SCI2_RXI_EVENT)
#define ELC_EVENT_SCI2_TXI  (MCU_SCI2_RXI_EVENT + 1)
#define ELC_EVENT_SCI2_TEI  (MCU_SCI2_RXI_EVENT + 2)
#define ELC_EVENT_SCI2_ERI  (MCU_SCI2_RXI_EVENT + 3)
#endif /* MCU_HAS_UART2 */

#if defined(MCU_HAS_UART) || defined(MCU_HAS_UART2)
/* Release the module-stop state for a given SCI instance */
static void mcu_sci_clock_enable(uint8_t sci)
{
	switch (sci)
	{
	case 0: R_MSTP->MSTPCRB &= ~R_MSTP_SCI0; break;
	case 1: R_MSTP->MSTPCRB &= ~R_MSTP_SCI1; break;
	case 2: R_MSTP->MSTPCRB &= ~R_MSTP_SCI2; break;
	case 9: R_MSTP->MSTPCRB &= ~R_MSTP_SCI9; break;
	}
}
#endif

/* ======================================================================== */
/* UART1 buffered I/O on the SCI instance selected by UART_PORT             */
/* ======================================================================== */
#ifdef MCU_HAS_UART

#ifndef UART_TX_BUFFER_SIZE
#define UART_TX_BUFFER_SIZE 64
#endif
DECL_BUFFER(uint8_t, uart_tx, UART_TX_BUFFER_SIZE);
DECL_BUFFER(uint8_t, uart_rx, RX_BUFFER_SIZE);

/* UART1 interrupt handlers (ISR names resolved via MCU_SERIAL*_ISR) */
void MCU_SERIAL_ISR(void)
{
	uint8_t c = COM_INREG;
#if !defined(DETACH_UART_FROM_MAIN_PROTOCOL)
	if (mcu_com_rx_cb(c))
	{
		if (!BUFFER_TRY_ENQUEUE(uart_rx, &c))
		{
			STREAM_OVF(c);
		}
	}
#else
	mcu_uart_rx_cb(c);
#endif
}

void MCU_SERIAL_TXI_ISR(void)
{
	uint8_t c = 0;
	if (!BUFFER_TRY_DEQUEUE(uart_tx, &c))
	{
		/* TX buffer empty - disable TX interrupt */
		COM_UART->SCR &= ~(1 << 7); /* clear TIE (bit 7 of SCR) */
		return;
	}
	COM_OUTREG = c;
}

void MCU_SERIAL_TEI_ISR(void)
{
	/* Transmit end - no action needed in basic implementation */
}

void MCU_SERIAL_ERI_ISR(void)
{
	/* Error handler - read SSR to clear error flags */
	uint8_t ssr = COM_UART->SSR;
	(void)ssr;
}

/* Install UART1 handlers into the RAM vector table */
static void mcu_install_uart_handlers(void)
{
	uint32_t *vt = (uint32_t *)SCB->VTOR;
	vt[16 + (uint32_t)SCI_RXI_IRQn] = (uint32_t)MCU_SERIAL_ISR;
	vt[16 + (uint32_t)SCI_TXI_IRQn] = (uint32_t)MCU_SERIAL_TXI_ISR;
	vt[16 + (uint32_t)SCI_TEI_IRQn] = (uint32_t)MCU_SERIAL_TEI_ISR;
	vt[16 + (uint32_t)SCI_ERI_IRQn] = (uint32_t)MCU_SERIAL_ERI_ISR;
}

/* UART1 initialization (async mode, 8N1) */
void mcu_uart_init(void)
{
	volatile uint32_t delay;

	/* Enable the selected SCI instance clock (module stop clear) */
	mcu_sci_clock_enable(UART_PORT);

	/* Small delay for clock stabilization */
	for (delay = 0; delay < 100; delay++);

	/* Configure the SCI for async UART mode (8N1) */
	/* SMR: 0x00 = async, 8-bit data, no parity, 1 stop bit */
	COM_UART->SMR = 0x00;

	/* SEMR: 0x00 = normal SCI (not smart card), no bit modulation */
	COM_UART->SEMR = 0x00;

	/* SNFR: no noise filter */
	COM_UART->SNFR = 0x00;

	/* Set baud rate */
	COM_UART->BRR = MCU_BAUD_CALC(BAUDRATE);

	/* Wait for at least 1 bit period after BRR change */
	for (delay = 0; delay < 1000; delay++);

	/* SCR: enable TX, RX, RX interrupt (TE=bit4, RE=bit5, RIE=bit6) */
	COM_UART->SCR = (1 << 4) | (1 << 5) | (1 << 6);

	/* Clear SSR flags (write 1 to clear ORER, FER, PER) */
	COM_UART->SSR |= (1 << 5) | (1 << 4) | (1 << 3);

	/* Route TX/RX pins to the selected SCI instance (no-op when the board
	 * does not define UART_TX_PSEL/UART_RX_PSEL) */
	mcu_uart_pin_mux();
}

uint8_t mcu_uart_getc(void)
{
	uint8_t c = 0;
	BUFFER_DEQUEUE(uart_rx, &c);
	return c;
}

uint8_t mcu_uart_available(void)
{
	return BUFFER_READ_AVAILABLE(uart_rx);
}

void mcu_uart_clear(void)
{
	BUFFER_CLEAR(uart_rx);
}

void mcu_uart_putc(uint8_t c)
{
	BUFFER_ENQUEUE(uart_tx, &c);
	/* Enable TX interrupt (TIE=bit 7 of SCR) */
	COM_UART->SCR |= (1 << 7);
}

void mcu_uart_flush(void)
{
	while (BUFFER_READ_AVAILABLE(uart_tx));
}

#endif /* MCU_HAS_UART */

/* ======================================================================== */
/* UART2 buffered I/O on the SCI instance selected by UART2_PORT            */
/* ======================================================================== */
#ifdef MCU_HAS_UART2

#ifndef UART2_TX_BUFFER_SIZE
#define UART2_TX_BUFFER_SIZE 64
#endif
DECL_BUFFER(uint8_t, uart2_tx, UART2_TX_BUFFER_SIZE);
DECL_BUFFER(uint8_t, uart2_rx, RX_BUFFER_SIZE);

/* UART2 interrupt handlers (ISR names resolved via MCU_SERIAL2*_ISR) */
void MCU_SERIAL2_ISR(void)
{
	uint8_t c = COM2_INREG;
#if !defined(DETACH_UART2_FROM_MAIN_PROTOCOL)
	if (mcu_com_rx_cb(c))
	{
		if (!BUFFER_TRY_ENQUEUE(uart2_rx, &c))
		{
			STREAM_OVF(c);
		}
	}
#else
	mcu_uart2_rx_cb(c);
#endif
}

void MCU_SERIAL2_TXI_ISR(void)
{
	uint8_t c = 0;
	if (!BUFFER_TRY_DEQUEUE(uart2_tx, &c))
	{
		/* TX buffer empty - disable TX interrupt */
		COM2_UART->SCR &= ~(1 << 7); /* clear TIE (bit 7 of SCR) */
		return;
	}
	COM2_OUTREG = c;
}

void MCU_SERIAL2_TEI_ISR(void)
{
	/* Transmit end - no action needed in basic implementation */
}

void MCU_SERIAL2_ERI_ISR(void)
{
	/* Error handler - read SSR to clear error flags */
	uint8_t ssr = COM2_UART->SSR;
	(void)ssr;
}

/* Install UART2 handlers into the RAM vector table */
static void mcu_install_uart2_handlers(void)
{
	uint32_t *vt = (uint32_t *)SCB->VTOR;
	vt[16 + (uint32_t)SCI2_RXI_IRQn] = (uint32_t)MCU_SERIAL2_ISR;
	vt[16 + (uint32_t)SCI2_TXI_IRQn] = (uint32_t)MCU_SERIAL2_TXI_ISR;
	vt[16 + (uint32_t)SCI2_TEI_IRQn] = (uint32_t)MCU_SERIAL2_TEI_ISR;
	vt[16 + (uint32_t)SCI2_ERI_IRQn] = (uint32_t)MCU_SERIAL2_ERI_ISR;
}

/* UART2 initialization (async mode, 8N1) */
void mcu_uart2_init(void)
{
	volatile uint32_t delay;

	/* Enable the selected SCI instance clock (module stop clear) */
	mcu_sci_clock_enable(UART2_PORT);

	/* Small delay for clock stabilization */
	for (delay = 0; delay < 100; delay++);

	/* Configure the SCI for async UART mode (8N1) */
	COM2_UART->SMR = 0x00;
	COM2_UART->SEMR = 0x00;
	COM2_UART->SNFR = 0x00;

	/* Set baud rate */
	COM2_UART->BRR = MCU_BAUD_CALC(BAUDRATE2);

	/* Wait for at least 1 bit period after BRR change */
	for (delay = 0; delay < 1000; delay++);

	/* SCR: enable TX, RX, RX interrupt (TE=bit4, RE=bit5, RIE=bit6) */
	COM2_UART->SCR = (1 << 4) | (1 << 5) | (1 << 6);

	/* Clear SSR flags (write 1 to clear ORER, FER, PER) */
	COM2_UART->SSR |= (1 << 5) | (1 << 4) | (1 << 3);

	/* Route TX2/RX2 pins to the selected SCI instance (no-op when the
	 * board does not define UART2_TX_PSEL/UART2_RX_PSEL) */
	mcu_uart2_pin_mux();
}

uint8_t mcu_uart2_getc(void)
{
	uint8_t c = 0;
	BUFFER_DEQUEUE(uart2_rx, &c);
	return c;
}

uint8_t mcu_uart2_available(void)
{
	return BUFFER_READ_AVAILABLE(uart2_rx);
}

void mcu_uart2_clear(void)
{
	BUFFER_CLEAR(uart2_rx);
}

void mcu_uart2_putc(uint8_t c)
{
	BUFFER_ENQUEUE(uart2_tx, &c);
	/* Enable TX interrupt (TIE=bit 7 of SCR) */
	COM2_UART->SCR |= (1 << 7);
}

void mcu_uart2_flush(void)
{
	while (BUFFER_READ_AVAILABLE(uart2_tx));
}

#endif /* MCU_HAS_UART2 */

/* ======================================================================== */
/* USB-CDC via tinyUSB                                                      */
/* ======================================================================== */
#ifdef MCU_HAS_USB

void MCU_USB_ISR(void)
{
	tusb_cdc_isr_handler();
}

/* USB init for tinyUSB CDC-ACM device */
void mcu_usb_init(void)
{
	/* Release USBFS module from stop state */
	USBFS_CLOCK_ENABLE();

	/* Enable USB clock (UCLK = 48 MHz for device mode) */
	R_USBCKCR |= USBCKCR_USBCLKSEL;

	/* Small delay for clock stabilization */
	volatile uint32_t delay;
	for (delay = 0; delay < 1000; delay++);

	/* Configure SYSCFG for device controller mode:
	 * - DCFM = 0 (device controller mode)
	 * - DPRPU = 0 (D+ pull-up disabled initially; tinyUSB will enable it)
	 * - USBE = 1 (enable USBFS operation)
	 * - SCKE = 1 (enable internal clock supply) */
	USBFS_SYSCFG = USBFS_SYSCFG_USBE | USBFS_SYSCFG_SCKE;

	/* Enable NVIC for USB interrupt */
	NVIC_SetPriority(USBFS_IRQn, NVIC_USB_IRQ_Pri);
	NVIC_ClearPendingIRQ(USBFS_IRQn);
	NVIC_EnableIRQ(USBFS_IRQn);

	/* Initialize tinyUSB CDC stack */
	tusb_cdc_init();
}

static void mcu_install_usb_handler(void)
{
	uint32_t *vt = (uint32_t *)SCB->VTOR;
	vt[16 + (uint32_t)USBFS_IRQn] = (uint32_t)MCU_USB_ISR;
}

/* USB-CDC buffered I/O */
DECL_BUFFER(uint8_t, usb_rx, RX_BUFFER_SIZE);

uint8_t mcu_usb_getc(void)
{
	uint8_t c = 0;
	BUFFER_TRY_DEQUEUE(usb_rx, &c);
	return c;
}

uint8_t mcu_usb_available(void)
{
	return BUFFER_READ_AVAILABLE(usb_rx);
}

void mcu_usb_clear(void)
{
	BUFFER_CLEAR(usb_rx);
}

void mcu_usb_putc(uint8_t c)
{
	if (!tusb_cdc_write_available())
	{
		mcu_usb_flush();
	}
	tusb_cdc_write(c);
}

void mcu_usb_flush(void)
{
	tusb_cdc_flush();
	while (!tusb_cdc_write_available())
	{
		if (!tusb_cdc_connected)
		{
			return;
		}
	}
}

#endif /* MCU_HAS_USB */

/* ======================================================================== */
/* ITP (step pulse generation) via GPT0 overflow                            */
/* ======================================================================== */

/* ITP state - toggles between step_cb and step_reset_cb */
static volatile uint8_t mcu_itp_toggle;

void GPT0_IRQHandler(void)
{
	/* Clear overflow flag (TCFPO = bit 8 of GTST, write 1 to clear) */
	R_GPT0->GTST = (1UL << 8);

	if (mcu_itp_toggle)
	{
		mcu_step_reset_cb();
	}
	else
	{
		mcu_step_cb();
	}
	mcu_itp_toggle ^= 1;
}

static void mcu_install_itp_handler(void)
{
	uint32_t *vt = (uint32_t *)SCB->VTOR;
	vt[16 + (uint32_t)ITP_IRQn] = (uint32_t)GPT0_IRQHandler;
}

/* ======================================================================== */
/* ONESHOT timer via GPT3 overflow                                          */
/* ======================================================================== */
#ifdef MCU_HAS_ONESHOT_TIMER

void GPT3_IRQHandler(void)
{
	/* Clear overflow flag */
	ONESHOT_TIMER_REG->GTST = (1UL << 8);
	/* Stop timer */
	ONESHOT_TIMER_REG->GTCR &= ~1UL; /* clear CST bit */
	/* Call timeout callback */
	mcu_timeout_cb();
}

static void mcu_install_oneshot_handler(void)
{
	uint32_t *vt = (uint32_t *)SCB->VTOR;
	vt[16 + (uint32_t)ONESHOT_IRQn] = (uint32_t)GPT3_IRQHandler;
}

#endif /* MCU_HAS_ONESHOT_TIMER */

/* ======================================================================== */
/* RTC (SysTick) handler - 1ms tick                                         */
/* ======================================================================== */

/* mcu_runtime_ms - incremented by SysTick RTC */
static volatile uint32_t mcu_runtime_ms;

void SysTick_Handler(void)
{
	mcu_runtime_ms++;
}

/* ======================================================================== */
/* mcu_init - full MCU initialization                                       */
/* ======================================================================== */

void mcu_init(void)
{
	extern uint32_t SystemCoreClock;

	/* Install ISR handlers into RAM vector table */
	mcu_install_itp_handler();
#ifdef MCU_HAS_UART
	mcu_install_uart_handlers();
#endif
#ifdef MCU_HAS_UART2
	mcu_install_uart2_handlers();
#endif
#ifdef MCU_HAS_ONESHOT_TIMER
	mcu_install_oneshot_handler();
#endif
#ifdef MCU_HAS_USB
	mcu_install_usb_handler();
#endif

	/* Configure ICU routing for GPT interrupts */
	/* ITP: Route GPT0 overflow to ITP_IRQn slot */
	ICU_IELS_SET(ITP_IRQn, ELC_EVENT_GPT0_OVERFLOW);
#ifdef MCU_HAS_ONESHOT_TIMER
	/* ONESHOT: Route GPT3 overflow to ONESHOT_IRQn slot */
	ICU_IELS_SET(ONESHOT_IRQn, ELC_EVENT_GPTn_OVERFLOW(ONESHOT_TIMER));
#endif
#ifdef MCU_HAS_UART
	/* UART1: route the selected SCI events to the UART1 NVIC slots */
	ICU_IELS_SET(SCI_RXI_IRQn, ELC_EVENT_SCI_RXI);
	ICU_IELS_SET(SCI_TXI_IRQn, ELC_EVENT_SCI_TXI);
	ICU_IELS_SET(SCI_TEI_IRQn, ELC_EVENT_SCI_TEI);
	ICU_IELS_SET(SCI_ERI_IRQn, ELC_EVENT_SCI_ERI);
#endif
#ifdef MCU_HAS_UART2
	/* UART2: route the selected SCI events to the UART2 NVIC slots */
	ICU_IELS_SET(SCI2_RXI_IRQn, ELC_EVENT_SCI2_RXI);
	ICU_IELS_SET(SCI2_TXI_IRQn, ELC_EVENT_SCI2_TXI);
	ICU_IELS_SET(SCI2_TEI_IRQn, ELC_EVENT_SCI2_TEI);
	ICU_IELS_SET(SCI2_ERI_IRQn, ELC_EVENT_SCI2_ERI);
#endif

	/* Enable GPT clock (module stop clear) */
	ITP_CLOCK_ENABLE();
#ifdef MCU_HAS_ONESHOT_TIMER
	ONESHOT_CLOCK_ENABLE();
#endif

	/* Unlock PFS registers for pin configuration */
	mcu_unlock_pfs();

	/* Initialize UART */
#ifdef MCU_HAS_UART
	mcu_uart_init();
#endif

	/* Initialize USB-CDC */
#ifdef MCU_HAS_USB
	mcu_usb_init();
#endif

	/* Initialize SysTick for RTC (1ms) */
	SysTick->LOAD = (SystemCoreClock / 1000) - 1;
	SysTick->VAL = 0;
	SysTick->CTRL = 0x07; /* Enable, interrupt, use CPU clock */

	/* Enable NVIC for GPT interrupts */
	NVIC_SetPriority(ITP_IRQn, NVIC_ITP_IRQ_Pri);
	NVIC_EnableIRQ(ITP_IRQn);
#ifdef MCU_HAS_ONESHOT_TIMER
	NVIC_SetPriority(ONESHOT_IRQn, NVIC_ONESHOT_IRQ_Pri);
	NVIC_EnableIRQ(ONESHOT_IRQn);
#endif
#ifdef MCU_HAS_UART
	/* Enable NVIC for UART1 interrupts */
	NVIC_SetPriority(SCI_RXI_IRQn, NVIC_UART_IRQ_Pri);
	NVIC_EnableIRQ(SCI_RXI_IRQn);
	NVIC_SetPriority(SCI_TXI_IRQn, NVIC_UART_IRQ_Pri);
	NVIC_EnableIRQ(SCI_TXI_IRQn);
	NVIC_SetPriority(SCI_TEI_IRQn, NVIC_UART_IRQ_Pri);
	NVIC_EnableIRQ(SCI_TEI_IRQn);
	NVIC_SetPriority(SCI_ERI_IRQn, NVIC_UART_IRQ_Pri);
	NVIC_EnableIRQ(SCI_ERI_IRQn);
#endif
#ifdef MCU_HAS_UART2
	/* Enable NVIC for UART2 interrupts */
	NVIC_SetPriority(SCI2_RXI_IRQn, NVIC_UART_IRQ_Pri);
	NVIC_EnableIRQ(SCI2_RXI_IRQn);
	NVIC_SetPriority(SCI2_TXI_IRQn, NVIC_UART_IRQ_Pri);
	NVIC_EnableIRQ(SCI2_TXI_IRQn);
	NVIC_SetPriority(SCI2_TEI_IRQn, NVIC_UART_IRQ_Pri);
	NVIC_EnableIRQ(SCI2_TEI_IRQn);
	NVIC_SetPriority(SCI2_ERI_IRQn, NVIC_UART_IRQ_Pri);
	NVIC_EnableIRQ(SCI2_ERI_IRQn);
#endif

	/* Run IO initialization */
	mcu_io_init();
}

/* ======================================================================== */
/* mcu_millis - return milliseconds since boot                              */
/* ======================================================================== */

uint32_t mcu_millis(void)
{
	return mcu_runtime_ms;
}

uint32_t mcu_micros(void)
{
	return (mcu_runtime_ms * 1000UL) + mcu_free_micros();
}

/* ======================================================================== */
/* mcu_dotasks - background processing (empty for ISR-driven design)        */
/* ======================================================================== */

void mcu_dotasks(void)
{
#ifdef MCU_HAS_USB
	tusb_cdc_task(); /* tinyUSB device task */

	while (tusb_cdc_available())
	{
		uint8_t c = (uint8_t)tusb_cdc_read();
#if !defined(DETACH_USB_FROM_MAIN_PROTOCOL)
		if (mcu_com_rx_cb(c))
		{
			if (!BUFFER_TRY_ENQUEUE(usb_rx, &c))
			{
				STREAM_OVF(c);
			}
		}
#else
		mcu_usb_rx_cb(c);
#endif
	}
#endif
}

/* ======================================================================== */
/* ITP timer control                                                        */
/* ======================================================================== */

void mcu_freq_to_clocks(float frequency, uint16_t *ticks, uint16_t *prescaller)
{
	/* GPT0 is 32-bit. Clock = MCU_PCLKD = 48 MHz.
	 * ticks = clock / (prescaler * freq)
	 * We use the prescaler to keep ticks in a reasonable range.
	 * GPT TPCS prescaler options: 1, 2, 4, 8, 16, 32, 64, 128, 256, 512, 1024, 2048
	 * (TPCS=0=>/1, 1=>/2, 2=>/4, 3=>/8, 4=>/16, 5=>/32, 6=>/64, 7=>/128,
	 *  8=>/256, 9=>/512, 10=>/1024, 11=>/2048)
	 *
	 * For ~100kHz max step rate, with min prescaler=1: ticks = 48M/100k = 480 (fits in 16-bit)
	 * For 4Hz min step rate, with prescaler=2048: ticks = 48M/(2048*4) = 5859 (fits in 16-bit)
	 * A 16-bit ticks value is sufficient.
	 */

	float clock = (float)ITP_TIMER_CLOCK;

	/* Find prescaler that keeps ticks within 16-bit range */
	uint16_t presc = 0;
	float max_ticks;
	do
	{
		max_ticks = clock / ((float)(1UL << presc) * frequency);
		if (max_ticks <= 65535.0f && max_ticks >= 1.0f)
		{
			break;
		}
		presc++;
	} while (presc <= 11);

	if (presc > 11)
	{
		presc = 11;
		max_ticks = clock / ((float)(1UL << presc) * frequency);
	}

	*prescaller = presc;
	*ticks = (uint16_t)(max_ticks + 0.5f);
	if (*ticks < 2) *ticks = 2;
}

float mcu_clocks_to_freq(uint16_t ticks, uint16_t prescaller)
{
	/* Invert of above */
	return (float)ITP_TIMER_CLOCK / ((float)(1UL << prescaller) * (float)ticks);
}

void mcu_start_itp_isr(uint16_t ticks, uint16_t prescaller)
{
	/* Disable timer */
	R_GPT0->GTCR &= ~1UL;  /* clear CST */

	/* Reset counter and clear status */
	R_GPT0->GTCNT = 0;
	R_GPT0->GTST = 0x3FF;  /* clear all status flags (write 1 to clear) */

	/* Set prescaler in GTCR.TPCS[3:0] (bits 26:23) */
	R_GPT0->GTCR = (R_GPT0->GTCR & ~(0xFUL << 23)) | ((uint32_t)(prescaller & 0xF) << 23);

	/* Set sawtooth mode counting up: MD[2:0] = 0 (bits 18:16) */
	R_GPT0->GTCR &= ~(0x7UL << 16);

	/* Set period */
	R_GPT0->GTPR = (uint32_t)ticks;

	/* Enable overflow interrupt (GTINTAD.TCFOIE = bit 16) */
	R_GPT0->GTINTAD |= (1UL << 16);

	/* Initialize toggle state */
	mcu_itp_toggle = 0;

	/* Start counter (CST = bit 0) */
	R_GPT0->GTCR |= 1UL;
}

void mcu_change_itp_isr(uint16_t ticks, uint16_t prescaller)
{
	/* Stop timer */
	R_GPT0->GTCR &= ~1UL;

	/* Update prescaler and period */
	R_GPT0->GTCR = (R_GPT0->GTCR & ~(0xFUL << 23)) | ((uint32_t)(prescaller & 0xF) << 23);
	R_GPT0->GTPR = (uint32_t)ticks;

	/* Reset counter */
	R_GPT0->GTCNT = 0;

	/* Clear status */
	R_GPT0->GTST = 0x3FF;

	/* Restart */
	R_GPT0->GTCR |= 1UL;
}

void mcu_stop_itp_isr(void)
{
	/* Disable timer and overflow interrupt */
	R_GPT0->GTCR &= ~1UL;
	R_GPT0->GTINTAD &= ~(1UL << 16);
	R_GPT0->GTST = 0x3FF;
}

/* ======================================================================== */
/* ONESHOT timer control                                                    */
/* ======================================================================== */

#ifdef MCU_HAS_ONESHOT_TIMER

void mcu_config_timeout(mcu_timeout_delgate fp, uint32_t timeout)
{
	/* Store callback delegate (mcu_timeout_cb is set by mcu.c) */
	(void)fp;

	/* Calculate timer ticks for 1MHz base (prescaler = 48 => 1MHz) */
	uint32_t prescaler = 5;  /* TPCS=5 => /32, 48MHz/32 = 1.5MHz... */
	/* Better: use TPCS=5 (/32) for ~48MHz/32 = 1.5MHz clock */
	/* Actually let's use TPCS=0 (/1) and timeout in us directly */

	/* Disable timer */
	ONESHOT_TIMER_REG->GTCR &= ~1UL;

	/* Set prescaler (TPCS=0 => /1) */
	ONESHOT_TIMER_REG->GTCR &= ~(0xFUL << 23);

	/* Sawtooth mode */
	ONESHOT_TIMER_REG->GTCR &= ~(0x7UL << 16);

	/* Set timeout period (in system clock cycles, ~48MHz) */
	ONESHOT_TIMER_REG->GTPR = (uint32_t)(timeout * 1000UL);  /* timeout in ms → μs → clock cycles */
	/* Actually timeout is in ms. For 48MHz clock: GTPR = timeout * 48000 (approx) */
	/* But we should use a fixed prescaler and verify the range */

	/* For now: TPCS=6 => /64, giving 750kHz tick. GTPR = timeout * 750 */
	ONESHOT_TIMER_REG->GTCR = (ONESHOT_TIMER_REG->GTCR & ~(0xFUL << 23)) | (6UL << 23);
	ONESHOT_TIMER_REG->GTPR = (uint32_t)(timeout * 750UL);

	/* Enable overflow interrupt */
	ONESHOT_TIMER_REG->GTINTAD |= (1UL << 16);

	/* Reset counter */
	ONESHOT_TIMER_REG->GTCNT = 0;

	/* Clear status */
	ONESHOT_TIMER_REG->GTST = 0x3FF;
}

void mcu_start_timeout(void)
{
	/* Start timer */
	ONESHOT_TIMER_REG->GTCR |= 1UL;
}

#endif /* MCU_HAS_ONESHOT_TIMER */

/* ======================================================================== */
/* PWM setup (GPT4 for PWM0)                                                */
/* ======================================================================== */

#if defined(PWM0_TIMER) && defined(MCU_HAS_PWM)
#warning "PWM initialization through mcu_ra4.c is not yet implemented"
/* Future: GPT4 GTCCRA compare output for PWM0 on P103 */
#endif

#endif /* MCU == MCU_RA4M1 */