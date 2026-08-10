//*****************************************************************************
//
//! \file wizchip_conf.h
//! \brief WIZCHIP Config Header File.
//! \version 1.0.0
//! \date 2013/10/21
//! \par  Revision history
//!       <2015/02/05> Notice
//!        The version history is not updated after this point.
//!        Download the latest version directly from GitHub. Please visit the our GitHub repository for ioLibrary.
//!        >> https://github.com/Wiznet/ioLibrary_Driver
//!       <2013/10/21> 1st Release
//! \author MidnightCow
//! \copyright
//!
//! Copyright (c)  2013, WIZnet Co., LTD.
//! All rights reserved.
//!
//! Redistribution and use in source and binary forms, with or without
//! modification, are permitted provided that the following conditions
//! are met:
//!
//!     * Redistributions of source code must retain the above copyright
//! notice, this list of conditions and the following disclaimer.
//!     * Redistributions in binary form must reproduce the above copyright
//! notice, this list of conditions and the following disclaimer in the
//! documentation and/or other materials provided with the distribution.
//!     * Neither the name of the <ORGANIZATION> nor the names of its
//! contributors may be used to endorse or promote products derived
//! from this software without specific prior written permission.
//!
//! THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
//! AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
//! IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
//! ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
//! LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
//! CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
//! SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
//! INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
//! CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
//! ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
//! THE POSSIBILITY OF SUCH DAMAGE.
//
//*****************************************************************************

/**
    @defgroup extra_functions 2. WIZnet Extra Functions

    @brief These functions is optional function. It could be replaced at WIZCHIP I/O function because they were made by WIZCHIP I/O functions.
    @details There are functions of configuring WIZCHIP, network, interrupt, phy, network information and timer. \n

*/

#ifndef  _WIZCHIP_CONF_H_
#define  _WIZCHIP_CONF_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
/**
    @brief Select WIZCHIP.
    @todo You should select one, \b W5100, \b W5100S, \b W5200, \b W5300, \b W5500 or etc. \n\n
         ex> <code> #define \_WIZCHIP_      W5500 </code>
*/

#define W5100						5100
#define W5100S						5100+5
#define W5200						5200
#define W5300						5300
#define W5500						5500
#define W6100						6100
#define W6300                 6300


#ifndef _WIZCHIP_
// NOTE_LIHAN: Some sections of this code are not yet fully defined.
#define _WIZCHIP_                      W5500   // W5100, W5100S, W5200, W5300, W5500, 6300
#endif

//
//#ifndef _WIZCHIP_
//   #error  please Define your WIZnet chip numer
//#endif


#define _WIZCHIP_IO_MODE_NONE_         0x0000
#define _WIZCHIP_IO_MODE_BUS_          0x0100 /**< Bus interface mode */
#define _WIZCHIP_IO_MODE_SPI_          0x0200 /**< SPI interface mode */
//#define _WIZCHIP_IO_MODE_IIC_          0x0400
//#define _WIZCHIP_IO_MODE_SDIO_         0x0800
// Add to
//

#define _WIZCHIP_IO_MODE_BUS_DIR_      (_WIZCHIP_IO_MODE_BUS_ + 1) /**< BUS interface mode for direct  */
#define _WIZCHIP_IO_MODE_BUS_INDIR_    (_WIZCHIP_IO_MODE_BUS_ + 2) /**< BUS interface mode for indirect */

#define _WIZCHIP_IO_MODE_SPI_VDM_      (_WIZCHIP_IO_MODE_SPI_ + 1) /**< SPI interface mode for variable length data*/
#define _WIZCHIP_IO_MODE_SPI_FDM_      (_WIZCHIP_IO_MODE_SPI_ + 2) /**< SPI interface mode for fixed length data mode*/
#define _WIZCHIP_IO_MODE_SPI_5500_     (_WIZCHIP_IO_MODE_SPI_ + 3) /**< SPI interface mode for fixed length data mode*/
//teddy 240122
#define _WIZCHIP_IO_MODE_SPI_QSPI_     (_WIZCHIP_IO_MODE_SPI_ + 4) /**< SPI interface mode for QSPI mode*/



/**
    @brief PHY can be accessed by @ref _PHYCR0_, _PHYCR1_.
    @details It provides hardware access method.
    @note It is smaller s/w footprint than @ref _PHY_IO_MODE_MII_.
    @sa _PHY_IO_MODE_MII_, _PHY_IO_MODE_
    @sa ctlwizchip(), getPHYCR0(), getPHYCR1(), setPHYCR1(), getPHYSR()
*/
#define _PHY_IO_MODE_PHYCR_            0x0000

/**
    @brief PHY can be accessed by MDC/MDIO signals of MII interface.
    @details It provide software access method.
    @note It is bigger s/w footprint than @ref _PHY_IO_MODE_PHYCR_.
    @sa _PHY_IO_MODE_PHYCR_, _PHY_IO_MODE_
    @sa ctlwizchip(), wiz_read_mdio(), wiz_write_mdio()
*/
#define _PHY_IO_MODE_MII_              0x0010

/**
    @brief Select PHY Access Mode
    @details @ref _PHY_IO_MODE_ selects PHY access method.
    @todo You should select one of @ref _PHY_IO_MODE_PHYCR_ or @ref _PHY_IO_MODE_MII_.
    @sa ctlwizchip()
*/
#define _PHY_IO_MODE_                  _PHY_IO_MODE_MII_ //_PHY_IO_MODE_MII_



#if   (_WIZCHIP_ == W5100)
#define _WIZCHIP_ID_                "W5100\0"
/**
    @brief Define interface mode.
    @todo you should select interface mode as chip. Select one of @ref \_WIZCHIP_IO_MODE_SPI_ , @ref \_WIZCHIP_IO_MODE_BUS_DIR_ or @ref \_WIZCHIP_IO_MODE_BUS_INDIR_
*/
// 	#define _WIZCHIP_IO_MODE_           _WIZCHIP_IO_MODE_BUS_DIR_
//	#define _WIZCHIP_IO_MODE_           _WIZCHIP_IO_MODE_BUS_INDIR_
#define _WIZCHIP_IO_MODE_           _WIZCHIP_IO_MODE_SPI_

//A20150601 : Define the unit of IO DATA.
typedef   uint8_t   iodata_t;
//A20150401 : Indclude W5100.h file
#include "W5100/w5100.h"

#elif (_WIZCHIP_ == W5100S)
#define _WIZCHIP_ID_                "W5100S\0"
/**
    @brief Define interface mode.
    @todo you should select interface mode as chip. Select one of @ref \_WIZCHIP_IO_MODE_SPI_ , @ref \_WIZCHIP_IO_MODE_BUS_DIR_ or @ref \_WIZCHIP_IO_MODE_BUS_INDIR_
*/
#if 0
#define _WIZCHIP_IO_MODE_           _WIZCHIP_IO_MODE_BUS_INDIR_
#elif 0
#define _WIZCHIP_IO_MODE_           _WIZCHIP_IO_MODE_SPI_5500_
#else
#define _WIZCHIP_IO_MODE_           _WIZCHIP_IO_MODE_SPI_
#endif

//A20150601 : Define the unit of IO DATA.
typedef   uint8_t   iodata_t;
//A20150401 : Indclude W5100.h file
#include "W5100S/w5100s.h"
#elif (_WIZCHIP_ == W5200)
#define _WIZCHIP_ID_                "W5200\0"
/**
    @brief Define interface mode.
    @todo you should select interface mode as chip. Select one of @ref \_WIZCHIP_IO_MODE_SPI_ or @ref \	_WIZCHIP_IO_MODE_BUS_INDIR_
*/
#ifndef _WIZCHIP_IO_MODE_
// #define _WIZCHIP_IO_MODE_           _WIZCHIP_IO_MODE_BUS_INDIR_
#define _WIZCHIP_IO_MODE_           _WIZCHIP_IO_MODE_SPI_
#endif
//A20150601 : Define the unit of IO DATA.
typedef   uint8_t   iodata_t;
#include "W5200/w5200.h"
#elif (_WIZCHIP_ == W5500)
#define _WIZCHIP_ID_                 "W5500\0"

