#ifdef __cplusplus
extern "C"
{
#endif

/* Includes ------------------------------------------------------------------- */
#include <stdint.h>


/* Public Types --------------------------------------------------------------- */
/** @defgroup LPC_Types_Public_Types LPC_Types Public Types
 * @{
 */

/**
 * @brief Boolean Type definition
 */
typedef enum {FALSE = 0, TRUE = !FALSE} Bool;

/**
 * @brief Flag Status and Interrupt Flag Status type definition
 */
typedef enum {RESET = 0, SET = !RESET} FlagStatus, IntStatus, SetState;
#define PARAM_SETSTATE(State) ((State==RESET) || (State==SET))

/**
 * @brief Functional State Definition
 */
typedef enum {DISABLE = 0, ENABLE = !DISABLE} FunctionalState;
#define PARAM_FUNCTIONALSTATE(State) ((State==DISABLE) || (State==ENABLE))

/**
 * @ Status type definition
 */
typedef enum {ERROR = 0, SUCCESS = !ERROR} Status;


/**
 * Read/Write transfer type mode (Block or non-block)
 */
typedef enum
{
	NONE_BLOCKING = 0,		/**< None Blocking type */
	BLOCKING				/**< Blocking type */
} TRANSFER_BLOCK_Type;


/** Pointer to Function returning Void (any number of parameters) */
typedef void (*PFV)();

/** Pointer to Function returning int32_t (any number of parameters) */
typedef int32_t(*PFI)();

/**
 * @}
 */


/* Public Macros -------------------------------------------------------------- */
/** @defgroup LPC_Types_Public_Macros  LPC_Types Public Macros
 * @{
 */

/* _BIT(n) sets the bit at position "n"
 * _BIT(n) is intended to be used in "OR" and "AND" expressions:
 * e.g., "(_BIT(3) | _BIT(7))".
 */
#undef _BIT
/* Set bit macro */
#define _BIT(n)	(1<<n)

/* _SBF(f,v) sets the bit field starting at position "f" to value "v".
 * _SBF(f,v) is intended to be used in "OR" and "AND" expressions:
 * e.g., "((_SBF(5,7) | _SBF(12,0xF)) & 0xFFFF)"
 */
#undef _SBF
/* Set bit field macro */
#define _SBF(f,v) (v<<f)

/* _BITMASK constructs a symbol with 'field_width' least significant
 * bits set.
 * e.g., _BITMASK(5) constructs '0x1F', _BITMASK(16) == 0xFFFF
 * The symbol is intended to be used to limit the bit field width
 * thusly:
 * <a_register> = (any_expression) & _BITMASK(x), where 0 < x <= 32.
 * If "any_expression" results in a value that is larger than can be
 * contained in 'x' bits, the bits above 'x - 1' are masked off.  When
 * used with the _SBF example above, the example would be written:
 * a_reg = ((_SBF(5,7) | _SBF(12,0xF)) & _BITMASK(16))
 * This ensures that the value written to a_reg is no wider than
 * 16 bits, and makes the code easier to read and understand.
 */
#undef _BITMASK
/* Bitmask creation macro */
#define _BITMASK(field_width) ( _BIT(field_width) - 1)

/* NULL pointer */
#ifndef NULL
  #define NULL ((void*) 0)
#endif

/* Number of elements in an array */
#define NELEMENTS(array)  (sizeof (array) / sizeof (array[0]))

/* Static data/function define */
#define STATIC static
/* External data/function define */
#define EXTERN extern

#ifndef MAX
  #define MAX(a, b) (((a) > (b)) ? (a) : (b))
#endif
#ifndef MIN
  #define MIN(a, b) (((a) < (b)) ? (a) : (b))
#endif

/**
 * @}
 */


/* Old Type Definition compatibility ------------------------------------------ */
/** @addtogroup LPC_Types_Public_Types LPC_Types Public Types
 * @{
 */

/** SMA type for character type */
typedef char CHAR;

/** SMA type for 8 bit unsigned value */
typedef uint8_t UNS_8;

/** SMA type for 8 bit signed value */
typedef int8_t INT_8;

/** SMA type for 16 bit unsigned value */
typedef	uint16_t UNS_16;

/** SMA type for 16 bit signed value */
typedef	int16_t INT_16;

/** SMA type for 32 bit unsigned value */
typedef	uint32_t UNS_32;

/** SMA type for 32 bit signed value */
typedef	int32_t INT_32;

/** SMA type for 64 bit signed value */
typedef int64_t INT_64;

/** SMA type for 64 bit unsigned value */
typedef uint64_t UNS_64;

/** 32 bit boolean type */
typedef Bool BOOL_32;

/** 16 bit boolean type */
typedef Bool BOOL_16;

/** 8 bit boolean type */
typedef Bool BOOL_8;

/**
 * @}
 */

/* Public Macros -------------------------------------------------------------- */
/** @defgroup PINSEL_Public_Macros PINSEL Public Macros
 * @{
 */

/*********************************************************************//**
 *!< Macros define for PORT Selection
 ***********************************************************************/
#define PINSEL_PORT_0 	((0))	/**< PORT 0*/
#define PINSEL_PORT_1 	((1))	/**< PORT 1*/
#define PINSEL_PORT_2 	((2))	/**< PORT 2*/
#define PINSEL_PORT_3 	((3))	/**< PORT 3*/
#define PINSEL_PORT_4 	((4))	/**< PORT 4*/

/***********************************************************************
 * Macros define for Pin Function selection
 **********************************************************************/
#define PINSEL_FUNC_0	((0))	/**< default function*/
#define PINSEL_FUNC_1	((1))	/**< first alternate function*/
#define PINSEL_FUNC_2	((2))	/**< second alternate function*/
#define PINSEL_FUNC_3	((3))	/**< third or reserved alternate function*/

/***********************************************************************
 * Macros define for Pin Number of Port
 **********************************************************************/
#define PINSEL_PIN_0 	((0)) 	/**< Pin 0 */
#define PINSEL_PIN_1 	((1)) 	/**< Pin 1 */
#define PINSEL_PIN_2 	((2)) 	/**< Pin 2 */
#define PINSEL_PIN_3 	((3)) 	/**< Pin 3 */
#define PINSEL_PIN_4 	((4)) 	/**< Pin 4 */
#define PINSEL_PIN_5 	((5)) 	/**< Pin 5 */
#define PINSEL_PIN_6 	((6)) 	/**< Pin 6 */
#define PINSEL_PIN_7 	((7)) 	/**< Pin 7 */
#define PINSEL_PIN_8 	((8)) 	/**< Pin 8 */
#define PINSEL_PIN_9 	((9)) 	/**< Pin 9 */
#define PINSEL_PIN_10 	((10)) 	/**< Pin 10 */
#define PINSEL_PIN_11 	((11)) 	/**< Pin 11 */
#define PINSEL_PIN_12 	((12)) 	/**< Pin 12 */
#define PINSEL_PIN_13 	((13)) 	/**< Pin 13 */
#define PINSEL_PIN_14 	((14)) 	/**< Pin 14 */
#define PINSEL_PIN_15 	((15)) 	/**< Pin 15 */
#define PINSEL_PIN_16 	((16)) 	/**< Pin 16 */
#define PINSEL_PIN_17 	((17)) 	/**< Pin 17 */
#define PINSEL_PIN_18 	((18)) 	/**< Pin 18 */
#define PINSEL_PIN_19 	((19)) 	/**< Pin 19 */
#define PINSEL_PIN_20 	((20)) 	/**< Pin 20 */
#define PINSEL_PIN_21 	((21)) 	/**< Pin 21 */
#define PINSEL_PIN_22 	((22)) 	/**< Pin 22 */
#define PINSEL_PIN_23 	((23)) 	/**< Pin 23 */
#define PINSEL_PIN_24 	((24)) 	/**< Pin 24 */
#define PINSEL_PIN_25 	((25)) 	/**< Pin 25 */
#define PINSEL_PIN_26 	((26)) 	/**< Pin 26 */
#define PINSEL_PIN_27 	((27)) 	/**< Pin 27 */
#define PINSEL_PIN_28 	((28)) 	/**< Pin 28 */
#define PINSEL_PIN_29 	((29)) 	/**< Pin 29 */
#define PINSEL_PIN_30 	((30)) 	/**< Pin 30 */
#define PINSEL_PIN_31 	((31)) 	/**< Pin 31 */

/***********************************************************************
 * Macros define for Pin mode
 **********************************************************************/
#define PINSEL_PINMODE_PULLUP		((0))	/**< Internal pull-up resistor*/
#define PINSEL_PINMODE_TRISTATE 	((2))	/**< Tri-state */
#define PINSEL_PINMODE_PULLDOWN 	((3)) 	/**< Internal pull-down resistor */

/***********************************************************************
 * Macros define for Pin mode (normal/open drain)
 **********************************************************************/
#define	PINSEL_PINMODE_NORMAL		((0))	/**< Pin is in the normal (not open drain) mode.*/
#define	PINSEL_PINMODE_OPENDRAIN	((1)) 	/**< Pin is in the open drain mode */

/***********************************************************************
 * Macros define for I2C mode
 ***********************************************************************/
#define	PINSEL_I2C_Normal_Mode		((0))	/**< The standard drive mode */
#define	PINSEL_I2C_Fast_Mode		((1)) 	/**<  Fast Mode Plus drive mode */

/**
 * @}
 */

/* Private Macros ------------------------------------------------------------- */
/** @defgroup PINSEL_Private_Macros PINSEL Private Macros
 * @{
 */

/* Pin selection define */
/* I2C Pin Configuration register bit description */
#define PINSEL_I2CPADCFG_SDADRV0 	_BIT(0) /**< Drive mode control for the SDA0 pin, P0.27 */
#define PINSEL_I2CPADCFG_SDAI2C0	_BIT(1) /**< I2C mode control for the SDA0 pin, P0.27 */
#define PINSEL_I2CPADCFG_SCLDRV0	_BIT(2) /**< Drive mode control for the SCL0 pin, P0.28 */
#define PINSEL_I2CPADCFG_SCLI2C0	_BIT(3) /**< I2C mode control for the SCL0 pin, P0.28 */

/**
 * @}
 */


/* Public Types --------------------------------------------------------------- */
/** @defgroup PINSEL_Public_Types PINSEL Public Types
 * @{
 */

/** @brief Pin configuration structure */
typedef struct
{
	uint8_t Portnum;	/**< Port Number, should be PINSEL_PORT_x,
						where x should be in range from 0 to 4 */
	uint8_t Pinnum;		/**< Pin Number, should be PINSEL_PIN_x,
						where x should be in range from 0 to 31 */
	uint8_t Funcnum;	/**< Function Number, should be PINSEL_FUNC_x,
						where x should be in range from 0 to 3 */
	uint8_t Pinmode;	/**< Pin Mode, should be:
						- PINSEL_PINMODE_PULLUP: Internal pull-up resistor
						- PINSEL_PINMODE_TRISTATE: Tri-state
						- PINSEL_PINMODE_PULLDOWN: Internal pull-down resistor */
	uint8_t OpenDrain;	/**< OpenDrain mode, should be:
						- PINSEL_PINMODE_NORMAL: Pin is in the normal (not open drain) mode
						- PINSEL_PINMODE_OPENDRAIN: Pin is in the open drain mode */
} PINSEL_CFG_Type;

/**
 * @}
 */


/* Public Macros -------------------------------------------------------------- */
/** @defgroup CLKPWR_Public_Macros CLKPWR Public Macros
 * @{
 */

/**********************************************************************
 * Peripheral Clock Selection Definitions
 **********************************************************************/
/** Peripheral clock divider bit position for WDT */
#define	CLKPWR_PCLKSEL_WDT  		((uint32_t)(0))
/** Peripheral clock divider bit position for TIMER0 */
#define	CLKPWR_PCLKSEL_TIMER0  		((uint32_t)(2))
/** Peripheral clock divider bit position for TIMER1 */
#define	CLKPWR_PCLKSEL_TIMER1  		((uint32_t)(4))
/** Peripheral clock divider bit position for UART0 */
#define	CLKPWR_PCLKSEL_UART0  		((uint32_t)(6))
/** Peripheral clock divider bit position for UART1 */
#define	CLKPWR_PCLKSEL_UART1  		((uint32_t)(8))
/** Peripheral clock divider bit position for PWM1 */
#define	CLKPWR_PCLKSEL_PWM1  		((uint32_t)(12))
/** Peripheral clock divider bit position for I2C0 */
#define	CLKPWR_PCLKSEL_I2C0  		((uint32_t)(14))
/** Peripheral clock divider bit position for SPI */
#define	CLKPWR_PCLKSEL_SPI  		((uint32_t)(16))
/** Peripheral clock divider bit position for SSP1 */
#define	CLKPWR_PCLKSEL_SSP1  		((uint32_t)(20))
/** Peripheral clock divider bit position for DAC */
#define	CLKPWR_PCLKSEL_DAC  		((uint32_t)(22))
/** Peripheral clock divider bit position for ADC */
#define	CLKPWR_PCLKSEL_ADC  		((uint32_t)(24))
/** Peripheral clock divider bit position for CAN1 */
#define	CLKPWR_PCLKSEL_CAN1 		((uint32_t)(26))
/** Peripheral clock divider bit position for CAN2 */
#define	CLKPWR_PCLKSEL_CAN2 		((uint32_t)(28))
/** Peripheral clock divider bit position for ACF */
#define	CLKPWR_PCLKSEL_ACF  		((uint32_t)(30))
/** Peripheral clock divider bit position for QEI */
#define	CLKPWR_PCLKSEL_QEI  		((uint32_t)(32))
/** Peripheral clock divider bit position for PCB */
#define	CLKPWR_PCLKSEL_PCB  		((uint32_t)(36))
/** Peripheral clock divider bit position for  I2C1 */
#define	CLKPWR_PCLKSEL_I2C1  		((uint32_t)(38))
/** Peripheral clock divider bit position for SSP0 */
#define	CLKPWR_PCLKSEL_SSP0  		((uint32_t)(42))
/** Peripheral clock divider bit position for TIMER2 */
#define	CLKPWR_PCLKSEL_TIMER2  		((uint32_t)(44))
/** Peripheral clock divider bit position for  TIMER3 */
#define	CLKPWR_PCLKSEL_TIMER3  		((uint32_t)(46))
/** Peripheral clock divider bit position for UART2 */
#define	CLKPWR_PCLKSEL_UART2  		((uint32_t)(48))
/** Peripheral clock divider bit position for UART3 */
#define	CLKPWR_PCLKSEL_UART3  		((uint32_t)(50))
/** Peripheral clock divider bit position for I2C2 */
#define	CLKPWR_PCLKSEL_I2C2  		((uint32_t)(52))
/** Peripheral clock divider bit position for I2S */
#define	CLKPWR_PCLKSEL_I2S  		((uint32_t)(54))
/** Peripheral clock divider bit position for RIT */
#define	CLKPWR_PCLKSEL_RIT  		((uint32_t)(58))
/** Peripheral clock divider bit position for SYSCON */
#define	CLKPWR_PCLKSEL_SYSCON  		((uint32_t)(60))
/** Peripheral clock divider bit position for MC */
#define	CLKPWR_PCLKSEL_MC  			((uint32_t)(62))

/** Macro for Peripheral Clock Selection register bit values
 * Note: When CCLK_DIV_8, Peripheral�s clock is selected to
 * PCLK_xyz = CCLK/8 except for CAN1, CAN2, and CAN filtering
 * when �11�selects PCLK_xyz = CCLK/6 */
/* Peripheral clock divider is set to 4 from CCLK */
#define	CLKPWR_PCLKSEL_CCLK_DIV_4  ((uint32_t)(0))
/** Peripheral clock divider is the same with CCLK */
#define	CLKPWR_PCLKSEL_CCLK_DIV_1  ((uint32_t)(1))
/** Peripheral clock divider is set to 2 from CCLK */
#define	CLKPWR_PCLKSEL_CCLK_DIV_2  ((uint32_t)(2))


/********************************************************************
* Power Control for Peripherals Definitions
**********************************************************************/
/** Timer/Counter 0 power/clock control bit */
#define	 CLKPWR_PCONP_PCTIM0	((uint32_t)(1<<1))
/* Timer/Counter 1 power/clock control bit */
#define	 CLKPWR_PCONP_PCTIM1	((uint32_t)(1<<2))
/** UART0 power/clock control bit */
#define	 CLKPWR_PCONP_PCUART0  	((uint32_t)(1<<3))
/** UART1 power/clock control bit */
#define	 CLKPWR_PCONP_PCUART1  	((uint32_t)(1<<4))
/** PWM1 power/clock control bit */
#define	 CLKPWR_PCONP_PCPWM1	((uint32_t)(1<<6))
/** The I2C0 interface power/clock control bit */
#define	 CLKPWR_PCONP_PCI2C0	((uint32_t)(1<<7))
/** The SPI interface power/clock control bit */
#define	 CLKPWR_PCONP_PCSPI  	((uint32_t)(1<<8))
/** The RTC power/clock control bit */
#define	 CLKPWR_PCONP_PCRTC  	((uint32_t)(1<<9))
/** The SSP1 interface power/clock control bit */
#define	 CLKPWR_PCONP_PCSSP1	((uint32_t)(1<<10))
/** A/D converter 0 (ADC0) power/clock control bit */
#define	 CLKPWR_PCONP_PCAD  	((uint32_t)(1<<12))
/** CAN Controller 1 power/clock control bit */
#define	 CLKPWR_PCONP_PCAN1  	((uint32_t)(1<<13))
/** CAN Controller 2 power/clock control bit */
#define	 CLKPWR_PCONP_PCAN2 	((uint32_t)(1<<14))
/** GPIO power/clock control bit */
#define	CLKPWR_PCONP_PCGPIO 	((uint32_t)(1<<15))
/** Repetitive Interrupt Timer power/clock control bit */
#define	CLKPWR_PCONP_PCRIT 		((uint32_t)(1<<16))
/** Motor Control PWM */
#define CLKPWR_PCONP_PCMC 		((uint32_t)(1<<17))
/** Quadrature Encoder Interface power/clock control bit */
#define CLKPWR_PCONP_PCQEI 		((uint32_t)(1<<18))
/** The I2C1 interface power/clock control bit */
#define	 CLKPWR_PCONP_PCI2C1  	((uint32_t)(1<<19))
/** The SSP0 interface power/clock control bit */
#define	 CLKPWR_PCONP_PCSSP0	((uint32_t)(1<<21))
/** Timer 2 power/clock control bit */
#define	 CLKPWR_PCONP_PCTIM2	((uint32_t)(1<<22))
/** Timer 3 power/clock control bit */
#define	 CLKPWR_PCONP_PCTIM3	((uint32_t)(1<<23))
/** UART 2 power/clock control bit */
#define	 CLKPWR_PCONP_PCUART2  	((uint32_t)(1<<24))
/** UART 3 power/clock control bit */
#define	 CLKPWR_PCONP_PCUART3  	((uint32_t)(1<<25))
/** I2C interface 2 power/clock control bit */
#define	 CLKPWR_PCONP_PCI2C2	((uint32_t)(1<<26))
/** I2S interface power/clock control bit*/
#define	 CLKPWR_PCONP_PCI2S  	((uint32_t)(1<<27))
/** GP DMA function power/clock control bit*/
#define	 CLKPWR_PCONP_PCGPDMA  	((uint32_t)(1<<29))
/** Ethernet block power/clock control bit*/
#define	 CLKPWR_PCONP_PCENET	((uint32_t)(1<<30))
/** USB interface power/clock control bit*/
#define	 CLKPWR_PCONP_PCUSB  	((uint32_t)(1<<31))


/**
 * @}
 */
/* Private Macros ------------------------------------------------------------- */
/** @defgroup CLKPWR_Private_Macros CLKPWR Private Macros
 * @{
 */

/* --------------------- BIT DEFINITIONS -------------------------------------- */
/*********************************************************************//**
 * Macro defines for Clock Source Select Register
 **********************************************************************/
/** Internal RC oscillator */
#define CLKPWR_CLKSRCSEL_CLKSRC_IRC			((uint32_t)(0x00))
/** Main oscillator */
#define CLKPWR_CLKSRCSEL_CLKSRC_MAINOSC		((uint32_t)(0x01))
/** RTC oscillator */
#define CLKPWR_CLKSRCSEL_CLKSRC_RTC			((uint32_t)(0x02))
/** Clock source selection bit mask */
#define CLKPWR_CLKSRCSEL_BITMASK			((uint32_t)(0x03))

/*********************************************************************//**
 * Macro defines for Clock Output Configuration Register
 **********************************************************************/
/* Clock Output Configuration register definition */
/** Selects the CPU clock as the CLKOUT source */
#define CLKPWR_CLKOUTCFG_CLKOUTSEL_CPU		((uint32_t)(0x00))
/** Selects the main oscillator as the CLKOUT source */
#define CLKPWR_CLKOUTCFG_CLKOUTSEL_MAINOSC	((uint32_t)(0x01))
/** Selects the Internal RC oscillator as the CLKOUT source */
#define CLKPWR_CLKOUTCFG_CLKOUTSEL_RC		((uint32_t)(0x02))
/** Selects the USB clock as the CLKOUT source */
#define CLKPWR_CLKOUTCFG_CLKOUTSEL_USB		((uint32_t)(0x03))
/** Selects the RTC oscillator as the CLKOUT source */
#define CLKPWR_CLKOUTCFG_CLKOUTSEL_RTC		((uint32_t)(0x04))
/** Integer value to divide the output clock by, minus one */
#define CLKPWR_CLKOUTCFG_CLKOUTDIV(n)		((uint32_t)((n&0x0F)<<4))
/** CLKOUT enable control */
#define CLKPWR_CLKOUTCFG_CLKOUT_EN			((uint32_t)(1<<8))
/** CLKOUT activity indication */
#define CLKPWR_CLKOUTCFG_CLKOUT_ACT			((uint32_t)(1<<9))
/** Clock source selection bit mask */
#define CLKPWR_CLKOUTCFG_BITMASK			((uint32_t)(0x3FF))

/*********************************************************************//**
 * Macro defines for PPL0 Control Register
 **********************************************************************/
/** PLL 0 control enable */
#define CLKPWR_PLL0CON_ENABLE		((uint32_t)(0x01))
/** PLL 0 control connect */
#define CLKPWR_PLL0CON_CONNECT		((uint32_t)(0x02))
/** PLL 0 control bit mask */
#define CLKPWR_PLL0CON_BITMASK		((uint32_t)(0x03))

/*********************************************************************//**
 * Macro defines for PPL0 Configuration Register
 **********************************************************************/
/** PLL 0 Configuration MSEL field */
#define CLKPWR_PLL0CFG_MSEL(n)		((uint32_t)(n&0x7FFF))
/** PLL 0 Configuration NSEL field */
#define CLKPWR_PLL0CFG_NSEL(n)		((uint32_t)((n<<16)&0xFF0000))
/** PLL 0 Configuration bit mask */
#define CLKPWR_PLL0CFG_BITMASK		((uint32_t)(0xFF7FFF))


/*********************************************************************//**
 * Macro defines for PPL0 Status Register
 **********************************************************************/
/** PLL 0 MSEL value */
#define CLKPWR_PLL0STAT_MSEL(n)		((uint32_t)(n&0x7FFF))
/** PLL NSEL get value  */
#define CLKPWR_PLL0STAT_NSEL(n)		((uint32_t)((n>>16)&0xFF))
/** PLL status enable bit */
#define CLKPWR_PLL0STAT_PLLE		((uint32_t)(1<<24))
/** PLL status Connect bit */
#define CLKPWR_PLL0STAT_PLLC		((uint32_t)(1<<25))
/** PLL status lock */
#define CLKPWR_PLL0STAT_PLOCK		((uint32_t)(1<<26))

/*********************************************************************//**
 * Macro defines for PPL0 Feed Register
 **********************************************************************/
/** PLL0 Feed bit mask */
#define CLKPWR_PLL0FEED_BITMASK			((uint32_t)0xFF)

/*********************************************************************//**
 * Macro defines for PLL1 Control Register
 **********************************************************************/
/** USB PLL control enable */
#define CLKPWR_PLL1CON_ENABLE		((uint32_t)(0x01))
/** USB PLL control connect */
#define CLKPWR_PLL1CON_CONNECT		((uint32_t)(0x02))
/** USB PLL control bit mask */
#define CLKPWR_PLL1CON_BITMASK		((uint32_t)(0x03))

/*********************************************************************//**
 * Macro defines for PLL1 Configuration Register
 **********************************************************************/
/** USB PLL MSEL set value */
#define CLKPWR_PLL1CFG_MSEL(n)		((uint32_t)(n&0x1F))
/** USB PLL PSEL set value */
#define CLKPWR_PLL1CFG_PSEL(n)		((uint32_t)((n&0x03)<<5))
/** USB PLL configuration bit mask */
#define CLKPWR_PLL1CFG_BITMASK		((uint32_t)(0x7F))

/*********************************************************************//**
 * Macro defines for PLL1 Status Register
 **********************************************************************/
/** USB PLL MSEL get value  */
#define CLKPWR_PLL1STAT_MSEL(n)		((uint32_t)(n&0x1F))
/** USB PLL PSEL get value  */
#define CLKPWR_PLL1STAT_PSEL(n)		((uint32_t)((n>>5)&0x03))
/** USB PLL status enable bit */
#define CLKPWR_PLL1STAT_PLLE		((uint32_t)(1<<8))
/** USB PLL status Connect bit */
#define CLKPWR_PLL1STAT_PLLC		((uint32_t)(1<<9))
/** USB PLL status lock */
#define CLKPWR_PLL1STAT_PLOCK		((uint32_t)(1<<10))

/*********************************************************************//**
 * Macro defines for PLL1 Feed Register
 **********************************************************************/
/** PLL1 Feed bit mask */
#define CLKPWR_PLL1FEED_BITMASK		((uint32_t)0xFF)

/*********************************************************************//**
 * Macro defines for CPU Clock Configuration Register
 **********************************************************************/
/** CPU Clock configuration bit mask */
#define CLKPWR_CCLKCFG_BITMASK		((uint32_t)(0xFF))

/*********************************************************************//**
 * Macro defines for USB Clock Configuration Register
 **********************************************************************/
/** USB Clock Configuration bit mask */
#define CLKPWR_USBCLKCFG_BITMASK	((uint32_t)(0x0F))

/*********************************************************************//**
 * Macro defines for IRC Trim Register
 **********************************************************************/
/** IRC Trim bit mask */
#define CLKPWR_IRCTRIM_BITMASK		((uint32_t)(0x0F))

/*********************************************************************//**
 * Macro defines for Peripheral Clock Selection Register 0 and 1
 **********************************************************************/
/** Peripheral Clock Selection 0 mask bit */
#define CLKPWR_PCLKSEL0_BITMASK		((uint32_t)(0xFFF3F3FF))
/** Peripheral Clock Selection 1 mask bit */
#define CLKPWR_PCLKSEL1_BITMASK		((uint32_t)(0xFCF3F0F3))
/** Macro to set peripheral clock of each type
 * p: position of two bits that hold divider of peripheral clock
 * n: value of divider of peripheral clock  to be set */
#define CLKPWR_PCLKSEL_SET(p,n)		_SBF(p,n)
/** Macro to mask peripheral clock of each type */
#define CLKPWR_PCLKSEL_BITMASK(p)	_SBF(p,0x03)
/** Macro to get peripheral clock of each type */
#define CLKPWR_PCLKSEL_GET(p, n)	((uint32_t)((n>>p)&0x03))

/*********************************************************************//**
 * Macro defines for Power Mode Control Register
 **********************************************************************/
/** Power mode control bit 0 */
#define CLKPWR_PCON_PM0			((uint32_t)(1<<0))
/** Power mode control bit 1 */
#define CLKPWR_PCON_PM1			((uint32_t)(1<<1))
/** Brown-Out Reduced Power Mode */
#define CLKPWR_PCON_BODPDM		((uint32_t)(1<<2))
/** Brown-Out Global Disable */
#define CLKPWR_PCON_BOGD		((uint32_t)(1<<3))
/** Brown Out Reset Disable */
#define CLKPWR_PCON_BORD		((uint32_t)(1<<4))
/** Sleep Mode entry flag */
#define CLKPWR_PCON_SMFLAG		((uint32_t)(1<<8))
/** Deep Sleep entry flag */
#define CLKPWR_PCON_DSFLAG		((uint32_t)(1<<9))
/** Power-down entry flag */
#define CLKPWR_PCON_PDFLAG		((uint32_t)(1<<10))
/** Deep Power-down entry flag */
#define CLKPWR_PCON_DPDFLAG		((uint32_t)(1<<11))

/*********************************************************************//**
 * Macro defines for Power Control for Peripheral Register
 **********************************************************************/
/** Power Control for Peripherals bit mask */
#define CLKPWR_PCONP_BITMASK	0xEFEFF7DE

/**
 * @}
 */

/* Private Macros ------------------------------------------------------------- */
/** @defgroup I2C_Private_Macros I2C Private Macros
 * @{
 */

/* --------------------- BIT DEFINITIONS -------------------------------------- */
/*******************************************************************//**
 * I2C Control Set register description
 *********************************************************************/
#define I2C_I2CONSET_AA				((0x04)) /*!< Assert acknowledge flag */
#define I2C_I2CONSET_SI				((0x08)) /*!< I2C interrupt flag */
#define I2C_I2CONSET_STO			((0x10)) /*!< STOP flag */
#define I2C_I2CONSET_STA			((0x20)) /*!< START flag */
#define I2C_I2CONSET_I2EN			((0x40)) /*!< I2C interface enable */

/*******************************************************************//**
 * I2C Control Clear register description
 *********************************************************************/
/** Assert acknowledge Clear bit */
#define I2C_I2CONCLR_AAC			((1<<2))
/** I2C interrupt Clear bit */
#define I2C_I2CONCLR_SIC			((1<<3))
/** I2C STOP Clear bit */
#define I2C_I2CONCLR_STOC			((1<<4))
/** START flag Clear bit */
#define I2C_I2CONCLR_STAC			((1<<5))
/** I2C interface Disable bit */
#define I2C_I2CONCLR_I2ENC			((1<<6))

/********************************************************************//**
 * I2C Status Code definition (I2C Status register)
 *********************************************************************/
/* Return Code in I2C status register */
#define I2C_STAT_CODE_BITMASK		((0xF8))

/* I2C return status code definitions ----------------------------- */

/** No relevant information */
#define I2C_I2STAT_NO_INF						((0xF8))

/** Bus Error */
#define I2C_I2STAT_BUS_ERROR					((0x00))

/* Master transmit mode -------------------------------------------- */
/** A start condition has been transmitted */
#define I2C_I2STAT_M_TX_START					((0x08))

/** A repeat start condition has been transmitted */
#define I2C_I2STAT_M_TX_RESTART					((0x10))

/** SLA+W has been transmitted, ACK has been received */
#define I2C_I2STAT_M_TX_SLAW_ACK				((0x18))

/** SLA+W has been transmitted, NACK has been received */
#define I2C_I2STAT_M_TX_SLAW_NACK				((0x20))

/** Data has been transmitted, ACK has been received */
#define I2C_I2STAT_M_TX_DAT_ACK					((0x28))

/** Data has been transmitted, NACK has been received */
#define I2C_I2STAT_M_TX_DAT_NACK				((0x30))

/** Arbitration lost in SLA+R/W or Data bytes */
#define I2C_I2STAT_M_TX_ARB_LOST				((0x38))

/* Master receive mode -------------------------------------------- */
/** A start condition has been transmitted */
#define I2C_I2STAT_M_RX_START					((0x08))

/** A repeat start condition has been transmitted */
#define I2C_I2STAT_M_RX_RESTART					((0x10))

/** Arbitration lost */
#define I2C_I2STAT_M_RX_ARB_LOST				((0x38))

/** SLA+R has been transmitted, ACK has been received */
#define I2C_I2STAT_M_RX_SLAR_ACK				((0x40))

/** SLA+R has been transmitted, NACK has been received */
#define I2C_I2STAT_M_RX_SLAR_NACK				((0x48))

/** Data has been received, ACK has been returned */
#define I2C_I2STAT_M_RX_DAT_ACK					((0x50))

/** Data has been received, NACK has been return */
#define I2C_I2STAT_M_RX_DAT_NACK				((0x58))

/* Slave receive mode -------------------------------------------- */
/** Own slave address has been received, ACK has been returned */
#define I2C_I2STAT_S_RX_SLAW_ACK				((0x60))

/** Arbitration lost in SLA+R/W as master */
#define I2C_I2STAT_S_RX_ARB_LOST_M_SLA			((0x68))

/** General call address has been received, ACK has been returned */
#define I2C_I2STAT_S_RX_GENCALL_ACK				((0x70))

/** Arbitration lost in SLA+R/W (GENERAL CALL) as master */
#define I2C_I2STAT_S_RX_ARB_LOST_M_GENCALL		((0x78))

/** Previously addressed with own SLV address;
 * Data has been received, ACK has been return */
#define I2C_I2STAT_S_RX_PRE_SLA_DAT_ACK			((0x80))

/** Previously addressed with own SLA;
 * Data has been received and NOT ACK has been return */
#define I2C_I2STAT_S_RX_PRE_SLA_DAT_NACK		((0x88))

/** Previously addressed with General Call;
 * Data has been received and ACK has been return */
#define I2C_I2STAT_S_RX_PRE_GENCALL_DAT_ACK		((0x90))

/** Previously addressed with General Call;
 * Data has been received and NOT ACK has been return */
#define I2C_I2STAT_S_RX_PRE_GENCALL_DAT_NACK	((0x98))

/** A STOP condition or repeated START condition has
 * been received while still addressed as SLV/REC
 * (Slave Receive) or SLV/TRX (Slave Transmit) */
#define I2C_I2STAT_S_RX_STA_STO_SLVREC_SLVTRX	((0xA0))

/** Slave transmit mode */
/** Own SLA+R has been received, ACK has been returned */
#define I2C_I2STAT_S_TX_SLAR_ACK				((0xA8))

/** Arbitration lost in SLA+R/W as master */
#define I2C_I2STAT_S_TX_ARB_LOST_M_SLA			((0xB0))

/** Data has been transmitted, ACK has been received */
#define I2C_I2STAT_S_TX_DAT_ACK					((0xB8))

/** Data has been transmitted, NACK has been received */
#define I2C_I2STAT_S_TX_DAT_NACK				((0xC0))

/** Last data byte in I2DAT has been transmitted (AA = 0);
 ACK has been received */
#define I2C_I2STAT_S_TX_LAST_DAT_ACK			((0xC8))

/** Time out in case of using I2C slave mode */
#define I2C_SLAVE_TIME_OUT						0x10000UL

/********************************************************************//**
 * I2C Data register definition
 *********************************************************************/
/** Mask for I2DAT register*/
#define I2C_I2DAT_BITMASK			((0xFF))

/** Idle data value will be send out in slave mode in case of the actual
 * expecting data requested from the master is greater than its sending data
 * length that can be supported */
#define I2C_I2DAT_IDLE_CHAR			(0xFF)

/********************************************************************//**
 * I2C Monitor mode control register description
 *********************************************************************/
#define I2C_I2MMCTRL_MM_ENA			((1<<0))		/**< Monitor mode enable */
#define I2C_I2MMCTRL_ENA_SCL		((1<<1))		/**< SCL output enable */
#define I2C_I2MMCTRL_MATCH_ALL		((1<<2))		/**< Select interrupt register match */
#define I2C_I2MMCTRL_BITMASK		((0x07))		/**< Mask for I2MMCTRL register */

/********************************************************************//**
 * I2C Data buffer register description
 *********************************************************************/
/** I2C Data buffer register bit mask */
#define I2DATA_BUFFER_BITMASK		((0xFF))

/********************************************************************//**
 * I2C Slave Address registers definition
 *********************************************************************/
/** General Call enable bit */
#define I2C_I2ADR_GC				((1<<0))

/** I2C Slave Address registers bit mask */
#define I2C_I2ADR_BITMASK			((0xFF))

/********************************************************************//**
 * I2C Mask Register definition
 *********************************************************************/
/** I2C Mask Register mask field */
#define I2C_I2MASK_MASK(n)			((n&0xFE))

/********************************************************************//**
 * I2C SCL HIGH duty cycle Register definition
 *********************************************************************/
/** I2C SCL HIGH duty cycle Register bit mask */
#define I2C_I2SCLH_BITMASK			((0xFFFF))

/********************************************************************//**
 * I2C SCL LOW duty cycle Register definition
 *********************************************************************/
/** I2C SCL LOW duty cycle Register bit mask */
#define I2C_I2SCLL_BITMASK			((0xFFFF))


/* I2C status values */
#define I2C_SETUP_STATUS_ARBF   (1<<8)	/**< Arbitration false */
#define I2C_SETUP_STATUS_NOACKF (1<<9)	/**< No ACK returned */
#define I2C_SETUP_STATUS_DONE   (1<<10)	/**< Status DONE */

/*********************************************************************//**
 * I2C monitor control configuration defines
 **********************************************************************/
#define I2C_MONITOR_CFG_SCL_OUTPUT	I2C_I2MMCTRL_ENA_SCL		/**< SCL output enable */
#define I2C_MONITOR_CFG_MATCHALL	I2C_I2MMCTRL_MATCH_ALL		/**< Select interrupt register match */

/* ---------------- CHECK PARAMETER DEFINITIONS ---------------------------- */
/* Macros check I2C slave address */
#define PARAM_I2C_SLAVEADDR_CH(n)	(n<=3)

/** Macro to determine if it is valid SSP port number */
#define PARAM_I2Cx(n)	((((uint32_t *)n)==((uint32_t *)LPC_I2C0)) \
|| (((uint32_t *)n)==((uint32_t *)LPC_I2C1)) \
|| (((uint32_t *)n)==((uint32_t *)LPC_I2C2)))