/**
    @brief Define interface mode. \n
    @todo Should select interface mode as chip.
          - @ref \_WIZCHIP_IO_MODE_SPI_ \n
            -@ref \_WIZCHIP_IO_MODE_SPI_VDM_ : Valid only in @ref \_WIZCHIP_ == W5500 \n
            -@ref \_WIZCHIP_IO_MODE_SPI_FDM_ : Valid only in @ref \_WIZCHIP_ == W5500 \n
          - @ref \_WIZCHIP_IO_MODE_BUS_ \n
            - @ref \_WIZCHIP_IO_MODE_BUS_DIR_ \n
            - @ref \_WIZCHIP_IO_MODE_BUS_INDIR_ \n
          - Others will be defined in future. \n\n
          ex> <code> #define \_WIZCHIP_IO_MODE_ \_WIZCHIP_IO_MODE_SPI_VDM_ </code>

*/
#ifndef _WIZCHIP_IO_MODE_
//#define _WIZCHIP_IO_MODE_           _WIZCHIP_IO_MODE_SPI_FDM_
#define _WIZCHIP_IO_MODE_           _WIZCHIP_IO_MODE_SPI_
#endif
//A20150601 : Define the unit of IO DATA.
typedef   uint8_t   iodata_t;
#include "W5500/w5500.h"
#elif ( _WIZCHIP_ == W5300)
#define _WIZCHIP_ID_                 "W5300\0"
/**
    @brief Define interface mode.
    @todo you should select interface mode as chip. Select one of @ref \_WIZCHIP_IO_MODE_SPI_ , @ref \_WIZCHIP_IO_MODE_BUS_DIR_ or @ref \_WIZCHIP_IO_MODE_BUS_INDIR_
*/
#ifndef _WIZCHIP_IO_MODE_
#define _WIZCHIP_IO_MODE_           _WIZCHIP_IO_MODE_BUS_DIR_
// #define _WIZCHIP_IO_MODE_           _WIZCHIP_IO_MODE_BUS_INDIR_
#endif

//A20150601 : Define the unit and bus width of IO DATA.
/**
    @brief Select the data width 8 or 16 bits.
    @todo you should select the bus width. Select one of 8 or 16.
*/
#ifndef _WIZCHIP_IO_BUS_WIDTH_
#define _WIZCHIP_IO_BUS_WIDTH_       16  // 8
#endif
#if _WIZCHIP_IO_BUS_WIDTH_ == 8
typedef   uint8_t   iodata_t;
#elif _WIZCHIP_IO_BUS_WIDTH_ == 16
typedef   uint16_t   iodata_t;
#else
#error "Unknown _WIZCHIP_IO_BUS_WIDTH_. It should be 8 or 16."
#endif
//
#include "W5300/w5300.h"


#elif ( _WIZCHIP_ == W6100)

#define _WIZCHIP_ID_                "W6100\0"

/**
    @brief Define @ref _WIZCHIP_ interface mode.
    @todo You should select interface mode of @ref _WIZCHIP_.\n\n
        Select one of @ref _WIZCHIP_IO_MODE_SPI_VDM_, @ref _WIZCHIP_IO_MODE_SPI_FDM_, and @ref _WIZCHIP_IO_MODE_BUS_INDIR_
    @sa WIZCHIP_READ(), WIZCHIP_WRITE(), WIZCHIP_READ_BUF(), WIZCHIP_WRITE_BUF()
*/
#if 1
// 20231103 taylor
#define _WIZCHIP_IO_MODE_           _WIZCHIP_IO_MODE_SPI_VDM_
#elif 0
#define _WIZCHIP_IO_MODE_         _WIZCHIP_IO_MODE_SPI_VDM_
#else
#define _WIZCHIP_IO_MODE_         _WIZCHIP_IO_MODE_SPV_FDM_
#endif

typedef   uint8_t   iodata_t;       ///< IO access unit. bus width
typedef   int16_t   datasize_t;     ///< sent or received data size
#include "./W6100/w6100.h"
#include "../Application/Application.h"

//teddy 240122
#elif ( _WIZCHIP_ == W6300)

#define _WIZCHIP_ID_                "W6300\0"




/**
    @brief Define @ref _WIZCHIP_ interface mode.
    @todo You should select interface mode of @ref _WIZCHIP_.\n\n
        Select one of @ref _WIZCHIP_IO_MODE_SPI_QSPI_, @ref _WIZCHIP_IO_MODE_SPI_VDM_,@ref _WIZCHIP_IO_MODE_BUS_INDIR_
    @sa WIZCHIP_READ(), WIZCHIP_WRITE(), WIZCHIP_READ_BUF(), WIZCHIP_WRITE_BUF()
*/

#define QSPI_SINGLE_MODE            (0x00 << 6) // 0b0000 0000 // 0x00
#define QSPI_DUAL_MODE              (0x01 << 6) // 0b0100 0000 // 0x40
#define QSPI_QUAD_MODE              (0x02 << 6) // 0b1000 0000 // 0x80 

#ifndef _WIZCHIP_QSPI_MODE_
#define _WIZCHIP_QSPI_MODE_          QSPI_SINGLE_MODE
#endif

//#define _WIZCHIP_IO_MODE_         _WIZCHIP_IO_MODE_BUS_INDIR_
#define _WIZCHIP_IO_MODE_           ((_WIZCHIP_IO_MODE_SPI_ & 0xff00) | (_WIZCHIP_QSPI_MODE_ & 0x00ff))



typedef   uint8_t   iodata_t;       ///< IO access unit. bus width
typedef   int16_t   datasize_t;     ///< sent or received data size
#include "./W6300/w6300.h"
#include "../Application/Application.h"

#else
#error "Unknown defined _WIZCHIP_. You should define one of 5100, 5200, 5300, 5500, 6100 and 6300!!!"
#endif

#ifndef _WIZCHIP_IO_MODE_
#error "Undefined _WIZCHIP_IO_MODE_. You should define it !!!"
#endif

/**
    @brief Define I/O base address when BUS IF mode.
    @todo Should re-define it to fit your system when BUS IF Mode (@ref \_WIZCHIP_IO_MODE_BUS_,
         @ref \_WIZCHIP_IO_MODE_BUS_DIR_, @ref \_WIZCHIP_IO_MODE_BUS_INDIR_). \n\n
         ex> <code> #define \_WIZCHIP_IO_BASE_      0x00008000 </code>
*/
#if _WIZCHIP_IO_MODE_ & _WIZCHIP_IO_MODE_BUS_
#if 1
// 20231108 taylor
#if (_WIZCHIP_ == W6300)
#define _WIZCHIP_IO_BASE_            0x60000000   // for W6100-EVB
#elif (_WIZCHIP_ == W6100)
#define _WIZCHIP_IO_BASE_            0x60000000   // for W5100S-EV
#elif (_WIZCHIP_ == W5100S)
#define _WIZCHIP_IO_BASE_            0x60000000   // for W5100S-EVB
#elif (_WIZCHIP_ == W5300)
#define _WIZCHIP_IO_BASE_            0x68000000   // for W5300 by javakys 20210408 @66c27e960a813f7ea6e8b1ce083d12b3e7e86fc0
#else
#define _WIZCHIP_IO_BASE_            0x00000000
#endif

#else
#if (_WIZCHIP_ == W6100)
#define _WIZCHIP_IO_BASE_            0x60000000   // for W6100 BUS
#else
//	#define _WIZCHIP_IO_BASE_				0x60000000	// for 5100S IND
#define _WIZCHIP_IO_BASE_				0x68000000	// for W5300
#endif
#endif
#elif _WIZCHIP_IO_MODE_ & _WIZCHIP_IO_MODE_SPI_
//#define _WIZCHIP_IO_BASE_				0x00000000	// for 5100S SPI
#endif

#ifndef _WIZCHIP_IO_BASE_
#if 1
// 20231108 taylor
#define _WIZCHIP_IO_BASE_              0x00000000
#else
#define _WIZCHIP_IO_BASE_              0x00000000  // 0x8000
#endif
#endif

//M20150401 : Typing Error
//#if _WIZCHIP_IO_MODE_ & _WIZCHIP_IO_MODE_BUS
#if _WIZCHIP_IO_MODE_ & _WIZCHIP_IO_MODE_BUS_
#ifndef _WIZCHIP_IO_BASE_
#error "You should be define _WIZCHIP_IO_BASE to fit your system memory map."
#endif
#endif

#if _WIZCHIP_ >= W5200
#define _WIZCHIP_SOCK_NUM_   8   ///< The count of independant socket of @b WIZCHIP
#else
#define _WIZCHIP_SOCK_NUM_   4   ///< The count of independant socket of @b WIZCHIP
#endif


/********************************************************
    WIZCHIP BASIC IF functions for SPI, SDIO, I2C , ETC.
*********************************************************/
/**
    @ingroup DATA_TYPE
    @brief The set of callback functions for W5500:@ref WIZCHIP_IO_Functions W5200:@ref WIZCHIP_IO_Functions_W5200
*/
typedef struct __WIZCHIP {
    uint16_t  if_mode;               ///< host interface mode
    uint8_t   id[8];                 ///< @b WIZCHIP ID such as @b 5100, @b 5100S, @b 5200, @b 5500, and so on.
    /**
        The set of critical section callback func.
    */
    struct _CRIS {
        void (*_enter)  (void);       ///< crtical section enter
        void (*_exit) (void);         ///< critial section exit
    } CRIS;
    /**
        The set of @ref \_WIZCHIP_ select control callback func.
    */
    struct _CS {
        void (*_select)  (void);      ///< @ref \_WIZCHIP_ selected
        void (*_deselect)(void);      ///< @ref \_WIZCHIP_ deselected
    } CS;
    /**
        The set of interface IO callback func.
    */
    union _IF {
        /**
            For BUS interface IO
        */
        //M20156501 : Modify the function name for integrating with W5300
        //struct
        //{
        //   uint8_t  (*_read_byte)  (uint32_t AddrSel);
        //   void     (*_write_byte) (uint32_t AddrSel, uint8_t wb);
        //}BUS;
        struct {
            iodata_t  (*_read_data)   (uint32_t AddrSel);
            void      (*_write_data)  (uint32_t AddrSel, iodata_t wb);
#if 1
            // 20231103 taylor
            void      (*_read_data_buf)  (uint32_t AddrSel, iodata_t* pBuf, int16_t len, uint8_t addrinc);  ///< Read @ref iodata_t as many as <i>len</i> from @ref _WIZCHIP_ through BUS
            void      (*_write_data_buf) (uint32_t AddrSel, iodata_t* pBuf, int16_t len, uint8_t addrinc);  ///< Write @ref iodata_t data as many as <i>len</i> to @ref _WIZCHIP_ through BUS
#endif
        } BUS;

        /**
            For SPI interface IO
        */
        struct {
            uint8_t (*_read_byte)   (void);
            void    (*_write_byte)  (uint8_t wb);
            void    (*_read_burst)  (uint8_t* pBuf, uint16_t len);
            void    (*_write_burst) (uint8_t* pBuf, uint16_t len);
        } SPI;

        /**
            For QSPI interface IO
        */
        //teddy 240122
        struct {
            void    (*_read_qspi)  (uint8_t opcode, uint16_t addr, uint8_t* pBuf, uint16_t len);
            void    (*_write_qspi) (uint8_t opcode, uint16_t addr, uint8_t* pBuf, uint16_t len);
        } QSPI;
        // To be added
        //
    } IF;
} _WIZCHIP;

extern _WIZCHIP  WIZCHIP;

/**
    @ingroup DATA_TYPE
    WIZCHIP control type enumration used in @ref ctlwizchip().
*/
typedef enum {
    CW_SYS_LOCK,           ///< Lock or Unlock @ref _WIZCHIP_ with @ref SYS_CHIP_LOCK, @ref SYS_PHY_LOCK, and @ref SYS_NET_LOCK
    CW_SYS_UNLOCK,         ///< Lock or Unlock @ref _WIZCHIP_ with @ref SYS_CHIP_LOCK, @ref SYS_PHY_LOCK, and @ref SYS_NET_LOCK
    CW_GET_SYSLOCK,        ///< Get the lock status of @ref _WIZCHIP_ with @ref SYS_CHIP_LOCK, @ref SYS_PHY_LOCK, and @ref SYS_NET_LOCK

    CW_RESET_WIZCHIP,      ///< Reset @ref _WIZCHIP_ by software
    CW_INIT_WIZCHIP,       ///< Initialize to SOCKETn buffer size with n byte array typed uint8_t
    CW_GET_INTERRUPT,      ///< Get the interrupt status with @ref intr_kind
    CW_CLR_INTERRUPT,      ///< Clear the interrupt with @ref intr_kind
    CW_SET_INTRMASK,       ///< Set the interrupt mask with @ref intr_kind
    CW_GET_INTRMASK,       ///< Get the interrupt mask with @ref intr_kind
    CW_SET_INTRTIME,       ///< Set the interrupt pending time
    CW_GET_INTRTIME,       ///< Get the interrupt pending time
    CW_SET_IEN,            ///< Set the global interrupt enable only when @ref SYS_CHIP_LOCK is not set
    CW_GET_IEN,            ///< Get the global interrupt enable

    CW_GET_ID,             ///< Get @ref _WIZCHIP_ name.
    CW_GET_VER,            ///< Get the version of TCP/IP TOE engine

    CW_SET_SYSCLK,         ///< Set the system clock with @ref SYSCLK_100MHZ or SYSCLK_10MHZ only when @ref SYS_CHIP_LOCK is unlock
    CW_GET_SYSCLK,         ///< Get the system clock with @ref SYSCLK_100MHZ or SYSCLK_10MHZ

    CW_RESET_PHY,          ///< Reset PHY
    CW_SET_PHYCONF,        ///< Set PHY operation mode (Manual/Auto, 10/100, Half/Full) with @ref wiz_PhyConf
    CW_GET_PHYCONF,        ///< Get PHY operation mode (Manual/Auto, 10/100, Half/Full) with @ref wiz_PhyConf
    CW_GET_PHYSTATUS,      ///< Get real operation mode with @ref wiz_PhyConf when PHY is linked up.
    CW_SET_PHYPOWMODE,     ///< Set PHY power mode with @ref PHY_POWER_NORM or PHY_POWER_DOWN
    CW_GET_PHYPOWMODE,     ///< Get PHY Power mode with @ref PHY_POWER_NORM or PHY_POWER_DOWN
    CW_GET_PHYLINK         ///< Get PHY Link status with @ref PHY_LINK_ON or @ref PHY_LINK_OFF
} ctlwizchip_type;

/**
    @ingroup DATA_TYPE
    Network control type enumration used in @ref ctlnetwork().
*/
typedef enum {
    CN_SET_NETINFO,  ///< Set Network with @ref wiz_NetInfo
    CN_GET_NETINFO,  ///< Get Network with @ref wiz_NetInfo
    CN_SET_NETMODE,  ///< Set network mode as WOL, PPPoE, Ping Block, and Force ARP mode
    CN_GET_NETMODE,  ///< Get network mode as WOL, PPPoE, Ping Block, and Force ARP mode
    CN_SET_TIMEOUT,  ///< Set network timeout as retry count and time.
    CN_GET_TIMEOUT,  ///< Get network timeout as retry count and time.
    //teddy 240122
#if ((_WIZCHIP_ == W6100)||(_WIZCHIP_ == W6300))
    CN_SET_PREFER,   ///< Set the preferred source IPv6 address of @ref _SLCR_.\n Refer to @ref IPV6_ADDR_AUTO, @ref IPV6_ADDR_LLA, @ref IPV6_ADDR_GUA
    CN_GET_PREFER,   ///< Get the preferred source IPv6 address of @ref _SLCR_.\n Refer to @ref IPV6_ADDR_AUTO, @ref IPV6_ADDR_LLA, @ref IPV6_ADDR_GUA
#endif
} ctlnetwork_type;

//teddy 240122
#if ((_WIZCHIP_ == W6100)||(_WIZCHIP_ == W6300))
/**
    @ingroup DATA_TYPE
    @brief  Network Service Control Type enumeration
    @details @ref ctlnetservice_type includes network management or monitor functions for @ref _WIZCHIP_.
    @sa ctlnetservice(), wiz_IPAddress, wiz_Prefix
*/
typedef enum {
    CNS_ARP,            ///< ARP process with @ref wiz_IPAddress
    CNS_PING,           ///< PING process with @ref wiz_IPAddress
    CNS_DAD,            ///< Duplicated IPv6 Address Detection
    /**
        @brief Stateless Address Auto-configuration(SLAAC) with @ref wiz_Prefix.
        @details @ref CNS_SLAAC sends first RS message to all-router and then receives RA message from a router.
        @note It is valid only when the first received RA option is the source link-layer address(0x01) and the second is prefix information(0x03).\n
             Refer to @ref SLIR_RS.
        @sa ctlnetservice()
        @sa CNS_GET_PREFIX
    */
    CNS_SLAAC,
    CNS_UNSOL_NA,       ///< Unsolicited Neighbor Advertisement for update @ref _WIZCHIP_ network information to neighbors
    /**
        @brief Get prefix information with @ref wiz_Prefix.
        @details @ref CNS_GET_PREFIX can get prefix information of RA message to be sent by a router without RS message.
        @note It is valid only when @ref IK_SOCKL_RA is set and the prefix information(0x03) of RA option is first received.
        @sa ctlnetservice()
        @sa CNS_SLAAC
    */
    CNS_GET_PREFIX
} ctlnetservice_type;
#endif