/* Macros check I2C monitor configuration type */
#define PARAM_I2C_MONITOR_CFG(n) ((n==I2C_MONITOR_CFG_SCL_OUTPUT) || (I2C_MONITOR_CFG_MATCHALL))

/* I2C state handle return values */
#define I2C_OK					0x00
#define I2C_BYTE_SENT				0x01
#define I2C_BYTE_RECV				0x02
#define I2C_LAST_BYTE_RECV		0x04
#define I2C_SEND_END				0x08
#define I2C_RECV_END 				0x10
#define I2C_STA_STO_RECV			0x20

#define I2C_ERR				        (0x10000000)
#define I2C_NAK_RECV				(0x10000000 |0x01)

#define I2C_CheckError(ErrorCode)	(ErrorCode & 0x10000000)

/**
 * @}
 */



/* Public Types --------------------------------------------------------------- */
/** @defgroup I2C_Public_Types I2C Public Types
 * @{
 */

typedef enum
{
	I2C_0 = 0,
	I2C_1,
	I2C_2
} en_I2C_unitId;

typedef enum
{
	I2C_MASTER_MODE,
	I2C_SLAVE_MODE,
	I2C_GENERAL_MODE,
} en_I2C_Mode;
/**
 * @brief I2C Own slave address setting structure
 */