#if (_WIZCHIP_ == W5100 || _WIZCHIP_ == W5100S || _WIZCHIP_ == W5200 || _WIZCHIP_ == W5300 || _WIZCHIP_ == W5500)
/**
    @ingroup DATA_TYPE
    Interrupt kind when CW_SET_INTRRUPT, CW_GET_INTERRUPT, CW_SET_INTRMASK
    and CW_GET_INTRMASK is used in @ref ctlnetwork().
    It can be used with OR operation.
*/
typedef enum {
#if   _WIZCHIP_ == W5500
    IK_WOL               = (1 << 4),   ///< Wake On Lan by receiving the magic packet. Valid in W500.
#elif _WIZCHIP_ == W5300
    IK_FMTU              = (1 << 4),   ///< Received a ICMP message (Fragment MTU)
#endif

    IK_PPPOE_TERMINATED  = (1 << 5),   ///< PPPoE Disconnected

#if _WIZCHIP_ != W5200
    IK_DEST_UNREACH      = (1 << 6),   ///< Destination IP & Port Unreachable, No use in W5200
#endif

    IK_IP_CONFLICT       = (1 << 7),   ///< IP conflict occurred

    IK_SOCK_0            = (1 << 8),   ///< Socket 0 interrupt
    IK_SOCK_1            = (1 << 9),   ///< Socket 1 interrupt
    IK_SOCK_2            = (1 << 10),  ///< Socket 2 interrupt
    IK_SOCK_3            = (1 << 11),  ///< Socket 3 interrupt
#if _WIZCHIP_ > W5100S
    IK_SOCK_4            = (1 << 12),  ///< Socket 4 interrupt, No use in 5100
    IK_SOCK_5            = (1 << 13),  ///< Socket 5 interrupt, No use in 5100
    IK_SOCK_6            = (1 << 14),  ///< Socket 6 interrupt, No use in 5100
    IK_SOCK_7            = (1 << 15),  ///< Socket 7 interrupt, No use in 5100
#endif

#if _WIZCHIP_ > W5100S
    IK_SOCK_ALL          = (0xFF << 8) ///< All Socket interrupt
#else
    IK_SOCK_ALL          = (0x0F << 8) ///< All Socket interrupt
#endif
} intr_kind;
//teddy 240122
#elif ((_WIZCHIP_ == W6100)||(_WIZCHIP_ == W6300))
/**
    @ingroup DATA_TYPE
    @brief Interrupt Kind
    @details @ref intr_kind can be used as the interrupt bits of @ref _IR_, @ref _SIR_, and @ref _SLIR_,\n
            It can be used as the interrupt mask bits of @ref _IMR_, @ref _SIMR_, and @ref _SLIMR_,\n
            and also, It can be used as the interrupt clear bits of @ref _IRCLR_, @ref _Sn_IRCLR_, and @ref _SLIRCLR_.
    @note It can be used with @b OR operation.
    @sa ctlwizchip(), CW_GET_INTERRUPT, CW_CLR_INTERRUPT, CW_GET_INTRMASK, CW_SET_INTRMASK
    @sa ctlnetservice(), ctlnetservice_type
    @sa wizchip_getinterrupt(), wizchip_clrinterrupt(), wizchip_getinterruptmask(), wizchip_setinterruptmask()
*/
typedef enum {
    IK_PPPOE_TERMINATED = (1 << 0),     ///< PPPoE Termination Interrupt
    IK_DEST_UNREACH     = (1 << 1),     ///< ICMPv4 Destination Unreachable Interrupt
    IK_IP_CONFLICT      = (1 << 2),     ///< IPv4 Address Conflict Interrupt
    IK_DEST_UNREACH6    = (1 << 4),     ///< ICMPv6 Destination Unreachable Interrupt
    IK_WOL              = (1 << 7),     ///< WOL magic packet Interrupt
    IK_NET_ALL          = (0x97),       ///< All Network Interrupt

    IK_SOCK_0           = (1 << 8),     ///< Socket 0 Interrupt
    IK_SOCK_1           = (1 << 9),     ///< Socket 1 Interrupt
    IK_SOCK_2           = (1 << 10),    ///< Socket 2 Interrupt
    IK_SOCK_3           = (1 << 11),    ///< Socket 3 Interrupt
    IK_SOCK_4           = (1 << 12),    ///< Socket 4 Interrupt
    IK_SOCK_5           = (1 << 13),    ///< Socket 5 Interrupt
    IK_SOCK_6           = (1 << 14),    ///< Socket 6 Interrupt
    IK_SOCK_7           = (1 << 15),    ///< Socket 7 Interrupt
    IK_SOCK_ALL         = (0xFF << 8),  ///< All Socket Interrupt

    IK_SOCKL_TOUT       = (1 << 16),    ///< @ref _SLCR_ Timeout Interrupt.\n Refer to @ref ctlnetservice_type.
    IK_SOCKL_ARP4       = (1 << 17),    ///< @ref _SLCR_ APR4 Interrupt.\n Refer to @ref CNS_ARP.
    IK_SOCKL_PING4      = (1 << 18),    ///< @ref _SLCR_ PING4 Interrupt.\n Refer to @ref CNS_PING.
    IK_SOCKL_ARP6       = (1 << 19),    ///< @ref _SLCR_ ARP6 Interrupt.\n Refer to @ref CNS_ARP.
    IK_SOCKL_PING6      = (1 << 20),    ///< @ref _SLCR_ PING6 Interrupt.\n Refer to @ref CNS_PING.
    IK_SOCKL_NS         = (1 << 21),    ///< @ref _SLCR_ NS Interrupt.\n Refer to @ref CNS_DAD.
    IK_SOCKL_RS         = (1 << 22),    ///< @ref _SLCR_ RS Interrupt.\n Refer to @ref CNS_SLAAC.
    IK_SOCKL_RA         = (1 << 23),    ///< @ref _SLCR_ RA Interrupt.\n Refer to @ref CNS_GET_PREFIX.
    IK_SOCKL_ALL        = (0xFF << 16), ///< @ref _SLCR_ All Interrupt

    IK_INT_ALL          = (0x00FFFF97)   ///< All Interrupt
} intr_kind;
#endif

//teddy 240122
#if ((_WIZCHIP_ == W6100)||(_WIZCHIP_ == W6300))
#define SYS_CHIP_LOCK           (1<<2)   ///< CHIP LOCK. \n Refer to @ref CW_SYS_LOCK, @ref CW_SYS_UNLOCK, and @ref CW_GET_SYSLOCK.
#define SYS_NET_LOCK            (1<<1)   ///< NETWORK Information LOCK. \n Refer to @ref CW_SYS_LOCK, @ref CW_SYS_UNLOCK, and @ref CW_GET_SYSLOCK.
#define SYS_PHY_LOCK            (1<<0)   ///< PHY LOCK.\n Refer to @ref CW_SYS_LOCK, @ref CW_SYS_UNLOCK, and @ref CW_GET_SYSLOCK.

#define SYSCLK_100MHZ            0       ///< System Clock 100MHz.\n Refer to Refer to @ref CW_SET_SYSCLK and  @ref CW_GET_SYSCLK.
#define SYSCLK_25MHZ             1       ///< System Clock 25MHz.\n Refer to Refer to @ref CW_SET_SYSCLK and  @ref CW_GET_SYSCLK.

#define PHY_MODE_MANUAL          0       ///< Configured PHY operation mode with user setting.\n Refer to @ref CW_SET_PHYCONF and @ref CW_GET_PHYCONF.
#define PHY_MODE_AUTONEGO        1       ///< Configured PHY operation mode with auto-negotiation.\n Refer to @ref CW_SET_PHYCONF and @ref CW_GET_PHYCONF.
#define PHY_MODE_TE              2

#define IPV6_ADDR_AUTO           0x00    ///< IPv6 Address Type : Auto.\n Refer to @ref CN_SET_PREFER, @ref CN_GET_PREFER.
#define IPV6_ADDR_LLA            0x02    ///< IPv6 Address Type : LLA. \n Refer to @ref CN_SET_PREFER, @ref CN_GET_PREFER, @ref CNS_DAD.
#define IPV6_ADDR_GUA            0x03    ///< IPv6 Address Type : GUA. \n Refer to @ref CN_SET_PREFER, @ref CN_GET_PREFER, @ref CNS_DAD.
#endif

#define PHY_CONFBY_HW            0     ///< Configured PHY operation mode by HW pin
#define PHY_CONFBY_SW            1     ///< Configured PHY operation mode by SW register   
#define PHY_MODE_MANUAL          0     ///< Configured PHY operation mode with user setting.
#define PHY_MODE_AUTONEGO        1     ///< Configured PHY operation mode with auto-negotiation
#define PHY_SPEED_10             0     ///< Link Speed 10
#define PHY_SPEED_100            1     ///< Link Speed 100
#define PHY_DUPLEX_HALF          0     ///< Link Half-Duplex
#define PHY_DUPLEX_FULL          1     ///< Link Full-Duplex
#define PHY_LINK_OFF             0     ///< Link Off
#define PHY_LINK_ON              1     ///< Link On
#define PHY_POWER_NORM           0     ///< PHY power normal mode
#define PHY_POWER_DOWN           1     ///< PHY power down mode 