typedef struct {
	uint8_t SlaveAddrChannel;	/**< Slave Address channel in I2C control,
								should be in range from 0..3
								*/
	uint8_t SlaveAddr_7bit;		/**< Value of 7-bit slave address */
	uint8_t GeneralCallState;	/**< Enable/Disable General Call Functionality
								when I2C control being in Slave mode, should be:
								- ENABLE: Enable General Call function.
								- DISABLE: Disable General Call function.
								*/
	uint8_t SlaveAddrMaskValue;	/**< Any bit in this 8-bit value (bit 7:1)
								which is set to '1' will cause an automatic compare on
								the corresponding bit of the received address when it
								is compared to the SlaveAddr_7bit value associated with this
								mask register. In other words, bits in SlaveAddr_7bit value
								which are masked are not taken into account in determining
								an address match
								*/
} I2C_OWNSLAVEADDR_CFG_Type;


/**
 * @brief Master transfer setup data structure definitions
 */
typedef struct
{
  uint32_t          sl_addr7bit;				/**< Slave address in 7bit mode */
  __IO uint8_t*     tx_data;					/**< Pointer to Transmit data - NULL if data transmit
													  is not used */
  uint32_t          tx_length;					/**< Transmit data length - 0 if data transmit
													  is not used*/
  __IO uint32_t     tx_count;					/**< Current Transmit data counter */
  __IO uint8_t*     rx_data;					/**< Pointer to Receive data - NULL if data receive
													  is not used */
  uint32_t          rx_length;					/**< Receive data length - 0 if data receive is
													   not used */
  __IO uint32_t     rx_count;					/**< Current Receive data counter */
  uint32_t          retransmissions_max;		/**< Max Re-Transmission value */
  uint32_t          retransmissions_count;		/**< Current Re-Transmission counter */
  __IO uint32_t     status;						/**< Current status of I2C activity */
  void 				(*callback)(void);			/**< Pointer to Call back function when transmission complete
													used in interrupt transfer mode */
} I2C_M_SETUP_Type;