//teddy 240122
#if _WIZCHIP_ == W5100S || _WIZCHIP_ == W5500 || _WIZCHIP_ == W6100 || _WIZCHIP_ == W6300
/**
    @ingroup DATA_TYPE
    It configures PHY configuration when CW_SET PHYCONF or CW_GET_PHYCONF in W5500,
    and it indicates the real PHY status configured by HW or SW in all WIZCHIP. \n
    Valid only in W5500.
*/
typedef struct wiz_PhyConf_t {
    uint8_t by;       ///< set by @ref PHY_CONFBY_HW or @ref PHY_CONFBY_SW
    uint8_t mode;     ///< set by @ref PHY_MODE_MANUAL or @ref PHY_MODE_AUTONEGO
    uint8_t speed;    ///< set by @ref PHY_SPEED_10 or @ref PHY_SPEED_100
    uint8_t duplex;   ///< set by @ref PHY_DUPLEX_HALF @ref PHY_DUPLEX_FULL
    //uint8_t power;  ///< set by @ref PHY_POWER_NORM or @ref PHY_POWER_DOWN
    //uint8_t link;   ///< Valid only in CW_GET_PHYSTATUS. set by @ref PHY_LINK_ON or PHY_DUPLEX_OFF
} wiz_PhyConf;
#endif

#if (_WIZCHIP_ == W5100 || _WIZCHIP_ == W5100S || _WIZCHIP_ == W5200 || _WIZCHIP_ == W5300 || _WIZCHIP_ == W5500)
/**
    @ingroup DATA_TYPE
    It used in setting dhcp_mode of @ref wiz_NetInfo.
*/
typedef enum {
    NETINFO_STATIC = 1,    ///< Static IP configuration by manually.
    NETINFO_DHCP           ///< Dynamic IP configruation from a DHCP sever
} dhcp_mode;

/**
    @ingroup DATA_TYPE
    Network Information for WIZCHIP
*/
typedef struct wiz_NetInfo_t {
    uint8_t mac[6];  ///< Source Mac Address
    uint8_t ip[4];   ///< Source IP Address
    uint8_t sn[4];   ///< Subnet Mask
    uint8_t gw[4];   ///< Gateway IP Address
    uint8_t dns[4];  ///< DNS server IP Address
    dhcp_mode dhcp;  ///< 1 - Static, 2 - DHCP
} wiz_NetInfo;

/**
    @ingroup DATA_TYPE
    Network mode
*/
typedef enum {
#if _WIZCHIP_ == W5500
    NM_FORCEARP    = (1 << 1), ///< Force to APP send whenever udp data is sent. Valid only in W5500
#endif
    NM_WAKEONLAN   = (1 << 5), ///< Wake On Lan
    NM_PINGBLOCK   = (1 << 4), ///< Block ping-request
    NM_PPPOE       = (1 << 3), ///< PPPoE mode
} netmode_type;

/**
    @ingroup DATA_TYPE
    Used in CN_SET_TIMEOUT or CN_GET_TIMEOUT of @ref ctlwizchip() for timeout configruation.
*/
typedef struct wiz_NetTimeout_t {
    uint8_t  retry_cnt;     ///< retry count
    uint16_t time_100us;    ///< time unit 100us
} wiz_NetTimeout;
//teddy 240122
#elif ((_WIZCHIP_ == W6100)||(_WIZCHIP_ == W6300))
/**
    @ingroup DATA_TYPE
    @brief IP Address Configuration Mode
    @details @ref ipconf_mode can be used to save the DHCP mode running on your system.
    @sa ctlnetwork(), CN_SET_NETINFO, CN_GET_NETINFO
    @sa wizchip_setnetinfo(), wizchip_getnetinfo(), wiz_NetInfo
*/
typedef enum {
    NETINFO_NONE       = 0x00,    ///< No use DHCP
    NETINFO_STATIC_V4  = 0x01,    ///< Static IPv4 configuration by manually.
    NETINFO_STATIC_V6  = 0x02,    ///< Static IPv6 configuration by manually.
    NETINFO_STATIC_ALL = 0x03,    ///< Static IPv4 and IPv6 configuration by manually.
    NETINFO_SLAAC_V6   = 0x04,    ///< Stateless Adders Auto Configuration for IPv6
    NETINFO_DHCP_V4    = 0x10,    ///< Dynamic IPv4 configuration from a DHCP sever
    NETINFO_DHCP_V6    = 0x20,    ///< Dynamic IPv6 configuration from a DHCP sever
    NETINFO_DHCP_ALL   = 0x30     ///< Dynamic IPv4 and IPv6 configuration from a DHCP sever
} ipconf_mode;

/**
    @ingroup DATA_TYPE
    @brief Network Information for @ref _WIZCHIP_
    @details @ref wiz_NetInfo is a structure type to configure or indicate the network information of @ref _WIZCHIP_.
    @sa ctlnetwork(), CN_SET_NETINFO, CN_GET_NETINFO
    @sa wizchip_setnetinfo(), wizchip_getnetinfo()
*/

typedef enum {
    NETINFO_STATIC = 1,    ///< Static IP configuration by manually.
    NETINFO_DHCP           ///< Dynamic IP configruation from a DHCP sever
} dhcp_mode;

typedef struct wiz_NetInfo_t {
    uint8_t     mac[6];    ///< Source Hardware Address
    uint8_t     ip[4];     ///< Source IPv4 Address
    uint8_t     sn[4];     ///< Subnet Mask value
    uint8_t     gw[4];     ///< Gateway IPv4 Address
    uint8_t     lla[16];   ///< Source Link Local Address
    uint8_t     gua[16];   ///< Source Global Unicast Address
    uint8_t     sn6[16];   ///< IPv6 Prefix
    uint8_t     gw6[16];   ///< Gateway IPv6 Address
    uint8_t     dns[4];    ///< DNS server IPv4 Address
    uint8_t     dns6[16];  ///< DNS server IPv6 Address
    ipconf_mode ipmode;    ///< IP Configuration Mode
    dhcp_mode dhcp;  ///< 1 - Static, 2 - DHCP
} wiz_NetInfo;

/**
    @ingroup DATA_TYPE
    @brief Network mode Configuration
    @details @ref netmode_type includes the network mode control function such as ping, TCP/RST block and etc.
    @sa ctlnetwork(), CN_SET_NETMODE, CN_GET_NETMODE
*/
typedef enum {
    // NETMR Bit Values
    NM_IPB_V4            = (1 << 0),      ///< IPv4 Packet Block
    NM_IPB_V6            = (1 << 1),      ///< IPv6 Packet Block
    NM_WOL               = (1 << 2),      ///< Wake On Lan(WOL) Mode
    NM_PB6_MULTI         = (1 << 4),      ///< PING6 request from multicasting group address Block
    NM_PB6_ALLNODE       = (1 << 5),      ///< PING6 request from all-node multicasting address Block
    NM_MR_MASK           = (0x37),        ///< @ref _NETMR_ Mask value

    // NETMR2 Bit Values
    NM_PPPoE             = (1 << 8),      ///< PPPoE Mode
    NM_DHA_SELECT        = (1 << 15),     ///< Destination Hardware Address Select
    NM_MR2_MASK          = (0x09 << 8),   ///< @ref _NETMR2_ Mask value

    //NET4MR Bit Values
    NM_PB4_ALL           = (1 << 16),     ///< All PING4 request Block
    NM_TRSTB_V4          = (1 << 17),     ///< TCP RST packet for IPv4 Send Block
    NM_PARP_V4           = (1 << 18),     ///< ARP request for IPv4 before PINGv4 Replay
    NM_UNRB_V4           = (1 << 19),     ///< Unreachable Destination for IPv4 Block
    NM_NET4_MASK         = (0x0F << 16),  ///< @ref _NET4MR_ Mask value

    //NET4MR Bit Values
    NM_PB6_ALL           = (1 << 24),     ///< All PING6 request Block
    NM_TRSTB_V6          = (1 << 25),     ///< TCP RST packet for IPv6 Send Block
    NM_PARP_V6           = (1 << 26),     ///< ARP request for IPv6 before PINGv4 Replay
    NM_UNRB_V6           = (1 << 27),     ///< Unreachable Destination for IPv6 Block
    NM_NET6_MASK         = (0x0F << 24),  ///< @ref _NET6MR_ Mask value

    NM_MASK_ALL          = (0x0F0F0937)   ///< @ref netmode_type all mask value
} netmode_type;

/**
    @ingroup DATA_TYPE
    @brief Network Timeout for @ref _WIZCHIP_
    @details @ref wiz_NetInfo is a structure type to configure or indicate the network timeout of @ref _WIZCHIP_.
    @sa ctlnetwork(), CN_SET_TIMEOUT, CN_GET_TIMEOUT
    @sa wizchip_settimeout(), wizchip_gettimeout()
*/
typedef struct wiz_NetTimeout_t {
    uint8_t  s_retry_cnt;         ///< The default retry count of SOCKETn
    uint16_t s_time_100us;        ///< The retransmission time of SOCKETn (unit 100us)
    uint8_t  sl_retry_cnt;        ///< The retry count of SOCKET-less
    uint16_t sl_time_100us;       ///< The retransmission time of SOCKET-less (unit 100us)
} wiz_NetTimeout;

/**
    @ingroup DATA_TYPE
    @brief Destination Information for Network Service of @ref _WIZCHIP_
    @details @ref wiz_NetInfo is a structure type to configure or indicate a destination information of network service.
    @sa ctlnetservice(), CNS_ARP, CNS_PING
    @sa IK_SOCKL_TOUT, IK_SOCKL_ARP4, IK_SOCKL_ARP6, IK_SOCKL_PING4, IK_SOCKL_PING6
*/
typedef struct wiz_IPAddress_t {
    uint8_t ip[16];               ///< Destination IP Address. \n IPv4 index : 0 to 3, IPv6 index : 0 to 15
    uint8_t len;                  ///< Destination IP Address Length.\n IPv4 : 4, IPv6 : 16.
} wiz_IPAddress;

/**
    @ingroup DATA_TYPE
    @brief Prefix Information
    @details @ref wiz_Prefix is a structure type to indicate a prefix information(0x03) of received RA message from a router.
    @sa ctlnetservice(), CNS_SLAAC, IK_SOCKL_RS
    @sa IK_SOCKL_TOUT, IK_SOCKL_RA, CNS_GET_PREFIX
*/
typedef struct wiz_Prefix_t {
    uint8_t  len;                 ///< Prefix Length. \n It is used to set @ref _SUB6R_ to 1 as many as <i>len</i> from LSB bit.
    uint8_t  flag;                ///< Prefix Flag
    uint32_t valid_lifetime;      ///< Valid Lifetime
    uint32_t preferred_lifetime;  ///< Preferred Lifetime
    uint8_t  prefix[16];          ///< Prefix
} wiz_Prefix;

/**
    @ingroup DATA_TYPE
    @brief Destination Information & Destination Hardware Address for @ref CNS_ARP
    @details @ref wiz_ARP is a structure type to set a destination IP address for ARP-request or \n
            indicate a destination hardware address in APR-reply.
    @sa ctlnetservice(), CNS_ARP
    @sa IK_SOCKL_TOUT, IK_SOCKL_ARP4, IK_SOCKL_ARP6
*/
typedef struct wiz_ARP_t {
    wiz_IPAddress destinfo;       ///< Destination IP address for ARP-request
    uint8_t dha[6];               ///< Destination Hardware Address when ARP-reply is received from the destination.
} wiz_ARP;

/**
    @ingroup DATA_TYPE
    @brief Destination Information & Destination Hardware Address for @ref CNS_ARP
    @details @ref wiz_PING is a structure type to set a ID, sequence number, destination IP address for PING-request.
    @sa ctlnetservice(), CNS_PING
    @sa IK_SOCKL_TOUT, IK_SOCKL_PING4, IK_SOCKL_PING6
*/
typedef struct wiz_PING_t {
    uint16_t id;
    uint16_t seq;
    wiz_IPAddress destinfo;
} wiz_PING;
#endif

/**
    @brief Registers call back function for critical section of I/O functions such as
    \ref WIZCHIP_READ, @ref WIZCHIP_WRITE, @ref WIZCHIP_READ_BUF and @ref WIZCHIP_WRITE_BUF.
    @param cris_en : callback function for critical section enter.
    @param cris_ex : callback function for critical section exit.
    @todo Describe @ref WIZCHIP_CRITICAL_ENTER and @ref WIZCHIP_CRITICAL_EXIT marco or register your functions.
    @note If you do not describe or register, default functions(@ref wizchip_cris_enter & @ref wizchip_cris_exit) is called.
*/
void reg_wizchip_cris_cbfunc(void(*cris_en)(void), void(*cris_ex)(void));


/**
    @brief Registers call back function for WIZCHIP select & deselect.
    @param cs_sel : callback function for WIZCHIP select
    @param cs_desel : callback fucntion for WIZCHIP deselect
    @todo Describe @ref wizchip_cs_select and @ref wizchip_cs_deselect function or register your functions.
    @note If you do not describe or register, null function is called.
*/
void reg_wizchip_cs_cbfunc(void(*cs_sel)(void), void(*cs_desel)(void));

/**
    @brief Registers call back function for bus interface.
    @param bus_rb   : callback function to read byte data using system bus
    @param bus_wb   : callback function to write byte data using system bus
    @todo Describe @ref wizchip_bus_readbyte and @ref wizchip_bus_writebyte function
    or register your functions.
    @note If you do not describe or register, null function is called.
*/
//M20150601 : For integrating with W5300
//void reg_wizchip_bus_cbfunc(uint8_t (*bus_rb)(uint32_t addr), void (*bus_wb)(uint32_t addr, uint8_t wb));
void reg_wizchip_bus_cbfunc(iodata_t (*bus_rb)(uint32_t addr), void (*bus_wb)(uint32_t addr, iodata_t wb));

/**
    @brief Registers call back function for SPI interface.
    @param spi_rb : callback function to read byte using SPI
    @param spi_wb : callback function to write byte using SPI
    @todo Describe \ref wizchip_spi_readbyte and \ref wizchip_spi_writebyte function
    or register your functions.
    @note If you do not describe or register, null function is called.
*/
#if _WIZCHIP_ == W6100
void reg_wizchip_spi_cbfunc(uint8_t (*spi_rb)(void),
                            void (*spi_wb)(uint8_t wb),
                            void (*spi_rbuf)(uint8_t* buf, datasize_t len),
                            void (*spi_wbuf)(uint8_t* buf, datasize_t len));
#else
void reg_wizchip_spi_cbfunc(uint8_t (*spi_rb)(void), void (*spi_wb)(uint8_t wb));
#endif

/**
    @brief Registers call back function for SPI interface.
    @param spi_rb : callback function to burst read using SPI
    @param spi_wb : callback function to burst write using SPI
    @todo Describe \ref wizchip_spi_readbyte and \ref wizchip_spi_writebyte function
    or register your functions.
    @note If you do not describe or register, null function is called.
*/
void reg_wizchip_spiburst_cbfunc(void (*spi_rb)(uint8_t* pBuf, uint16_t len), void (*spi_wb)(uint8_t* pBuf, uint16_t len));

//teddy 240122
/**
    @brief Registers call back function for QSPI interface.
    @param spi_rb : callback function to read using QSPI
    @param spi_wb : callback function to write using QSPI
    @todo Describe \ref wizchip_qspi_read and \ref wizchip_qspi_write function
    or register your functions.
    @note If you do not describe or register, null function is called.
*/
void reg_wizchip_qspi_cbfunc(void (*qspi_rb)(uint8_t opcode, uint16_t addr, uint8_t* pBuf, uint16_t len), void (*qspi_wb)(uint8_t opcode, uint16_t addr, uint8_t* pBuf, uint16_t len));

/**
    @ingroup extra_functions
    @brief Controls to the WIZCHIP.
    @details Resets WIZCHIP & internal PHY, Configures PHY mode, Monitor PHY(Link,Speed,Half/Full/Auto),
    controls interrupt & mask and so on.
    @param cwtype : Decides to the control type
    @param arg : arg type is dependent on cwtype.
    @return  0 : Success \n
           -1 : Fail because of invalid \ref ctlwizchip_type or unsupported \ref ctlwizchip_type in WIZCHIP
*/
int8_t ctlwizchip(ctlwizchip_type cwtype, void* arg);

/**
    @ingroup extra_functions
    @brief Controls to network.
    @details Controls to network environment, mode, timeout and so on.
    @param cntype : Input. Decides to the control type
    @param arg : Inout. arg type is dependent on cntype.
    @return -1 : Fail because of invalid \ref ctlnetwork_type or unsupported \ref ctlnetwork_type in WIZCHIP \n
            0 : Success
*/
int8_t ctlnetwork(ctlnetwork_type cntype, void* arg);

//teddy 240122
#if ((_WIZCHIP_ == W6100)||(_WIZCHIP_ == W6300))
/**
    @ingroup extra_functions
    @brief Controls to network service.
    @details Controls to network environment, mode, timeout and so on.
    @param cnstype : Decides to the control type
    @param arg : arg type is dependent on cnstype.
    @return -1 : Fail because of invalid @ref ctlnetwork_type or unsupported @ref ctlnetwork_type \n
            0 : Success
*/
int8_t ctlnetservice(ctlnetservice_type cnstype, void* arg);
#endif

/*
    The following functions are implemented for internal use.
    but You can call these functions for code size reduction instead of ctlwizchip() and ctlnetwork().
*/

/**
    @ingroup extra_functions
    @brief Reset WIZCHIP by softly.
*/
void   wizchip_sw_reset(void);