/**
 * @brief Slave transfer setup data structure definitions
 */
typedef struct
{
  __IO uint8_t*         tx_data;
  uint32_t              tx_length;
  __IO uint32_t         tx_count;
  __IO uint8_t*         rx_data;
  uint32_t              rx_length;
  __IO uint32_t         rx_count;
  __IO uint32_t         status;
  void 				(*callback)(void);
} I2C_S_SETUP_Type;

/**
 * @brief Transfer option type definitions
 */
typedef enum {
	I2C_TRANSFER_POLLING = 0,		/**< Transfer in polling mode */
	I2C_TRANSFER_INTERRUPT			/**< Transfer in interrupt mode */
} I2C_TRANSFER_OPT_Type;


/**
 * @}
 */

/* Private macros ------------------------------------------------------------- */
/** @defgroup ADC_Private_Macros ADC Private Macros
 * @{
 */

/* -------------------------- BIT DEFINITIONS ----------------------------------- */
/*********************************************************************//**
 * Macro defines for ADC  control register
 **********************************************************************/
/**  Selects which of the AD0.0:7 pins is (are) to be sampled and converted */
#define ADC_CR_CH_SEL(n)	((1UL << n))
/**  The APB clock (PCLK) is divided by (this value plus one)
* to produce the clock for the A/D */
#define ADC_CR_CLKDIV(n)	((n<<8))
/**  Repeated conversions A/D enable bit */
#define ADC_CR_BURST		((1UL<<16))
/**  ADC convert in power down mode */
#define ADC_CR_PDN			((1UL<<21))
/**  Start mask bits */
#define ADC_CR_START_MASK	((7UL<<24))
/**  Select Start Mode */
#define ADC_CR_START_MODE_SEL(SEL)	((SEL<<24))
/**  Start conversion now */
#define ADC_CR_START_NOW	((1UL<<24))
/**  Start conversion when the edge selected by bit 27 occurs on P2.10/EINT0 */
#define ADC_CR_START_EINT0	((2UL<<24))
/** Start conversion when the edge selected by bit 27 occurs on P1.27/CAP0.1 */
#define ADC_CR_START_CAP01	((3UL<<24))
/**  Start conversion when the edge selected by bit 27 occurs on MAT0.1 */
#define ADC_CR_START_MAT01	((4UL<<24))
/**  Start conversion when the edge selected by bit 27 occurs on MAT0.3 */
#define ADC_CR_START_MAT03	((5UL<<24))
/**  Start conversion when the edge selected by bit 27 occurs on MAT1.0 */
#define ADC_CR_START_MAT10	((6UL<<24))
/**  Start conversion when the edge selected by bit 27 occurs on MAT1.1 */
#define ADC_CR_START_MAT11	((7UL<<24))
/**  Start conversion on a falling edge on the selected CAP/MAT signal */
#define ADC_CR_EDGE			((1UL<<27))

/*********************************************************************//**
 * Macro defines for ADC Global Data register
 **********************************************************************/
/** When DONE is 1, this field contains result value of ADC conversion */
#define ADC_GDR_RESULT(n)		(((n>>4)&0xFFF))
/** These bits contain the channel from which the LS bits were converted */
#define ADC_GDR_CH(n)			(((n>>24)&0x7))
/** This bit is 1 in burst mode if the results of one or
 * more conversions was (were) lost */
#define ADC_GDR_OVERRUN_FLAG	((1UL<<30))
/** This bit is set to 1 when an A/D conversion completes */
#define ADC_GDR_DONE_FLAG		((1UL<<31))

/** This bits is used to mask for Channel */
#define ADC_GDR_CH_MASK		((7UL<<24))
/*********************************************************************//**
 * Macro defines for ADC Interrupt register
 **********************************************************************/
/** These bits allow control over which A/D channels generate
 * interrupts for conversion completion */
#define ADC_INTEN_CH(n)			((1UL<<n))
/** When 1, enables the global DONE flag in ADDR to generate an interrupt */
#define ADC_INTEN_GLOBAL		((1UL<<8))

/*********************************************************************//**
 * Macro defines for ADC Data register
 **********************************************************************/
/** When DONE is 1, this field contains result value of ADC conversion */
#define ADC_DR_RESULT(n)		(((n>>4)&0xFFF))
/** These bits mirror the OVERRRUN status flags that appear in the
 * result register for each A/D channel */
#define ADC_DR_OVERRUN_FLAG		((1UL<<30))
/** This bit is set to 1 when an A/D conversion completes. It is cleared
 * when this register is read */
#define ADC_DR_DONE_FLAG		((1UL<<31))

/*********************************************************************//**
 * Macro defines for ADC Status register
**********************************************************************/
/** These bits mirror the DONE status flags that appear in the result
 * register for each A/D channel */
#define ADC_STAT_CH_DONE_FLAG(n)		((n&0xFF))
/** These bits mirror the OVERRRUN status flags that appear in the
 * result register for each A/D channel */
#define ADC_STAT_CH_OVERRUN_FLAG(n)		(((n>>8)&0xFF))
/** This bit is the A/D interrupt flag */
#define ADC_STAT_INT_FLAG				((1UL<<16))

/*********************************************************************//**
 * Macro defines for ADC Trim register
**********************************************************************/
/** Offset trim bits for ADC operation */
#define ADC_ADCOFFS(n)		(((n&0xF)<<4))
/** Written to boot code*/
#define ADC_TRIM(n)		    (((n&0xF)<<8))

/* ------------------- CHECK PARAM DEFINITIONS ------------------------- */
/** Check ADC parameter */
#define PARAM_ADCx(n)    (((uint32_t *)n)==((uint32_t *)LPC_ADC))

/** Check ADC state parameter */
#define PARAM_ADC_START_ON_EDGE_OPT(OPT)    ((OPT == ADC_START_ON_RISING)||(OPT == ADC_START_ON_FALLING))

/** Check ADC state parameter */
#define PARAM_ADC_DATA_STATUS(OPT)    ((OPT== ADC_DATA_BURST)||(OPT== ADC_DATA_DONE))

/** Check ADC rate parameter */
#define PARAM_ADC_RATE(rate)	((rate>0)&&(rate<=200000))

/** Check ADC channel selection parameter */
#define PARAM_ADC_CHANNEL_SELECTION(SEL)	((SEL == ADC_CHANNEL_0)||(ADC_CHANNEL_1)\
||(SEL == ADC_CHANNEL_2)|(ADC_CHANNEL_3)\
||(SEL == ADC_CHANNEL_4)||(ADC_CHANNEL_5)\
||(SEL == ADC_CHANNEL_6)||(ADC_CHANNEL_7))

/** Check ADC start option parameter */
#define PARAM_ADC_START_OPT(OPT)    ((OPT == ADC_START_CONTINUOUS)||(OPT == ADC_START_NOW)\
||(OPT == ADC_START_ON_EINT0)||(OPT == ADC_START_ON_CAP01)\
||(OPT == ADC_START_ON_MAT01)||(OPT == ADC_START_ON_MAT03)\
||(OPT == ADC_START_ON_MAT10)||(OPT == ADC_START_ON_MAT11))

/** Check ADC interrupt type parameter */
#define PARAM_ADC_TYPE_INT_OPT(OPT)    ((OPT == ADC_ADINTEN0)||(OPT == ADC_ADINTEN1)\
||(OPT == ADC_ADINTEN2)||(OPT == ADC_ADINTEN3)\
||(OPT == ADC_ADINTEN4)||(OPT == ADC_ADINTEN5)\
||(OPT == ADC_ADINTEN6)||(OPT == ADC_ADINTEN7)\
||(OPT == ADC_ADGINTEN))

/**
 * @}
 */


/* Public Types --------------------------------------------------------------- */
/** @defgroup ADC_Public_Types ADC Public Types
 * @{
 */

/*********************************************************************//**
 * @brief ADC enumeration
 **********************************************************************/