/**
    @ingroup extra_functions
    @brief Initializes WIZCHIP with socket buffer size
    @param txsize Socket tx buffer sizes. If null, initialized the default size 2KB.
    @param rxsize Socket rx buffer sizes. If null, initialized the default size 2KB.
    @return 0 : succcess \n
          -1 : fail. Invalid buffer size
*/
int8_t wizchip_init(uint8_t* txsize, uint8_t* rxsize);

/**
    @ingroup extra_functions
    @brief Clear Interrupt of WIZCHIP.
    @param intr : @ref intr_kind value operated OR. It can type-cast to uint16_t.
*/
void wizchip_clrinterrupt(intr_kind intr);

/**
    @ingroup extra_functions
    @brief Get Interrupt of WIZCHIP.
    @return @ref intr_kind value operated OR. It can type-cast to uint16_t.
*/
intr_kind wizchip_getinterrupt(void);

/**
    @ingroup extra_functions
    @brief Mask or Unmask Interrupt of WIZCHIP.
    @param intr : @ref intr_kind value operated OR. It can type-cast to uint16_t.
*/
void wizchip_setinterruptmask(intr_kind intr);

/**
    @ingroup extra_functions
    @brief Get Interrupt mask of WIZCHIP.
    @return : The operated OR vaule of @ref intr_kind. It can type-cast to uint16_t.
*/
intr_kind wizchip_getinterruptmask(void);

//todo
#if _WIZCHIP_ > W5100
int8_t wizphy_getphylink(void);              ///< get the link status of phy in WIZCHIP. No use in W5100
int8_t wizphy_getphypmode(void);             ///< get the power mode of PHY in WIZCHIP. No use in W5100
#endif

#if _WIZCHIP_ == W5100S || _WIZCHIP_ == W5500
void   wizphy_reset(void);                   ///< Reset phy. Vailid only in W5500
/**
    @ingroup extra_functions
    @brief Set the phy information for WIZCHIP without power mode
    @param phyconf : @ref wiz_PhyConf
*/
void   wizphy_setphyconf(wiz_PhyConf* phyconf);
/**
    @ingroup extra_functions
    @brief Get phy configuration information.
    @param phyconf : @ref wiz_PhyConf
*/
void   wizphy_getphyconf(wiz_PhyConf* phyconf);
/**
    @ingroup extra_functions
    @brief Get phy status.
    @param phyconf : @ref wiz_PhyConf
*/
void   wizphy_getphystat(wiz_PhyConf* phyconf);
/**
    @ingroup extra_functions
    @brief set the power mode of phy inside WIZCHIP. Refer to @ref PHYCFGR in W5500, @ref PHYSTATUS in W5200
    @param pmode Settig value of power down mode.
*/
int8_t wizphy_setphypmode(uint8_t pmode);
//teddy 240122
#elif ((_WIZCHIP_ == W6100)||(_WIZCHIP_ == W6300))
/**
    @ingroup extra_functions
    @brief Reset the integrated PHY.
    @details @ref wizphy_reset() resets the integrated Ethernet PHY \n
            through @ref _PHY_IO_MODE_PHYCR_ or @ref _PHY_IO_MODE_MII_. \n
    @note In @ref _PHY_IO_MODE_PHYCR_, It needs a stable reset time. \n
         So you need to wait for the stable reset time.\n
         The stable reset time for each @ref _WIZCHIP_ maybe different.
    @sa ctlwizchip(), CW_RESET_PHY
    @sa _PHY_IO_MODE_
*/
void wizphy_reset(void);                   ///< Reset phy. Vailid only in W5500

/**
    @ingroup extra_functions
    @details @ref wizphy_setphyconf() set a operation mode of the integrated Ethernet PHY \n
            through @ref _PHY_IO_MODE_PHYCR_ or @ref _PHY_IO_MODE_MII_.\n
    @param phyconf : @ref wiz_PhyConf
    @note The operation mode can be applied to Ethernet PHY after the Ethernet PHY is reset by @ref wizphy_reset().
    @sa ctlwizchip(), CW_SET_PHYCONF, CW_GET_PHYCONF, CW_GET_PHYSTATUS, CW_RESET_PHY
    @sa _PHY_IO_MODE_, wizphy_getphyconf(), wizphy_getphystatus(), wizphy_reset()
*/
void wizphy_setphyconf(wiz_PhyConf* phyconf);

/**
    @ingroup extra_functions
    @brief Get the integrated Ethernet PHY operation mode.
    @details @ref wizphy_getphyconf() gets a operation mode of the integrated Ethernet PHY \n
            through @ref _PHY_IO_MODE_PHYCR_ or @ref _PHY_IO_MODE_MII_.\n
    @param phyconf : @ref wiz_PhyConf
    @note It gets just the configured value for PHY operation, not real PHY operation.\n
         To get real PHY operation, you can call @ref wizphy_getphystatus()
    @sa ctlwizchip(), CW_GET_PHYCONF, CW_SET_PHYCONF, CW_GET_PHYSTATUS
    @sa _PHY_IO_MODE_, wizphy_setphyconf(), wizphy_getphystatus()
*/
void wizphy_getphyconf(wiz_PhyConf* phyconf);

/**
    @ingroup extra_functions
    @brief Get the real PHY operation status when link is established.
    @details @ref wizphy_getphystatus() gets a operation mode of the integrated Ethernet PHY. \n
    @param phyconf : @ref wiz_PhyConf
    @sa ctlwizchip(), CW_GET_PHYSTATUS, CW_GET_PHYCONF, CW_SET_PHYCONF
    @sa wizphy_setphyconf(), wizphy_getphyconf()
*/
void wizphy_getphystat(wiz_PhyConf* phyconf);

/**
    @ingroup extra_functions
    @brief Set the power mode of integrated Ethernet PHY.
    @details @ref wizphy_setphypmode() sets a power mode of the integrated Ethernet PHY \n
            through @ref _PHY_IO_MODE_PHYCR_ or @ref _PHY_IO_MODE_MII_.\n
    @param pmode @ref PHY_POWER_NORM or @ref PHY_POWER_DOWN
    @note When the integrated Ethernet PHY enters in power down mode, \n
         the system clock of @ref _WIZCHIP_ is changed to the lowest speed. \n
         So, you should adjust the access time of @ref _WIZCHIP_ to the changed system clock.
    @sa ctlwizchip(), CW_SET_PHYPOWMODE, CW_GET_PHYPOWMODE
    @sa _PHY_IO_MODE_, wizphy_setphypmode(), wizphy_getphypmode()
*/
void wizphy_setphypmode(uint8_t pmode);

/**
    @ingroup extra_functions
    @brief get the power mode of integrated Ethernet PHY.
    @details @ref wizphy_getphypmode() gets a power mode of the integrated Ethernet PHY \n
            through @ref _PHY_IO_MODE_PHYCR_ or @ref _PHY_IO_MODE_MII_.\n
    @return @ref PHY_POWER_NORM or @ref PHY_POWER_DOWN
    @note When the integrated Ethernet PHY enters in power down mode,\n
         the system clock of @ref _WIZCHIP_ is changed to the lowest speed. \n
         So, you should adjust the access time of @ref _WIZCHIP_ to the changed system clock.
    @sa ctlwizchip(), CW_SET_PHYPOWMODE, CW_GET_PHYPOWMODE
    @sa _PHY_IO_MODE_, wizphy_setphypmode(), wizphy_getphypmode()
*/
int8_t wizphy_getphypmode(void);

/**
    @ingroup extra_functions
    @brief Set the network information for @ref _WIZCHIP_
    @param pnetinfo : @ref wiz_NetInfo
    @sa ctlnetwork(), CN_SET_NETINFO, CN_GET_NETINFO
    @sa wizchip_getnetinfo()
*/
void wizchip_setnetinfo(wiz_NetInfo* pnetinfo);

/**
    @ingroup extra_functions
    @brief Get the network information of @ref _WIZCHIP_
    @param pnetinfo : @ref wiz_NetInfo
    @sa ctlnetwork(), CN_GET_NETINFO, CN_SET_NETINFO
    @sa wizchip_setnetinfo()
*/
void wizchip_getnetinfo(wiz_NetInfo* pnetinfo);

/**
    @ingroup extra_functions
    @brief Set the network mode such as WOL, PPPoE, PING Block, and etc.
    @param netmode : @ref netmode_type.
    @sa ctlnetwork(), CN_SET_NETMODE, CN_GET_NETMODE
    @sa wizchip_getnetmode()
*/
void wizchip_setnetmode(netmode_type netmode);

/**
    @ingroup extra_functions
    @brief Get the network mode such as WOL, PPPoE, PING Block, and etc.
    @return @ref netmode_type.
    @sa ctlnetwork(), CN_GET_NETMODE, CN_SET_NETMODE
    @sa wizchip_setnetmode()
*/
netmode_type wizchip_getnetmode(void);