/** @brief Channel Selection */
typedef enum
{
	ADC_CHANNEL_0  = 0, /*!<  Channel 0 */
	ADC_CHANNEL_1,		/*!<  Channel 1 */
	ADC_CHANNEL_2,		/*!<  Channel 2 */
	ADC_CHANNEL_3,		/*!<  Channel 3 */
	ADC_CHANNEL_4,		/*!<  Channel 4 */
	ADC_CHANNEL_5,		/*!<  Channel 5 */
	ADC_CHANNEL_6,		/*!<  Channel 6 */
	ADC_CHANNEL_7		/*!<  Channel 7 */
}ADC_CHANNEL_SELECTION;

/** @brief Type of start option */
typedef enum
{
	ADC_START_CONTINUOUS =0,	/*!< Continuous mode */
	ADC_START_NOW,				/*!< Start conversion now */
	ADC_START_ON_EINT0,			/*!< Start conversion when the edge selected
								 * by bit 27 occurs on P2.10/EINT0 */
	ADC_START_ON_CAP01,			/*!< Start conversion when the edge selected
								 * by bit 27 occurs on P1.27/CAP0.1 */
	ADC_START_ON_MAT01,			/*!< Start conversion when the edge selected
								 * by bit 27 occurs on MAT0.1 */
	ADC_START_ON_MAT03,			/*!< Start conversion when the edge selected
								 * by bit 27 occurs on MAT0.3 */
	ADC_START_ON_MAT10,			/*!< Start conversion when the edge selected
								  * by bit 27 occurs on MAT1.0 */
	ADC_START_ON_MAT11			/*!< Start conversion when the edge selected
								  * by bit 27 occurs on MAT1.1 */
} ADC_START_OPT;


/** @brief Type of edge when start conversion on the selected CAP/MAT signal */
typedef enum
{
	ADC_START_ON_RISING = 0,	/*!< Start conversion on a rising edge
								*on the selected CAP/MAT signal */
	ADC_START_ON_FALLING		/*!< Start conversion on a falling edge
								*on the selected CAP/MAT signal */
} ADC_START_ON_EDGE_OPT;

/** @brief* ADC type interrupt enum */
typedef enum
{
	ADC_ADINTEN0 = 0,		/*!< Interrupt channel 0 */
	ADC_ADINTEN1,			/*!< Interrupt channel 1 */
	ADC_ADINTEN2,			/*!< Interrupt channel 2 */
	ADC_ADINTEN3,			/*!< Interrupt channel 3 */
	ADC_ADINTEN4,			/*!< Interrupt channel 4 */
	ADC_ADINTEN5,			/*!< Interrupt channel 5 */
	ADC_ADINTEN6,			/*!< Interrupt channel 6 */
	ADC_ADINTEN7,			/*!< Interrupt channel 7 */
	ADC_ADGINTEN			/*!< Individual channel/global flag done generate an interrupt */
}ADC_TYPE_INT_OPT;

/** @brief ADC Data  status */
typedef enum
{
	ADC_DATA_BURST = 0,		/*Burst bit*/
	ADC_DATA_DONE		 /*Done bit*/
}ADC_DATA_STATUS;

/**
 * @}
 */

/* Private Macros ------------------------------------------------------------- */
/** @defgroup PWM_Private_Macros PWM Private Macros
 * @{
 */

/* --------------------- BIT DEFINITIONS -------------------------------------- */
/**********************************************************************
* IR register definitions
**********************************************************************/
/** Interrupt flag for PWM match channel for 6 channel */
#define PWM_IR_PWMMRn(n)    	((uint32_t)((n<4)?(1<<n):(1<<(n+4))))
/** Interrupt flag for capture input */
#define PWM_IR_PWMCAPn(n)		((uint32_t)(1<<(n+4)))
/**  IR register mask */
#define PWM_IR_BITMASK			((uint32_t)(0x0000073F))

/**********************************************************************
* TCR register definitions
**********************************************************************/
/** TCR register mask */
#define PWM_TCR_BITMASK				((uint32_t)(0x0000000B))
#define PWM_TCR_COUNTER_ENABLE      ((uint32_t)(1<<0)) /*!< PWM Counter Enable */
#define PWM_TCR_COUNTER_RESET       ((uint32_t)(1<<1)) /*!< PWM Counter Reset */
#define PWM_TCR_PWM_ENABLE          ((uint32_t)(1<<3)) /*!< PWM Enable */

/**********************************************************************
* CTCR register definitions
**********************************************************************/
/** CTCR register mask */
#define PWM_CTCR_BITMASK			((uint32_t)(0x0000000F))
/** PWM Counter-Timer Mode */
#define PWM_CTCR_MODE(n)        	((uint32_t)(n&0x03))
/** PWM Capture input select */
#define PWM_CTCR_SELECT_INPUT(n)	((uint32_t)((n&0x03)<<2))

/**********************************************************************
* MCR register definitions
**********************************************************************/
/** MCR register mask */
#define PWM_MCR_BITMASK				((uint32_t)(0x001FFFFF))
/** generate a PWM interrupt when a MATCHn occurs */
#define PWM_MCR_INT_ON_MATCH(n)     ((uint32_t)(1<<(((n&0x7)<<1)+(n&0x07))))
/** reset the PWM when a MATCHn occurs */
#define PWM_MCR_RESET_ON_MATCH(n)   ((uint32_t)(1<<(((n&0x7)<<1)+(n&0x07)+1)))
/** stop the PWM when a MATCHn occurs */
#define PWM_MCR_STOP_ON_MATCH(n)    ((uint32_t)(1<<(((n&0x7)<<1)+(n&0x07)+2)))

/**********************************************************************
* CCR register definitions
**********************************************************************/
/** CCR register mask */
#define PWM_CCR_BITMASK				((uint32_t)(0x0000003F))
/** PCAPn is rising edge sensitive */
#define PWM_CCR_CAP_RISING(n) 	 	((uint32_t)(1<<(((n&0x2)<<1)+(n&0x1))))
/** PCAPn is falling edge sensitive */
#define PWM_CCR_CAP_FALLING(n) 		((uint32_t)(1<<(((n&0x2)<<1)+(n&0x1)+1)))
/** PWM interrupt is generated on a PCAP event */
#define PWM_CCR_INT_ON_CAP(n)  		((uint32_t)(1<<(((n&0x2)<<1)+(n&0x1)+2)))

/**********************************************************************
* PCR register definitions
**********************************************************************/
/** PCR register mask */
#define PWM_PCR_BITMASK			(uint32_t)0x00007E7C
/** PWM output n is a single edge controlled output */
#define PWM_PCR_PWMSELn(n)   	((uint32_t)(((n&0x7)<2) ? 0 : (1<<n)))
/** enable PWM output n */
#define PWM_PCR_PWMENAn(n)   	((uint32_t)(((n&0x7)<1) ? 0 : (1<<(n+8))))

/**********************************************************************
* LER register definitions
**********************************************************************/
/** LER register mask*/
#define PWM_LER_BITMASK				((uint32_t)(0x0000007F))
/** PWM MATCHn register update control */
#define PWM_LER_EN_MATCHn_LATCH(n)   ((uint32_t)((n<7) ? (1<<n) : 0))

/* ---------------- CHECK PARAMETER DEFINITIONS ---------------------------- */
/** Macro to determine if it is valid PWM peripheral or not */
#define PARAM_PWMx(n)	(((uint32_t *)n)==((uint32_t *)LPC_PWM1))

/** Macro check PWM1 match channel value */
#define PARAM_PWM1_MATCH_CHANNEL(n)		(n<=6)

/** Macro check PWM1 channel value */
#define PARAM_PWM1_CHANNEL(n)			((n>=1) && (n<=6))

/** Macro check PWM1 edge channel mode */
#define PARAM_PWM1_EDGE_MODE_CHANNEL(n)			((n>=2) && (n<=6))

/** Macro check PWM1 capture channel mode */
#define PARAM_PWM1_CAPTURE_CHANNEL(n)	((n==0) || (n==1))

/** Macro check PWM1 interrupt status type */
#define PARAM_PWM_INTSTAT(n)	((n==PWM_INTSTAT_MR0) || (n==PWM_INTSTAT_MR1) || (n==PWM_INTSTAT_MR2) \
|| (n==PWM_INTSTAT_MR3) || (n==PWM_INTSTAT_MR4) || (n==PWM_INTSTAT_MR5) \
|| (n==PWM_INTSTAT_MR6) || (n==PWM_INTSTAT_CAP0) || (n==PWM_INTSTAT_CAP1))
/**
 * @}
 */


/* Public Types --------------------------------------------------------------- */
/** @defgroup PWM_Public_Types PWM Public Types
 * @{
 */

/** @brief Configuration structure in PWM TIMER mode */
typedef struct {

	uint8_t PrescaleOption;		/**< Prescale option, should be:
								- PWM_TIMER_PRESCALE_TICKVAL: Prescale in absolute value
								- PWM_TIMER_PRESCALE_USVAL: Prescale in microsecond value
								*/
	uint8_t Reserved[3];
	uint32_t PrescaleValue;		/**< Prescale value, 32-bit long, should be matched
								with PrescaleOption
								*/
} PWM_TIMERCFG_Type;

/** @brief Configuration structure in PWM COUNTER mode */
typedef struct {

	uint8_t CounterOption;		/**< Counter Option, should be:
								- PWM_COUNTER_RISING: Rising Edge
								- PWM_COUNTER_FALLING: Falling Edge
								- PWM_COUNTER_ANY: Both rising and falling mode
								*/
	uint8_t CountInputSelect;	/**< Counter input select, should be:
								- PWM_COUNTER_PCAP1_0: PWM Counter input selected is PCAP1.0 pin
								- PWM_COUNTER_PCAP1_1: PWM Counter input selected is PCAP1.1 pin
								*/
	uint8_t Reserved[2];
} PWM_COUNTERCFG_Type;

/** @brief PWM Match channel configuration structure */
typedef struct {
	uint8_t MatchChannel;	/**< Match channel, should be in range
							from 0..6 */
	uint8_t IntOnMatch;		/**< Interrupt On match, should be:
							- ENABLE: Enable this function.
							- DISABLE: Disable this function.
							*/
	uint8_t StopOnMatch;	/**< Stop On match, should be:
							- ENABLE: Enable this function.
							- DISABLE: Disable this function.
							*/
	uint8_t ResetOnMatch;	/**< Reset On match, should be:
							- ENABLE: Enable this function.
							- DISABLE: Disable this function.
							*/
} PWM_MATCHCFG_Type;


/** @brief PWM Capture Input configuration structure */
typedef struct {
	uint8_t CaptureChannel;	/**< Capture channel, should be in range
							from 0..1 */
	uint8_t RisingEdge;		/**< caption rising edge, should be:
							- ENABLE: Enable rising edge.
							- DISABLE: Disable this function.
							*/
	uint8_t FallingEdge;		/**< caption falling edge, should be:
							- ENABLE: Enable falling edge.
							- DISABLE: Disable this function.
								*/
	uint8_t IntOnCaption;	/**< Interrupt On caption, should be:
							- ENABLE: Enable interrupt function.
							- DISABLE: Disable this function.
							*/
} PWM_CAPTURECFG_Type;

/* Timer/Counter in PWM configuration type definition -----------------------------------*/

/** @brief PMW TC mode select option */
typedef enum {
	PWM_MODE_TIMER = 0,		/*!< PWM using Timer mode */
	PWM_MODE_COUNTER		/*!< PWM using Counter mode */
} PWM_TC_MODE_OPT;

#define PARAM_PWM_TC_MODE(n) ((n==PWM_MODE_TIMER) || (n==PWM_MODE_COUNTER))


/** @brief PWM Timer/Counter prescale option */
typedef enum
{
	PWM_TIMER_PRESCALE_TICKVAL = 0,			/*!< Prescale in absolute value */
	PWM_TIMER_PRESCALE_USVAL				/*!< Prescale in microsecond value */
} PWM_TIMER_PRESCALE_OPT;

#define PARAM_PWM_TIMER_PRESCALE(n) ((n==PWM_TIMER_PRESCALE_TICKVAL) || (n==PWM_TIMER_PRESCALE_USVAL))


/** @brief PWM Input Select in counter mode */
typedef enum {
	PWM_COUNTER_PCAP1_0 = 0,		/*!< PWM Counter input selected is PCAP1.0 pin */
	PWM_COUNTER_PCAP1_1			/*!< PWM counter input selected is CAP1.1 pin */
} PWM_COUNTER_INPUTSEL_OPT;

#define PARAM_PWM_COUNTER_INPUTSEL(n) ((n==PWM_COUNTER_PCAP1_0) || (n==PWM_COUNTER_PCAP1_1))

/** @brief PWM Input Edge Option in counter mode */
typedef enum {
    PWM_COUNTER_RISING = 1,		/*!< Rising edge mode */
    PWM_COUNTER_FALLING = 2,	/*!< Falling edge mode */
    PWM_COUNTER_ANY = 3			/*!< Both rising and falling mode */
} PWM_COUNTER_EDGE_OPT;

#define PARAM_PWM_COUNTER_EDGE(n)	((n==PWM_COUNTER_RISING) || (n==PWM_COUNTER_FALLING) \
|| (n==PWM_COUNTER_ANY))


/* PWM configuration type definition ----------------------------------------------------- */
/** @brief PWM operating mode options */
typedef enum {
    PWM_CHANNEL_SINGLE_EDGE,	/*!< PWM Channel Single edge mode */
    PWM_CHANNEL_DUAL_EDGE		/*!< PWM Channel Dual edge mode */
} PWM_CHANNEL_EDGE_OPT;

#define PARAM_PWM_CHANNEL_EDGE(n)	((n==PWM_CHANNEL_SINGLE_EDGE) || (n==PWM_CHANNEL_DUAL_EDGE))


/** @brief PWM update type */
typedef enum {
	PWM_MATCH_UPDATE_NOW = 0,			/**< PWM Match Channel Update Now */
	PWM_MATCH_UPDATE_NEXT_RST			/**< PWM Match Channel Update on next
											PWM Counter resetting */
} PWM_MATCH_UPDATE_OPT;

#define PARAM_PWM_MATCH_UPDATE(n)	((n==PWM_MATCH_UPDATE_NOW) || (n==PWM_MATCH_UPDATE_NEXT_RST))


/** @brief PWM interrupt status type definition ----------------------------------------------------- */
/** @brief PWM Interrupt status type */
typedef enum
{
	PWM_INTSTAT_MR0 = PWM_IR_PWMMRn(0), 	/**< Interrupt flag for PWM match channel 0 */
	PWM_INTSTAT_MR1 = PWM_IR_PWMMRn(1),		/**< Interrupt flag for PWM match channel 1 */
	PWM_INTSTAT_MR2 = PWM_IR_PWMMRn(2),		/**< Interrupt flag for PWM match channel 2 */
	PWM_INTSTAT_MR3 = PWM_IR_PWMMRn(3),		/**< Interrupt flag for PWM match channel 3 */
	PWM_INTSTAT_CAP0 = PWM_IR_PWMCAPn(0),	/**< Interrupt flag for capture input 0 */
	PWM_INTSTAT_CAP1 = PWM_IR_PWMCAPn(1),	/**< Interrupt flag for capture input 1 */
	PWM_INTSTAT_MR4 = PWM_IR_PWMMRn(4),		/**< Interrupt flag for PWM match channel 4 */
	PWM_INTSTAT_MR6 = PWM_IR_PWMMRn(5),		/**< Interrupt flag for PWM match channel 5 */
	PWM_INTSTAT_MR5 = PWM_IR_PWMMRn(6)		/**< Interrupt flag for PWM match channel 6 */
}PWM_INTSTAT_TYPE;

/** @brief Match update structure */
typedef struct
{
	uint32_t Matchvalue;
	FlagStatus Status;
}PWM_Match_T;

/**
 * @}
 */

/* Private Macros ------------------------------------------------------------- */
/** @defgroup TIM_Private_Macros TIM Private Macros
 * @{
 */

/* --------------------- BIT DEFINITIONS -------------------------------------- */
/**********************************************************************
** Interrupt information
**********************************************************************/
/** Macro to clean interrupt pending */
#define TIM_IR_CLR(n) _BIT(n)

/**********************************************************************
** Timer interrupt register definitions
**********************************************************************/
/** Macro for getting a timer match interrupt bit */
#define TIM_MATCH_INT(n)		(_BIT(n & 0x0F))
/** Macro for getting a capture event interrupt bit */
#define TIM_CAP_INT(n)     (_BIT(((n & 0x0F) + 4)))

/**********************************************************************
* Timer control register definitions
**********************************************************************/
/** Timer/counter enable bit */
#define TIM_ENABLE			((uint32_t)(1<<0))
/** Timer/counter reset bit */
#define TIM_RESET			((uint32_t)(1<<1))
/** Timer control bit mask */
#define TIM_TCR_MASKBIT		((uint32_t)(3))

/**********************************************************************
* Timer match control register definitions
**********************************************************************/
/** Bit location for interrupt on MRx match, n = 0 to 3 */
#define TIM_INT_ON_MATCH(n)      	(_BIT((n * 3)))
/** Bit location for reset on MRx match, n = 0 to 3 */
#define TIM_RESET_ON_MATCH(n)    	(_BIT(((n * 3) + 1)))
/** Bit location for stop on MRx match, n = 0 to 3 */
#define TIM_STOP_ON_MATCH(n)     	(_BIT(((n * 3) + 2)))
/** Timer Match control bit mask */
#define TIM_MCR_MASKBIT			   ((uint32_t)(0x0FFF))
/** Timer Match control bit mask for specific channel*/
#define	TIM_MCR_CHANNEL_MASKBIT(n)		((uint32_t)(7<<(n*3)))

/**********************************************************************
* Timer capture control register definitions
**********************************************************************/
/** Bit location for CAP.n on CRx rising edge, n = 0 to 3 */
#define TIM_CAP_RISING(n)   	(_BIT((n * 3)))
/** Bit location for CAP.n on CRx falling edge, n = 0 to 3 */
#define TIM_CAP_FALLING(n)   	(_BIT(((n * 3) + 1)))
/** Bit location for CAP.n on CRx interrupt enable, n = 0 to 3 */
#define TIM_INT_ON_CAP(n)    	(_BIT(((n * 3) + 2)))
/** Mask bit for rising and falling edge bit */
#define TIM_EDGE_MASK(n)		(_SBF((n * 3), 0x03))
/** Timer capture control bit mask */
#define TIM_CCR_MASKBIT			((uint32_t)(0x3F))
/** Timer Capture control bit mask for specific channel*/
#define	TIM_CCR_CHANNEL_MASKBIT(n)		((uint32_t)(7<<(n*3)))

/**********************************************************************
* Timer external match register definitions
**********************************************************************/
/** Bit location for output state change of MAT.n when external match
   happens, n = 0 to 3 */
#define TIM_EM(n)    			_BIT(n)
/** Output state change of MAT.n when external match happens: no change */
#define TIM_EM_NOTHING    	((uint8_t)(0x0))
/** Output state change of MAT.n when external match happens: low */
#define TIM_EM_LOW         	((uint8_t)(0x1))
/** Output state change of MAT.n when external match happens: high */
#define TIM_EM_HIGH        	((uint8_t)(0x2))
/** Output state change of MAT.n when external match happens: toggle */
#define TIM_EM_TOGGLE      	((uint8_t)(0x3))
/** Macro for setting for the MAT.n change state bits */
#define TIM_EM_SET(n,s) 	(_SBF(((n << 1) + 4), (s & 0x03)))
/** Mask for the MAT.n change state bits */
#define TIM_EM_MASK(n) 		(_SBF(((n << 1) + 4), 0x03))
/** Timer external match bit mask */
#define TIM_EMR_MASKBIT	0x0FFF