/**
    @ingroup extra_functions
    @brief Set retransmission time values and retry counts.
    @param nettime : @ref wiz_NetTimeout.
    @sa ctlnetwork(), CN_SET_TIMEOUT, CN_GET_TIMEOUT
    @sa wizchip_gettimeout()
*/
void wizchip_settimeout(wiz_NetTimeout* nettime);

/**
    @ingroup extra_functions
    @brief Get retransmission time values and retry counts.
    @param nettime : @ref wiz_NetTimeout.
    @sa ctlnetwork(), CN_GET_TIMEOUT, CN_SET_TIMEOUT
    @sa wizchip_settimeout()
*/
void wizchip_gettimeout(wiz_NetTimeout* nettime);

/**
    @ingroup extra_functions
    @brief ARP process.
    @details @ref wizchip_arp() processes ARP. \n
            It sends the APR-request to destination and waits to receive the ARP-reply.
    @param arp @ref wiz_ARP.\n
              It sets a destination IP address and indicates the destination hardware address.
    @return 0 : success, destination hardware address is valid.\n
           -1 : fail. destination hardware address is invalid because timeout is occurred.\n
    @sa ctlnetservice(), CNS_ARP
*/
int8_t wizchip_arp(wiz_ARP* arp);

/**
    @ingroup extra_functions
    @brief PING process.
    @details @ref wizchip_ping() processes PING. \n
            It sends the PING-request to destination and waits to receive the PING-reply.
    @param ping @ref wiz_PING, It sets a destination IP address, ID, SEQ of PING-request message
    @return 0 : success, PING-reply is successfully received.\n
           -1 : fail. Timeout is occurred.\n
    @sa ctlnetservice(), CNS_PING
*/
int8_t wizchip_ping(wiz_PING* ping);

/**
    @ingroup extra_functions
    @brief DAD(Duplcated Address Detection) process.
    @details @ref wizchip_dad() detects the duplication of source IPv6 address.\n
            It sends a NA message for DAD to all-node multicasting address(FF02::01).
    @param ipv6 : IPv6 address to be detected the duplication.
    @return 0 : success, There is no duplicated address. \n
           -1 : fail. @ref _WIZCHIP_ source IP address to use is duplicated with a neighbor's one.
    @sa ctlnetservice(), CNS_DAD
*/
int8_t wizchip_dad(uint8_t* ipv6);

/**
    @ingroup extra_functions
    @brief Stateless Address Auto Configuration(SLAAC) process.
    @details @ref wizchip_slaac() get a prefix information from a router for SLAAC.\n
            It sends first a RS message to all-router and waits to receive a RS message with prefix information option from a router.
    @param prefix @ref wiz_Prefix
    @return 0 : success, RA message is successfully received, and <i>prefix</i> is valid.  \n
           -1 : fail. Timeout is occurred.
    @note It is valid only when the prefix information type(0x03) of RA option received first.\n
         The prefix option should be in the order of prefix length, prefix flag, valid lifetime, default lifetime and prefix address. \n
         For more detail, Refer to @ref SLIR_RS.
    @sa ctlnetservice(), CNS_SLAAC
*/
int8_t wizchip_slaac(wiz_Prefix* prefix);

/**
    @ingroup extra_functions
    @brief Unsolicited NA process.
    @details @ref wizchip_unsolicited() updates the network information of @ref _WIZCHIP_ to neighbors.\n
            It sends a unsolicited NA message with @ref _LLAR_ or @ref _GUAR_ to neighbors \n
            in order to update the network information of @ref _WIZCHIP_.\n
            Because the unsolicited NA message have no reply, timeout is always occurred.
    @return always 0. Timeout is occurred.
    @sa ctlnetservice(), CNS_UNSOL
*/
int8_t wizchip_unsolicited(void);

/**
    @ingroup extra_functions
    @brief Get a prefix information of RA message from a router.
    @details @ref wizchip_getprefix() get a prefix information of RA is periodically sent by a router. \n
    @return 0 : success, a RS message is successfully received from a router.
           -1 : fail, a RS message is not received from a router yet.
    @note It is valid only when the prefix information type(0x03) of RA option received first.\n
         The prefix option should be in the order of prefix length, prefix flag, valid lifetime, default lifetime and prefix address. \n
         For more detail, Refer to @ref SLIR_RS.
    @sa ctlnetservice(), CNS_GET_PREFIX
*/
int8_t wizchip_getprefix(wiz_Prefix * prefix);
#endif

#if (_WIZCHIP_ == W5100 || _WIZCHIP_ == W5100S || _WIZCHIP_ == W5200 || _WIZCHIP_ == W5300 || _WIZCHIP_ == W5500)
/**
    @ingroup extra_functions
    @brief Set the network information for WIZCHIP
    @param pnetinfo : @ref wizNetInfo
*/
void wizchip_setnetinfo(wiz_NetInfo* pnetinfo);

/**
    @ingroup extra_functions
    @brief Get the network information for WIZCHIP
    @param pnetinfo : @ref wizNetInfo
*/
void wizchip_getnetinfo(wiz_NetInfo* pnetinfo);

/**
    @ingroup extra_functions
    @brief Set the network mode such WOL, PPPoE, Ping Block, and etc.
    @param pnetinfo Value of network mode. Refer to @ref netmode_type.
*/
int8_t wizchip_setnetmode(netmode_type netmode);

/**
    @ingroup extra_functions
    @brief Get the network mode such WOL, PPPoE, Ping Block, and etc.
    @return Value of network mode. Refer to @ref netmode_type.
*/
netmode_type wizchip_getnetmode(void);

/**
    @ingroup extra_functions
    @brief Set retry time value(@ref _RTR_) and retry count(@ref _RCR_).
    @details @ref _RTR_ configures the retransmission timeout period and @ref _RCR_ configures the number of time of retransmission.
    @param nettime @ref _RTR_ value and @ref _RCR_ value. Refer to @ref wiz_NetTimeout.
*/
void wizchip_settimeout(wiz_NetTimeout* nettime);

/**
    @ingroup extra_functions
    @brief Get retry time value(@ref _RTR_) and retry count(@ref _RCR_).
    @details @ref _RTR_ configures the retransmission timeout period and @ref _RCR_ configures the number of time of retransmission.
    @param nettime @ref _RTR_ value and @ref _RCR_ value. Refer to @ref wiz_NetTimeout.
*/
void wizchip_gettimeout(wiz_NetTimeout* nettime);
//teddy 240122
#elif ((_WIZCHIP_ == W6100)||(_WIZCHIP_ == W6300))
/**
    @ingroup extra_functions
    @brief Set the network information for @ref _WIZCHIP_
    @param pnetinfo : @ref wiz_NetInfo
    @sa ctlnetwork(), CN_SET_NETINFO, CN_GET_NETINFO
    @sa wizchip_getnetinfo()
*/
void wizchip_setnetinfo(wiz_NetInfo* pnetinfo);

/**
    @ingroup extra_functions
    @brief Get the network information of @ref _WIZCHIP_
    @param pnetinfo : @ref wiz_NetInfo
    @sa ctlnetwork(), CN_GET_NETINFO, CN_SET_NETINFO
    @sa wizchip_setnetinfo()
*/
void wizchip_getnetinfo(wiz_NetInfo* pnetinfo);

/**
    @ingroup extra_functions
    @brief Set the network mode such as WOL, PPPoE, PING Block, and etc.
    @param netmode : @ref netmode_type.
    @sa ctlnetwork(), CN_SET_NETMODE, CN_GET_NETMODE
    @sa wizchip_getnetmode()
*/
void wizchip_setnetmode(netmode_type netmode);

/**
    @ingroup extra_functions
    @brief Get the network mode such as WOL, PPPoE, PING Block, and etc.
    @return @ref netmode_type.
    @sa ctlnetwork(), CN_GET_NETMODE, CN_SET_NETMODE
    @sa wizchip_setnetmode()
*/
netmode_type wizchip_getnetmode(void);

/**
    @ingroup extra_functions
    @brief Set retransmission time values and retry counts.
    @param nettime : @ref wiz_NetTimeout.
    @sa ctlnetwork(), CN_SET_TIMEOUT, CN_GET_TIMEOUT
    @sa wizchip_gettimeout()
*/
void wizchip_settimeout(wiz_NetTimeout* nettime);

/**
    @ingroup extra_functions
    @brief Get retransmission time values and retry counts.
    @param nettime : @ref wiz_NetTimeout.
    @sa ctlnetwork(), CN_GET_TIMEOUT, CN_SET_TIMEOUT
    @sa wizchip_settimeout()
*/
void wizchip_gettimeout(wiz_NetTimeout* nettime);

#endif

#ifdef __cplusplus
}
#endif

#endif   // _WIZCHIP_CONF_H_