/**********************************************************************
* Timer Count Control Register definitions
**********************************************************************/
/** Mask to get the Counter/timer mode bits */
#define TIM_CTCR_MODE_MASK  0x3
/** Mask to get the count input select bits */
#define TIM_CTCR_INPUT_MASK 0xC
/** Timer Count control bit mask */
#define TIM_CTCR_MASKBIT	0xF
#define TIM_COUNTER_MODE ((uint8_t)(1))


/* ---------------- CHECK PARAMETER DEFINITIONS ---------------------------- */
/** Macro to determine if it is valid TIMER peripheral */
#define PARAM_TIMx(n)	((((uint32_t *)n)==((uint32_t *)LPC_TIM0)) || (((uint32_t *)n)==((uint32_t *)LPC_TIM1)) \
|| (((uint32_t *)n)==((uint32_t *)LPC_TIM2)) || (((uint32_t *)n)==((uint32_t *)LPC_TIM3)))

/* Macro check interrupt type */
#define PARAM_TIM_INT_TYPE(TYPE)	((TYPE ==TIM_MR0_INT)||(TYPE ==TIM_MR1_INT)\
||(TYPE ==TIM_MR2_INT)||(TYPE ==TIM_MR3_INT)\
||(TYPE ==TIM_CR0_INT)||(TYPE ==TIM_CR1_INT))

/* Macro check TIMER mode */
#define PARAM_TIM_MODE_OPT(MODE)	((MODE == TIM_TIMER_MODE)||(MODE == TIM_COUNTER_RISING_MODE)\
|| (MODE == TIM_COUNTER_RISING_MODE)||(MODE == TIM_COUNTER_RISING_MODE))

/* Macro check TIMER prescale value */
#define PARAM_TIM_PRESCALE_OPT(OPT)	((OPT == TIM_PRESCALE_TICKVAL)||(OPT == TIM_PRESCALE_USVAL))

/* Macro check TIMER counter intput mode */
#define PARAM_TIM_COUNTER_INPUT_OPT(OPT)	((OPT == TIM_COUNTER_INCAP0)||(OPT == TIM_COUNTER_INCAP1))

/* Macro check TIMER external match mode */
#define PARAM_TIM_EXTMATCH_OPT(OPT)	((OPT == TIM_EXTMATCH_NOTHING)||(OPT == TIM_EXTMATCH_LOW)\
||(OPT == TIM_EXTMATCH_HIGH)||(OPT == TIM_EXTMATCH_TOGGLE))

/* Macro check TIMER external match mode */
#define PARAM_TIM_CAP_MODE_OPT(OPT)	((OPT == TIM_CAPTURE_NONE)||(OPT == TIM_CAPTURE_RISING) \
||(OPT == TIM_CAPTURE_FALLING)||(OPT == TIM_CAPTURE_ANY))

/**
 * @}
 */


/* Public Types --------------------------------------------------------------- */
/** @defgroup TIM_Public_Types TIM Public Types
 * @{
 */

/***********************************************************************
 * Timer device enumeration
**********************************************************************/
/** @brief interrupt type */
typedef enum
{
	TIM_MR0_INT =0, /*!< interrupt for Match channel 0*/
	TIM_MR1_INT =1, /*!< interrupt for Match channel 1*/
	TIM_MR2_INT =2, /*!< interrupt for Match channel 2*/
	TIM_MR3_INT =3, /*!< interrupt for Match channel 3*/
	TIM_CR0_INT =4, /*!< interrupt for Capture channel 0*/
	TIM_CR1_INT =5 /*!< interrupt for Capture channel 1*/
}TIM_INT_TYPE;

/** @brief Timer/counter operating mode */
typedef enum
{
	TIM_TIMER_MODE = 0,				/*!< Timer mode */
	TIM_COUNTER_RISING_MODE,		/*!< Counter rising mode */
	TIM_COUNTER_FALLING_MODE,		/*!< Counter falling mode */
	TIM_COUNTER_ANY_MODE			/*!< Counter on both edges */
} TIM_MODE_OPT;

/** @brief Timer/Counter prescale option */
typedef enum
{
	TIM_PRESCALE_TICKVAL = 0,		/*!< Prescale in absolute value */
	TIM_PRESCALE_USVAL				/*!< Prescale in microsecond value */
} TIM_PRESCALE_OPT;

/** @brief Counter input option */
typedef enum
{
	TIM_COUNTER_INCAP0 = 0,			/*!< CAPn.0 input pin for TIMERn */
	TIM_COUNTER_INCAP1,				/*!< CAPn.1 input pin for TIMERn */
} TIM_COUNTER_INPUT_OPT;

/** @brief Timer/Counter external match option */
typedef enum
{
	TIM_EXTMATCH_NOTHING = 0,		/*!< Do nothing for external output pin if match */
	TIM_EXTMATCH_LOW,				/*!< Force external output pin to low if match */
	TIM_EXTMATCH_HIGH,				/*!< Force external output pin to high if match */
	TIM_EXTMATCH_TOGGLE				/*!< Toggle external output pin if match */
}TIM_EXTMATCH_OPT;

/** @brief Timer/counter capture mode options */
typedef enum {
	TIM_CAPTURE_NONE = 0,	/*!< No Capture */
	TIM_CAPTURE_RISING,		/*!< Rising capture mode */
	TIM_CAPTURE_FALLING,	/*!< Falling capture mode */
	TIM_CAPTURE_ANY			/*!< On both edges */
} TIM_CAP_MODE_OPT;

/** @brief Configuration structure in TIMER mode */
typedef struct
{

	uint8_t PrescaleOption;		/**< Timer Prescale option, should be:
									- TIM_PRESCALE_TICKVAL: Prescale in absolute value
									- TIM_PRESCALE_USVAL: Prescale in microsecond value
									*/
	uint8_t Reserved[3];		/**< Reserved */
	uint32_t PrescaleValue;		/**< Prescale value */
} TIM_TIMERCFG_Type;

/** @brief Configuration structure in COUNTER mode */
typedef struct {

	uint8_t CounterOption;		/**< Counter Option, should be:
								- TIM_COUNTER_INCAP0: CAPn.0 input pin for TIMERn
								- TIM_COUNTER_INCAP1: CAPn.1 input pin for TIMERn
								*/
	uint8_t CountInputSelect;
	uint8_t Reserved[2];
} TIM_COUNTERCFG_Type;

/** @brief Match channel configuration structure */
typedef struct {
	uint8_t MatchChannel;	/**< Match channel, should be in range
							from 0..3 */
	uint8_t IntOnMatch;		/**< Interrupt On match, should be:
							- ENABLE: Enable this function.
							- DISABLE: Disable this function.
							*/
	uint8_t StopOnMatch;	/**< Stop On match, should be:
							- ENABLE: Enable this function.
							- DISABLE: Disable this function.
							*/
	uint8_t ResetOnMatch;	/**< Reset On match, should be:
							- ENABLE: Enable this function.
							- DISABLE: Disable this function.
							*/

	uint8_t ExtMatchOutputType;	/**< External Match Output type, should be:
							 -	 TIM_EXTMATCH_NOTHING:	Do nothing for external output pin if match
							 -   TIM_EXTMATCH_LOW:	Force external output pin to low if match
							 - 	 TIM_EXTMATCH_HIGH: Force external output pin to high if match
							 -   TIM_EXTMATCH_TOGGLE: Toggle external output pin if match.
							*/
	uint8_t Reserved[3];	/** Reserved */
	uint32_t MatchValue;	/** Match value */
} TIM_MATCHCFG_Type;

/** @brief Capture Input configuration structure */
typedef struct {
	uint8_t CaptureChannel;	/**< Capture channel, should be in range
							from 0..1 */
	uint8_t RisingEdge;		/**< caption rising edge, should be:
							- ENABLE: Enable rising edge.
							- DISABLE: Disable this function.
							*/
	uint8_t FallingEdge;		/**< caption falling edge, should be:
							- ENABLE: Enable falling edge.
							- DISABLE: Disable this function.
								*/
	uint8_t IntOnCaption;	/**< Interrupt On caption, should be:
							- ENABLE: Enable interrupt function.
							- DISABLE: Disable this function.
							*/

} TIM_CAPTURECFG_Type;

/**
 * @}
 */

/* Private Macros ------------------------------------------------------------- */
/** @defgroup SYSTICK_Private_Macros SYSTICK Private Macros
 * @{
 */
/*********************************************************************//**
 * Macro defines for System Timer Control and status (STCTRL) register
 **********************************************************************/
#define ST_CTRL_ENABLE		((uint32_t)(1<<0))
#define ST_CTRL_TICKINT		((uint32_t)(1<<1))
#define ST_CTRL_CLKSOURCE	((uint32_t)(1<<2))
#define ST_CTRL_COUNTFLAG	((uint32_t)(1<<16))

/*********************************************************************//**
 * Macro defines for System Timer Reload value (STRELOAD) register
 **********************************************************************/
#define ST_RELOAD_RELOAD(n)		((uint32_t)(n & 0x00FFFFFF))

/*********************************************************************//**
 * Macro defines for System Timer Current value (STCURRENT) register
 **********************************************************************/
#define ST_RELOAD_CURRENT(n)	((uint32_t)(n & 0x00FFFFFF))

/*********************************************************************//**
 * Macro defines for System Timer Calibration value (STCALIB) register
 **********************************************************************/
#define ST_CALIB_TENMS(n)		((uint32_t)(n & 0x00FFFFFF))
#define ST_CALIB_SKEW			((uint32_t)(1<<30))
#define ST_CALIB_NOREF			((uint32_t)(1<<31))

#define CLKSOURCE_EXT			((uint32_t)(0))
#define CLKSOURCE_CPU			((uint32_t)(1))

/**
 * @}
 */

#ifdef __cplusplus
}
#endif