//* ****************************************************************************
//! \file W6300.h
//! \brief W6300 HAL Header File.
//! \version 1.0.0
//! \date 2019/01/01
//! \par  Revision history
//!             <2019/01/01> 1st Release
//! \author MidnightCow
//! \copyright
//!
//! Copyright (c)  2019, WIZnet Co., LTD.
//!
//! Permission is hereby granted, free of charge, to any person obtaining a copy
//! of this software and associated documentation files (the "Software"), to deal
//! in the Software without restriction, including without limitation the rights
//! to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
//! copies of the Software, and to permit persons to whom the Software is
//! furnished to do so, subject to the following conditions:
//!
//! The above copyright notice and this permission notice shall be included in
//! all copies or substantial portions of the Software.
//!
//! THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//! IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//! FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//! AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//! LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//! OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
//! SOFTWARE.
//!
//*****************************************************************************


#ifndef      _W6300_H_
#define      _W6300_H_

#include <stdint.h>
#include "../wizchip_conf.h"


#ifdef __cplusplus
extern "C" {
#endif

/// @cond DOXY_APPLY_CODE
#if      (_WIZCHIP_ == W6300)
/// @endcond

#define _W6300_SPI_READ_                  (0x00 << 5)        ///< SPI interface Read operation in Control Phase
#define _W6300_SPI_WRITE_                 (0x01 << 5)        ///< SPI interface Write operation in Control Phase

#define WIZCHIP_CREG_BLOCK                (0x00   )       ///< Common register block
#define WIZCHIP_SREG_BLOCK(N)             ((1+4*N))       ///< SOCKETn register block
#define WIZCHIP_TXBUF_BLOCK(N)            ((2+4*N))       ///< SOCKETn Tx buffer address block
#define WIZCHIP_RXBUF_BLOCK(N)            ((3+4*N))       ///< SOCKETn Rx buffer address block

#define WIZCHIP_OFFSET_INC(ADDR, N) (ADDR + (N<<8)) ///< Increase offset address

#if (_WIZCHIP_IO_MODE_ & _WIZCHIP_IO_MODE_BUS_)
#define IDM_AR0                        ((_WIZCHIP_IO_BASE_ + 0x0000))      ///< Indirect High Address Register
#define IDM_AR1                        ((_WIZCHIP_IO_BASE_ + 0x0001))      ///< Indirect Low Address Register
#define IDM_BSR                        ((_WIZCHIP_IO_BASE_ + 0x0002))      ///< Block Select Register
#define IDM_DR                         ((_WIZCHIP_IO_BASE_ + 0x0003))      ///< Indirect Data Register
#define _W6300_IO_BASE_       _WIZCHIP_IO_BASE_
#elif (_WIZCHIP_IO_MODE_ & _WIZCHIP_IO_MODE_SPI_)
#define IDM_AR0                        ((_WIZCHIP_IO_BASE_ + 0x0000))      ///< Indirect High Address Register
#define IDM_AR1                        ((_WIZCHIP_IO_BASE_ + 0x0001))      ///< Indirect Low Address Register
#define IDM_BSR                        ((_WIZCHIP_IO_BASE_ + 0x0002))      ///< Block Select Register
#define IDM_DR                         ((_WIZCHIP_IO_BASE_ + 0x0003))      ///< Indirect Data Register
#define _W6300_IO_BASE_       0x00000000
#define _W6100_IO_BASE_       0x00000000
#endif


//-----------       defgroup --------------------------------

/**
    @defgroup W6300 W6300
    @brief @ref _WIZCHIP_ register defines and I/O functions
    @details
     - @ref WIZCHIP_register_W6300 : @ref Common_register_group_W6300, @ref Socket_register_group_W6300
     - @ref WIZCHIP_IO_Functions_W6300 : @ref Basic_IO_function_W6300, @ref Common_register_access_function_W6300, @ref Socket_register_access_function_W6300
*/

/**
    @defgroup WIZCHIP_register_W6300 WIZCHIP register
    @ingroup W6300
    @brief @ref WIZCHIP_register_W6300 defines register group of @b W6300.
    @details
    - @ref Common_register_group_W6300 : Common register group W6300
    - @ref Socket_register_group_W6300 : SOCKET n register group W6300
*/

/**
    @defgroup Basic_IO_function_W6300 Basic I/O function
    @ingroup WIZCHIP_IO_Functions_W6300
    @brief These are basic input/output functions to read values from register or write values to register.
*/

/**
    @defgroup Common_register_access_function_W6300 Common register access functions
    @ingroup WIZCHIP_IO_Functions_W6300
    @brief These are functions to access @ref Common_register_group_W6300.
*/

/**
    @defgroup Socket_register_access_function_W6300 Socket register access functions
    @ingroup WIZCHIP_IO_Functions_W6300
    @brief These are functions to access @ref Socket_register_group_W6300.
*/

/**
    @defgroup WIZCHIP_IO_Functions_W6300 WIZCHIP I/O functions
    @ingroup W6300
    @brief @ref WIZCHIP_IO_Functions_W6300 supports the basic I/O functions for @ref WIZCHIP_register_W6300.
    @details
     - @ref WIZCHIP_IO_Functions_W6300 \n
       WIZCHIP_READ(), WIZCHIP_WRITE(), WIZCHIP_READ_BUF(), WIZCHIP_WRITE_BUF()
     - @ref Common_register_access_function_W6300 \n
       - @ref _WIZCHIP_ Mode \n
         getCIDR(), getVER() \n
         getSYSR()  \n
         setCHPLCKR(), setNETLCKR(), setPHYLCKR() \n
         setSYCR0(), getSYCR1(), setSYCR1()
       - Network Mode \n
         getNET4MR(), setNET4MR(), getNET6MR(), setNET6MR(), getNETMR(), setNETMR(), getNETMR2(), setNETMR2()
       - Interrupt \n
         getIR(), setIRCLR(), getIMR(), setIMR() \n
         getSIR(), getSIMR(), setSIMR() \n
         getSLIR(), setSLIRCLR(), getSLIMR(), setSLIMR() \n
         getINTPTMR(), setINTPTMR()
       - Network Information \n
         NETLOCK(), NETUNLOCK() \n
         getSHAR(), setSHAR(), getGAR(), setGAR(), getSUBR(), setSUBR(), getSIPR(), setSIPR() \n
         getLLAR(), setLLAR(), getGUAR(), setGUAR(), getGA6R(), setGA6R(), getSUB6R(), setSUB6R() \n
         getPLR(), getPFR(), getVLTR(), getPLTR(), getPAR() \n
       - SOCKET-less Commands for PING, ARP and IPv6 Auto-Configuration \n
         getSLCR(), setSLCR() \n
         getPINGIDR(), setPINGIDR(), getPINGSEQR(), setPINGSEQR() \n
         getSLDHAR(), getSLDIPR(), setSLDIPR(), getSLDIP4R(), setSLDIP4R(), getSLDIP6R(), setSLDIP6R(), getSLHOPR(), setSLHOPR() \n
       - Retransmission  \n
         getRCR(), setRCR() \n
         getSLRCR(), setSLRCR(), getSLRTR(), setSLRTR() \n
       - ICMP \n
         getUIPR(), getUIP6R(), getUPORTR(), getUPORT6R() \n
         getICMP6BLKR(), setICMP6BLKR() \n
       - PPPoE \n
         getPTMR(), setPTMR(), getPMNR(), getPMNR() \n
         getPHAR(), setPHAR(), getPSIDR(), setPSIDR(), getPMRUR(), setPMRUR()
       - PHY Configuration \n
         getPHYSR(), setPHYLCKR(), PHYLOCK(), PHYUNLOCK() \n
         setPHYCR0(), getPHYCR1(), setPHYCR1() \n
         getPHYRAR(), setPHYRAR(), setPHYDIR(), getPHYDOR(), getPHYACR(), setPHYACR(), getPHYDIVR(), setPHYDIVR()
       - etc \n
         getTCNTR(), setTCNTRCLR()
     - @ref Socket_register_access_function_W6300 \n
       - SOCKET control \n
         getSn_MR(), setSn_MR(), getSn_MR2(), setSn_MR2(), getSn_PSR(), setSn_PSR(), getSn_CR(), setSn_CR() \n
         getSn_IR(), setSn_IRCLR(), getSn_IMR(), setSn_IMR()  \n
         getSn_RTR(), setSn_RTR(), getSn_RCR(), setSn_RCR(), getSn_KPALVTR(), setSn_KPALVTR()
       - SOCKET information \n
         getSn_SR(), getSn_ESR() \n
         getSn_DHAR(), setSn_DHAR(),  getSn_PORTR(), setSn_PORTR(), getSn_DPORTR(), setSn_DPORTR() \n
         getSn_DIPR(), setSn_DIPR(), getSn_DIP6R(), setSn_DIP6R() \n
         getSn_MSSR(), setSn_MSSR()
       - SOCKET communication \n
         getSn_RX_BSR(), setSn_RX_BSR(), getSn_TX_BSR(), setSn_TX_BSR() \n
         getSn_TX_RD(), getSn_TX_WR(), setSn_TX_WR() \n
         getSn_RX_RD(), setSn_RX_RD(), getSn_RX_WR() \n
         getSn_TX_FSR(), getSn_RX_RSR()
       - IP header field \n
         getSn_FRGR(), setSn_FRGR(),  getSn_TOSR(), setSn_TOSR() \n
         getSn_TTLR(), setSn_TTLR()
*/

/**
    @defgroup Common_register_group_W6300 Common register
    @ingroup WIZCHIP_register_W6300
    @brief Common register group \n
    @details It set the general configuration such as interrupt, network information, ICMP, and etc.
    @sa
    <table width=0>
      <tr><td> @ref _WIZCHIP_ Information   </td><td>: _CIDR_, _VER_                                                                   </td></tr>
      <tr><td> @ref _WIZCHIP_ Mode          </td><td>: _SYSR_, _SYCR0_, _SYCR1_, _CHPLCKR_, _NETLCKR_, _PHYLCKR_                       </td></tr>
      <tr><td> Network Mode                 </td><td>: _NET4MR_, _NET6MR_, _NETMR_, _NETMR2_                                           </td></tr>
      <tr><td> Network Information          </td><td>: _GAR_, _SUBR_, _SHAR_, _SIPR_, _GA6R_, _LLAR_, _GUAR_, _SUB6R_                  </td></tr>
      <tr><td> Interrupt                    </td><td>: _IR_, _IRCLR_, _IMR_, _SIR_, _SIMR_, _SLIR_, _SLIMR_, _SLIRCLR_, _INTPTMR_      </td></tr>
      <tr><td> Data retransmission          </td><td>: _RTR_, _RCR_, _SLRTR_, _SLRCR_, _SLHOPR_                                        </td></tr>
      <tr><td> PPPoE                        </td><td>: _PHAR_, _PSIDR_, _PMRUR_, _PTMR_, _PMNR_                                        </td></tr>
      <tr><td> SOCKET-less command          </td><td>: _SLCR_, _SLIR_, _SLDIPR_, _SLDIP4R_, _SLDIP6R_, _SLDHAR_, _PINGIDR_, _PINGSEQR_ </td></tr>
      <tr><td> ICMP v4 & v6                 </td><td>: _UIPR_, _UPORTR_, _UIP6R_, _UPORT6R_, _ICMP6BLKR_                               </td></tr>
      <tr><td> IPv6 Auto-configuration      </td><td>: _PLR_, _PFR_, _VLTR_, _PAR_                                                     </td></tr>
      <tr><td> PHY Configuration            </td><td>: _PHYSR_, _PHYCR0_, _PHYCR1_, _PHYRAR_, _PHYDIR_, _PHYDOR_, _PHYACR_, _PHYDIVR_  </td></tr>
    </table>
*/


/**
    @defgroup Socket_register_group_W6300 Socket register
    @ingroup WIZCHIP_register_W6300
    @brief Socket register group\n
    @details
    SOCKETn registers configure and control SOCKETn which is necessary to data communication.
    @sa
    <table width=0>
      <tr><td> SOCKETn Control       </td><td>: _Sn_MR_, _Sn_MR2_, _Sn_CR_, _Sn_IR_, _Sn_IRCLR_, _Sn_IMR_, _Sn_PSR_                                </td></tr>
      <tr><td> SOCKETn Information   </td><td>: _Sn_SR_, _Sn_ESR_, _Sn_PORTR_, _Sn_DHAR_, _Sn_DIPR_, _Sn_DIP6R_, _Sn_DPORTR_                       </td></tr>
      <tr><td> SOCKETn Retransmission </td><td>: _Sn_RTR_, _Sn_RCR_                                                                                 </td></tr>
      <tr><td> Internet protocol     </td><td>: _Sn_MSSR_, _Sn_TOSR_, _Sn_TTLR_, _Sn_FRGR_                                                         </td></tr>
      <tr><td> Data communication    </td><td>: _Sn_RX_BSR_, _Sn_TX_BSR_, _Sn_TX_FSR_, _Sn_TX_RD_, _Sn_TX_WR_, _Sn_RX_RSR_, _Sn_RX_RD_, _Sn_RX_WR_ </td></tr>
    </table>
*/

//-----------------------------------------------------------------------------------

//----------------------------- W6300 Common Registers IOMAP -----------------------------

/**
    @addtogroup Common_register_group_W6300
    @{
*/

/**
    @brief Chip Identification Register address [RO] [0x6100]
    @sa getCIDR()
*/
#define _CIDR_               (_W6300_IO_BASE_ + (0x0000 << 8) + WIZCHIP_CREG_BLOCK)

/**
    @brief Chip Identification Register address [RO] [0x11]
    @sa getRTL()
*/
#define _RTL_                 (_W6300_IO_BASE_ + (0x0004 << 8) + WIZCHIP_CREG_BLOCK)

/**
    @brief Chip Version Register address [RO] [0x4661]
    @sa getVER()
*/
#define _VER_                (_W6300_IO_BASE_ + (0x0002 << 8) + WIZCHIP_CREG_BLOCK)

/**
    @brief System Status Register address [RO] [0xEU]
    @details @ref _SYSR_ shows the information such as CHIP, NET, PHY Locking and Host I/F
    <table>
      <tr> <td>7   </td> <td>6   </td> <td>5   </td> <td>4 ~ 2   </td> <td>1  </td> <td>0  </td> </tr>
      <tr> <td>CHPL</td> <td>NETL</td> <td>PHYL</td> <td>Reserved</td> <td>IND</td> <td>SPI</td> </tr>
    </table>
    - @ref SYSR_CHPL
    - @ref SYSR_NETL
    - @ref SYSR_PHYL
    - @ref SYSR_IND  : HOST use Parallel BUS Interface(Indirect Bus Mode)
    - @ref SYSR_SPI  : HOST use SPI Interface

    @sa _CHPLCKR_, _NETLCKR_, _PHYLCKR_,
    @sa getSYSR(), setCHPLCKR(), getCHPLCKR(), CHIPLOCK(), CHIPUNLOCK(), setNETLCKR(), getNETLCKR(), NETLOCK(), NETUNLOCK() \n
          setPHYLCKR(), getPHYLCKR(), PHYLOCK(), PHYUNLOCK()
*/
#define _SYSR_               (_W6300_IO_BASE_ + (0x2000 << 8) + WIZCHIP_CREG_BLOCK)

/**
    @brief System Config Register 0 address [WO][0x80]
    @details @ref _SYCR0_ softly reset to _WIZCHIP_.
    <table>
      <tr> <td>7  </td> <td>6 ~ 0   </td> </tr>
      <tr> <td>RST</td> <td>Reserved</td> </tr>
    </table>
     - @ref SYCR0_RST : Software Reset.

    @note It can be accessed only when @ref SYSR_CHPL = 1.
    @sa _CHPLCKR_, _SYSR_, SYSR_CHPL
    @sa setSYCR0(), setCHPLCKR(), getCHPLCKR(), CHIPLOCK(), CHIPUNLOCK(), getSYSR()
*/
#define _SYCR0_              (_W6300_IO_BASE_ + (0x2004 << 8) + WIZCHIP_CREG_BLOCK)

/**
    @brief System Config Register 1 address [R=W][0x80]
    @details @ref _SYCR1_ controls the global interrupt enable, and selects the system clock.
    <table>
      <tr> <td>7  </td> <td>6 ~ 1   </td> <td>0     </td> </tr>
      <tr> <td>IEN</td> <td>Reserved</td> <td>CLKSEL</td> </tr>
    </table>
     - @ref SYCR1_IEN
     - @ref SYCR1_CLKSEL

    @note SYCR1_CLKSEL bit can be accessed only when @ref SYSR_CHPL = 1.
    @sa _CHPLCKR_, _SYSR_, SYSR_CHPL
    @sa getSYCR1(), setSYCR1(), setCHPLCKR(), getCHPLCKR(), CHIPLOCK(), CHIPUNLOCK(), getSYSR()
*/
#define _SYCR1_              (WIZCHIP_OFFSET_INC(_SYCR0_,1))

/**
    @brief Ticker Counter Register address [RO][0x0000]
    @details @ref _TCNTR_ increase by 1 every 100us after _WIZCHIP_ reset.
    @sa _TCNTRCLR_
    @sa getTCNTR(), setTCNTRCLR()
*/
#define _TCNTR_              (_W6300_IO_BASE_ + (0x2016 << 8) + WIZCHIP_CREG_BLOCK)

/**
    @brief Ticker Counter Clear Register address [RO][0x00]
    @details @ref _TCNTRCLR_ clear @ref _TCNTR_.
    @sa setTCNTRCLR(), getTCNTR()
*/
#define _TCNTRCLR_           (_W6300_IO_BASE_ + (0x2020 << 8) + WIZCHIP_CREG_BLOCK)

/**
    @brief Interrupt Register address [RO][0x00]
    @details @ref _IR_ indicates the interrupt status.
            If @ref _IR_ is not equal to x00 INTn PIN is asserted to low until it is x00.

    <table>
      <tr> <td>7  </td> <td>6 ~ 5   </td> <td>4   </td> <td>3       </td> <td>2     </td> <td>1   </td> <td>0    </td> </tr>
      <tr> <td>WOL</td> <td>Reserved</td> <td>UNR6</td> <td>Reserved</td> <td>IPCONF</td> <td>UNR4</td> <td>PTERM</td> </tr>
    </table>
    - @ref IR_WOL    : Wake On LAN
    - @ref IR_UNR6   : Destination Port Unreachable for IPv6
    - @ref IR_IPCONF : @ref _SIPR_ is Conflict
    - @ref IR_UNR4   : Destination Port Unreachable for IPv4
    - @ref IR_PTERM  : PPPoE Terminated

    @sa _IMR_, _IRCLR_, SYCR1_IEN, _CHIPLCKR_, _SYSR_, SYSR_CHPL
    @sa getIR(), setIRCLR(), getSYCR1(), setSYCR1(), getCHPLCKR(), setCHPLCKR(), CHIPLOCK(), CHIPUNLOCK(), getSYSR()
*/
#define _IR_                 (_W6300_IO_BASE_ + (0x2100 << 8) + WIZCHIP_CREG_BLOCK)

/**
    @brief SOCKET Interrupt Register address [RO][0x00]
    @details @ref _SIR_ indicates whether a socket interrupt is occurred or not.\n
            Each bit of @ref _SIR_ be still until @ref _Sn_IR_ is cleared by @ref _Sn_IRCLR_
    @sa _SIMR_, _Sn_IR_, _Sn_IRCLR_, _Sn_IMR_, SYCR1_IEN , _CHIPLCKR_, _SYSR_, SYSR_CHPL
    @sa getSIR(), getSn_IR(), setSn_IRCLR(), getSIMR(), setSIMR(), getSn_IMR(), setSn_IMR(), getSYCR1(), setSYCR1(), \n
       getSYCR1(), setSYCR1(), getCHPLCKR(), setCHPLCKR(), CHIPLOCK(), CHIPUNLOCK(), getSYSR()
*/
#define _SIR_                (_W6300_IO_BASE_ + (0x2101 << 8) + WIZCHIP_CREG_BLOCK)

/**
    @brief SOCKET-less Interrupt Register address [RO][0x00]
    @details @ref _SLIR_ indicates the completion of @ref _SLCR_ or timeout.
    <table>
      <tr> <td>7   </td> <td>6   </td> <td>5    </td> <td>4   </td> <td>3    </td> <td>2 </td> <td>1 </td> <td>0 </td> </tr>
      <tr> <td>TOUT</td> <td>ARP4</td> <td>PING4</td> <td>ARP6</td> <td>PING6</td> <td>NS</td> <td>RS</td> <td>RA</td> </tr>
    </table>
    - @ref SLIR_TOUT  : The timeout occurrence after @ref _SLCR_ is performed
    - @ref SLIR_ARP4  : The completion of @ref SLCR_ARP4
    - @ref SLIR_PING4 : The completion of @ref SLCR_PING4
    - @ref SLIR_ARP6  : The completion f @ref SLCR_ARP6
    - @ref SLIR_PING6 : The completion of @ref SLCR_PING6
    - @ref SLIR_NS    : The completion of @ref SLCR_NS
    - @ref SLIR_RS    : The completion of @ref SLIR_RS
    - @ref SLIR_RA    : The reception from Router Advertisement

    @sa _SLIRCLR_, _SLIMR_, SYCR1_IEN, _CHPLCKR_, _SYSR_, SYSR_CHPL
    @sa getSLIR(), setSLIRCLR(),  getSLIR(), getSLIMR(), setSLIMR(), \n
       getSYCR1(), setSYCR1(), getCHPLCKR(), setCHPLCKR(), CHIPLOCK(), CHIPUNLOCK(), getSYSR()
*/
#define _SLIR_               (_W6300_IO_BASE_ + (0x2102 << 8) + WIZCHIP_CREG_BLOCK)

/**
    @brief Interrupt Mask Register address [R=W][0x00]
    @details @ref _IMR_ is used to mask interrupts of @ref _IR_.\n
            When a bit of @ref _IMR_ and the corresponding bit of @ref _IR_ is set, an interrupt will be issued.
    @sa _IR_, SYCR1_IEN, _CHPLCKR_, _SYSR_, SYSR_CHPL
    @sa getIMR(), setIMR(),  getIR(), setIRCLR(), \n
       getSYCR1(), setSYCR1(), getCHPLCKR(), setCHPLCKR(), CHIPLOCK(), CHIPUNLOCK(), getSYSR()
*/
#define _IMR_                (_W6300_IO_BASE_ + (0x2104 << 8) + WIZCHIP_CREG_BLOCK)

/**
    @brief @ref _IR_  Clear Register address [WO][0x00]
    @details @ref _IRCLR_ clears @ref _IR_
    @sa _IR_, _IMR_, SYCR1_IEN, _CHPLCKR_, _SYSR_, SYSR_CHPL
    @sa setIRCLR(), getIR(), getIMR(), getSYCR1(), setSYCR1(), getCHPLCKR(), setCHPLCKR(), CHIPLOCK(), CHIPUNLOCK(), getSYSR()
*/
#define _IRCLR_              (_W6300_IO_BASE_ + (0x2108 << 8) + WIZCHIP_CREG_BLOCK)

/**
    @brief SOCKET Interrupt Mask Register address [R=W]][0x00]
    @details @ref _SIMR_ is used to mask interrupts of @ref _SIR_.\n
            When a bit of @ref _SIMR_ and the corresponding bit of @ref _SIR_ is set, an interrupt will be issued.\n
            when @ref _Sn_IR_ is not 0,  The N-th bit of @ref _SIR_ is set. Otherwise, this bit is automatically clear.\n
    @sa _SIR_, _Sn_IR_, _Sn_IRCLR_, _Sn_IMR_, SYCR1_IEN, _CHPLCKR_, _SYSR_, SYSR_CHPL
    @sa getSIMR(), setSIMR(), getSIR(), setSn_IRCLR(), getSn_IMR(), setSn_IMR(), \n
       getSYCR1(), setSYCR1(), getCHPLCKR(), setCHPLCKR(), CHIPLOCK(), CHIPUNLOCK(), getSYSR()
*/
#define _SIMR_               (_W6300_IO_BASE_ + (0x2114 << 8) + WIZCHIP_CREG_BLOCK)

/**
    @brief SOCKET-less Interrupt Mask Register address [R=W][0x00]
    @details @ref _SLIMR_ is used to mask interrupts of @ref _SLIR_\n
            When a bit of @ref _SLIMR_ and the corresponding bit of @ref _SLIR_ is set, an interrupt will be issued.
    @sa _SLIR_, _SLIRCLR_, SYCR1_IEN, _CHPLCKR_, _SYSR_, SYSR_CHPL
    @sa getSLIMR(), setSLIMR(), getSLIR(), setSLIRCLR(), \n
       getSYCR1(), setSYCR1(), getCHPLCKR(), setCHPLCKR(), CHIPLOCK(), CHIPUNLOCK(), getSYSR()
*/
#define _SLIMR_              (_W6300_IO_BASE_ + (0x2124 << 8) + WIZCHIP_CREG_BLOCK)

/**
    @brief SOCKET-less Interrupt Clear Register address [WO][0x00]
    @details @ref _SLIRCLR_ clears @ref _SLIR_
    @sa _SLIR_, _SLIRCLR_, _SLIMR_, SYCR1_IEN, _CHPLCKR_, _SYSR_, SYSR_CHPL
    @sa getSLIR(), setSLIRCLR(), getSLIMR(), setSLIMR(), \n
       getSYCR1(), setSYCR1(), getCHPLCKR(), setCHPLCKR(), CHIPLOCK(), CHIPUNLOCK(), getSYSR()
*/
#define _SLIRCLR_            (_W6300_IO_BASE_ + (0x2128 << 8) + WIZCHIP_CREG_BLOCK)

/**
    @brief SOCKET-less Prefer Source IPv6 Address Register address [R=W][0x00]
    @details @ref _SLPSR_ select the Source IPv6 Address to transmit a packet by @ref _SLCR_.
     - @ref PSR_AUTO
     - @ref PSR_LLA
     - @ref PSR_GUA
    @sa _SLCR_, _Sn_PSR_
    @sa getSLPSR(), setSLPSR(), getSLCR(), setSLCR(), getSn_PSR(), setSn_PSR()
*/
#define _SLPSR_              (_W6300_IO_BASE_ + (0x212C << 8) + WIZCHIP_CREG_BLOCK)

/**
    @brief SOCKET-less Command Register address [RW,AC][0x00]
    @details @ref _SLCR_ can be request a message such like as ARP, PING, and ICMPv6 without SOCKET.
    <table>
      <tr> <td>7       </td> <td>6   </td> <td>5    </td> <td>4   </td> <td>3    </td> <td>2 </td> <td>1 </td> <td>0  </td> </tr>
      <tr> <td>Reserved</td> <td>ARP4</td> <td>PING4</td> <td>ARP6</td> <td>PING6</td> <td>NS</td> <td>RS</td> <td>UNA</td> </tr>
    </table>
     - @ref SLCR_ARP4
     - @ref SLCR_PING4
     - @ref SLCR_ARP6
     - @ref SLCR_PING6
     - @ref SLCR_NS
     - @ref SLCR_RS
     - @ref SLCR_UNA

    @sa _SLIR_, _SLIMR_, _SLDIPR_, _SLDIP4R_, _SLDIP6R_, _SLDHAR_, SYCR1_IEN, _CHPLCKR_, _SYSR_, SYSR_CHPL
    @sa getSLCR(), setSLCR(), getSLIR(), setSLIRCLR(), getSLIMR(), setSLIMR(), getSLDIPR(),setSLDIPR(), getSLDIP4R(),setSLDIP4R(),
       getSLDIP6R(), setSLDIP6R(), getSLDHAR(), getSYCR1(), setSYCR1(), getCHPLCKR(), setCHPLCKR(), CHIPLOCK(), CHIPUNLOCK(), getSYSR()
*/
#define _SLCR_               (_W6300_IO_BASE_ + (0x2130 << 8) + WIZCHIP_CREG_BLOCK)

/**
    @brief PHY Status Register address [RO][0x00]
    @details @ref _PHYSR_ shows the operation mode of PHY, the link status and etc.
     - @ref PHYSR_CAB  : @ref PHYSR_CAB_OFF, @ref PHYSR_CAB_ON
     - @ref PHYSR_MODE : @ref PHYSR_MODE_AUTO, @ref PHYSR_MODE_100F, @ref PHYSR_MODE_100H, @ref PHYSR_MODE_10F, @ref PHYSR_MODE_10H
     - @ref PHYSR_DPX  : @ref PHYSR_DPX_FULL, @ref PHYSR_DPX_HALF
     - @ref PHYSR_SPD  : @ref PHYSR_SPD_100M, @ref PHYSR_SPD_10M
     - @ref PHYSR_LNK  : @ref PHYSR_LNK_UP, @ref PHYSR_LNK_DOWN

    @sa _PHYCR0_, _PHYLCKR_, _SYSR_, SYSR_PHYL
    @sa getPHYSR(), setPHYCR0(), setPHYLCKR(), getPHYLCKR(), PHYLOCK(), PHYUNLOCK(), getSYSR()
*/
#define _PHYSR_              (_W6300_IO_BASE_ + (0x3000 << 8) + WIZCHIP_CREG_BLOCK)

/**
    @brief PHY Internal Register Address Register address(R/W)
    @details @ref _PHYRAR_ specifies the address of register in the Ethernet PHY.
     - @ref PHYRAR_BMCR
     - @ref PHYRAR_BMSR
    @sa _PHYACR_, _PHYDOR_, _PHYDIR_, _PHYDIVR_
    @sa getPHYACR(), setPHYACR(), getPHYDOR(), setPHYDIR(), getPHYDIVR(), setPHYDIVR()
*/
#define _PHYRAR_             (_W6300_IO_BASE_ + (0x3008 << 8) + WIZCHIP_CREG_BLOCK)

/**
    @brief PHY Data Input Register address [R=W][0x00]
    @details @ref _PHYDIR_ specifies the value to write to the register in PHY
    @sa _PHYRAR_, _PHYACR_, _PHYDOR_, _PHYDIVR_
    @sa setPHYDIR(), getPHYRAR(), setPHYRAR(), getPHYACR(), setPHYACR(), getPHYDOR(), setPHYDIR(), getPHYDIVR(), setPHYDIVR()
*/
#define _PHYDIR_             (_W6300_IO_BASE_ + (0x300C << 8) + WIZCHIP_CREG_BLOCK)

/**
    @brief PHY Data Output Register address [WO][0x00]
    @details @ref _PHYDOR_ read the value from the register in PHY
    @sa _PHYRAR_, _PHYACR_, _PHYDIR_, _PHYDIVR_
    @sa getPHYDOR(), getPHYRAR(), setPHYRAR(), getPHYACR(), setPHYACR(), setPHYDIR(), getPHYDIVR(), setPHYDIVR()
*/
#define _PHYDOR_             (_W6300_IO_BASE_ + (0x3010 << 8) + WIZCHIP_CREG_BLOCK)

/**
    @brief PHY Access Register address [RW,AC][0x00]
    @details @ref _PHYACR_ write(read)  to(from) the value of register in the Ethernet PHY
     - @ref PHYACR_READ
     - @ref PHYACR_WRITE
    @sa _PHYRAR_, _PHYDOR_, _PHYDIR_, _PHYDIVR_
    @sa getPHYACR(), setPHYACR(), getPHYDOR(), getPHYRAR(), setPHYRAR(), setPHYDIR(), getPHYDIVR(), setPHYDIVR()
*/
#define _PHYACR_             (_W6300_IO_BASE_ + (0x3014 << 8) + WIZCHIP_CREG_BLOCK)

/**
    @brief PHY's MDC Clock Division Register address [R=W][0x01]
    @details @ref _PHYDIVR_ divides the system clock for the MDC clock of Ethernet PHY'
     - @ref PHYDIVR_32
     - @ref PHYDIVR_64
     - @ref PHYDIVR_128
    @sa _PHYRAR_, _PHYACR_, _PHYDOR_, _PHYDIR_, _PHYDIVR_
    @sa getPHYDIVR(), setPHYDIVR(), getPHYRAR(), setPHYRAR(), getPHYACR(), setPHYACR(), getPHYDOR(), setPHYDIR()
*/
#define _PHYDIVR_            (_W6300_IO_BASE_ + (0x3018 << 8) + WIZCHIP_CREG_BLOCK)

/**
    @brief PHY Control Register address [WO][0x00]
    @details @ref _PHYCR0_ controls the operation mode of PHY.
            The result will be checked by @ref _PHYSR_ after PHY HW reset by @ref PHYCR1_RST.
     - @ref PHYCR0_AUTO
     - @ref PHYCR0_100F
     - @ref PHYCR0_100H
     - @ref PHYCR0_10F
     - @ref PHYCR0_10H

    @note It can be only accessed when @ref SYSR_PHYL is unlock.
    @sa _SYSR_, _PHYCR1_
    @sa setPHYCR0(), getSYSR(), getPHYCR1(), setPHYCR1()
*/
#define _PHYCR0_             (_W6300_IO_BASE_ + (0x301C << 8) + WIZCHIP_CREG_BLOCK)

/**
    @brief PHY Control Register address [R=W][0x40]
    @details @ref _PHYCR1_ controls the Ethernet PHY function such as HW reset, Power down and etc.
    <table>
      <tr> <td>7       </td> <td>6       </td> <td>5   </td> <td>4      </td> <td>3 </td> <td>2 ~ 1   </td> <td>0  </td> </tr>
      <tr> <td>Reserved</td> <td>Always 1</td> <td>PWDN</td> <td>Reseved</td> <td>TE</td> <td>Reserved</td> <td>RST</td> </tr>
    </table>
     - @ref PHYCR1_PWDN
     - @ref PHYCR1_TE
     - @ref PHYCR1_RST

    @note It can be only accessed when @ref SYSR_PHYL is unlock.
    @sa _SYSR_, _PHYCR0_
    @sa getPHYCR1(), setPHYCR1(), setPHYCR0(), getSYSR()
*/
#define _PHYCR1_             WIZCHIP_OFFSET_INC(_PHYCR0_,1)

/**
    @brief Network IPv4 Mode Register address [R=W][0x00]
    @details @ref _NET4MR_ can block the transmission such like as unreachable message, TCP reset, and ping relay.\n
            It can ARP request before ping relpy.

    <table>
      <tr> <td>7 ~ 4   </td> <td>3   </td> <td>2   </td> <td>1   </td> <td>0 </td> </tr>
      <tr> <td>Reserved</td> <td>UNRB</td> <td>PARP</td> <td>RSTB</td> <td>PB</td> </tr>
    </table>
     - @ref NETxMR_UNRB
     - @ref NETxMR_PARP
     - @ref NETxMR_RSTB
     - @ref NETxMR_PB
    @sa _NET6MR_
    @sa getNET4MR(), setNET4MR(), getNET6MR(), setNET6MR()
*/
#define _NET4MR_             (_W6300_IO_BASE_ + (0x4000 << 8) + WIZCHIP_CREG_BLOCK)

/**
    @brief Network IPv6 Mode Register address [R=W][0x00]
    @details @ref _NET6MR_ can block the transmission such like as unreachable message, TCP reset, and ping relay.\n
            It can ARP request before ping reply.

    <table>
      <tr> <td>7 ~ 4   </td> <td>3   </td> <td>2   </td> <td>1   </td> <td>0 </td> </tr>
      <tr> <td>Reserved</td> <td>UNRB</td> <td>PARP</td> <td>RSTB</td> <td>PB</td> </tr>
    </table>
     - @ref NETxMR_UNRB
     - @ref NETxMR_PARP
     - @ref NETxMR_RSTB
     - @ref NETxMR_PB
    @sa _NET4MR_
    @sa getNET6MR(), setNET6MR(), getNET4MR(), setNET4MR()
*/
#define _NET6MR_             (_W6300_IO_BASE_ + (0x4004 << 8) + WIZCHIP_CREG_BLOCK)

/**
    @brief Network Mode Register address [R=W][0x00]
    @details @ref _NETMR_ set WOL(Wake On Lan) mode.\n
            It also can block a packet such as \n
            IPv6 PING request from an all-node broadcasting, \n
            IPv6 PING request from a solicited mulitcasting address,\n
            IPv4 packets, \n
            and IPv6 packets.

    <table>
      <tr> <td>7 ~ 6   </td> <td>5  </td> <td>4  </td> <td>3       </td> <td>2  </td> <td>1   </td> <td>0  </td> </tr>
      <tr> <td>Reserved</td> <td>ANB</td> <td>M6B</td> <td>Always 0</td> <td>WOL</td> <td>IP6B</td> <td>IP4B</td> </tr>
    </table>
    - @ref NETMR_ANB
    - @ref NETMR_M6B
    - @ref NETMR_WOL
    - @ref NETMR_IP6B
    - @ref NETMR_IP4B
    @sa getNETMR(), setNETMR()

*/
#define _NETMR_              (_W6300_IO_BASE_ + (0x4008 << 8) + WIZCHIP_CREG_BLOCK)

/**
    @brief Network Mode Register 2 address [R=W][0x00]
    @details @ref _NETMR2_ set PPPoE mode.\n
            It also can select the destination hardware address to either Ethernet frame MAC or target MAC in the ARP-reply message
    <table>
      <tr> <td>7   </td> <td>6 ~ 1</td> <td>0    </td> </tr>
      <tr> <td>DHAS</td> <td>6 ~ 1</td> <td>PPPoE</td> </tr>
    </table>
     - @ref NETMR2_DHAS : @ref NETMR2_DHAS_ARP, @ref NETMR2_DHAS_ETH
     - @ref NETMR2_PPPoE
    @sa getNETMR2(), setNETMR2()
*/
#define _NETMR2_             (_W6300_IO_BASE_ + (0x4009 << 8) + WIZCHIP_CREG_BLOCK)

/**
    @brief PPP LCP request Timer Register address [R=W][0x28]
    @details @ref _PTMR_ sets the time for sending LCP echo request.\n
            The unit of time is 25ms.
    @sa _PMNR_, _PHAR_, _PSIDR_, _PMRUR_, NETMR2_PPPoE
    @sa getPTMR(), setPTMR(), getPMNR(), setPMNR(), getPHAR(), setPHAR(), getPSIDR(), setPSIDR(), getPMRUR(), setPMRUR(), getNETMR2(), setNETMR2()
*/
#define _PTMR_               (_W6300_IO_BASE_ + (0x4100 << 8) + WIZCHIP_CREG_BLOCK)

/**
    @brief PPP LCP Magic Number Register address [R=W][0x00]
    @details @ref _PMNR_ sets the 4bytes magic number to be used in LCP negotiation.
    @sa _PTMR_, _PHAR_, _PSIDR_, _PMRUR_, NETMR2_PPPoE
    @sa getPMNR(), setPMNR(), getPTMR(), setPTMR(), getPHAR(), setPHAR(), getPSIDR(), setPSIDR(), getPMRUR(), setPMRUR(), getNETMR2(), setNETMR2()
*/
#define _PMNR_               (_W6300_IO_BASE_ + (0x4104 << 8) + WIZCHIP_CREG_BLOCK)

/**
    @brief PPPoE Hardware Address Register address [R=W][0x00]
    @details @ref _PHAR_ sets the PPPoE server hardware address that is acquired during PPPoE connection process.
    @sa _PTMR_, _PMNR_, _PSIDR_, _PMRUR_, NETMR2_PPPoE
    @sa getPHAR(), setPHAR(), getPTMR(), setPTMR(), getPMNR(), setPMNR(), getPSIDR(), setPSIDR(), getPMRUR(), setPMRUR(), getNETMR2(), setNETMR2()
*/
#define _PHAR_               (_W6300_IO_BASE_ + (0x4108 << 8) + WIZCHIP_CREG_BLOCK)

/**
    @brief PPP Session ID Register address [R=W][0X0000]
    @details @ref  _PSIDR_ sets the PPPoE sever session ID acquired during PPPoE connection process.
    @sa _PTMR_, _PMNR_, _PHAR_, _PMRUR_, NETMR2_PPPoE
    @sa getPSIDR(), setPSIDR(), getPTMR(), setPTMR(), getPMNR(), setPMNR(), getPHAR(), setPHAR(), getPMRUR(), setPMRUR(), getNETMR2(), setNETMR2()
*/
#define _PSIDR_              (_W6300_IO_BASE_ + (0x4110 << 8) + WIZCHIP_CREG_BLOCK)

/**
    @brief PPP Maximum Receive Unit Register address [R=W][0xFFFF]
    @details @ref _PMRUR_ sets the maximum receive unit of PPPoE.
    @sa _PTMR_, _PMNR_, _PHAR_, _PSIDR_, NETMR2_PPPoE
    @sa  getPMRUR(), setPMRUR(), getPTMR(), setPTMR(), getPMNR(), setPMNR(), getPHAR(), setPHAR(), getPSIDR(), setPSIDR(), getNETMR2(), setNETMR2()
*/
#define _PMRUR_              (_W6300_IO_BASE_ + (0x4114 << 8) + WIZCHIP_CREG_BLOCK)

/**
    @brief Source Hardware Address Register address [R=W][00:00:00:00:00:00]
    @details @ref _SHAR_ sets the source hardware address.
    @note It can be accessed only when @ref SYSR_NETL is unlock.
    @sa SYSR_NETL, _NETLCKR_
    @sa getSHAR(), setSHAR(), getSYSR(), setNETLCKR(), getNETLCKR(), NETLOCK(), NETUNLOCK()
*/
#define _SHAR_               (_W6300_IO_BASE_ + (0x4120 << 8) + WIZCHIP_CREG_BLOCK)

/**
    @brief IPv4 Gateway Address Register address [R=W][0.0.0.0]
    @details @ref _GAR_ sets the default gateway IPv4 address.
    @note It can be accessed only when @ref SYSR_NETL is unlock.
    @sa SYSR_NETL, _NETLCKR_, _GA6R_
    @sa getGAR(), setGAR(), getSYSR(), setNETLCKR(), getNETLCKR(), NETLOCK(), NETUNLOCK(), getGA6R(), setGA6R()
*/
#define _GAR_                (_W6300_IO_BASE_ + (0x4130 << 8) + WIZCHIP_CREG_BLOCK)
#define _GA4R_               (_GAR_)      ///< Refer to @ref _GAR_
/**
    @brief IPv4 Subnet Mask Register address [R=W][0.0.0.0]
    @details @ref _SUBR_ sets the default subnet mask address of IPv4.
    @note It can be accessed only when @ref SYSR_NETL is unlock.
    @sa SYSR_NETL, _NETLCKR_, _SUB6R_
    @sa getSUBR(), setSUBR(), getSYSR(), setNETLCKR(), getNETLCKR(), NETLOCK(), NETUNLOCK(), getSUB6R(), setSUB6R()
*/
#define _SUBR_               (_W6300_IO_BASE_ + (0x4134 << 8) + WIZCHIP_CREG_BLOCK)
#define _SUB4R_              (_SUBR_)      ///< Refer to @ref _SUBR_

/**
    @brief IPv4 Source IP Register address [R=W][0.0.0.0]
    @details @ref _SIPR_ sets the source IPv4 address.
    @note It can be accessed only when @ref SYSR_NETL is unlock.
    @sa SYSR_NETL, _NETLCKR_, _LLAR_, _GUAR_
    @sa getSIPR(), setSIPR(), getSYSR(), setNETLCKR(), getNETLCKR(), NETLOCK(), NETUNLOCK(), getLLAR(), setLLAR(), getGUAR(),setGUAR()
*/
#define _SIPR_               (_W6300_IO_BASE_ + (0x4138 << 8) + WIZCHIP_CREG_BLOCK)
#define _SIP4R_              (_SIPR_)   ///< Refer to @ref _SIPR_.

/**
    @brief IPv6 LLA(Link Local Address) Register address [R=W][::]
    @details @ref _LLAR_ sets the LLA address of IPv6.
    @note It can be accessed only when @ref SYSR_NETL is unlock.
    @sa SYSR_NETL, _NETLCKR_, _GUAR_, _SIPR_
    @sa getLLAR(), setLLAR(), getSYSR(), setNETLCKR(), getNETLCKR(), NETLOCK(), NETUNLOCK(), getGUAR(),setGUAR(), getSIPR(), setSIPR()
*/
#define _LLAR_               (_W6300_IO_BASE_ + (0x4140 << 8) + WIZCHIP_CREG_BLOCK)

/**
    @brief IPv6 GUA(Global Unicast Address) Register address [R=W][::]
    @details @ref _GUAR_ sets the GUA address of IPv6.
    @note It can be accessed only when @ref SYSR_NETL is unlock.
    @sa SYSR_NETL, _NETLCKR_, _LLAR_, _SIPR_
    @sa getGUAR(), setGUAR(), getSYSR(), setNETLCKR(), getNETLCKR(), NETLOCK(), NETUNLOCK(), getLLAR(),setLLAR(), getSIPR(), setSIPR()
*/
#define _GUAR_               (_W6300_IO_BASE_ + (0x4150 << 8) + WIZCHIP_CREG_BLOCK)

/**
    @brief IPv6 Subnet Mask Register address [R=W][]
    @details @ref _SUB6R_ sets the default subnet mask address of IPv6.
    @note It can be accessed only when @ref SYSR_NETL is unlock.
    @sa SYSR_NETL, _NETLCKR_, _SUBR_
    @sa getSUB6R(), setSUB6R(), getSYSR(), setNETLCKR(), getNETLCKR(), NETLOCK(), NETUNLOCK(), getSUBR(), setSUBR()
*/
#define _SUB6R_              (_W6300_IO_BASE_ + (0x4160 << 8) + WIZCHIP_CREG_BLOCK)

/**
    @brief IPv6 Gateway Address Register address [R/W][::]
    @details @ref _GA6R_ sets the default gateway IPv6 address.
    @sa _GAR_
    @sa getGA6R(), setGA6R(), getGAR(), setGAR()
*/
#define _GA6R_               (_W6300_IO_BASE_ + (0x4170 << 8) + WIZCHIP_CREG_BLOCK)

/**
    @brief SOCKET-less Peer IPv6 Register address [R=W][::]
    @details @ref _SLDIP6R_ sets the destination IP address of @ref _SLCR_.
    @sa _SLDIP6R_, _SLCR_, _SLIR_, _SLIRCLR_, _SLIMR_, _SLDHAR_, _SLDIPR_, _SLDIP4R_
    @sa getSLDIP6R(), setSLDIP6R(), getSLCR(), setSLCR(), getSLIR(), setSLIRCLR(), getSLIMR(), setSLIMR(), getSLDHAR(),
       getSLDIPR(), setSLDIPR(), getSLDIP4R(), setSLDIP4R()
*/
#define _SLDIP6R_            (_W6300_IO_BASE_ + (0x4180 << 8) + WIZCHIP_CREG_BLOCK)

/**
    @brief SOCKET-less Peer IPv6 Register address [R=W][0.0.0.0]
    @details @ref _SLDIPR_(= @ref _SLDIP4R_) sets the destination IPv4 address of @ref _SLCR_.
    @sa _SLDIP4R_, _SLCR_, _SLIR_, _SLIRCLR_, _SLIMR_, _SLDHAR_, _SLDIP6R_
    @sa getSLDIPR(), setSLDIPR(), getSLDIP4R(), setSLDIP4R(), getSLCR(), setSLCR(), getSLIR(), setSLIRCLR(), getSLIMR(), setSLIMR(), getSLDHAR(),
       getSLDIP6R(), setSLDIP6R()
*/
#define _SLDIPR_             (_W6300_IO_BASE_ + (0x418C << 8) + WIZCHIP_CREG_BLOCK)
#define _SLDIP4R_            (_SLDIPR_)            ///< Refer to @ref _SLDIPR_.


/**
    @brief SOCKET-less Peer Hardware Address Register address [RO][00:00:00:00:00:00]
    @details @ref _SLDHAR_ gets the destination hardware address acquired by of @ref SLCR_ARP4, SLCR_ARP6, SLCR_PING4, and SLCR_PING6.
    @sa _SLDIP4R_, _SLDIP6R_, _SLCR_, _SLIR_, _SLIRCLR_, _SLIMR_
    @sa getSLDHAR(), getSLDIP4R(), setSLDIP4R(), getSLDIP6R(), setSLDIP6R(), getSLCR(), setSLCR(), \n
       getSLIR(), setSLIRCLR(), getSLIMR(), setSLIMR()
*/
#define _SLDHAR_             (_W6300_IO_BASE_ + (0x4190 << 8) + WIZCHIP_CREG_BLOCK)

/**
    @brief SOCKET-less Ping ID Register address [R=W][0x00]
    @details @ref _PINGIDR_ sets the PING-request ID to be sent by @ref SLCR_PING4 or @ref SLCR_PING6.
    @sa _SLCR_, _PINGSEQR_, _SLDIPR_, _SLDIP4R_, _SLDIP6R_, _SLDHAR_, _SLIR_, _SLIRCLR_, _SLIMR_
    @sa getPINGIDR(), setPINGIDR(), getSLCR(), setSLCR(), getPINGSEQR(), setPINGSEQR(), getSLDIPR(), setSLDIPR(),
       getSLDIP4R(), setSLDIP4R(), getSLDIP6R(), setSLDIP6R(), getSLDHAR(), getSLIR(), setSLIRCLR(), getSLIMR(), setSLIMR()
*/
#define _PINGIDR_            (_W6300_IO_BASE_ + (0x4198 << 8) + WIZCHIP_CREG_BLOCK)

/**
    @brief SOCKET-less ping Sequence number Register address [R=W][0x0000]
    @details @ref _PINGIDR_ sets the PING-request sequence number to be sent by @ref SLCR_PING4 or @ref SLCR_PING6.
    @sa _SLCR_, _PINGIDR_, _SLDIPR_, _SLDIP4R_, _SLDIP6R_, _SLDHAR_, _SLIR_, _SLIRCLR_, _SLIMR_
    @sa getPINGSEQR(), setPINGSEQR(), getSLCR(), setSLCR(), getPINGIDR(), setPINGIDR(), getSLDIPR(), setSLDIPR(), getSLDIP4R(), setSLDIP4R(),
       getSLDIP6R(), setSLDIP6R(), getSLDHAR(), getSLIR(), setSLIRCLR(), getSLIMR(), setSLIMR()
*/
#define _PINGSEQR_           (_W6300_IO_BASE_ + (0x419C << 8) + WIZCHIP_CREG_BLOCK)

/**
    @brief IPv4 Unreachable Address Register address [RO][0.0.0.0]
    @details @ref _UIPR_ is set when a unreachable ICMPv4 message is received.
    @sa _UPORTR_, _UIP6R_, _UPORT6R_
    @sa getUIPR(), setUIPR(), getUPORTR(), setUPORTR(), getUIPR6(), setUIPR6(), getUPORT6R(), setUPORT6R()
*/
#define _UIPR_               (_W6300_IO_BASE_ + (0x41A0 << 8) + WIZCHIP_CREG_BLOCK)
#define _UIP4R_              (_UIPR_)   ///< Refer to @ref _UPORTR_

/**
    @brief IPv4 Unreachable Port number Register address [RO][0x0000]
    @details @ref _UPORTR_ is set when a unreachable ICMPv4 message is received.
    @sa _UIPR_, _UIP6R_, _UPORT6R_
    @sa getUPORTR(), setUPORTR(), getUIPR(), setUIPR(), getUIPR6(), setUIPR6(), getUPORT6R(), setUPORT6R()
*/
#define _UPORTR_             (_W6300_IO_BASE_ + (0x41A4 << 8) + WIZCHIP_CREG_BLOCK)
#define _UPORT4R_            (_UPORTR_)   ///< Refer to @ref _UPORTR_
/**
    @brief IPv6 Unreachable IP Address Register address [RO][::]
    @details @ref _UIP6R_ is set when a unreachable ICMPv6 message is received.
    @sa _UIPR_, _UPORTR_, _UIP6R_, _UPORT6R_
    @sa getUIPR6(), setUIPR6(), getUIPR(), setUIPR(), getUPORTR(), setUPORTR(), getUPORT6R(), setUPORT6R()
*/
#define _UIP6R_              (_W6300_IO_BASE_ + (0x41B0 << 8) + WIZCHIP_CREG_BLOCK)

/**
    @brief IPv6 Unreachable Port number Register address [RO][0x0000]
    @details @ref _UIP6R_ is set when a unreachable ICMPv6 message is received.
    @sa _UIPR_, _UPORTR_, _UIP6R_, _UPORT6R_
    @sa getUIPR6(), setUIPR6(), getUIPR(), setUIPR(), getUPORTR(), setUPORTR(), getUPORT6R(), setUPORT6R()
*/
#define _UPORT6R_            (_W6300_IO_BASE_ + (0x41C0 << 8) + WIZCHIP_CREG_BLOCK)

/**
    @brief Interrupt Pending Time Register address [R=w][0x0000]
    @details @ref _INTPTMR_ pends the next interrupt issued by the INTn pin of @ref _WIZCHIP_.\n
            It is decreased 1 every 4 SYS_CLK. \n
            If it is zero and some interrupt is still remained, the INTn pin is issued.
    @sa _IR_, _IRCLR_, _IMR_, _SIR_, _Sn_IRCLR_, _SIMR_, _SLIR_, _SLIRCLR_, _SLIMR_, SYCR_IEN
    @sa getINTPTMR(), setINTPTMR(), getIR(), setIRCLR(), getIMR(), setIMR(), getSIR(), setSn_IRCLR(), getSIMR(), setSIMR(), \n
       getSLIR(), setSLIRCLR(), getSLIMR(), setSLIMR(), getSYCR1(), setSYCR1()
*/
#define _INTPTMR_            (_W6300_IO_BASE_ + (0x41C5 << 8) + WIZCHIP_CREG_BLOCK)

/**
    @brief RA Prefix Length Register address [RO][0x00]
    @details @ref _PLR_ is set when RA packet is received from a router.
    @sa SLIR_RA, _SLIRCLR_, _PFR_, _VLTR_, _PLTR_, _PAR_
    @sa getPLR(), getSLIR(), setSLIRCLR(), getPFR(), getVLTR(), getPLTR(), getPAR()
*/
#define _PLR_                (_W6300_IO_BASE_ + (0x41D0 << 8) + WIZCHIP_CREG_BLOCK)

/**
    @brief RA Prefix Flag Register address [RO][0x00]
    @details @ref _PFR_ is set when RA packet is received from a router.
    @sa SLIR_RA, _SLIRCLR_, _PLR_, _VLTR_, _PLTR_, _PAR_
    @sa getPFR(), getSLIR(), setSLIRCLR(), getPLR(), getVLTR(), getPLTR(), getPAR()
*/
#define _PFR_                (_W6300_IO_BASE_ + (0x41D4 << 8) + WIZCHIP_CREG_BLOCK)

/**
    @brief RA Valid Life Time Register address [RO][0x00000000]
    @details @ref _VLTR_ is set when RA packet is received from a router.
    @sa SLIR_RA, _SLIRCLR_, _PLR_, _PFR_, _PLTR_, _PAR_
    @sa getVLTR(), getSLIR(), setSLIRCLR(), getPLR(), getPFR(),  getPLTR(), getPAR()
*/
#define _VLTR_               (_W6300_IO_BASE_ + (0x41D8 << 8) + WIZCHIP_CREG_BLOCK)

/**
    @brief RA Prefered Life Time Register address [RO][0x00000000]
    @details @ref _PLTR_ is set when RA packet is received from a router.
    @sa SLIR_RA, _SLIRCLR_, _PLR_, _PFR_, _PLTR_, _PAR_
    @sa getPLTR(), getSLIR(), setSLIRCLR(), getPLR(), getPFR(), getVLTR(), getPAR()
*/
#define _PLTR_               (_W6300_IO_BASE_ + (0x41DC << 8) + WIZCHIP_CREG_BLOCK)

/**
    @brief RA Prefix Address Register address[RO][::]
    @details @ref _PAR_ is set when RA packet is received from a router.
    @sa SLIR_RA, _SLIRCLR_, _PLR_, _PFR_, _VLTR_, _PLTR_, _PAR_
    @sa getPAR(), getPLTR(), getSLIR(), setSLIRCLR(), getPLR(), getPFR(), getVLTR()
*/
#define _PAR_                (_W6300_IO_BASE_ + (0x41E0 << 8) + WIZCHIP_CREG_BLOCK)

/**
    @brief ICMPv6 Block Register address [R=W][0x00]
    @details @ref _ICMP6BLKR_ can block ICMPv6 message such like as PING, MLD, RA, NS and NA.\n
            In this blocked case, @ref Sn_MR_IPRAW6 SOCKET can receive it.
    <table>
      <tr> <td>7 ~ 5</td> <td>4    </td> <td>3  </td> <td>2 </td> <td>1 </td> <td>0 </td> </tr>
      <tr> <td>7 ~ 5</td> <td>PING6</td> <td>MLD</td> <td>RA</td> <td>NA</td> <td>NS</td> </tr>
    </table>
     - @ref ICMP6BLKR_PING6 : The same as @ref NETxMR_PB
     - @ref ICMP6BLKR_MLD
     - @ref ICMP6BLKR_RA
     - @ref ICMP6BLKR_NA
     - @ref ICMP6BLKR_NS

    @note The blocked message can be accepted by SOCKETn opened with @ref Sn_MR_IPRAW6.
    @sa NETxMR_PB
    @sa getICMP6BLKR(), setICMP6BLKR(), getNET6MR(), setNET6MR()
*/
#define _ICMP6BLKR_          (_W6300_IO_BASE_ + (0x41F0 << 8) + WIZCHIP_CREG_BLOCK)

/**
    @brief Chip configuration Lock Register address [WO][0x00]
    @details @ref _CHPLCKR_ can lock or unlock to access @ref _SYCR0_ and @ref _SYCR1_.\n
            The lock state can be checked from @ref SYSR_CHPL.
    @sa _SYCR0_, _SYCR1_, _SYSR_, SYSR_CHPL
    @sa getCHPLCKR(), setCHPLCKR(), CHIPLOCK(), CHIPUNLOCK(), getSYSR()
*/
#define _CHPLCKR_            (_W6300_IO_BASE_ + (0x41F4 << 8) + WIZCHIP_CREG_BLOCK)

/**
    @brief Network configuration Lock Register address [WO][0x00]
    @details @ref _NETLCKR_ can lock or unlock to access the network information register such as @ref _SIPR_, @ref _LLAR_, and etc.\n
            The lock state can be checked from @ SYSR_NETL.
    @sa _SHAR_, _SIPR_, _SUBR_, _GAR_, _LLAR_, _GUAR_, _SUB6R_, _SYSR_, SYSR_NETL
    @sa getNETLCKR(), setNETLCKR(), NETLOCK(), NETUNLOCK(), getSHAR(), setSHAR(), getSIPR(), getSIPR(), getSUBR(), setSUBR(), \n
       getGAR(), setGAR(), getLLAR(), setLLAR(), getGUAR(), setGUAR(), getSUB6R(), setSUB6R(), getSYSR()
*/
#define _NETLCKR_            (_W6300_IO_BASE_ + (0x41F5 << 8) + WIZCHIP_CREG_BLOCK)

/**
    @brief PHY configuration Lock Register address [WO][0x00]
    @details @ref _PHYLCKR_ can lock or unlock to access @ref _PHYCR0_ and @ref _PHYCR1_.\n
            The lock state can be checked from @ref SYSR_PHYL.
    @sa _PHYCR0_, _PHYCR1_, _SYSR_, SYSR_PHYL.
    @sa getPHYLCKR(), setPHYLCKR(), PHYLOCK(), PHYUNLOCK(), setPHYCR0(), getPHYCR1(), setPHYCR1(), getSYSR()
*/
#define _PHYLCKR_            (_W6300_IO_BASE_ + (0x41F6 << 8) + WIZCHIP_CREG_BLOCK)

/**
    @brief Retransmission Time Register address [R=W][0x07D0]
    @details @ref _RTR_ sets the default timeout value of @ref _Sn_RTR_.\n
            When @ref _Sn_RTR_ is 0, @ref _Sn_RTR_ is reset to @ref _RTR_ after @ref Sn_CR_OPEN.
    @sa _Sn_RTR_, _RCR_, _Sn_RCR_, _Sn_CR_, Sn_CR_OPEN, _Sn_IR_, _Sn_IRCLR_, Sn_IR_TIMEOUT
    @sa getRTR(), setRTR(), getSn_RTR(), setSn_RTR(), getRCR(), setRCR(), getSn_RCR(), setSn_RCR(),  \n
       getSn_CR(), getSn_CR(), getSn_IR(), setSn_IRCLR()
*/
#define _RTR_                (_W6300_IO_BASE_ + (0x4200 << 8) + WIZCHIP_CREG_BLOCK)

/**
    @brief Retransmission Counter Register address [R=W][0x08]
    @details @ref _RCR_ sets the default retransmission count of @ref _Sn_RCR_.\n
            When @ref _Sn_RCR_ is 0, @ref _Sn_RCR_ is initialized as @ref _Sn_RTR_ after @ref Sn_CR_OPEN.
    @sa _Sn_RCR_, _RTR_, _Sn_RTR_, _Sn_CR_, Sn_CR_OPEN, _Sn_IR_, _Sn_IRCLR_, Sn_IR_TIMEOUT
    @sa getRCR(), setRCR(), getSn_RCR(), setSn_RCR(), getRTR(), setRTR(), getSn_RTR(), setSn_RTR(), \n
       getSn_CR(), getSn_CR(), getSn_IR(), setSn_IRCLR()
*/
#define _RCR_                (_W6300_IO_BASE_ + (0x4204 << 8) + WIZCHIP_CREG_BLOCK)

/**
    @brief SOCKET-less Retransmission Time Register address [R=W][0x07D0]
    @details @ref _SLRTR_ sets the timeout value of packet to be retransmitted by @ref _SLCR_.
    @sa _SLRCR_, _SLIR_, _SLIRCLR_, SLIR_TOUT
    @sa getSLRTR(), setSLRTR(), getSLRCR(), setSLRCR(), getSLIR(), setSLIRCLR()
*/
#define _SLRTR_              (_W6300_IO_BASE_ + (0x4208 << 8) + WIZCHIP_CREG_BLOCK)

/**
    @brief SOCKET-less Retransmission Count Register address [R=W][0x00]
    @details @ref _SLRCR_ sets the retry counter of packet to be retransmitted by @ref _SLCR_.
    @sa _SLRTR_, _SLIR_, _SLIRCLR_, SLIR_TOUT
    @sa getSLRCR(), setSLRCR(), getSLRTR(), setSLRTR(), setSLIRCLR(), getSLIR(), setSLIRCLR(),
*/
#define _SLRCR_              (_W6300_IO_BASE_ + (0x420C << 8) + WIZCHIP_CREG_BLOCK)

/**
    @brief SOCKET-less Hop Limit Register address [R=W][0x80]
    @details @ref _SLHOPR_ sets the hop limit value of packet to be transmitted by @ref _SLCR_.
    @sa _SLCR_
    @sa getSLHOPR(), setSLHOPR(), getSLCR(), setSLCR()
*/
#define _SLHOPR_             (_W6300_IO_BASE_ + (0x420F << 8) + WIZCHIP_CREG_BLOCK)

/**
    @}
*/

//----------------------------- W6300 Socket Registers -----------------------------
/**
    @addtogroup Socket_register_group_W6300
    @{
*/
/**
    @brief Socket Mode Register Address [R=W][0x00]
    @details @ref _Sn_MR_ sets the option or protocol type of SOCKETn before @ref Sn_CR_OPEN is performed.\n\n
            Each bit of @ref _Sn_MR_ is defined as the following.
    <table>
      <tr> <td>7       </td> <td>6        </td> <td>5            </td> <td>4        </td> <td>3 ~ 0 </td> </tr>
      <tr> <td>MULTI/MF</td> <td>BRDB/FPSH</td> <td>ND/MC/SMB/MMB</td> <td>UNIB/MMB6</td> <td>P[3:0]</td> </tr>
    </table>
     - @ref Sn_MR_MULTI : Support UDP Multicasting
     - @ref Sn_MR_MF    : Support MAC Filter Enable
     - @ref Sn_MR_BRDB  : Broadcast Block
     - @ref Sn_MR_FPSH  : Force PSH flag
     - @ref Sn_MR_ND    : No Delay ACK flag
     - @ref Sn_MR_MC    : IGMP ver2, ver1
     - @ref Sn_MR_SMB   : Solicited Multicast Block
     - @ref Sn_MR_MMB   : IPv4 Multicast block
     - @ref Sn_MR_UNIB  : Unicast Block
     - @ref Sn_MR_MMB6  : IPv6 UDP Multicast Block
     - <b>P[3:0]</b>
    <table>
      <tr> <td> P[3:0] </td> <td> Protocol Mode  </td> </tr>
      <tr> <td> 0000   </td> <td> SOCKET Closed  </td> </tr>
      <tr> <td> 0001   </td> <td> TCP4           </td> </tr>
      <tr> <td> 0010   </td> <td> UDP4           </td> </tr>
      <tr> <td> 0011   </td> <td> IPRAW4         </td> </tr>
      <tr> <td> 0100   </tr> <td> MACRAW         </td> </tr>
      <tr> <td> 1001   </td> <td> TCP6           </td> </tr>
      <tr> <td> 1010   </td> <td> UDP6           </td> </tr>
      <tr> <td> 1100   </td> <td> IPRAW6         </td> </tr>
      <tr> <td> 1101   </td> <td> TCP Dual(TCPD) </td> </tr>
      <tr> <td> 1110   </td> <td> UDP Dual (UDPD)</td> </tr>
    </table>
     - @ref Sn_MR_CLOSE                      : SOCKET Closed
     - @ref Sn_MR_TCP4(= @ref Sn_MR_TCP)     : TCP4 mode
     - @ref Sn_MR_UDP4(= @ref Sn_MR_UDP)     : UDP4 mode
     - @ref Sn_MR_IPRAW4(= @ref Sn_MR_IPRAW) : IPRAW4 mode
     - @ref Sn_MR_MACRAW                     : MACRAW mode
     - @ref Sn_MR_TCP6                       : TCP6 mode
     - @ref Sn_MR_UDP6                       : UDP6 mode
     - @ref Sn_MR_IPRAW6                     : IPRAW6 mode
     - @ref Sn_MR_TCPD                       : TCP Dual (TCPD) mode
     - @ref Sn_MR_UDPD                       : UDP Dual (UDPD) mode

    @note MACRAW mode should be only used in Socket 0.
    @sa _Sn_CR_, Sn_CR_OPEN, _Sn_SR_, _Sn_MR2_
    @sa getSn_MR(), setSn_MR(), getSn_CR(), setSn_CR(), getSn_SR(), getSn_MR2(), setSn_MR2()
*/
#define _Sn_MR_(N)           (_W6300_IO_BASE_ + (0x0000 << 8) + WIZCHIP_SREG_BLOCK(N))

/**
    @brief SOCKET n Prefer Source IPv6 Address Register Address [R=W][0x00]
    @details @ref _Sn_PSR_ select the Source IPv6 Address to transmit a packet by @ref _Sn_CR_.
    This function is same as @ref _SLPSR_.
     - @ref PSR_AUTO
     - @ref PSR_LLA
     - @ref PSR_GUA
    @sa _Sn_CR_, _Sn_PSR_, _SLPSR_
    @sa getSn_PSR(), setSn_PSR(), getSLCR(), setSLCR(), getSLPSR(), setSLPSR(),
*/
#define _Sn_PSR_(N)         (_W6300_IO_BASE_ + (0x0004 << 8) + WIZCHIP_SREG_BLOCK(N))

/**
    @brief Socket Command Register Address [RW,AC][0x00]
    @details @ref _Sn_CR_ is used to set the command for SOCKET n such as OPEN, CLOSE, CONNECT, LISTEN, SEND, and RECEIVE.\n
            It is automatically cleared to 0x00 after the command is recognized by @ref _WIZCHIP_.\n
            Even though @ref _Sn_CR_ is cleared to 0x00, the command is still being processed.\n
            To check whether the command is completed or not, please check the @ref _Sn_IR_ or @ref _Sn_SR_.
     - @ref Sn_CR_OPEN            : Initialize or open socket.
     - @ref Sn_CR_LISTEN       : Wait connection request on TCP4/TCP6/TCPD mode(<b>Server mode</b>)
     - @ref Sn_CR_CONNECT      : Send connection request on TCP4/TCPD mode(<b>Client mode</b>)
     - @ref Sn_CR_CONNECT6  : Send connection request on TCP6/TCPD mode(<b>Client mode</b>):nohl
     - @ref Sn_CR_DISCON       : Send closing request on TCP/TCP6/TCPD mode.
     - @ref Sn_CR_CLOSE        : Close socket.
     - @ref Sn_CR_SEND            : Update TX buffer pointer and send data in IPv4 socket.
     - @ref Sn_CR_SEND6        : Update TX buffer pointer and send data in IPv6 socket.
     - @ref Sn_CR_SEND_KEEP : Send keep alive message.
     - @ref Sn_CR_RECV            : Update RX buffer pointer and receive data.

    @note These commands should be exclusive executed.\n That is, the other command can not executed when one command is not cleared yet.
    @sa _Sn_IR_, _Sn_IRCLR_, Sn_IMR_, _SIR_, _Sn_SR_
    @sa getSn_CR(), setSn_CR(), getSn_IR(), setSn_IRCLR(), getSn_IMR(), setSn_IMR(), getSIR(), getSn_SR()
*/
#define _Sn_CR_(N)           (_W6300_IO_BASE_ + (0x0010 << 8) + WIZCHIP_SREG_BLOCK(N))

/**
    @brief SOCKETn Interrupt Register Address [RO][0x00]
    @details @ref _Sn_IR_ gets the status of SOCKETn interrupt such as establishment, termination, receiving data, timeout.\n
            If SOCKETn interrupt occurs and the n-th bit of @ref _SIMR_ is set, then @ref SIR_INT(n) is set.\n
            In order to clear the @ref _Sn_IR_ bit, Set the corresponding bit of _Sn_IRCLR_ to 1.\n
            If all @ref _Sn_IR_ bits are cleared, the @ref SIR_INT(n) is automatically cleared.
    <table>
       <tr> <td>7 ~ 5   </td> <td>4     </td> <td>3      </td> <td>2   </td> <td>1     </td> <td>0  </td> </tr>
       <tr> <td>Reserved</td> <td>SENDOK</td> <td>TIMEOUT</td> <td>RECV</td> <td>DISCON</td> <td>CON</td> </tr>
    </table>
     - @ref Sn_IR_SENDOK
     - @ref Sn_IR_TIMEOUT
     - @ref Sn_IR_RECV
     - @ref Sn_IR_DISCON
     - @ref Sn_IR_CON

    @sa _Sn_IRCLR_, _Sn_IMR_, _SIR_, _SIMR_
    @sa getSn_IR(), setSn_IRCLR(), getSn_IMR(), setSn_IMR(), getSIR(), getSIMR(), setSIMR()
*/
#define _Sn_IR_(N)           (_W6300_IO_BASE_ + (0x0020 << 8) + WIZCHIP_SREG_BLOCK(N))

/**
    @brief SOCKETn Interrupt Mask Register Address [R=W][0xFF]
    @details @ref _Sn_IMR_ is used to mask interrupts of @ref _Sn_IR_.
    @sa _Sn_IR_, _Sn_IRCR_, _SIR_, _SIMR_
    @sa getSn_IMR(), setSn_IMR(), getSn_IR(), setSn_IRCLR(), getSIR(), getSIMR(), setSIMR()
*/
#define _Sn_IMR_(N)          (_W6300_IO_BASE_ + (0x0024 << 8) + WIZCHIP_SREG_BLOCK(N))

/**
    @brief SOCKETn Interrupt Clear Register Address [WO][0x00]
    @details @ref _Sn_IRCLR_ clears @ref _Sn_IR_
    @sa _Sn_IR_, _SIR_, _SIMR_
    @sa setSn_IRCLR(), getSn_IR(), getSIR(), getSIMR(), setSIMR()
*/
#define _Sn_IRCLR_(N)        (_W6300_IO_BASE_ + (0x0028 << 8) + WIZCHIP_SREG_BLOCK(N))



/**
    @brief SOCKETn Status Register Address [RO][0x00]
    @details @ref _Sn_SR_ indicates the status of SOCKETn.\n
            The status of SOCKETn can be changed by @ref _Sn_CR_, some TCP packets such as SYN, FIN, RST packet, or @ref Sn_IR_TIMEOUT.
    - Normal status
      - @ref SOCK_CLOSED      : Closed
      - @ref SOCK_INIT        : Initiate state
      - @ref SOCK_LISTEN      : Listen state
      - @ref SOCK_ESTABLISHED : Success to connect
      - @ref SOCK_CLOSE_WAIT  : Closing state
      - @ref SOCK_UDP         : UDP socket
      - @ref SOCK_IPRAW       : IPRAW socket
      - @ref SOCK_IPRAW6      : IPv6 IPRAW socket
      - @ref SOCK_MACRAW      : MAC raw mode socket
    - Temporary status during changing the status of SOCKETn .
      - @ref SOCK_SYNSENT     : This indicates SOCKETn  sent the connect-request packet (SYN packet) to a peer.
      - @ref SOCK_SYNRECV     : It indicates SOCKETn  successfully received the connect-request packet (SYN packet) from a peer.
      - @ref SOCK_FIN_WAIT    : Connection state
      - @ref SOCK_TIME_WAIT   : Closing state
      - @ref SOCK_LAST_ACK    : Closing state

    @sa _Sn_CR_, _Sn_IR_, _Sn_IRCLR_, Sn_IR_TIMEOUT
    @sa getSn_SR(), getSn_CR(), setSn_CR(), getSn_IR(), setSn_IRCLR()

    <table width=0 >
       <tr> <td>@image html SocketStatus.png "<SOCKETn Status Transition>"</td> </tr>
    </table>

*/
#define _Sn_SR_(N)           (_W6300_IO_BASE_ + (0x0030 << 8) + WIZCHIP_SREG_BLOCK(N))

/**
    @brief SOCKETn Extension Status Register Address [RO][0x00]
    @details @ref _Sn_ESR_ indicates the connected client IP address information such as IP version, IPv6 address type(LLA or GUA), \n
            and TCP operation mode such as <b>TCP SERVER</b> and <b>TCP CLIENT</b>
    <table>
      <tr> <td> 7 ~ 3   </td> <td>2   </td> <td>1    </td> <td>0   </td> </tr>
      <tr> <td> Reserved</td> <td>TCPM</td> <td>TCPOP</td> <td>IP6T</td> </tr>
    </table>
    - @ref Sn_ESR_TCPM  : @ref Sn_ESR_TCPM_IPV4, @ref Sn_ESR_TCPM_IPV6
    - @ref Sn_ESR_TCPOP : @ref Sn_ESR_TCPOP_SVR, @ref Sn_ESR_TCPOP_CLT
    - @ref Sn_ESR_IP6T  : @ref Sn_ESR_IP6T_LLA,  @ref Sn_ESR_IP6T_GUA

    @note It is valid only on TCP mode such as @ref Sn_MR_TCP4, @ref Sn_MR_TCP6, and @ref Sn_MR_TCPD.
    @sa _Sn_MR_, _Sn_PSR_
    @sa getSn_ESR(), getSn_MR(), setSn_MR(), getSn_PSR(), setSn_PSR()
*/
#define _Sn_ESR_(N)          (_W6300_IO_BASE_ + (0x0031 << 8) + WIZCHIP_SREG_BLOCK(N))

/**
    @brief SOCKETn IP Protocol Number(PN) Register Address [R/W][0x0000]
    @details \ref _Sn_PNR_ that sets the protocol number/next header field of the IPv4/IPv6 header at the IP layer.
    @note It is valid only in IPRAW mode such as @ref Sn_MR_IPRAW4 and @ref Sn_MR_IPRAW6.
    @note It is set before @ref Sn_CR_OPEN is performed.
    @sa _Sn_NHR_, _Sn_MR_, Sn_CR_OPEN
    @sa getSn_PNR(), setSn_PNR(), getSn_NHR(), setSn_NHR(), getSn_MR(), setSn_MR(), getSn_CR(), setSn_CR()
*/
#define _Sn_PNR_(N)          (_W6300_IO_BASE_ + (0x0100 << 8) + WIZCHIP_SREG_BLOCK(N))
#define _Sn_NHR_(N)          (_Sn_PNR_(N))      ///< Refer to @ref _Sn_PNR_.

/**
    @brief SOCKETn IPv4 Type of Service(TOS) Register Address [R=W][0x00]
    @details @ref _Sn_TOSR_ sets the TOS(Type Of Service) field in IPv4 Header.
    @sa getSn_TOSR(), setSn_TOSR()
*/
#define _Sn_TOSR_(N)         (_W6300_IO_BASE_ + (0x0104 << 8) + WIZCHIP_SREG_BLOCK(N))

/**
    @brief SOCKETn IP Time to live(TTL) Register Address [R=W][0x80]
    @details @ref _Sn_TTLR_ sets the TTL(Time To Live)/HOP(Hop Limit) field in IPv4/IPv6 header at the IP layer.
    @sa _Sn_HOPR_
    @sa getSn_TTLR(), setSn_TTLR(), getSn_HOPR(), setSn_HOPR()
*/
#define _Sn_TTLR_(N)         (_W6300_IO_BASE_ + (0x0108 << 8) + WIZCHIP_SREG_BLOCK(N))
#define _Sn_HOPR_(N)         (_Sn_TTLR_(N))   ///< Refer to @ref _Sn_TTLR_.

/**
    @brief SOCKETn Fragment Register Address [R=W][0x4000]
    @details @ref _Sn_FRGR_ sets the fragment flag & offset in IPv4 header.
    @note @ref _WIZCHIP_ can not support IP fragment & re-assembly.\n So It is not recommended to set @ref _Sn_FRGR_ to any other value.
    @sa getSn_FRGR(), setSn_FRGR()
*/
#define _Sn_FRGR_(N)         (_W6300_IO_BASE_ + (0x010C << 8) + WIZCHIP_SREG_BLOCK(N))

/**
    @brief SOCKETn Maximum Segment Size(MSS) Register Address [RW][0x0000]
    @details @ref _Sn_MSSR_ sets or gets the MTU(Maximum Transfer Unit) of SOCKETn. \n
            The MTU of each protocol is as following.
    <table>
      <tr> <td> @ref _Sn_MR_[3:0] </td> <td>@ref NETMR2_PPPoE = 0 </td> <td>@ref NETMR2_PPPoE = '1'</td> </tr>
      <tr> <td> @ref Sn_MR_TCP4   </td> <td> 1 ~ 1460             </td> <td> 1 ~ 1452              </td> </tr>
      <tr> <td> @ref Sn_MR_TCP6   </td> <td> 1 ~ 1440             </td> <td> 1 ~ 1432              </td> </tr>
      <tr> <td> @ref Sn_MR_UDP4   </td> <td> 1 ~ 1472             </td> <td> 1 ~ 1464              </td> </tr>
      <tr> <td> @ref Sn_MR_UDP6   </td> <td> 1 ~ 1452             </td> <td> 1 ~ 1444              </td> </tr>
      <tr> <td> @ref Sn_MR_IPRAW4 </td> <td> 1 ~ 1480             </td> <td> 1 ~ 1472              </td> </tr>
      <tr> <td> @ref Sn_MR_IPRAW6 </td> <td> 1 ~ 1460             </td> <td> 1 ~ 1452              </td> </tr>
      <tr> <td> @ref Sn_MR_MACRAW </td> <td colspan = "2">      1 ~ 1514                                              </td> </tr>
    </table>

    @note It is not set exceeding the MTU for each protocol of SOCKETn even if _Sn_MSSR_ is set over the MTU.
    @sa _Sn_MR_, NETMR2_PPPoE
    @sa getSn_MSSR(), setSn_MSSR(), getSn_MR(), setSn_MR(), getNETMR2(), setNETMR2()
*/
#define _Sn_MSSR_(N)         (_W6300_IO_BASE_ + (0x0110 << 8) + WIZCHIP_SREG_BLOCK(N))

/**
    @brief SOCKETn Source Port Register Address [R=W][0x0000]
    @details @ref _Sn_PORTR_ sets the source port number of SOCKETn .
    @note It is valid in TCP(@ref Sn_MR_TCP4, @ref Sn_MR_TCP6, @ref Sn_MR_TCPD) and UDP(@ref Sn_MR_UDP4, @ref Sn_MR_UDP6, @ref Sn_MR_UDPD) mode.
    @note It should be set before @ref Sn_CR_OPEN is performed.
    @sa _Sn_MR_, Sn_CR_OPEN
    @sa getSn_PORTR(), getSn_PORTR(), getSn_MR(), setSn_MR(), getSn_CR(), setSn_CR()
*/
#define _Sn_PORTR_(N)        (_W6300_IO_BASE_ + (0x0114 << 8) + WIZCHIP_SREG_BLOCK(N))

/**
    @brief SOCKETn Destination Hardware Address Register Address [RW][00:00:00:00:00:00]
    @details @ref _Sn_DHAR_ sets or gets the destination hardware address of SOCKETn.\n
     - When @ref Sn_MR2_DHAM = 1 and @ref _Sn_MR_[3:0] != @ref Sn_MR_MACRAW
       The destination hardware address is set by @ref _Sn_DHAR_ without ARP processed by @ref Sn_CR_CONNECT, @ref Sn_CR_CONNECT6, @ref Sn_CR_SEND, and @ref Sn_CR_SEND6.\n
       Also, when SOCKETn is opened with @ref Sn_MR_UDP4 or @ref Sn_MR_UDP6 and @ref Sn_MR_MULTI is set, @ref _Sn_DHAR_ sets the Multicast Group Hardware address.
     - Others
       In TCP mode such as @ref Sn_MR_TCP4, @ref Sn_MR_TCP6, and @ref Sn_MR_TCPD, \n
       @ref _Sn_DHAR_ gets the destination hardware address when @ref _Sn_SR_ is @ref SOCK_ESTABLISHED.
    @sa _Sn_MR_, _Sn_MR2_, _Sn_CR_, _Sn_SR_
    @sa getSn_DHAR(), setSn_DHAR(), getSn_MR(), setSn_MR(), getSn_MR2(), setSn_MR2(), getSn_CR(), setSn_CR(), getSn_SR()
*/
#define _Sn_DHAR_(N)         (_W6300_IO_BASE_ + (0x0118 << 8) + WIZCHIP_SREG_BLOCK(N))

/**
    @brief SOCKETn Destination IPv4 Address Register Address [RW][0.0.0.0]
    @details @ref _Sn_DIPR_(= @ref _Sn_DIP4R_) sets or gets the destination IPv4 address of SOCKETn. \n
     - In TCP mode such as @ref Sn_MR_TCP4, and @ref Sn_MR_TCPD
        - <b>TCP CLIENT</b> mode : It sets the IPv4 address of <b>TCP SERVER</b> before @ref Sn_CR_CONNECT is performed.
        - <b>TCP SERVER</b> mode : It gets the IPv4 address of <b>TCP CLIENT</b> when @ref _Sn_SR_ is @ref SOCK_ESTABLISHED.
     - In UDP(@ref Sn_MR_UDP4, @ref Sn_MR_UDPD) mode & IPRAW4(@ref Sn_MR_IPRAW4) mode
       It sets the destination IPv4 address before @ref Sn_CR_SEND is performed. \n
       When Sn_MR_MULTI = 1, It sets the multicast group IPv4 address.
    @sa _Sn_DIP4R_, _Sn_MR_, _Sn_CR_, _Sn_SR_
    @sa getSn_DIPR(), getSn_DIPR(), getSn_DIP4R(), getSn_DIP4R(), getSn_MR(), setSn_MR(), getSn_CR(), setSn_CR(), getSn_SR()
*/
#define _Sn_DIPR_(N)         (_W6300_IO_BASE_ + (0x0120 << 8) + WIZCHIP_SREG_BLOCK(N))
#define _Sn_DIP4R_(N)        (_Sn_DIPR_(N))            ///< Refer to @ref _Sn_DIPR_.

/**
    @brief SOCKETn Destination IPv6 Address Register Address [RW][::]
    @details @ref _Sn_DIP6R_ sets or gets the destination IPv6 address of SOCKETn.
     - In TCP mode such as @ref Sn_MR_TCP6, and @ref Sn_MR_TCPD
        - <b>TCP CLIENT</b> mode : It sets the IPv6 address of <b>TCP SERVER</b> before @ref Sn_CR_CONNECT6 is performed.
        - <b>TCP SERVER</b> mode : It gets the IPv6 address of <b>TCP CLIENT</b> when @ref _Sn_SR_ is @ref SOCK_ESTABLISHED.
     - In UDP(@ref Sn_MR_UDP6, @ref Sn_MR_UDPD) mode & IPRAW4(@ref Sn_MR_IPRAW6) mode
       It sets the destination IPv6 address before @ref Sn_CR_SEND6 is performed.\n
       When Sn_MR_MULTI = 1, It sets the multicast group IPv6 address.
    @sa _Sn_MR_, _Sn_CR_, _Sn_SR_
    @sa getSn_DIP6R(), setSn_DIP6R(), getSn_MR(), setSn_MR(), getSn_CR(), setSn_CR(), getSn_SR()
*/
#define _Sn_DIP6R_(N)        (_W6300_IO_BASE_ + (0x0130 << 8) + WIZCHIP_SREG_BLOCK(N))

/**
    @brief SOCKETn Destination Port Register Address [RW][0x0000]
    @details @ref _Sn_DPORTR_ sets or gets the destination port number of SOCKETn.
     - In TCP mode such as @ref Sn_MR_TCP4, @ref Sn_MR_TCP6, and @ref Sn_MR_TCPD
        - <b>TCP CLIENT</b> mode : It sets the port number of <b>TCP SERVER</b> before @ref Sn_CR_CONNECT is performed.
        - <b>TCP SERVER</b> mode : It gets the port number of <b>TCP CLIENT</b> when @ref _Sn_SR_ is @ref SOCK_ESTABLISHED.
     - In UDP mode such as @ref Sn_MR_UDP4, @ref Sn_MR_UDP6, and @ref Sn_MR_UDPD
       It sets the destination port number before @ref Sn_CR_SEND is performed. \n
       When Sn_MR_MULTI = 1, It sets the multicast group group port number.

    @note It is valid SOCKETn is opened with @ref Sn_MR_TCP4,  @ref Sn_MR_TCP6, @ref Sn_MR_TCPD, @ref Sn_MR_UDP4, @ref Sn_MR_UDP4, and @ref Sn_MR_UDPD.
    @note It should be set before OPEN command is ordered.
    @sa _Sn_MR_, _Sn_CR_, _Sn_SR_
    @sa getSn_DPORTR(), getSn_DPORTR(), getSn_MR(), setSn_MR(), getSn_CR(), setSn_CR(), getSn_SR()
*/
#define _Sn_DPORTR_(N)       (_W6300_IO_BASE_ + (0x0140 << 8) + WIZCHIP_SREG_BLOCK(N))

/**
    @brief SOCKETn Mode Register 2 Address [R=W][0x00]
    @details @ref _Sn_MR2_ sets the option of SOCKETn with @ref _Sn_MR_, before @ref Sn_CR_OPEN is performed.\n
            Each bit of @ref _Sn_MR2_ is defined as the following.
    <table>
      <tr> <td>7 ~ 2   </td> <td>1   </td> <td>0   </td> </tr>
      <tr> <td>Reserved</td> <td>DHAM</td> <td>FARP</td> </tr>
    </table>
     - @ref Sn_MR2_DHAM : @ref Sn_MR2_DHAM_AUTO, @ref Sn_MR2_DHAM_MANUAL
     - @ref Sn_MR2_FARP
    @sa _Sn_MR_, _Sn_CR_
    @sa getSn_MR2(), setSn_MR2(), getSn_MR(), getSn_MR(), getSn_CR()
*/
#define _Sn_MR2_(N)          (_W6300_IO_BASE_ + (0x0144 << 8) + WIZCHIP_SREG_BLOCK(N))


/**
    @brief SOCKETn Retransmission Time Register Address [R=W][0x0000]
    @details @ref _Sn_RTR_ sets the timeout value of packet to be retransmitted by @ref _SLCR_.\n
    @note It should be set before @ref Sn_CR_OPEN is performed.\n
         It is initialized as @ref _RTR_ if you do not set it to none-zero value.
    @sa _RTR_, _Sn_CR_
    @sa getSn_RTR(), setSn_RTR(), getSn_CR(), setSn_CR()
*/
#define _Sn_RTR_(N)          (_W6300_IO_BASE_ + (0x0180 << 8) + WIZCHIP_SREG_BLOCK(N))

/**
    @brief SOCKETn Retransmission Count Register Address [R=W][0x00]
    @details @ref _Sn_RCR_ sets the retry count value of packet to be retransmitted by @ref _SLCR_.\n
    @note It should be set before @ref Sn_CR_OPEN is performed.\n
         It is initialized as @ref _RTR_ if you do not set it to any none-zero value.
    @sa _RTR_, _Sn_CR_
    @sa getSn_RTR(), setSn_RTR(), getSn_CR(), setSn_CR()
*/
#define _Sn_RCR_(N)          (_W6300_IO_BASE_ + (0x0184 << 8) + WIZCHIP_SREG_BLOCK(N))

/**
    @brief SOCKETn Keep Alive Time Register Address [R=W][0x00]
    @details @ref _Sn_KPALVTR_ sets the auto-retransmission time of KA(Keep Alive) packet. \n
            If the destination can not respond to the KA packet during the time set by @ref _Sn_KPALVTR_,\n
            the connection is terminated, @ref Sn_IR_TIMEOUT is set and then @ref _Sn_SR_ is changed @ref SOCK_CLOSED.\n
            Before the time is expierd, if the destination sends a KA/ACK packet or any packet, the connection is still valid,\n
            @ref _Sn_SR_ remained at @ref SOCK_ESTABLISHED.
    @note It is valid only after sending data over 1 byte in TCP mode such as @ref Sn_MR_TCP4, @ref Sn_MR_TCP6, and @ref Sn_MR_TCPD.
    @note If it is set to 0, KA packet can be sent by @ref Sn_CR_SEND_KEEP.
    @sa Sn_CR_SEND_KEEP, Sn_IR_TIMEOUT, Sn_IRCLR, Sn_SR, Sn_MR
    @sa getSn_KPALVTR(), setSn_KPALVTR(), getSn_IR(), setSn_IRCLR(), getSn_SR(), getSn_MR(), setSn_MR()
*/
#define _Sn_KPALVTR_(N)      (_W6300_IO_BASE_ + (0x0188 << 8) + WIZCHIP_SREG_BLOCK(N))

/**
    @brief SOCKETn TX Buffer Size Register Address [R=W][0x02]
    @details @ref _Sn_TX_BSR_ sets the TX buffer size of SOCKETn in the 16KB TX memory.\n
            It can be set only with 0,1,2,4,8, and 16K bytes.
    @note The 16KB TX memory is allocated as many as @ref _Sn_TX_BSR_ sequentially from SOCKET0 to SOCKETn(Here, 0 <= n <= @ref _WIZCHIP_SOCK_NUM_ - 1).\n
         The total sum of Sn_TX_BSR can not be exceed 16KB of TX memory. \n
         If the total size is exceeded, SOCKETn can't be normally sent data to a destination.
    @sa _Sn_RX_BSR_
    @sa getSn_TX_BSR(), setSn_TX_BSR(), getSn_TXBUF_SIZE(), setSn_TXBUF_SIZE(), getSn_TxMAX(), setSn_TX_BSR(), getSn_RX_BSR(), setSn_RX_BSR()
*/
#define _Sn_TX_BSR_(N)       (_W6300_IO_BASE_ + (0x0200 << 8) + WIZCHIP_SREG_BLOCK(N))

/**
    @brief SOCKETn TX Free Buffer Size Register Address [RO][0x0800]
    @details @ref _Sn_TX_FSR_ gets the transmittable free size of SOCKETn TX buffer.
    @note Data should not be saved bigger than it because the data overwrites the previous saved data not to be sent yet.\n
         Therefore, Check it before saving the data to the SOCKETn TX buffer. \n
         If the data size is equal or smaller than it, transmit the data with @ref Sn_CR_SEND / @ref Sn_CR_SEND6 after saving the data in SOCKETn TX buffer.\n
         If the data size is greater than it, transmit the data after dividing into it and saving in the SOCKETn TX buffer.
    @note \n
     - In TCP mode such as @ref Sn_MR_TCP4, @ref Sn_MR_TCP6 and @ref Sn_MR_TCPD, \n
       It is automatically increased by the absolute difference between @ref _Sn_TX_WR_ and interanl TX ACK pointer.
     - In other mode \n
       It is automatically increased by the absolute difference between @ref _Sn_TX_WR_ and @ref _Sn_TX_RD_.
    @sa _Sn_RX_RSR_, _Sn_TX_WR_, _Sn_TX_RD_, _Sn_CR_
    @sa getSn_TX_FSR(), getSn_TX_WR(), getSn_TX_WR(), getSn_TX_RD(), getSn_CR(), setSn_CR()
*/
#define _Sn_TX_FSR_(N)       (_W6300_IO_BASE_ + (0x0204 << 8) + WIZCHIP_SREG_BLOCK(N))

/**
    @brief SOCKET TX Memory Read Pointer Register Address[R][0x0000]
    @details @ref _Sn_TX_RD_ gets the start pointer of data to be sent by @ref Sn_CR_SEND. \n
            @ref Sn_CR_SEND / @ref Sn_CR_SEND6 starts to transmit the saved data from @ref _Sn_TX_RD_ to @ref _Sn_TX_WR_ in the SOCKETn TX Buffer,\n
            and when @ref Sn_IR_SENDOK is set, It is automatically increased to equal @ref _Sn_TX_WR_.
    @note It is initialized by @ref Sn_CR_OPEN, But, In TCP mode such as @ref Sn_MR_TCP4, @ref Sn_MR_TCP6, and @ref Sn_MR_TCPD,\n
         it is re-initialized when the TCP connection is completed.
    @note If it exceeds the maximum value 0xFFFF, (that is, it is greater than 0x10000 and the carry bit occurs),\n
         then the carry bit is ignored and it automatically is updated with its the lower 16bits value.
    @sa _Sn_TX_WR_, _Sn_TX_FSR_, _Sn_CR_, _Sn_IR_, _Sn_IRCLR_, _Sn_MR_
    @sa getSn_TX_RD(), getSn_TX_WR(), setSn_TX_WR(), getSn_TX_FSR(), getSn_CR(), setSn_CR(), getSn_IR(), setSn_IRCLR(), getSn_MR(), setSn_MR()
*/
#define _Sn_TX_RD_(N)        (_W6300_IO_BASE_ + (0x0208 << 8) + WIZCHIP_SREG_BLOCK(N))


/**
    @brief SOCKETn TX Memory Write Pointer Register Address [RW][0x0000]
    @details @ref _Sn_TX_WR_ gets the start pointer of data to be saved in the SOCKETn TX buffer, \n
            or sets the end pointer of data to be sent by @ref Sn_CR_SEND. \n
            If you have completed to save the data to be sent in the SOCKETn TX buffer,
            increase it as many as the saved size of data before @ref Sn_CR_SEND is performed.\n
            @ref Sn_CR_SEND starts to transmit the saved data from @ref _Sn_TX_RD_ to @ref _Sn_TX_WR_ in the SOCKETn TX Buffer, \n
            and when @ref Sn_IR_SENDOK is set, @ref _Sn_TX_RD_ is automatically increased to equal it.
    @note It is initialized by @ref Sn_CR_OPEN.\n
         But, In TCP mode such as @ref Sn_MR_TCP4, @ref Sn_MR_TCP6, and @ref Sn_MR_TCPD,\n
         it is re-initialized when the TCP connection is completed.
    @note The size of data to be saved can't exceed @ref _Sn_TX_FSR_.
    @note If it exceeds the maximum value 0xFFFF(that is, it is greater than 0x10000 and the carry bit occurs),\n
         then ignore the carry bit and update it with its lower 16bits value.
    @sa _Sn_TX_RD_, _Sn_TX_FSR_, _Sn_CR_, _Sn_IR_, _Sn_IRCLR_, _Sn_MR_
    @sa getSn_TX_WR(), setSn_TX_WR(), getSn_TX_RD(), getSn_TX_FSR(), getSn_CR(), setSn_CR(), getSn_IR(), setSn_IRCLR(), getSn_MR(), setSn_MR()
*/
#define _Sn_TX_WR_(N)        (_W6300_IO_BASE_ + (0x020C << 8) + WIZCHIP_SREG_BLOCK(N))

/**
    @brief SOCKETn RX Buffer Size Register Address [R=W][0x02]
    @details @ref _Sn_RX_BSR_ sets the RX buffer size of SOCKETn in the 16KB RX memory.\n
            It can be set only with 0,1,2,4,8, and 16K bytes.
    @note The 16KB RX memory is allocated as many as @ref _Sn_RX_BSR_ sequentially from SOCKET0 to SOCKETn(Here, 0 <= n <= @ref _WIZCHIP_SOCK_NUM_ - 1).\n
         The total sum of @ref _Sn_RX_BSR_ can not be exceed 16KB of RX memory. \n
         If the total size is exceeded, SOCKETn can't be normally received data from a destination.
    @sa _Sn_RX_BSR_
    @sa getSn_TX_BSR(), setSn_TX_BSR(), getSn_RXBUF_SIZE(), setSn_RXBUF_SIZE(), getSn_RxMAX(), getSn_RX_BSR(), setSn_RX_BSR()
*/
#define _Sn_RX_BSR_(N)       (_W6300_IO_BASE_ + (0x0220 << 8) + WIZCHIP_SREG_BLOCK(N))

/**
    @brief SOCKETn RX Received Size Register Address [RO][0x0000]
    @details @ref _Sn_RX_RSR_ gets the received data size of SOCKETn RX buffer.
    @note The real received data size maybe smaller than it, \n
         because it maybe included the size of 'PACKET NFO' such like as \n
         the destination IP address, destination port number and data size of the received DATA PACKET.
    @note Do not read bigger data than @ref _Sn_RX_RSR_.
    @note It is automatically increased by the absolute difference between @ref _Sn_RX_WR_ and @ref _Sn_RX_RD_ \n
         after @ref Sn_CR_RECV is performed.
    @sa _Sn_RX_RSR_, _Sn_TX_WR_, _Sn_TX_RD_, _Sn_CR_, _Sn_TX_FSR_
    @sa getSn_RX_RSR(), getSn_TX_WR(), getSn_TX_WR(), getSn_CR(), setSn_CR(), getSn_TX_FSR()
*/
#define _Sn_RX_RSR_(N)       (_W6300_IO_BASE_ + (0x0224 << 8) + WIZCHIP_SREG_BLOCK(N))

/**
    @brief SOCKET RX Memory Read Pointer Register Address[R][0x0000]
    @details @ref _Sn_RX_RD_ gets the start pointer of the received data in the SOCKETn RX buffer,\n
            or sets the end data pointer of the read completed data by @ref Sn_CR_RECV. \n
            You can read the received data from it to @ref _Sn_RX_WR_ in the SOCKET RX buffer.\n
            After completing to read data, you should increase it as many as the read size before @ref Sn_CR_RECV is performed.
    @note It is initialized by @ref Sn_CR_OPEN, But, In TCP mode such as @ref Sn_MR_TCP4, @ref Sn_MR_TCP6, and @ref Sn_MR_TCPD,\n
         it is re-initialized when the TCP connection is completed.
    @note If it exceeds the maximum value 0xFFFF, (that is, it is greater than 0x10000 and the carry bit occurs),\n
         Ignore the carry bit and update with its the lower 16bits value.
    @sa _Sn_RX_WR_, _Sn_RX_RSR_, _Sn_CR_, _Sn_IR_, _Sn_IRCLR_, _Sn_MR_
    @sa getSn_RX_WR(), setSn_RX_RD(), getSn_RX_WR(), getSn_TX_FSR(), getSn_CR(), setSn_CR(), getSn_IR(), setSn_IRCLR(), getSn_MR(), setSn_MR()
*/
#define _Sn_RX_RD_(N)        (_W6300_IO_BASE_ + (0x0228 << 8) + WIZCHIP_SREG_BLOCK(N))

/**
    @brief SOCKETn TX Memory Write Pointer Register Address [RW][0x0000]
    @details @ref _Sn_TX_WR_ gets the end pointer of the data that has been completely received in the SOCKETn RX buffer. \n
            Whenever a data has been completely received from a destination, \n
            It is automatically increased as many as the sum size of the received data and the 'PACKET INFO'. \n
            You can read the recevied data from @ref _Sn_RX_RD_ to it in the SOCKET RX buffer.
    @note It is initialized by @ref Sn_CR_OPEN. But, In TCP mode such as @ref Sn_MR_TCP4, @ref Sn_MR_TCP6, and @ref Sn_MR_TCPD,\n
         it is re-initialized when the TCP connection is completed.
    @note If it exceeds the maximum value 0xFFFF(that is, it is greater than 0x10000 and the carry bit occurs),\n
         then ignore the carry bit and update it with its lower 16bits value.
    @sa _Sn_TX_RD_, _Sn_TX_FSR_, _Sn_CR_, _Sn_IR_, Sn_IRCLR_, _Sn_MR_
    @sa getSn_TX_WR(), setSn_TX_WR(), getSn_TX_RD(), getSn_TX_FSR(), getSn_CR(), setSn_CR(), getSn_IR(), setSn_IRCLR(), getSn_MR(), setSn_MR()
*/
#define _Sn_RX_WR_(N)        (_W6300_IO_BASE_ + (0x022C << 8) + WIZCHIP_SREG_BLOCK(N))

/**
    @}
*/

/*----------------------------- W6300 Register values  -----------------------------*/

/* System Status Register Bit Definition */
/**
    @brief CHIP Lock staus bit of @ref _SYSR_.
    @details @ref SYSR_CHPL indicates the lock status of @ref _SYCR0_ and @ref _SYCR1_.\n
            1 : Lock \n
            0 : unlock
    @note It is set by only @ref _CHPLCKR_.
    @sa _SYSR_, _CHPLCKR_, _SYCR0_, _SYCR1_
    @sa getSYSR(), getCHPLCKR(), setCHPLCKR(), CHIPLOCK(), CHIPUNLOCK(), setSYCR0(), setSYCR1()
*/
#define SYSR_CHPL            (1 << 7)

/**
    @brief NET Lock status bit of @ref _SYSR_.
    @details @ref SYSR_NETL indicates the lock of network information registers such as
            @ref _SHAR_, @ref _GAR_, @ref _SUBR_, @ref _SIPR_, @ref _LLAR_, @ref _GUAR_, and @ref _SUB6R_. \n
            1 : Lock \n
            0 : unlock
    @note It is set by only @ref _NETLCKR_.
    @note @ref _GA6R_ can be accessed regardless of @ref SYSR_NETL.
    @sa _SYSR_, _NETLCKR_, _SHAR_, _GAR_, _SUBR_, _SIPR_, _LLAR_, _GUAR_, _SUB6R_
    @sa getSYSR(), getNETLCKR(), setNETLCKR(), NETLOCK(), NETUNLOCK(),\n
       getSHAR(), setSHAR(), getGAR(), setGAR(), getSUBR(), getSIR(), setSIPR(), \n
       getLLAR(), setLLAR(), getGUAR(),setGUAR(), getSUB6R(), setSUB6R()
*/
#define SYSR_NETL            (1 << 6)

/**
    @brief PHY Lock status bit of @ref _SYSR_. Refer to @ref _PHYLCKR_.
    @details @ref SYSR_PHYL indicates the lock status of @ref _PHYCR0_ and _PHYCR1_.\n
            1 : Lock \n
            0 : unlock
    @note It is set by only @ref _PHYLCKR_.
    @sa _SYSR_, _PHYCLKR_, _PHYCR0_, _PHYCR1_
    @sa getSYSR(), getPHYLCKR(), setPHYLCKR(), setPHYCR0(), getPHYCR1(), setPHYCR1()
*/
#define SYSR_PHYL            (1 << 5)

/**
    @brief Parallel Bus Mode bit of @ref _SYSR_
    @details @ref SYSR_IND is set when @ref _WIZCHIP_ PIN MODE[3:0] == "010X".
            It indicates to use the parallel BUS mode.
    @sa _SYSR_, _WIZCHIP_IO_MODE_BUS_
    @sa getSYSR()
*/
#define SYSR_IND             (1 << 5)

/**
    @brief SPI I/F Mode bit of @ref _SYSR_.
    @details @ref SYSR_SPI is set when @ref _WIZCHIP_ PIN MODE[3:0] == "000X".
            It indicates to use the SPI mode.
    @sa _SYSR_, _WIZCHIP_IO_MODE_SPI_
    @sa getSYSR()
*/
#define SYSR_SPI             (1 << 0)


/* System Config Register Bit Definition */
/**
    @brief RST bit of @ref _SYCR0_
    @details @ref SYCR0_RST resets to @ref _WIZCHIP_ softly. \n
            0 : Soft reset \n
            1 : Normal operation
    @note It can be set only when @ref SYSR_CHPL = 1.
    @sa _SYSR0_, _CHPLCKR_, _SYSR_, SYSR_CHPL
    @sa setSYCR0(), setCHPLCKR(), getCHPLCKR(), CHIPLOCK(), CHIPUNLOCK(), getSYSR()
*/
#define SYCR0_RST            (0x00)

/**
    @brief IEN bit of @ref _SYCR1_.
    @details @ref SYCR1_IEN is globally enable or disable the interrupt of @ref _WIZCHIP_,\n
            regardless of the related interrupt mask registers such as @ref _IMR_, @ref _SIMR_, @ref _SLIMR_, and @ref _Sn_IMR_.\n
            1 : Enable  \n
            0 : Disable
    @sa _SYCR1_, _IR_, _SIR_, _SLIR_, _Sn_IR_, _IRCLR_,  _SLIRCLR_, _Sn_IRCLR_
    @sa getSYCR1(), setSYCR1(), getIR(), getSIR(), getSLIR(), getSn_IR(), setIRCLR(), setSLIRCLR(), setSn_IRCLR()
*/
#define SYCR1_IEN            (1 << 7)

/**
    @brief System Clock select mask bit of @ref _SYCR1_.
    @details @ref SYCR1_CLKSEL selects a system clock to 100MHz or 25MHz. \n
            The masked bit values are as following.
              - @ref SYCR1_CLKSEL_25M
              - @ref SYCR1_CLKSEL_100M
    @note It can be set only when @ref SYSR_CHPL = 1.
    @note The system clock is automatically changed to 25MHz while the reset of @ref _WIZCHIP_ H/W reset, the Ethernet PHY H/W reset and power down. \n
         On the other hand, the system clock is set by @ref SYCR1_CLKSEL during normal operating.
    @sa _SYCR1_, _SYSR_, _CHPLCKR_, SYSL_CHPL, PHYCR1_RST, PHYCR1_PWDN
    @sa getSYCR1(), setSYCR1(), getSYSR(), getCHPLCKR(), setCHIPLCKR(), CHIPLOCK(), CHIPUNLOCK(), getPHYCR1(), setPHYCR1()
*/
#define SYCR1_CLKSEL         (1 << 0)

/**
    @brief System Clock - 25MHz
    @details @ref SYCR1_CLKSEL_25M selects a system clock to 25MHz.
    @note It can be set only when @ref SYSR_CHPL = 1.
    @sa _SYCR1_, SYCR1_CLKSEL, SYCR1_CLKSEL_100M
    @sa getSYCR1(), setSYCR1(), getSYSR(), getCHPLCKR(), setCHIPLCKR(), CHIPLOCK(), CHIPUNLOCK()
*/
#define SYCR1_CLKSEL_25M     1

/**
    @brief System Clock - 100MHz
    @details @ref SYCR1_CLKSEL_100M selects a system clock to 100MHz.
    @note It can be set only when @ref SYSR_CHPL = 1.
    @sa _SYCR1_, SYCR1_CLKSEL, SYCR1_CLKSEL_25M
    @sa getSYCR1(), setSYCR1(), getSYSR(), getCHPLCKR(), setCHIPLCKR(), CHIPLOCK(), CHIPUNLOCK()
*/
#define SYCR1_CLKSEL_100M    0


/* Interrupt Register Bit Definition */
/**
    @brief WOL bit of @ref _IR_
    @details @ref IR_WOL is set when @ref _WIZCHIP_ receives a magic packet of WOL.
    @sa _IR_, _IRCLR_, _IMR_
    @sa getIR(), setIRCLR(), getIMR(), setIMR()
*/
#define IR_WOL               (1<<7)

/**
    @brief UNR6 bit of @ref _IR_
    @details @ref IR_UNR6 is set when @ref _WIZCHIP_ receives the unreachable message of ICMPv6.
    @sa _IR_, _IRCLR_, _IMR_
    @sa getIR(), setIRCLR(), getIMR(), setIMR()
*/
#define IR_UNR6              (1<<4)

/**
    @brief IPCONF bit of @ref _IR_
    @details @ref IR_IPCONF is set when @ref _WIZCHIP_ receives a ARP reply with the same IPv4 address as @ref _SIPR_.
    @sa _IR_, _IRCLR_, _IMR_
    @sa getIR(), setIRCLR(), getIMR(), setIMR()
*/
#define IR_IPCONF            (1<<2)

/**
    @brief UNR4 bit of @ref _IR_
    @details @ref IR_UNR4 is set when @ref _WIZCHIP_ receives the unreachable message of ICMPv4.
    @sa _IR_, _IRCLR_, _IMR_
    @sa getIR(), setIRCLR(), getIMR(), setIMR()
*/
#define IR_UNR4              (1<<1)

/**
    @brief PTERM bit of @ref _IR_
    @details @ref IR_PTERM is set when @ref _WIZCHIP_ receives the PPP termination packet
    @sa _IR_, _IRCLR_, _IMR_
    @sa getIR(), setIRCLR(), getIMR(), setIMR()
*/
#define IR_PTERM             (1<<0)


/* SOCKET Interrupt Register Bit Definition */
/**
    @brief N-th INT bit of @@ref _SIR_
    @details @ref SIR_INT(N) is set when @ref _Sn_IR_(N) is not equal to zero.
    @sa _SIR_, _Sn_IRCLR_, _SIMR_
    @sa getSIR(), setSn_IRCLR(), getSIMR()
*/
#define SIR_INT(N)           (1<<N)


/* SOCKET-less Interrupt Register Bit Definition */
/**
    @brief TOUT bit of @ref _SLIR_
    @details @ref SLIR_TOUT is set when the timeout is occurred after @ref _SLCR_ is performed.
    @sa _SLIR_, _SLIRCLR_, _SLCR_
    @sa getSLIR(), setSLIRCLR(), getSLCR(), setSLCR()
*/
#define SLIR_TOUT            (1<<7)

/**
    @brief ARP4 bit of @ref _SLIR_
    @details @ref SLIR_ARP4 is set when the ARP-relay is received successfully after @ref SLCR_ARP4 is performed \n
            and the destination hardware address can be checked by @ref _SLDHAR_. \n
            Otherwise, @ref SLIR_TOUT is set.
    @sa _SLIR_, _SLIRCLR_, _SLCR_, _SLDIPR_, _SLDIP4R_, _SLDHAR_, SLIR_TOUT
    @sa getSLIR(), setSLIRCLR(), getSLCR(), setSLCR(), getSLDIPR(), setSLDIPR(), getSLDIP4R(), setSLDIP4R(), getSLDHAR()
*/
#define SLIR_ARP4            (1<<6)

/**
    @brief PING4 bit of @ref _SLIR_
    @details @ref SLIR_PING4 is set when the PING-relay is received successfully after @ref SLCR_PING4 is performed \n
            and the destination hardware address can be checked by @ref _SLDHAR_ like as @ref SLIR_ARP4.\n
            Otherwise, @ref SLIR_TOUT is set.
    @sa _SLIR_, _SLIRCLR_, _SLCR_, _SLDIPR_, _SLDIP4R_, _SLDHAR_, SLIR_TOUT
    @sa getSLIR(), setSLIRCLR(), getSLCR(), setSLCR(), getSLDIPR(), setSLDIPR(), getSLDIP4R(), setSLDIP4R(), getSLDHAR()
*/
#define SLIR_PING4           (1<<5)

/**
    @brief ARP6 bit of @ref _SLIR_
    @details @ref SLIR_ARP6 is set when the ARP6-relay is received successfully after @ref SLCR_ARP6 is performed \n
            and the destination hardware address can be checked by @ref _SLDHAR_. \n
            Otherwise, @ref SLIR_TOUT is set.
    @sa _SLIR_, _SLIRCLR_, _SLCR_, _SLDIP6R_, _SLDHAR_, SLIR_TOUT
    @sa getSLIR(), setSLIRCLR(), getSLCR(), setSLCR(), getSLDIP6R(), setSLDIP6R(), getSLDHAR()
*/
#define SLIR_ARP6            (1<<4)

/**
    @brief PING6 bit of @ref _SLIR_
    @details @ref SLIR_PING6 is set when the PING-relay is received successfully after @ref SLCR_PING6 is performed \n
            and the destination hardware address can be checked by @ref _SLDHAR_ like as @ref SLIR_ARP6. \n
            Otherwise, @ref SLIR_TOUT is set.
    @sa _SLIR_, _SLIRCLR_, _SLCR_, _SLDIP6R_, _SLDHAR_, SLIR_TOUT
    @sa getSLIR(), setSLIRCLR(), getSLCR(), setSLCR(), getSLDIP6R(), setSLDIP6R(), getSLDHAR()
*/
#define SLIR_PING6           (1<<3)

/**
    @brief NS bit of @ref _SLIR_
    @details @ref SLIR_NS is set when the ICMPv6 NA is received after @ref SLCR_NS is performed. \n
            Its set means IPv6 address such like as @ref _LLAR_ or @ref _GUAR_ is conflict. \n
            If @ref SLIR_TOUT is set, You can use @ref _SLDIP6R_ to @ref _LLAR_ or @ref _GUAR_.
    @note It is used for IPv6 state-less address auto-configuration(SLAAC).
    @sa _SLIR_, _SLIRCLR_, _SLCR_, _SLDIP6R_, SLIR_TOUT, _LLAR_, _GUAR_
    @sa getSLIR(), setSLIRCLR(), getSLCR(), setSLCR(), getSLDIP6R(), setSLDIP6R(), getLLAR(), setLLAR(), getGAUR(), setGUAR()
*/
#define SLIR_NS              (1<<2)

/**
    @brief RS bit of @ref _SLIR_
    @details @ref SLIR_RS is set when the ICMPv6 RA is received successfully after @ref SLCR_RS is performed \n
            and the prefix length, the prefix flag, the valid life time, the preferred life time and the prefix address of RA option message \n
            can be checked by @ref _PLR_, @ref _PFR_, @ref _VLTR_, @ref _PLTR_ and @ref _PAR_, respectively.\n
            Otherwise, @ref SLIR_TOUT is set.
    @bug Only when the first received RA option is the source link-layer address(0x01) and the second is prefix information(0x03),\n
        and the prefix information is in the order of prefix length, prefix flag, valid lifetime, default lifetime and prefix address,\n
        @ref _PLR_, @ref _PFR_, @ref _VLTR_, @ref _PLTR_ and @ref _PAR_ is correctly set.\n
        Other case, these registers are not valid.\n\n
        To solve this errata,\n
        You should use a IPRAW6 mode SOCKETn opened with Sn_MR_IPRAW6 and set the @ref _Sn_PNR_ to ICMPv6 number 58.\n
        This SOCKETn can be received a RA message or other ICMPv6 message, and a ICMPv6 message can be selectively filtered out by @ref _ICMP6BLKR_.\n
        For more details, Refer to "IPv6 Auto-configuration" document.

    @sa _SLIR_, _SLIRCLR_, _SLCR_, SLIR_TOUT
    @sa getSLIR(), setSLIRCLR(), getSLCR(), setSLCR()
*/
#define SLIR_RS              (1<<1)

/**
    @brief ICMPv6 RA Received Interrupt
    @details @ref SLIR_RA is set when a RA is received from a router without @ref SLCR_RS.\n
            Like as @ref SLIR_RS, a RA option message can be checked by @ref _PLR_, @ref _PFR_, @ref _VLTR_, @ref _PLTR_ and @ref _PAR_.\n
    @bug Only when RA options are received in the order of prefix length, prefix flag, valid lifetime, default lifetime and prefix address,\n
        @ref _PLR_, @ref _PFR_, @ref _VLTR_, @ref _PLTR_ and @ref _PAR_ is correctly set. \n
        Other case, these registers are not valid.\n\n
        To solve this errata, You should use a IPRAW6 mode SOCKETn opened with @ref Sn_MR_IPRAW6 and set the @ref _Sn_PNR_ to ICMPv6 number 58.\n
        This SOCKETn can be received a RA message or other ICMPv6 message, and a ICMPv6 message can be selectively filtered out by @ref _ICMP6BLKR_.\n
        For more details, Refer to "IPv6 Auto-configuration" document.

    @sa _SLIR_, _SLIRCLR_
    @sa getSLIR(), setSLIRCLR(),
*/
#define SLIR_RA              (1<<0)


/* SOCKET-less & SOCKETn  Prefer Source IPv6 Address Register Bit Definition */
/**
    @brief Select the source IPv6 address of the packet sent by @ref _SLCR_ or @ref _Sn_CR_ to AUTO
    @details The source IPv6 address depends on IPv6 address type of @ref _SLDIP6R_ or @ref _Sn_DIP6R_.\n
            If @ref _Sn_DIP6R_ is a link-local, the source IPv6 address is selected to @ref _LLAR_.\n
            Otherwise, the source IPv6 address is selected to @ref _GUAR_.
    @sa _SLPSR_, _Sn_PSR_, _SLCR_, _Sn_CR_, _SLDI6PR_, _Sn_DIP6R_, _LLAR_, _GUAR_
    @sa getSLPSR(), setSLPSR(), getSn_PSR(), setSn_PSR(), getSLCR(), setSLCR(), getSn_CR(), setSn_CR(), \n
       getSLDIP6R(), setSLDIP6R(), getSn_DIP6R(), setSn_DIP6R(), getLLAR(), setLLAR(), getGUAR(), setGUAR()
*/
#define PSR_AUTO             (0x00)

/**
    @brief Select the source IP address of the packet sent by @ref _SLCR_ or @ref _Sn_CR_ to @ref _LLAR_
    @details Regardless of the destination IPv6 address type of @ref _SLDIP6R_ or @ref _Sn_DIP6R_, the source IP is selected to @ref _LLAR_.
    @sa _SLPSR_, _Sn_PSR_, _SLCR_, _Sn_CR_, _SLDIP6R_, _Sn_DIP6R_, _LLAR_, _GUAR_
    @sa getSLPSR(), setSLPSR(), getSn_PSR(), setSn_PSR(), getSLCR(), setSLCR(), getSn_CR(), setSn_CR(), \n
       getSLDIP6R(), setSLDIP6R(), getSn_DIP6R(), setSn_DIP6R(), getLLAR(), setLLAR(), getGUAR(), setGUAR()
*/
#define PSR_LLA              (0x02)

/**
    @brief Select the source IP address of the packet sent by @ref _SLCR_ or @ref _Sn_CR_ to @ref _GUAR_
    @details Regardless of the destination IPv6 address type of @ref _SLDIP6R_, or @ref _Sn_DIP6R_, the source IP is selected to @ref _GUAR_.
    @sa _SLPSR_, _Sn_PSR_, _SLCR_, _Sn_CR_, _SLDIP6R_, _Sn_DIP6R_, _LLAR_, _GUAR_
    @sa getSLPSR(), setSLPSR(), getSn_PSR(), setSn_PSR(), getSLCR(), setSLCR(), getSn_CR(), setSn_CR(), \n
       getSLDIP6R(), setSLDIP6R(), getSn_DIP6R(), setSn_DIP6R(), getLLAR(), setLLAR(), getGUAR(), setGUAR()
*/
#define PSR_GUA              (0x03)


/* SOCKET-less Command Register Bit Definition */
/**
    @brief IPv4 ARP Command
    @details It sends a IPv4 ARP request message to @ref _SLDIP4R_ without SOCKETn. \n
            The results can be ether @ref SLIR_TOUT or @ref SLIR_ARP4.\n
            If the result is @ref SLIR_ARP4, It is success to receive the reply from @ref _SLDIP4R_. \n
            You can check the destination hardware address thru @ref _SLDHAR_. \n
            @ref SLIR_TOUT is set when it is no reply from @ref _SLDIP4R_ \n
            while both the time set by @ref _SLRTR_ and the retry count set by @ref _SLRCR_ are expired.
    @sa _SLCR_, _SLDIPR_, _SLDIP4R_ _SLDHAR_, _SLIR_, _SLIRCLR_, _SLRTR_, _SLRCR_
    @sa getSLCR(), setSLCR(), getSLDIPR(), setSLDIPR(), getSLDIP4R(), setSLDIP4R(), getSLDHAR(), getSLIR(), setSLIRCR(), getSLRTR(), setSLRTR(), getSLRCR(), setSLRTR()
*/
#define SLCR_ARP4            (1<<6)

/**
    @brief IPv4 PING Command
    @details It sends a IPv4 PING request message to @ref _SLDIP4R_ without SOCKETn.\n
            The results can be ether @ref SLIR_TOUT or @ref SLIR_PING4.\n
            If the result is @ref SLIR_PING4, It is success to receive the reply from @ref _SLDIP4R_. \n
            Also such like as @ref SLCR_ARP4, You can check the destination hardware address thru @ref _SLDHAR_.\n
            @ref SLIR_TOUT is set when it is no reply from @ref _SLDIP4R_ \n
            while both the time set by @ref _SLRTR_ and the retry count set by @ref _SLRCR_ are expired.
    @sa _SLCR_, _SLDIPR_, _SLDIP4R_, _SLDHAR_, _SLIR_, _SLIRCLR_, _SLRTR_, _SLRCR_
    @sa getSLCR(), setSLCR(), getSLDIPR(), setSLDIPR(), getSLDIP4R(), setSLDIP4R(), getSLDHAR(), getSLIR(), setSLIRCR(), getSLRTR(), setSLRTR(), getSLRCR(), setSLRTR()
*/
#define SLCR_PING4           (1<<5)

/**
    @brief IPv6 ARP Command
    @details It sends a IPv6 ARP request message to @ref _SLDIP6R_ without SOCKETn. \n
            The results can be either @ref SLIR_TOUT or @ref SLIR_ARP6.
            If the result is @ref SLIR_ARP6, It is success to receive the reply from @ref _SLDIP6R_. \n
            You can check the destination hardware address thru @ref _SLDHAR_.\n
            @ref SLIR_TOUT is set when it is no reply from @ref _SLDIP6R_
            while both the time set by @ref _SLRTR_ and the retry count set by @ref _SLRCR_ are expired.
    @sa _SLCR_, _SLDIP6R_, _SLDHAR_, _SLIR_, _SLIRCLR_, _SLRTR_, _SLRCR_
    @sa getSLCR(), setSLCR(), getSLDIP6R(), setSLDIP6R(), getSLDHAR(), getSLIR(), setSLIRCR(), getSLRTR(), setSLRTR(), getSLRCR(), setSLRTR()
*/
#define SLCR_ARP6            (1<<4)

/**
    @brief IPv6 PING Command
    @details It sends a IPv6 PING request message to @ref _SLDIP6R_ without SOCKET. \n
            The results can be either @ref SLIR_TOUT or @ref SLIR_PING6.\n
            If the result is @ref SLIR_PING6, It is success to receive the reply from @ref _SLDIP6R_.\n
            Also such like as @ref SLCR_ARP6, You can check the destination hardware address thru @ref _SLDHAR_.\n
            @ref SLIR_TOUT is set when it is no reply from @ref _SLDIP6R_ \n
            while both the time set by @ref _SLRTR_ and the retry count set by @ref _SLRCR_ are expired.
    @sa _SLCR_, _SLDIP6R_, _SLDHAR_, _SLIR_, _SLIRCLR_, _SLRTR_, _SLRCR_
    @sa getSLCR(), setSLCR(), getSLDIP6R(), setSLDIP6R(), getSLDHAR(), getSLIR(), setSLIRCR(), getSLRTR(), setSLRTR(), getSLRCR(), setSLRTR()
*/
#define SLCR_PING6           (1<<3)

/**
    @brief IPv6 DAD(Duplicate Address Detection) NS Command
    @details It sends NS message for DAD to @ref _SLDIP6R_ that is set the address to be used as as @ref _LLAR_ or @ref _GUAR_, without SOCKET.\n
            The result can be ether @ref SLIR_TOUT and @ref SLIR_NS.\n
            If @ref SLIR_TOUT is set then you can use @ref _SLDIP6R_ to @ref _LLAR_ or @ref _GUAR_,\n
            else if @ref SLIR_NS is set then you can not use _SLDIP6R_ to @ref _LLAR_ or @ref _GUAR_.\n
            That means the IPv6 Address are Conflict.
    @sa _SLCR_, _SLDIP6R_, _SLIR_, _SLIRCLR_, _SLRTR_, _SLRCR_, _LLAR_, _GUAR_
    @sa getSLCR(), setSLCR(), getSLDIP6R(), setSLDIP6R(), getSLIR(), setSLIRCR(), getSLRTR(), setSLRTR(), getSLRCR(), setSLRTR(), \n
          getLLAR(), setLLAR(), getGUAR(), setGUAR()
*/
#define SLCR_NS              (1<<2)

/**
    @brief IPv6 Auto-configuration RS Command
    @details It sends RS message to All-router for IPv6 Auto-configuration without SOCKET.\n
           The result can be ether @ref SLIR_RS or @ref SLIR_TOUT. \n
           If the result is @ref SLIR_RS, You can some information of router such as a prefix length, a Prefix flag, a valid life time, \n
           a preferred life time, and a prefix address respectively thru @ref _PLR_, @ref _PFR_, @ref _VLTR_, @ref _PLTR_, and @ref _PAR_.\n
           @ref SLIR_TOUT is set when it is no reply from a IPv6 router \n
           while both the time set by @ref _SLRTR_ and the retry count set by @ref _SLRCR_ are expired.
    @sa _SLCR_, _SLIR_, _SLIRCLR_, _SLRTR_, _SLRCR_, _PLR_, _PFR_, _VLTR_, _PLTR_, _PAR_.
    @sa getSLCR(), setSLCR(), getSLIR(), setSLIRCR(), getSLRTR(), setSLRTR(), getSLRCR(), setSLRTR(), \n
          getPLR(), getPFR(), getVLTR(), getPLTR(), getPAR().
*/
#define SLCR_RS              (1<<1)

/**
    @brief IPv6 Unsolicited NA Command
    @details It sends the IPv6 unsolicited NA message for updating the network information such as @ref _LLAR_, @ref _GUAR_, and @ref _SHAR_.\n
            The result is none.\n
            When @ref _SLPSR_ = @ref PSR_GUA, It can send the GUA unsolicited NA message.\n
            When @ref _SLPSR_ = Others, It can send the LLA unsolicited NA message.
    @sa _SLCR_, _SLIR_, _SLIRCLR_, _SLPSR_
    @sa getSLCR(), setSLCR(), getSLIR(), setSLIRCR(), getSLPFR(), setSLPFR()
*/
#define SLCR_UNA             (1<<0)



/* PHY Status Register Bit Definition */
/**
    @brief CAB mask bit
    @details @ref PHYSR_CAB masks the CAB bit of @ref _PHYSR_.\n
            The masked bit values are as following. \n
    - @ref PHYSR_CAB_OFF
    - @ref PHYSR_CAB_ON
    @sa getPHYSR()
*/
#define PHYSR_CAB            (1<<7)

/* PHY Status Register Bit Definition */
/**
    @brief Ethernet Cable Off
    @details @ref PHYSR_CAB_OFF indicates the cable is off the Ethernet PHY.
    @sa _PHYSR_, PHYSR_CAB, PHYSR_CAB_ON
    @sa getPHYSR()
*/
#define PHYSR_CAB_OFF        (1<<7)

/**
    @brief Ethernet PHY Cable On
    @details @ref PHYSR_CAB_OFF indicates the cable is on the Ethernet PHY.
    @sa _PHYSR_, PHYSR_CAB, PHYSR_CAB_OFF
    @sa getPHYSR()
*/
#define PHYSR_CAB_ON         (0<<7)

/**
    @brief Mask bits of @ref _PHYSR_
    @details @ref PHYSR_MODE masks the MODE bits of @ref _PHYSR_.\n
            The masked bits values are as following. \n
              - @ref PHYSR_MODE_AUTO
              - @ref PHYSR_MODE_100F
              - @ref PHYSR_MODE_100H
              - @ref PHYSR_MODE_10F
              - @ref PHYSR_MODE_10H
    @sa _PHYSR_, _PHYCR0_, _PHYCLKR_, _SYSR_, SYSR_NETL
    @sa getPHYSR(), setPHYCR0(), setPHYLCKR(), PHYLOCK(), PHYUNLOCK(), getSYSR()
*/
#define PHYSR_MODE           (7<<3)

/**
    @brief PHY Mode - AUTO
    @details @ref PHYSR_MODE_AUTO indicates the Ethernet PHY is operated as auto-negotiation mode.
    @sa _PHYSR_, PHYSR_MODE, PHYSR_MODE_100F, PHYSR_MODE_100H, PHYSR_MODE_10F, PHYSR_MODE_10H
    @sa getPHYSR()
*/
#define PHYSR_MODE_AUTO      (0<<3)

/**
    @brief PHY Mode - 100F
    @details @ref PHYSR_MODE_100F indicates the Ethernet PHY is operated as 100M full-duplex mode.
    @sa _PHYSR_, PHYSR_MODE, PHYSR_MODE_AUTO, PHYSR_MODE_100H, PHYSR_MODE_10F, PHYSR_MODE_10H
    @sa getPHYSR()
*/
#define PHYSR_MODE_100F      (4<<3)

/**
    @brief PHY Mode - 100H
    @details @ref PHYSR_MODE_100H indicates the Ethernet PHY is operated as 100M half-duplex mode.
    @sa _PHYSR_, PHYSR_MODE, PHYSR_MODE_AUTO, PHYSR_MODE_100F, PHYSR_MODE_10F, PHYSR_MODE_10H
    @sa getPHYSR()
*/
#define PHYSR_MODE_100H      (5<<3)

/**
    @brief PHY Mode - 10F
    @details @ref PHYSR_MODE_10F indicates the Ethernet PHY is operated as 10M full-duplex mode.
    @sa _PHYSR_, PHYSR_MODE, PHYSR_MODE_AUTO, PHYSR_MODE_100F, PHYSR_MODE_100H, PHYSR_MODE_10H
    @sa getPHYSR()
*/
#define PHYSR_MODE_10F       (6<<3)

/**
    @brief PHY Mode - 10H
    @details @ref PHYSR_MODE_10H indicates the Ethernet PHY is operated as 10M half-duplex mode.
    @sa _PHYSR_, PHYSR_MODE, PHYSR_MODE_AUTO, PHYSR_MODE_100F, PHYSR_MODE_100H, PHYSR_MODE_10F
    @sa getPHYSR()
*/
#define PHYSR_MODE_10H       (7<<3)

/**
    @brief DPX mask bit of @ref _PHYSR_
    @details @ref PHYSR_DPX masks the DPX bit of @ref _PHYSR_. \n
            The masked bit values are as following. \n
              - @ref PHYSR_DPX_HALF
              - @ref PHYSR_DPX_FULL
    @sa _PHYSR_, _PHYCR0_, _PHYCLKR_, _SYSR_, SYSR_NETL
    @sa getPHYSR(), setPHYCR0(), setPHYLCKR(), PHYLOCK(), PHYUNLOCK(), getSYSR()
*/
#define PHYSR_DPX            (1<<2)

/**
    @brief PHY Duplex - HALF
    @details @ref PHYSR_DPX_HALF indicates the Ethernet PHY is operated as half-duplex mode.
    @sa _PHYSR_, PHYSR_DPX_FULL
    @sa getPHYSR()
*/
#define PHYSR_DPX_HALF       (1<<2)

/**
    @brief PHY Duplex - FULL
    @details @ref PHYSR_DPX_FULL indicates the Ethernet PHY is operated as full-duplex mode.
    @sa _PHYSR_, PHYSR_DPX_HALF
    @sa getPHYSR()
*/
#define PHYSR_DPX_FULL       (0<<2)

/**
    @brief SPD mask bit of @ref _PHYSR_
    @details @ref PHYSR_SPD masks the SPD bit of @ref _PHYSR_. The masked bit values are as following. \n
     - @ref PHYSR_SPD_10M
     - @ref PHYSR_SPD_100M
    @sa _PHYSR_, _PHYCR0_, _PHYCLKR_, _SYSR_, SYSR_NETL
    @sa getPHYSR(), setPHYCR0(), setPHYLCKR(), PHYLOCK(), PHYUNLOCK(), getSYSR()
*/
#define PHYSR_SPD            (1<<1)

/**
    @brief PHY Speed - 10M
    @details @ref PHYSR_SPD_10M indicates the Ethernet PHY is operated as 10Mbps speed.
    @sa _PHYSR_, PHYSR_SPD_100M
    @sa getPHYSR()
*/
#define PHYSR_SPD_10M        (1<<1)

/**
    @brief PHY Speed - 100M
    @details @ref PHYSR_SPD_100M indicates the Ethernet PHY is operated as 100Mbps speed.
    @sa _PHYSR_, PHYSR_SPD_10M
    @sa getPHYSR()
*/
#define PHYSR_SPD_100M       (0<<1)

/**
    @brief LNK mask bit of @ref _PHYSR_
    @details @ref PHYSR_LNK masks the LNK bit of @ref _PHYSR_. The masked bit values are as following. \n
     - @ref PHYSR_LNK_DOWN
     - @ref PHYSR_LNK_UP
    @sa _PHYSR_, _PHYCR0_, _PHYCLKR_, _SYSR_, SYSR_NETL
    @sa getPHYSR(), setPHYCR0(), setPHYLCKR(), PHYLOCK(), PHYUNLOCK(), getSYSR()
*/
#define PHYSR_LNK            (1<<0)

/**
    @brief PHY Link - Up
    @details @ref PHYSR_LNK_UP indicates the link of Ethernet PHY is successfully established .\n
    @sa _PHYSR_, PHYSR_LNK_DOWN
    @sa getPHYSR()
*/
#define PHYSR_LNK_UP         (1<<0)

/**
    @brief PHY Link - Down
    @details @ref PHYSR_LNK_DOWN indicates the link of Ethernet PHY is not established yet.\n
    @sa _PHYSR_, PHYSR_LNK_UP
    @sa getPHYSR()
*/
#define PHYSR_LNK_DOWN       (0<<0)

/**
    @brief Read a value  from the Ethernet PHY register specified by @ref _PHYRAR_.\n
          The read value can be checked by _PHYDOR_.
    @sa _PHYACR_, _PHYDOR_, _PHYRAR_, _PHYDIR_, PHYACR_WRITE
    @sa getPHYACR(), setPHYACR(), getPHYDOR(), getPHYRAR(), setPHYRAR(), setPHYDIR()
*/
#define PHYACR_READ          (0x02)

/**
    @brief Write @ref _PHYDIR_ to the Ethernet PHY register specified by @ref _PHYRAR_.
    @sa _PHYACR_, _PHYDIR_, _PHYRAR_, _PHYDOR_, PHYACR_READ
    @sa getPHYACR(), setPHYACR(), setPHYDIR(), getPHYRAR(), setPHYRAR(), getPHYDOR()
*/
#define PHYACR_WRITE         (0x01)

/**
    @brief PHY's MDC clock is the divided the system clock by 32
    @sa _PHYDIVR_
    @sa getPHYDIVR(), setPHYDIVR()
*/
#define PHYDIVR_32           (0x00)

/**
    @brief PHY's MDC clock is the divided system clock by 64
    @sa _PHYDIVR_
    @sa getPHYDIVR(), setPHYDIVR()
*/
#define PHYDIVR_64           (0x01)

/**
    @brief PHY's MDC clock is the divided system clock by 128
    @sa _PHYDIVR_
    @sa getPHYDIVR(), setPHYDIVR()
*/
#define PHYDIVR_128          (0xFF)


/* PHY Command Register Bit Definition */
/**
    @brief PHY Operation Mode - Auto Negotiation
    @details @ref PHYCR0_AUTO sets the Ethernet PHY to operate on auto-negotiation mode.\n
            The Ethernet PHY can operate on auto-negotiation after @ref PHYCR1_RST is performed, \n
            and the result of @ref PHYCR0_AUTO can be checked by @ref PHYSR_SPD, @ref PHYSR_DPX, and @ref PHYSR_LNK.
    @note It can be set only when @ref SYSR_PHYL = 1.
    @sa _PHYCR0_, _PHYLCKR_, _SYSR_, SYSR_PHYL, _PHYSR_, PHYCR0_100F, PHYCR0_100H, PHYCR0_10F, PHYCR0_10H, BMCR_ANE
    @sa setPHYCR0(), getPHYLCKR(), setPHYLCKR(), PHYLOCK(), PHYUNLOCK(), getSYSR(), getPHYSR(), getPHYRAR_BMCR(), setPHYRAR_BMCR()
*/
#define PHYCR0_AUTO          (0x00)

/**
    @brief PHY Operation Mode - 100F
    @details @ref PHYCR0_100F sets the Ethernet PHY to operate on 100F\n
            The Ethernet PHY can operate on 100F after @ref PHYCR1_RST is performed,\n
            and the result of @ref PHYCR0_100F can be checked by @ref PHYSR_SPD, @ref PHYSR_DPX, and @ref PHYSR_LNK.
    @note It can be set only when @ref SYSR_PHYL = 1.
    @sa _PHYCR0_, _PHYLCKR_, _SYSR_, SYSR_PHYL, _PHYSR_, BMCR_SPD, BMCR_DPX
    @sa setPHYCR0(), getPHYLCKR(), setPHYLCKR(), PHYLOCK(), PHYUNLOCK(), getSYSR(), getPHYSR(), getPHYRAR_BMCR(), setPHYRAR_BMCR()
*/
#define PHYCR0_100F          (0x04)

/**
    @brief PHY Operation Mode - 100H
    @details @ref PHYCR0_100H sets the Ethernet PHY to operate on 100H \n
            The Ethernet PHY can operate 100H after @ref PHYCR1_RST is performed, \n
            and the result of @ref PHYCR0_100H can be checked by @ref PHYSR_SPD, @ref PHYSR_DPX, and @ref PHYSR_LNK.
    @note It can be set only when @ref SYSR_PHYL = 1.
    @sa _PHYCR0_, _PHYLCKR_, _SYSR_, SYSR_PHYL, _PHYSR_, PHYCR0_AUTO, PHYCR0_100H, PHYCR0_10F, PHYCR0_10H, BMCR_SPD, BMCR_DPX
    @sa setPHYCR0(), getPHYLCKR(), setPHYLCKR(), PHYLOCK(), PHYUNLOCK(), getSYSR(), getPHYSR(), getPHYRAR_BMCR(), setPHYRAR_BMCR()
*/
#define PHYCR0_100H          (0x05)

/**
    @brief PHY Operation Mode - 10F
    @details @ref PHYCR0_10F sets the Ethernet PHY to operate on 10F \n
            The Ethernet PHY can operate 10H after @ref PHYCR1_RST is performed, \n
            and the result of @ref PHYCR0_10F can be checked by @ref PHYSR_SPD, @ref PHYSR_DPX, and @ref PHYSR_LNK.
    @note It can be set only when @ref SYSR_PHYL = 1.
    @sa _PHYCR0_, _PHYLCKR_, _SYSR_, SYSR_PHYL, _PHYSR_, PHYCR0_AUTO, PHYCR0_100F, PHYCR0_100H, PHYCR0_10H, BMCR_SPD, BMCR_DPX
    @sa setPHYCR0(), getPHYLCKR(), setPHYLCKR(), PHYLOCK(), PHYUNLOCK(), getSYSR(), getPHYSR(), getPHYRAR_BMCR(), setPHYRAR_BMCR()
*/
#define PHYCR0_10F           (0x06)

/**
    @brief PHY Operation Mode - 10H
    @details @ref PHYCR0_10H sets the Ethernet PHY to operate on 10H \n
            The Ethernet PHY can operate 10H after @ref PHYCR1_RST is performed,  \n
            and the result of @ref PHYCR0_10H can be checked by @ref PHYSR_SPD, @ref PHYSR_DPX, and @ref PHYSR_LNK.
    @note It can be set only when @ref SYSR_PHYL = 1.
    @sa _PHYCR0_, _PHYLCKR_, _SYSR_, SYSR_PHYL, _PHYSR_, PHYCR0_AUTO, PHYCR0_100F, PHYCR0_100H, PHYCR0_10H, BMCR_SPD, BMCR_DPX
    @sa setPHYCR0(), getPHYLCKR(), setPHYLCKR(), PHYLOCK(), PHYUNLOCK(), getSYSR(), getPHYSR(), getPHYRAR_BMCR(), setPHYRAR_BMCR()
*/
#define PHYCR0_10H           (0x07)


/**
    @brief PHY function - Power Down
    @details @ref PHYCR1_PWDN enters the Ethernet PHY in power down mode. \n
            0 : Normal mode \n
            1 : Power down mode
    @note The system clock changes to 25MHz in power down mode, and depends on @ref SYCR1_CLKSEL in normal mode.
    @note It can be set only when @ref SYSR_PHYL = 1.
    @sa _PHYCR1_, SYCR1_CLKSEL, BMCR_PWDN
    @sa getPHYCR1(), setPHYCR1(), getSYCR1(), setSYCR1(), getPHYRAR_BMCR(), setPHYRAR_BMCR()
*/
#define PHYCR1_PWDN          (1<<5)

/**
    @brief PHY function - 10Base-TE Mode
    @details @ref PHYCR1_TE sets the operation of Ethernet PHY to 10base-Te.
    @note It is valid only when @ref PHYSR_MODE = @ref PHYSR_MODE_AUTO.
    @note It can be set only when @ref SYSR_PHYL = 1.
    @sa _PHYCR1_
    @sa getPHYCR1(), setPHYCR1()
*/
#define PHYCR1_TE            (1<<3)

/**
    @brief PHY function - HW Reset
    @details @ref PHYCR1_RST resets the Ethernet PHY in hardware, \n
            and it is automatically cleared after the H/W reset and it takes 60.3ms to stabilize.\n
            0 : Normal mode \n
            1 : H/W Reset \n
    @note The system clock changes to 25MHz in H/W reset time, and depends on @ref SYCR1_CLKSEL in normal mode.
    @note It can be set only when @ref SYSR_PHYL = 1.
    @sa _PHYCR1_, BMCR_RST
    @sa getPHYCR1(), setPHYCR1(), getPHYRAR_BMCR(), setPHYRAR_BMCR()
*/
#define PHYCR1_RST           (1<<0)



/* IPv4 Network Mode Register Bit Definition */
/**
    @brief UDP Unreachable Packet Block
    @details @ref NETxMR_UNRB can block sending a ICMPv or ICMPv6 unreachable message to a peer.
    @sa _NET4MR_, _NET6MR_
    @sa getNET4MR(), setNET4MR(), getNET6MR(), setNET6MR()
*/
#define NETxMR_UNRB          (1<<3)

/**
    @brief PING ARP Request
    @details @ref NETxMR_PARP can send a ARP request before sending a ICMPv4 or ICMPv6 PING reply.
    @sa _NET4MR_, _NET6MR_
    @sa getNET4MR(), setNET4MR(), getNET6MR(), setNET6MR()
*/
#define NETxMR_PARP          (1<<2)

/**
    @brief TCP Reset Packet Block
    @details @ref NETxMR_RSTB can block sending a TCP RST packet based on IPv4 or IPv6 \n
            when there is no SOCKET n opened with a listen port.
    @sa _NET4MR_, _NET6MR_
    @sa getNET4MR(), setNET4MR(), getNET6MR(), setNET6MR()
*/
#define NETxMR_RSTB          (1<<1)

/**
    @brief PING Reply Block
    @details @ref NETxMR_PB can block sending a ICMPv4 or ICMPv6 PING reply to a peer.
    @sa _NET4MR_, _NET6MR_
    @sa getNET4MR(), setNET4MR(), getNET6MR(), setNET6MR()
*/
#define NETxMR_PB            (1<<0)


/* Network Mode Register Bit Definition */
/**
    @brief All-node Multicating PING Reply Block
    @details @ref NETMR_ANB can block sending a IPv6 PING reply to the peer ping requested to all-node multicast address.
    @sa _NETMR_
    @sa getNETMR(), setNETMR()
*/
#define NETMR_ANB            (1<<5)

/**
    @brief Solicited Multicasting PING Reply Block
    @details @ref NETMR_M6B can block sending a IPv6 PING reply to the peer ping requested to the own solicited multicast address.
    @sa _NETMR_
    @sa getNETMR(), setNETMR()
*/
#define NETMR_M6B            (1<<4)

/**
    @brief Wake On LAN mode
    @details @ref NETMR_WOL can receive a magic packet of WOL.
    @sa _NETMR_
    @sa getNETMR(), setNETMR()
*/
#define NETMR_WOL            (1<<2)

/**
    @brief IPv6 Packet Block
    @details @ref NETMR_IP6B can block receiving all IPv6 packets.
    @sa _NETMR_
    @sa getNETMR(), setNETMR()
*/
#define NETMR_IP6B           (1<<1)

/**
    @brief IPv4 Packet Block
    @details @ref NETMR_IP4B can block receiving all IPv4 packets.
    @sa _NETMR_
    @sa getNETMR(), setNETMR()
*/
#define NETMR_IP4B           (1<<0)


/**
    @brief Destination Hardware Address Select
    @details @ref NETMR2_DHAS masks the DHAS bit of @ref _NETMR2_. \n
            The masked bit values are as following.
              - @ref NETMR2_DHAS_ARP
              - @ref NETMR2_DHAS_ETH

    @note It is useful when the destination hardware address of Ethernet frame is different from the target address of ARP.
    @sa _NETMR2_
    @sa getNETMR2(), setNETMR2()
*/
#define NETMR2_DHAS          (1<<7)

/**
    @brief Destination Hardware Address Select - ARP
    @details @ref NETMR2_DHAS_ARP select the target address of ARP-replay packet to the destination hardware address.
    @sa _NETMR2_, NETMR2_DHAS_ETH
    @sa getNETMR2(), setNETMR2()
*/
#define NETMR2_DHAS_ARP      (1<<7)

/**
    @brief Destination Hardware Address Select - Ethernet Frame
    @details @ref NETMR2_DHAS_ETH select the destination address of Ethernet frame to the destination hardware address.
    @sa _NETMR2_, NETMR2_DHAS_ARP
    @sa getNETMR2(), setNETMR2()
*/
#define NETMR2_DHAS_ETH      (0<<7)

/**
    @brief PPPoE Mode
    @details @ref NETMR2_PPPoE enables PPPoE mode \n
            0 : Disable \n
            1 : Enable
    @note For enabling a PPPoE mode, some information such like as _PTMR_, _PHAR_, _PSIDR_, and _PMRUR_ are needed. \n
         To get these information, You can use a SOCKET0 opened with @ref Sn_MR_MACRAW.
    @sa _NETMR2_, _PTMR_, _PHAR_, _PSIDR_, _PMRUR_, Sn_MR_MACRAW
    @sa getNETMR2(), setNETMR2()
*/
#define NETMR2_PPPoE         (1<<0)

/* ICMPv6 Block Register Bit Definition */
/**
    @brief ICMPv6 PING Block
    @details @ref ICMP6BLKR_PING6 can block a ping request from a peer
    @sa _IMCP6BLKR_, NETxMR_PB
    @sa getICMP6BLKR(), setICMP6BLKR(), getNET6MR(), setNET6MR()
*/
#define ICMP6BLKR_PING6      (1<<4)

/**
    @brief ICMPv6 MLD Block
    @details @ref ICMP6BLKR_MLD can block a multicast listener discovery(MLD) query.
    @sa _ICMP6BLKR_
    @sa getICMP6BLKR(), setICMP6BLKR()
*/
#define ICMP6BLKR_MLD        (1<<3)

/**
    @brief ICMPv6 RA Block
    @details @ref ICMP6BLKR_RA can block a RA packet from a router
    @sa _ICMP6BLKR_
    @sa getICMP6BLKR(), setICMP6BLKR()
*/
#define ICMP6BLKR_RA         (1<<2)

/**
    @brief ICMPv6 NA Block
    @details @ref ICMP6BLKR_NA can block a Neighbor Advertisement(NA) from a peer
    @sa _ICMP6BLKR_
    @sa getICMP6BLKR(), setICMP6BLKR()
*/
#define ICMP6BLKR_NA         (1<<1)

/**
    @brief ICMPv6 NS Block
    @details @ref ICMP6BLKR_NS can block a Neighbor Solicitation(NS) from a peer
    @sa _ICMP6BLKR_
    @sa getICMP6BLKR(), setICMP6BLKR()
*/
#define ICMP6BLKR_NS         (1<<0)


/* Sn_MR values */
/**
    @brief UDP Multicasting
    @details  @ref Sn_MR_MULTI enables to a multicast packet from a multicast group in UDP mode SOCKETn. \n
              To use multicasting, @ref _Sn_DIPR_, @ref _Sn_DIP6R_, & @ref _Sn_DPORTR_ should be respectively set with \n
              the multicast group IPv4, IPv6 address & port number before @ref Sn_CR_OPEN. \n
              0 : Disable Multicasting \n
              1 : Enable Multicasting \n
    @note It is valid only in UDP mode such like as @ref Sn_MR_UDP4, @ref Sn_MR_UDP6, and @ref Sn_MR_UDPD.
    @sa _Sn_MR_, _Sn_DIPR_, _Sn_DIP6R_, _Sn_DPORTR_
    @sa getSn_MR(), setSn_MR(), getSn_DIPR(), setSn_DIPR(), getSn_DIP6R(), setSn_DIP6R(), getSn_DPORTR().
*/
#define Sn_MR_MULTI          (1<<7)

/**
    @brief MAC Filter
    @details @ref Sn_MR_MF filters other packets except broadcasting, multicasting, and packets sent to your own.\n
            0 : Disable MAC Filtering \n
            1 : Enable MAC Filtering \n
    @note It is valid only in MACRAW SOCKET0 opened with @ref Sn_MR_MACRAW \n
    @note If you want to implement a hybrid TCP/IP stack, \n
         It is recommended that @ref Sn_MR_MF enable for reducing host overhead to process the all received packets.
    @sa _Sn_MR_, Sn_MR_MULTI
    @sa getSn_MR(), setSn_MR()
*/
#define Sn_MR_MF             (1<<7)

/**
    @brief Broadcasting packet block
    @details @ref Sn_MR_BRDB can block a broadcasting packet in MACRAW SOCKET0 or UDP mode SOCKETn. \n
            0 : Disable Broadcast Blocking \n
            1 : Enable Broadcast Blocking \n
    @note It is valid only in MACRAW mode such as @ref Sn_MR_MACRAW, or in UDP mode such as @ref Sn_MR_UDP4, @ref Sn_MR_UDP6, and @ref Sn_MR_UDPD.
    @sa _Sn_MR_, Sn_MR_FPSH
    @sa getSn_MR(), setSn_MR()
*/
#define Sn_MR_BRDB          (1<<6)

/**
    @brief Force PUSH flag
    @details When @ref Sn_MR_FPSH is set, all TCP DATA packets with PSH flag set can be transmitted by @ref Sn_CR_SEND. \n
            When @ref Sn_MR_FPSH is not set, the PSH flag is set only in the last DATA packet among the DATA packets transmitted by @ref Sn_CR_SEND. \n
            0 : No Force PSH flag \n
            1 : Force PSH flag \n
    @note It is valid only in TCP mode such as @ref Sn_MR_TCP4, @ref Sn_MR_TCP6, and @ref Sn_MR_TCPD.
    @sa _Sn_MR_, Sn_CR_SEND, Sn_MR_BRDB
    @sa getSn_MR(), setSn_MR(), getSn_CR(), setSn_CR()
*/
#define Sn_MR_FPSH           (1<<6)

/**
    @brief No Delayed Ack
    @details When @ref Sn_MR_FPSH is set, It sends the ACK packet without delay as soon as a DATA packet is received from a peer.\n
            Otherwise, It sends the ACK packet after waiting the time set by @ref _Sn_RTR_. \n
            0 : Delayed ACK \n
            1 : No Delayed ACK \n
    @note It is valid only in TCP mode such as @ref Sn_MR_TCP4, @ref Sn_MR_TCP6, and @ref Sn_MR_TCP6.
    @note Regardless of @ref Sn_MR_ND, It sends the ACK packet when SOCKETn window size is less than MSS after @ref Sn_CR_RECV.
    @sa _Sn_MR_, _Sn_RTR_, Sn_CR_RECV, Sn_MR_MC, Sn_MR_SMB, Sn_MR_MMB
    @sa getSn_MR(), setSn_MR(), getSn_RTR(), setSn_RTR(), getSn_CR(), setSn_CR()
*/
#define Sn_MR_ND            (1<<5)

/**
    @brief IGMP version for IPv4 Multicasting
    @details @ref Sn_MR_MC decides IGMP version. \n
            0 : IGMPv2 \n
            1 : IGMPv1 \n
    @note It is valid only when @ref Sn_MR_MULTI = '1' and UDP mode is @ref Sn_MR_UDP4.
    @note IGMP packet can be automatically sent to the multicast group by @ref Sn_CR_OPEN.
    @note @ref _WIZCHIP_ doesn't not support IGMP version 3.
    @sa _Sn_MR_, Sn_MR_MULTI, Sn_MR_ND, Sn_MR_SMB, Sn_MR_MMB
    @sa getSn_MR(), setSn_MR()
*/
#define Sn_MR_MC             (1<<5)

/**
    @brief Solicited Mulitcast Block
    @details @ref Sn_MR_SMB can block a received packet that have your own solicited multicast address. \n
            0 : Unblock a solicited multicast packet \n
            1 : Block a solicited multicast packet \n
    @note It is valid only when UDP mode is @ref Sn_MR_UDP6 or @ref Sn_MR_UDPD.
    @sa _Sn_MR_,  Sn_MR_ND, Sn_MR_MC, Sn_MR_MMB
    @sa getSn_MR(), setSn_MR()
*/
#define Sn_MR_SMB            (1<<5)

/**
    @brief UDP4 Multicast Block
    @details @ref Sn_MR_MMB can block the UDP4 multicast packet when SOCKET0 is opend with @ref Sn_MR_MACRAW and @ref Sn_MR_MF is set.\n
            0 : Unblock a UDP multicast packet with IPv4 address \n
            1 : Block a UDP multicast packet with IPv4 address \n
    @note It is valid only in MACRAW SOCKET0 with Sn_MR_MF = '1'.
    @sa _Sn_MR_, Sn_MR_MMB6, Sn_MR_ND, Sn_MR_MC, Sn_MR_SMB
    @sa getSn_MR(), setSn_MR()
*/
#define Sn_MR_MMB            (1<<5)
#define Sn_MR_MMB4           (Sn_MR_MMB)  /// Refer to @ref Sn_MR_MMB.

/**
    @brief Unicast Block
    @details @ref Sn_MR_UNIB can block a unicast packet. \n
            0 : Unblock a UDP unicast packet \n
            1 : Block a UDP unicast packet \n
    @note It is valid only when SOCKETn is opened with UDP mode such as @ref Sn_MR_UDP4, @ref Sn_MR_UDP6 and @ref Sn_MR_UDPD, and @ref Sn_MR_MULTI is set.
    @sa _Sn_MR_, Sn_MR_MULTIL, Sn_MR_MMB6
    @sa getSn_MR(), setSn_MR()
*/
#define Sn_MR_UNIB           (1<<4)

/**
    @brief UDP6 Multicast Block
    @details @ref Sn_MR_MMB6 can block the UDP6 multicast packet. \n
            0 : Unblock a UDP multicast packet with IPv6 address \n
            1 : Block a UDP multicast packet with IPv6 address \n
    @note  It is valid only when SOCKET0 is opend with @ref Sn_MR_MACRAW and @ref Sn_MR_MF is set.
    @sa _Sn_MR_, Sn_MR_MMB
    @sa getSn_MR(), setSn_MR()
*/
#define Sn_MR_MMB6           (1<<4)

/**
    @brief SOCKETn Closed
    @details @ref Sn_MR_CLOSE is not opened yet.\n
            It is the default mode when @ref _WIZCHIP_ is reset.
    @sa _Sn_MR_, _Sn_CR_, _Sn_SR_
    @sa getSn_MR(), setSn_MR(), getSn_CR(), setSn_CR(), getSn_SR()
*/
#define Sn_MR_CLOSE          (0x00)

/**
    @brief IPv4 TCP mode
    @details @ref Sn_MR_TCP(= @ref Sn_MR_TCP4) sets SOCKETn to TCP4 mode. \n
            It should be set before @ref Sn_CR_OPEN is performed.\n
            After @ref Sn_CR_OPEN, SOCKETn is opend as TCP4 mode and @ref _Sn_SR_ is changed from @ref SOCK_CLOSED to @ref SOCK_INIT.
    @note In oder to connect a peer, You should use @ref Sn_CR_CONNECT as command and @ref _Sn_DIPR_ as destination.
    @note In order to send data, You should use @ref Sn_CR_SEND.
    @sa _Sn_MR_, _Sn_CR_, _Sn_SR_, Sn_MR_TCP6, Sn_MR_TCPD
    @sa getSn_MR(), setSn_MR(), getSn_CR(), setSn_CR(), getSn_SR()
*/
#define Sn_MR_TCP            (0x01)
#define Sn_MR_TCP4           (Sn_MR_TCP)   ///< Refer to @ref Sn_MR_TCP.

/**
    @brief IPv4 UDP mode
    @details @ref Sn_MR_UDP(= @ref Sn_MR_UDP4) sets SOCKETn to UDP4 mode.\n
            It should be set before @ref Sn_CR_OPEN is performed. \n
            After @ref Sn_CR_OPEN, SOCKETn is opend as UDP4 mode and @ref _Sn_SR_ is changed from @ref SOCK_CLOSED to @ref SOCK_UDP.
    @note In order to send data, You should use @ref Sn_CR_SEND as command and @ref _Sn_DIPR_ as destination.
    @sa _Sn_MR_, _Sn_CR_, _Sn_SR_, Sn_MR_UDP6, Sn_MR_UDPD
    @sa getSn_MR(), setSn_MR(), getSn_CR(), setSn_CR(), getSn_SR()
*/
#define Sn_MR_UDP            (0x02)
#define Sn_MR_UDP4           (Sn_MR_UDP)   ///< Refer to @ref Sn_MR_UDP

/**
    @brief IPv4 RAW mode
    @details @ref Sn_MR_IPRAW(= @ref Sn_MR_IPRAW4) sets SOCKETn to IPRAW4 mode. \n
            It should be set before @ref Sn_CR_OPEN is performed.\n
            After @ref Sn_CR_OPEN, SOCKETn is opend as IPRAW4 mode and @ref _Sn_SR_ is changed from @ref SOCK_CLOSED to @ref SOCK_IPRAW(= @ref SOCK_IPRAW4).
    @note In order to send data, You should use @ref Sn_CR_SEND as command and @ref _Sn_DIPR_ as destination.
    @sa _Sn_MR_, _Sn_CR_, _Sn_SR_, Sn_MR_IPRAW6
    @sa getSn_MR(), setSn_MR(), getSn_CR(), setSn_CR(), getSn_SR()
*/
#define Sn_MR_IPRAW          (0x03)
#define Sn_MR_IPRAW4         (Sn_MR_IPRAW)   ///< Refer to @ref Sn_MR_IPRAW.   

/**
    @brief MACRAW mode
    @details @ref Sn_MR_MACRAW sets only SOCKET0 to MACRAW mode.\n
            It should be set before @ref Sn_CR_OPEN is performed.\n
            After @ref Sn_CR_OPEN, SOCKET0 is opend as MACRAW mode and @ref _Sn_SR_ is changed from @ref SOCK_CLOSED to @ref SOCK_MACRAW.
    @note In order to send data, You should use @ref Sn_CR_SEND.
    @sa _Sn_MR_, _Sn_CR_, _Sn_SR_
    @sa getSn_MR(), setSn_MR(), getSn_CR(), setSn_CR(), getSn_SR()
*/
#define Sn_MR_MACRAW         (0x07)

/**
    @brief IPv6 TCP mode
    @details @ref Sn_MR_TCP6 sets SOCKETn to TCP6 mode. \n
            It should be set before @ref Sn_CR_OPEN is performed.\n
            After @ref Sn_CR_OPEN, SOCKETn is opend as TCP6 mode and @ref _Sn_SR_ is changed from @ref SOCK_CLOSED to @ref SOCK_INIT.
    @note In oder to connect a peer, You should use @ref Sn_CR_CONNECT6 as command and @ref _Sn_DIP6R_ as destination.
    @note In order to send data, You should use @ref Sn_CR_SEND, not @ref Sn_CR_SEND6.
    @sa _Sn_MR_, _Sn_CR_, _Sn_SR_, Sn_MR_TCP4, Sn_MR_TCPD
    @sa getSn_MR(), setSn_MR(), getSn_CR(), setSn_CR(), getSn_SR()
*/
#define Sn_MR_TCP6           (0x09)

/**
    @brief IPv6 UDP mode
    @details @ref Sn_MR_UDP6 sets SOCKETn to UDP6 mode.\n
            It should be set before @ref Sn_CR_OPEN is performed.\n
            After @ref Sn_CR_OPEN, SOCKETn is opend as UDP6 mode and @ref _Sn_SR_ is changed from @ref SOCK_CLOSED to @ref SOCK_UDP.
    @note In order to send data, You should use @ref Sn_CR_SEND6 as command and @ref _Sn_DIP6R_ as destination.
    @sa _Sn_MR_, _Sn_CR_, _Sn_SR_, Sn_MR_UDP4, Sn_MR_UDPD
    @sa getSn_MR(), setSn_MR(), getSn_CR(), setSn_CR(), getSn_SR()
*/
#define Sn_MR_UDP6           (0x0A)

/**
    @brief IPv6 RAW mode
    @details @ref Sn_MR_IPRAW6 sets SOCKETn to IPRAW6 mode.\n
            It should be set before @ref Sn_CR_OPEN is performed.\n
            After @ref Sn_CR_OPEN, SOCKETn is opened as IPRAW6 mode and @ref _Sn_SR_ is changed from @ref SOCK_CLOSED to @ref SOCK_IPRAW6.
    @note In order to send data, You should use @ref Sn_CR_SEND6 as command and @ref _Sn_DIP6R_ as destination.
    @sa _Sn_MR_, _Sn_CR_, _Sn_SR_, Sn_MR_IPRAW4
    @sa getSn_MR(), setSn_MR(), getSn_CR(), setSn_CR(), getSn_SR()
*/
#define Sn_MR_IPRAW6         (0x0B)

/**
    @brief Both IPv4 & IPv6 TCP mode (TCP dual mode)
    @details @ref Sn_MR_TCPD sets SOCKETn to both TCP4 & TCP6 mode. \n
            It should be set before @ref Sn_CR_OPEN is performed.\n
            After @ref Sn_CR_OPEN, SOCKETn is opened as TCP Dual mode and @ref _Sn_SR_ is changed from @ref SOCK_CLOSED to @ref SOCK_INIT.\n
            The real mode of TCP dual SOCKETn is decided when the connection with a peer is established.\n
     - In SOCKETn is operated as <b>TCP SERVER</b> mode
       If the connection request client have a IPv4 address, \n
       TCP dual SOCKETn is changed to TCP4 mode and a destination IP address can be checked thru @ref _Sn_DIPR_, \n
       else if the client have a IPv6 address, \n
       TCP dual SOCKETn is changed to IPv6 mode and destination IP address can be checked by thru @ref _Sn_DIP6R_.
     - In SOCKETn is operated as <b>TCP CLIENT</b> mode,
       If the IP address type of destination to connect is IPv4, \n
       the destination IP address should be set to @ref _Sn_DIPR_ and try to connect by @ref Sn_CR_CONNECT, \n
       else if the type is IPv6, \n
       the destination IP address should be set to @ref _Sn_DIP6R_ and try to connect by @ref Sn_CR_CONNECT6. \n

    @note In <b>TCP SERVER</b> mode, You can check the IP type of the client with @ref Sn_ESR_TCPM.
    @note If the connected client have a IPv6 address, You can check whether the address is LLA or GAU, thru @ref Sn_ESR_IP6T
    @sa _Sn_MR_, _Sn_CR_, _Sn_SR_, _Sn_ESR_TCPM_, Sn_MR_TCP4, Sn_MR_TCP6
    @sa getSn_MR(), setSn_MR(), getSn_CR(), setSn_CR(), getSn_SR(), getSn_ESR()
*/
#define Sn_MR_TCPD           (0x0D)

/**
    @brief UDP Dual mode
    @details @ref Sn_MR_UDPD sets SOCKETn to both UDP4 & UDP6 mode. \n
            It should be set before @ref Sn_CR_OPEN is performed. \n
            After @ref Sn_CR_OPEN, SOCKETn is opened as UDP dual mode \n
            and @ref _Sn_SR_ is changed from @ref SOCK_CLOSED to @ref SOCK_UDP.
    @note In order to send data, \n
         You can use both @ref Sn_CR_SEND and @ref Sn_CR_SEND6 as command and both @ref _Sn_DIPR_ and @ref _Sn_DIP6R_ as destination.
    @note You can know the destination IP address type is whether IPv6 or IPv4 thru @ref getsockopt() with @ref SO_PACKINFO.
    @sa _Sn_MR_, _Sn_CR_, _Sn_SR_, Sn_MR_UDP6, Sn_MR_UDP4
    @sa getSn_MR(), setSn_MR(), getSn_CR(), setSn_CR(), getSn_SR()
*/
#define Sn_MR_UDPD           (0x0E)

/* SOCKETn  Command Register BIt Definition */
/**
    @brief Initialize or Open SOCKETn.
    @details SOCKETn is initialized and opened according to the protocol mode selected by @ref _Sn_MR_ and with a source port set by @ref _Sn_PORTR_. \n
                     The table shows @ref _Sn_SR_ is changed according to @ref _Sn_MR_.\n
    <table>
      <tr> <td> <b>@ref _Sn_MR_</b> (P[3:0])                      </td> <td><b>@ref _Sn_SR_</b> </td> </tr>
      <tr> <td> @ref Sn_MR_CLOSE                                  </td> <td> @ref SOCK_CLOSED   </td> </tr>
      <tr> <td> @ref Sn_MR_TCP4, @ref Sn_MR_TCP6, @ref Sn_MR_TCPD </td> <td> @ref SOCK_INIT     </td> </tr>
      <tr> <td> @ref Sn_MR_UDP, @ref Sn_MR_UDP6, @ref Sn_MR_UDPD  </td> <td> @ref SOCK_UDP      </td> </tr>
      <tr> <td> @ref Sn_MR_IPRAW4                                 </td> <td> @ref SOCK_IPRAW4   </td> </tr>
      <tr> <td> @ref Sn_MR_IPRAW6                                 </td> <td> @ref SOCK_IPRAW6   </td> </tr>
      <tr> <td> @ref Sn_MR_MACRAW                                 </td> <td> @ref SOCK_MACRAW   </td> </tr>
    </table>

    @note If you want to use a SOCKETn option such as Sn_MR_MF, Sn_MR_ND, Sn_MR_MUTIL and etc, \n
         these options should be set before @ref Sn_CR_OPEN is performed.
    @note If you want to open a multicast UDP mode SOCKETn, \n
         You should set the multicast group with @ref _Sn_DIPR_ or @ref _Sn_DIP6R_ and @ref _Sn_DPORTR_ before @ref Sn_CR_OPEN is performed.
    @sa _Sn_CR_, _Sn_MR_, _Sn_SR_, _Sn_PORTR_, _Sn_DIPR_, _Sn_DIP6R_, _Sn_DPORTR_,
    @sa getSn_CR(), setSn_CR(), getSn_MR(), setSn_MR(), getSn_SR(), getSn_PORTR(), setSn_PORTR(), getSn_DIPR(), setSn_DIPR(),
       getSn_DIP6R(), setSn_DIP6R(), getSn_DPORTR(), setSn_DPORTR()
*/
#define Sn_CR_OPEN           (0x01)

/**
    @brief Wait a connection request in <b>TCP SERVER</b> mode
    @details SOCKETn operates as a <b>TCP SERVER</b> and waits for a connection-request (SYN packet) \n
            with corresponding @ref _Sn_PORTR_ port number from any <b>TCP CLIENT</b> \n
            The @ref _Sn_SR_ is changed from @ref SOCK_INIT to @ref SOCK_LISTEN. \n
            When a <b>TCP CLIENT</b> connection request is successfully accepted,\n
            the @ref _Sn_SR_ is changed from @ref SOCK_LISTEN to @ref SOCK_ESTABLISHED \n
            and the @ref Sn_IR_CON is set.\n
            But when a <b>TCP CLIENT</b> connection request is failed, \n
            @ref Sn_IR_TIMEOUT is set and @ref _Sn_SR_ is changed to SOCK_CLOSED.
    @note This is valid only in TCP mode such as @ref Sn_MR_TCP4, @ref Sn_MR_TCP6, and @ref Sn_MR_TCPD.
    @sa _Sn_CR_, _Sn_MR_, _Sn_SR_, _Sn_PORTR_
    @sa getSn_CR(), setSn_CR(), getSn_MR(), setSn_MR(), getSn_SR(), getSn_PORTR(), setSn_PORTR()
*/
#define Sn_CR_LISTEN         (0x02)

/**
    @brief Send a connection request in <b>TCP CLIENT</b> mode
    @details To establish a connection, a connect-request (SYN packet) is sent to <b>TCP SERVER</b> set by @ref _Sn_DIPR_ & @ref _Sn_DPORTR_.\n
            If the connect-request is successful accepted by a <b>TCP SERVER</b>, \n
            the @ref _Sn_SR_ is changed to @ref SOCK_ESTABLISHED and the @ref Sn_IR_CON is set. \n
            The connect-request fails in the following three cases, \n
            and @ref _Sn_SR_ is changed to @ref SOCK_CLOSED.\n\n
    1. Until a ARP timeout is occurred (@ref Sn_IR_TIMEOUT = 1), a destination hardware address can not be acquired through the ARP-process.\n
    2. Until a TCP tmeout occurred (@ref Sn_IR_TIMEOUT = 1), a SYN/ACK packet is not received from the server\n
    3. When a RST packet is received instead of a SYN/ACK packet \n

    @note This is valid only in TCP mode such as @ref Sn_MR_TCP4 and @ref Sn_MR_TCPD.
    @sa _Sn_CR_, _Sn_MR_, _Sn_SR_, _Sn_DIPR_, _Sn_DPORTR_, Sn_CR_CONNECT6, _Sn_IR_, _Sn_IRCLR_
    @sa getSn_CR(), setSn_CR(), getSn_MR(), setSn_MR(), getSn_SR(), getSn_DIPR(), setSn_DIPR(), getSn_DPORTR(), setSn_DPORTR(), getSn_IR(), setSn_IRCLR()
*/
#define Sn_CR_CONNECT        (0x04)

/**
    @brief Send connection request in <b>TCP CLIENT</b> mode
    @details To establish a connection, a connect-request (SYN packet) is sent to <b>TCP SERVER</b> set by @ref _Sn_DIP6R_ & @ref _Sn_DPORTR_.\n
            If the connect-request is successful accepted by a <b>TCP SERVER</b>, \n
            the @ref _Sn_SR_ is changed to @ref SOCK_ESTABLISHED and the @ref Sn_IR_CON is set. \n
            The connect-request fails in the following three cases, and @ref _Sn_SR_ is changed @ref SOCK_CLOSED.\n
     1. Until a ARP timeout is occurred (@ref Sn_IR_TIMEOUT = 1), a destination hardware address can not be acquired through the ARP-process.\n
     2. Until a TCP tmeout occurred (@ref Sn_IR_TIMEOUT = 1), a @b SYN/ACK packet is not received from the server\n
     3. When a RST packet is received instead of a @b SYN/ACK packet \n

    @note This is valid only in TCP mode such as @ref Sn_MR_TCP6 and @ref Sn_MR_TCPD.
    @sa _Sn_CR_, _Sn_MR_, _Sn_SR_, _Sn_DIP6R_, _Sn_DPORTR_, Sn_CR_CONNECT, _Sn_IR_, _Sn_IRCLR_
    @sa getSn_CR(), setSn_CR(), getSn_MR(), setSn_MR(), getSn_SR(), getSn_DIP6R(), setSn_DIP6R(), getSn_DPORTR(), setSn_DPORTR(), getSn_IR(), setSn_IRCLR()
*/
#define Sn_CR_CONNECT6       (0x84)

/**
    @brief Send a disconnect request in TCP mode
    @details Regardless of <b>TCP SERVER</b> or <b>TCP CLIENT</b>, \n
            @ref Sn_CR_DISCON processes the disconnect-process (Active or Passive close).\n
            When the disconnect-process is successful (that is, FIN/ACK packet is received successfully from/to each other),\n
            @ref _Sn_SR_ is changed to @ref SOCK_CLOSED.\n
            Otherwise, @ref Sn_IR_TIMEOUT is set and then @ref _Sn_SR_ is changed to @ref SOCK_CLOSED.
     - Active close
       It transmits first a disconnect-request(FIN packet) to the connected peer, and waits to receive two FIN/ACK and FIN packet from the peer. \n
       If two FIN/ACK and FIN packet is received successfully, @ref Sn_IR_DISCON is set and @ref _Sn_SR_ is changed @ref SOCK_CLOSED.
     - Passive close
       When a FIN packet is first received from the peer, the FIN/ACK packet is replied back to the peer and @ref Sn_IR_DISCON is set.\n
       And then, a FIN packet is sent by @ref Sn_CR_DISCON to the peer, and waits to receive the FIN/ACK packet from the peer. \n
       If the FIN/ACK packet is received successfully from the peer, @ref _Sn_SR_ is changed to @ref SOCK_CLOSED.

    @note It is valid only in TCP mode such as @ref Sn_MR_TCP4, @ref Sn_MR_TCP6, and @ref Sn_MR_TCPD.
    @sa _Sn_CR_, _Sn_MR_, _Sn_SR_, _Sn_IR_, _Sn_IRCLR_, Sn_IR_DISCON, Sn_IR_TIMEOUT
    @sa getSn_CR(), setSn_CR(), getSn_MR(), setSn_MR(), getSn_SR(), getSn_IR(), setSn_IRCLR()
*/
#define Sn_CR_DISCON         (0x08)

/**
    @brief Release or Close SOCKETn
    @details In TCP mode, @ref Sn_CR_CLOSE force to close a SOCKETn without the disconnect-process.\n
            In other SOCKETn mode, @ref Sn_CR_CLOSE just closes a SOCKET.\n
    @note @ref _Sn_SR_ can be changed from any status to @ref SOCK_CLOSED by @ref Sn_CR_CLOSE.
    @sa _Sn_CR_, _Sn_MR_, _Sn_SR_, Sn_CR_DISCON
    @sa getSn_CR(), setSn_CR(), getSn_MR(), setSn_MR(), getSn_SR()
*/
#define Sn_CR_CLOSE          (0x10)

/**
    @brief Send Data
    @details @ref Sn_CR_SEND send the saved data from @ref _Sn_TX_RD_ to @ref _Sn_TX_WR_ in the SOCKETn TX buffer \n
            to the destination specified by @ref _Sn_DIPR_ or @ref _Sn_DIP6R_, and @ref _Sn_DPORTR_.\n
     - TCP mode(@ref Sn_MR_TCP4, @ref Sn_MR_TCP6,  @ref Sn_MR_TCPD)
       If it starts to be sent the data by @ref Sn_CR_SEND, @ref Sn_IR_SENDOK is set. \n
       And after sending the data, if the ACK to the sent data can not be received during @ref _Sn_RTR_, \n
       the sent data can be retransmitted as many as @ref _Sn_RCR_. \n
       During the retransmission, \n
       If the ACK is received, @ref _Sn_TX_FSR_ is increased as many as the sent data size, \n
       Otherwise, @ref Sn_IR_TIMEOUT is set and @ref _Sn_SR_ is changed to @ref SOCK_CLOSED.
     - UDP mode(@ref Sn_MR_UDP4, @ref Sn_MR_UDPD) & IPRAW mode(@ref Sn_MR_IPRAW4)
       It first sends a ARP-request to a destination specified with @ref _Sn_DIPR_ before it starts to be sent data by @ref Sn_CR_SEND. \n
       If the ARP-reply can not be received during @ref _Sn_RTR_, the ARP-request can be retransmitted as many as @ref _Sn_RCR_. \n
       During the retransmission, \n
       If the ARP-reply is received and @ref Sn_IR_SENDOK is set, it starts to send data and then @ref _Sn_TX_FSR_ is increased as many as the sent data size. \n
       Otherwise, @ref Sn_IR_TIMEOUT is set but @ref _Sn_SR_ is not changed.
     - MACRAW mode(@ref Sn_MR_MACRAW)
       It just start to send data and @ref Sn_IR_SENDOK is set.

    @note Data size to be sent is calculated by the absolute difference between @ref _Sn_TX_WR_ and @ref _Sn_TX_RD_. \n
         In TCP or UDP mode, It can not be sent bigger data than @ref _Sn_TX_FSR_.\n
         In IPRAW or Macraw case, it can not be sent bigger data than MTU(Maximum Transmit Unit).
    @note In TCP or MACRAW mode, It can send data to a destination address whether IPv4 or IPv6. \n
         In UDP or IPRAW mode, It can send data only to a destination IPv4 address. \n
         For Sending to IPv6 address, It can be used with @ref Sn_CR_SEND6.
    @sa _Sn_CR_, _Sn_MR_, _Sn_DIPR_, _Sn_DIP6R_, _Sn_DPORTR_, _Sn_IR_, _Sn_IRCLR_, _Sn_TX_FSR_, _Sn_TX_WR_, _Sn_TX_RD_
    @sa getSn_CR(), setSn_CR(), getSn_MR(), setSn_MR(), getSn_SR(), getSn_DIPR(), setSn_DIPR(), getSn_DIP6R(), setSn_DIP6R(), \n
       getSn_DPORTR(), setSn_DPORTR(), getSn_IR(), setSn_IRCLR(), getSn_TX_FSR(), getSn_TX_WR(), setSn_TX_WR(), getSn_TX_RD()
*/
#define Sn_CR_SEND           (0x20)

/**
    @brief Send Data
    @details @ref Sn_CR_SEND6 sends the saved data from @ref _Sn_TX_RD_ to @ref _Sn_TX_WR_ in the SOCKETn TX buffer \n
            to the destination specified by @ref _Sn_DIP6R_, and @ref _Sn_DPORTR_.\n
     - TCP mode(@ref Sn_MR_TCP4, @ref Sn_MR_TCP6,  @ref Sn_MR_TCPD) & MACRAW mode(@ref Sn_MR_MACRAW)
       @ref Sn_CR_SEND6 is not recommended. In this case, Use @ref Sn_CR_SEND.
     - UDP mode(@ref Sn_MR_UDP6, @ref Sn_MR_UDPD) & IPRAW mode(@ref Sn_MR_IPRAW6)
       It first send a neighbor solicitation NS) of ICMPv6 to a destination specified with @ref _Sn_DIP6R_ \n
       before it starts to be sent data by @ref Sn_CR_SEND. \n
       If the neighbor advertisement(NA) of ICMPv6 can not be received during @ref _Sn_RTR_, \n
       the NS can be retransmitted as many as @ref _Sn_RCR_. \n
       During the retransmission, \n
       If the NA is received and @ref Sn_IR_SENDOK is set, \n
       it starts to send data and then @ref _Sn_TX_FSR_ is increased as many as the sent data size. \n
       Otherwise, @ref Sn_IR_TIMEOUT is set but @ref _Sn_SR_ is not changed.

    @note Data size to be sent is calculated by the absolute difference between @ref _Sn_TX_WR_ and @ref _Sn_TX_RD_. \n
         In TCP or UDP mode, It can not be sent bigger data than @ref _Sn_TX_FSR_.\n
         In IPRAW or Macraw case, it can not be sent bigger data than MTU(Maximum Transmit Unit).
    @note In UDP or IPRAW mode, It can send data only to a destination IPv6 address. \n
         For Sending to IPv4 address, It can be sent by @ref Sn_CR_SEND.
    @sa _Sn_CR_, _Sn_MR_, _Sn_DIP6R_, _Sn_DPORTR_, _Sn_IR_, _Sn_IRCLR_, _Sn_TX_FSR_, _Sn_TX_WR_, _Sn_TX_RD_
    @sa getSn_CR(), setSn_CR(), getSn_MR(), setSn_MR(), getSn_SR(), getSn_DIP6R(), setSn_DIP6R(), getSn_DPORTR(), setSn_DPORTR(), \n
       getSn_IR(), setSn_IRCLR(), getSn_TX_FSR(), getSn_TX_WR(), setSn_TX_WR(), getSn_TX_RD()
*/
#define Sn_CR_SEND6         (0xA0)

/**
    @brief Send keep alive message
    @details @ref Sn_CR_SEND_KEEP checks whether the connection is established or not by sending 1 byte KA(Keep Alive) packet.\n
            If the destination can not respond to the KA packet during the time set by @ref _Sn_RTR_ and @ref _Sn_RCR_, \n
            the connection is terminated, @ref Sn_IR_TIMEOUT is set and then @ref _Sn_SR_ is changed @ref SOCK_CLOSED.
    @note It is valid only after sending data over 1 byte in TCP mode such as @ref Sn_MR_TCP4, @ref Sn_MR_TCP6, and @ref Sn_MR_TCPD.
    @sa _Sn_CR_, _Sn_MR_, _Sn_SR_, _Sn_IR_, _Sn_IRCLR_, _Sn_RTR_, _Sn_RCR_, _Sn_KPALVTR_
    @sa getSn_CR(), setSn_CR(), getSn_MR(), setSn_MR(), getSn_SR(), getSn_IR(), setSn_IRCLR(), \n
          getSn_RTR(), setSn_RTR(), getSn_RCR(), setSn_RCR(), getSn_KPALVTR(), getSn_KPALVTR()
*/
#define Sn_CR_SEND_KEEP      (0x22)

/**
    @brief Receive data
    @details @ref Sn_CR_RECV reads the saved from @ref _Sn_RX_RD_ to @ref _Sn_RX_WR_ data in SOCKETn RX buffer.\n
            When a data is saved in the SOCKETn RX buffer, \n
            @ref Sn_IR_RECV is set and @ref _Sn_RX_RSR_ is increased as many as the saved data size.\n
            The total size of saved data is calculated by the absolute difference between @ref _Sn_RX_WR_ and @ref _Sn_RX_RD_,\n
            and it can be checked thru @ref _Sn_RX_RSR_.\n
            After reading data, @ref _Sn_RX_RD_ should be increased as many as the read size before @ref Sn_CR_RECV is performed.\n
            After @ref Sn_CR_RECV, @ref _Sn_RX_RSR_ is decreased as many as the read size.\n
            If @ref _Sn_RX_RSR_ is remained still at none-zero, @ref Sn_IR_RECV is set again.
    @sa _Sn_CR_, _Sn_MR_, _Sn_IR_, _Sn_IRCLR_, _Sn_RX_RD_, _Sn_RX_WR_, _Sn_RX_RSR_
    @sa getSn_CR(), setSn_CR(), getSn_MR(), setSn_MR(), getSn_IR(), getSn_IRCLR(), \n
       getSn_RX_RD(), setSn_RX_RD(), getSn_RX_TX(), getSn_RX_RSR()
*/
#define Sn_CR_RECV           (0x40)


/* Sn_IR values */
/**
    @brief SEND OK Interrupt
    @details @ref Sn_IR_SENDOK is set when it is started to be sent data by @ref Sn_CR_SEND.
    @note Even though @ref Sn_IR_SENDOK is set, it does not means that the destination receives data successfully.\n
           - In TCP mode, The sent data maybe still transmitting or retransmitting. \n
           - In other modes, The sent data maybe lost by media collision or an other reason such as network traffic.
    @sa _Sn_IR_, _Sn_IRCLR_, Sn_CR_SEND
    @sa getSn_IR(), setSn_IRCLR(), getSn_CR(), setSn_CR()
*/
#define Sn_IR_SENDOK         (0x10)

/**
    @brief TIMEOUT Interrupt
    @details @ref Sn_IR_TIMEOUT is set when a timeout occurs in ARP and ND process or TCP retransmission.
    @note In TCP mode, If it is set, @ref _Sn_SR_ is changed to @ref SOCK_CLOSED. \n
         In other modes, _Sn_SR_ is still remained at the previous status.
    @sa _Sn_IR_, _Sn_IRCLR_, Sn_CR_CONNECT, Sn_CR_CONNECT6, Sn_CR_SEND
    @sa getSn_IR(), setSn_IRCLR(), getSn_CR(), setSn_CR()
*/
#define Sn_IR_TIMEOUT        (0x08)

/**
    @brief RECV Interrupt
    @details @ref Sn_IR_RECV is set whenever data is received from a peer, \n
            or if @ref _Sn_RX_RSR_ is still at none-zero whenever @ref Sn_CR_RECV is performed.
    @sa _Sn_IR_, _Sn_IRCLR_, Sn_CR_RECV
    @sa getSn_IR(), setSn_IRCLR(), getSn_CR(), setSn_CR()
*/
#define Sn_IR_RECV           (0x04)

/**
    @brief DISCON Interrupt
    @details @ref Sn_IR_DISCON is set when a FIN or FIN/ACK packet is received from the connected peer.
    @note When first a FIN packet is received from the connected peer and @ref _Sn_SR_ is changed to SOCK_CLOSE_WAIT, \n
         you should perform @ref Sn_CR_DISCON for a successful disconnect. \n
         If the disconnect-process is completed or failed, @ref _Sn_SR_ is changed to @ref SOCK_CLOSED.
    @note It is valild only in TCP mode such as @ref Sn_MR_TCP4, @ ref Sn_MR_TCP6 and @ref Sn_MR_TCPD.
    @sa _Sn_IR_, _Sn_IRCLR_, Sn_IR_DISCON, _Sn_SR_
    @sa getSn_IR(), setSn_IRCLR(), getSn_CR(), setSn_CR(), getSn_SR()
*/
#define Sn_IR_DISCON         (0x02)

/**
    @brief CONNECT Interrupt
    @details @ref Sn_IR_CON is set once the connection with a peer is established and @ref _Sn_SR_ is changed to @ref SOCK_ESTABLISHED.
    @note It is valid only in TCP mode such as @ref Sn_MR_TCP4, @ ref Sn_MR_TCP6 adn @ref Sn_MR_TCPD.
    @sa _Sn_IR_, _Sn_IRCLR_, _Sn_SR_
    @sa getSn_IR(), setSn_IRCLR(), getSn_SR()
*/
#define Sn_IR_CON            (0x01)

/* Sn_SR values */
/**
    @brief SOCKETn Closed status
    @details @ref SOCK_CLOSED indicates that SOCKETn is closed and released.\n
            It is set when @ref Sn_CR_DISCON , @ref Sn_CR_CLOSE is performed, or when @ref Sn_IR_TIMEOUT is set.\n
            It can be changed to @ref SOCK_CLOSED regardless of previous status.
    @sa _Sn_SR_, _Sn_CR_, _Sn_IR_, _Sn_IRCLR_, Sn_IR_TIMEOUT
    @sa getSn_SR(), getSn_CR(), setSn_CR(), getSn_IR(), setSn_IRCLR()
*/
#define SOCK_CLOSED          (0x00)

/**
    @brief TCP SOCKETn initialized status
    @details @ref SOCK_INIT indicates SOCKETn is opened with TCP mode such as @ref Sn_MR_TCP4, @ref Sn_MR_TCP6, and @ref Sn_MR_TCP6.\n
            @ref _Sn_SR_ is changed from @ref SOCK_CLOSED to @ref SOCK_INIT when @ref Sn_CR_OPEN is performed in TCP mode.\n
            In @ref SOCK_INIT status, @ref Sn_CR_LISTEN for operating a <b>TCP SERVER</b> \n
            or @ref Sn_CR_CONNECT / @ref Sn_CR_CONNECT6 for operating a <b>TCP CLIENT</b> can be performed.
    @note It is valid only in TCP mode.
    @sa _Sn_SR_, _Sn_CR_, _Sn_MR_
    @sa getSn_SR, getSn_CR(), setSn_CR(), getSn_MR(), setSn_MR()
*/
#define SOCK_INIT            (0x13)

/**
    @brief TCP SOCKETn Listen status
    @details @ref SOCK_LISTEN indicates SOCKETn is operating as <b>TCP SERVER</b> mode \n
            and waiting for connection-request (SYN packet) from a peer (<b>TCP CLIENT</b>).\n
            @ref _Sn_SR_ is changed to @ref SOCK_SYNRECV when the connection-request(SYN packet) is successfully accepted \n
            and It is changed from @ref SOCK_SYNRECV to @ref SOCK_ESTABLISHED \n
            when the SYN/ACK packet is sent successfully to the peer and then the ACK packet of SYN/ACK is received successfully.\n
            Otherwise, it is changed to @ref SOCK_CLOSED and @ref Sn_IR_TIMEOUT is set.
    @note It is valid only in TCP mode such as @ref Sn_MR_TCP4, @ref Sn_MR_TCP6, and @ref Sn_MR_TCP6..
    @sa _Sn_SR_, _Sn_CR_, _Sn_IR_, _Sn_IRCLR_, _Sn_MR_
    @sa getSn_SR, getSn_CR(), getSn_IR(), setSn_IRCLR(), setSn_CR(), getSn_MR(), setSn_MR()
*/
#define SOCK_LISTEN          (0x14)

/**
    @brief TCP Connection Request status
    @details @ref SOCK_SYNSENT indicates TCP SOCKETn sent the connect-request packet(SYN packet)\n
            to the peer specified by @ref _Sn_DIPR_ / @ref _Sn_DIP6R_ and @ref _Sn_DPORTR_.\n
            It is temporarily shown when @ref _Sn_SR_ is changing from @ref SOCK_INIT to @ref SOCK_ESTABLISHED by @ref Sn_CR_CONNECT or @ref Sn_CR_CONNECT6.\n
            When the connect-accept packet (SYN/ACK packet) is received from the peer at @ref SOCK_SYNSENT and the ACK packet of SYN/ACK is sent successfully, \n
            @ref _Sn_SR_ is changed to @ref SOCK_ESTABLISHED.\n
            Otherwise, it is changed to @ref SOCK_CLOSED and @ref Sn_IR_TIMEOUT is set.
    @note It is valid only in TCP mode such as @ref Sn_MR_TCP4, @ref Sn_MR_TCP6, and @ref Sn_MR_TCP6..
    @sa _Sn_SR_, _Sn_CR_, _Sn_IR_, _Sn_IRCLR_, _Sn_MR_
    @sa getSn_SR, getSn_CR(), getSn_IR(), setSn_IRCLR(), setSn_CR(), getSn_MR(), setSn_MR()
*/
#define SOCK_SYNSENT         (0x15)

/**
    @brief TCP Connection Accept status
    @details @ref SOCK_SYNRECV indicates TCP SOCKETn is successfully received the connect-request packet (SYN packet) from a peer.\n
            It is temporarily shown when @ref _Sn_SR_ is changing from @ref SOCK_LISTEN to @ref SOCK_ESTABLISHED by the SYN packet\n
            If SOCKETn sends the response (SYN/ACK  packet) to the peer successfully and the ACK packet of SYS/ACK is received successfully,\n
            @ref _Sn_SR_ is changed to @ref SOCK_ESTABLISHED. \n
            Otherwise, @ref _Sn_SR_ is changed to @ref SOCK_CLOSED and @ref Sn_IR_TIMEOUT is set.
    @note It is valid only in TCP mode such as @ref Sn_MR_TCP4, @ref Sn_MR_TCP6, and @ref Sn_MR_TCP6..
    @sa _Sn_SR_, _Sn_CR_, _Sn_IR_, _Sn_IRCLR_, _Sn_MR_
    @sa getSn_SR, getSn_CR(), getSn_IR(), setSn_IRCLR(), setSn_CR(), getSn_MR(), setSn_MR()
*/
#define SOCK_SYNRECV         (0x16)

/**
    @brief TCP SOCKETn Established status
    @details @ref SOCK_ESTABLISHED indicates TCP SOCKETn is connected successfully with a peer.\n
            when the <b>TCP SERVER</b> processes the SYN packet from the <b>TCP CLIENT</b> during @ref SOCK_LISTEN or \n
            when the <b>TCP CLIENT</b> performs successfully @ref Sn_CR_CONNECT or @ref Sn_CR_CONNECT6,\n
            @ref _Sn_SR_ is changed to @ref SOCK_ESTABLISHED and @ref Sn_IR_CON is set. \n
            During @ref SOCK_ESTABLISHED, a DATA packet can be sent to or received from the peer by @ref Sn_CR_SEND or @ref Sn_CR_RECV.  \n
            If the DATA/ACK packet is not received from the peer during data re-transmission, @ref Sn_IR_TIMEOUT is set and @ref _Sn_SR_ is changed to @ref SOCK_CLOSED.\n
            Otherwise, @ref _Sn_SR_ is still at @ref SOCK_ESTABLISHED.
    @note In <b>TCP SERVER</b>, \n
         You can check the IPv4/IPv6 address and port number of connected peer thru @ref _Sn_DIPR_, @ref _Sn_DIP6R_, and @ref _Sn_DPORTR_ respectively.
    @note It is valid only in TCP mode such as @ref Sn_MR_TCP4, @ref Sn_MR_TCP6, and @ref Sn_MR_TCP6.
    @sa _Sn_SR_, _Sn_CR_, _Sn_IR_, _Sn_IRCLR_, _Sn_MR_, _Sn_DIPR_, _Sn_DIP6R_, _Sn_DPORTR_
    @sa getSn_SR, getSn_CR(), getSn_IR(), setSn_IRCLR(), setSn_CR(), getSn_MR(), setSn_MR(),
       getSn_DIPR(), setSn_DIPR(), getSn_DIP6R(), setSn_DIP6R(), getSn_DPORTR(), setSn_DPORTR().
*/
#define SOCK_ESTABLISHED     (0x17)

/**
    @brief TCP SOCKETn Closing status
    @details @ref SOCK_FIN_WAIT indicates TCP mode SOCKETn waits until the disconnect-process is completed. \n
            It is temporarily shown in disconnect-process such as active-close. \n
            When the disconnect-process is successfully completed or when @ref Sn_IR_TIMEOUT is set,\n
            @ref _Sn_SR_ is changed to @ref SOCK_CLOSED.
    @note It is valid only in TCP mode such as @ref Sn_MR_TCP4, @ref Sn_MR_TCP6, and @ref Sn_MR_TCPD.
    @sa _Sn_SR_, _Sn_CR_, _Sn_IR_, _Sn_IRCLR_, _Sn_MR_, SOCK_TIME_WAIT, SOCK_LAST_ACK
    @sa getSn_SR, getSn_CR(), getSn_IR(), setSn_IRCLR(), setSn_CR(), getSn_MR(), setSn_MR()

*/
#define SOCK_FIN_WAIT        (0x18)

/**
    @brief TCP SOCKETn Closing status
    @details @ref SOCK_TIME_WAIT indicates TCP SOCKETn waits until the disconnect-process is completed.\n
            It is temporarily shown in disconnect-process such as active-close. \n
            When the disconnect-process is successfully completed or when @ref Sn_IR_TIMEOUT is set,\n
            @ref _Sn_SR_ is changed to @ref SOCK_CLOSED.
    @note It is valid only in TCP mode such as @ref Sn_MR_TCP4, @ref Sn_MR_TCP6, and @ref Sn_MR_TCPD.
    @sa _Sn_SR_, _Sn_CR_, _Sn_IR_, _Sn_IRCLR_, _Sn_MR_, SOCK_FIN_WAIT, SOCK_LAST_ACK
    @sa getSn_SR, getSn_CR(), getSn_IR(), setSn_IRCLR(), setSn_CR(), getSn_MR(), setSn_MR()
*/
#define SOCK_TIME_WAIT       (0x1B)

/**
    @brief TCP SOCKETn Half Closing staus
    @details @ref SOCK_CLOSE_WAIT indicates TCP SOCKETn receives the disconnect-request (FIN packet) from the connected peer.\n
            It is a half-closing status, and a DATA packet can be still sent or received by @ref Sn_CR_SEND or @ref Sn_CR_RECV.\n
            If you do not have any more need to send or received a DATA packet, You can perform @ref Sn_CR_DISCON for a full-closing.
    @note If you have no need the successful closing, You maybe perform @ref Sn_CR_CLOSE.
    @note It is valid only in TCP mode such as @ref Sn_MR_TCP4, @ref Sn_MR_TCP6, and @ref Sn_MR_TCPD.
    @sa _Sn_SR_, _Sn_CR_, _Sn_IR_, _Sn_IRCLR_, _Sn_MR_
    @sa getSn_SR, getSn_CR(), getSn_IR(), setSn_IRCLR(), setSn_CR(), getSn_MR(), setSn_MR()
*/
#define SOCK_CLOSE_WAIT      (0x1C)

/**
    @brief TCP SOCKETn Closing status
    @details @ref SOCK_LAST_ACK indicates TCP SOCKETn waits until the disconnect-process is completed.\n
            It is temporarily shown in disconnect-process such as active-close and passive-close.\n
            When the disconnect-process is successfully completed or when @ref Sn_IR_TIMEOUT is set,\n
            @ref _Sn_SR_ is changed to @ref SOCK_CLOSED.
    @note It is valid only in TCP mode such as @ref Sn_MR_TCP4, @ref Sn_MR_TCP6, and @ref Sn_MR_TCPD.
    @sa _Sn_SR_, _Sn_CR_, _Sn_IR_, _Sn_IRCLR_, _Sn_MR_, SOCK_FIN_WAIT, SOCK_TIME_WAIT
    @sa getSn_SR, getSn_CR(), getSn_IR(), setSn_IRCLR(), setSn_CR(), getSn_MR(), setSn_MR()
*/
#define SOCK_LAST_ACK        (0x1D)

/**
    @brief UDP SOCKETn status
    @details @ref SOCK_UDP indicates SOCKETn is opened in UDP mode such as @ref Sn_MR_UDP4, @ref Sn_MR_UDP6, and @ref Sn_MR_UDPD.\n
            @ref _Sn_SR_ is changed from @ref SOCK_CLOSED to @ref SOCK_INIT when @ref Sn_CR_OPEN is performed in UDP mode.\n
            Unlike TCP mode, during @ref SOCK_UDP, \n
            a DATA packet can be sent to or received from a peer by @ref Sn_CR_SEND / @ref Sn_CR_SEND6 or @ref Sn_CR_RECV without a connect-process.\n
            Before a DATA packet is sent by @ref Sn_CR_SEND / @ref Sn_CR_SEND6,\n
            the ARP is requested to the peer specified by @ref _Sn_DIPR_ / @ref _Sn_DIP6R_.\n
            In ARP processing, @ref _Sn_SR_ is stll at @ref SOCK_UDP even if @ref Sn_IR_TIMEOUT is set.\n
            If you do not have any more need to send or received a DATA packet, \n
            You can perform @ref Sn_CR_CLOSE and @ref _Sn_SR_ is changed to @ref SOCK_CLOSED.
    @note It is valid only in UDP mode such as @ref Sn_MR_UDP4, @ref Sn_MR_UDP6, and @ref Sn_MR_UDPD.
    @sa _Sn_SR_, _Sn_CR_, _Sn_IR_, _Sn_IRCLR_, _Sn_MR_, _Sn_DIPR_, _Sn_DIP6R_
    @sa getSn_SR, getSn_CR(), getSn_IR(), setSn_IRCLR(), setSn_CR(), getSn_MR(), setSn_MR(),
       getSn_DIPR(), setSn_DIPR(), getSn_DIP6R(), setSn_DIP6R()
*/
#define SOCK_UDP             (0x22)

/**
    @brief IPRAW4 SOCKETn mode
    @details @ref SOCK_IPRAW4(= @ref SOCK_IPRAW) SOCKETn indicates SOCKETn is opened as IPv4 RAW mode.\n
            @ref _Sn_SR_ is changed from @ref SOCK_CLOSED to @ref SOCK_IPRAW4 when @ref Sn_CR_OPEN is performed with @ref Sn_MR_IPRAW4. \n
            A DATA packet can be send to or received from a peer without a connection like as @ref SOCK_UDP. \n
            Before a DATA packet is sent by @ref Sn_CR_SEND, \n
            the ARP is requested to the peer specified by @ref _Sn_DIPR_.\n
            In ARP processing, @ref _Sn_SR_ is still at @ref SOCK_IPRAW4 even if @ref Sn_IR_TIMEOUT is set.\n
            IPRAW4 SOCKETn can receive only the packet specified by @ref _Sn_PNR_, and it discards the others packets.\n
            If you do not have any more need to send or received a DATA packet, \n
            You can perform @ref Sn_CR_CLOSE and @ref _Sn_SR_ is changed to @ref SOCK_CLOSED.
    @note It is valid only in IPRAW4 mode such as @ref Sn_MR_IPRAW4.
    @sa _Sn_SR_, _Sn_CR_, _Sn_IR_, _Sn_IRCLR_, _Sn_MR_, _Sn_DIPR_, _Sn_PNR_
    @sa getSn_SR, getSn_CR(), getSn_IR(), setSn_IRCLR(), setSn_CR(), getSn_MR(), setSn_MR(),
       getSn_DIPR(), setSn_DIPR(), getSn_PNR(), setSn_PNR()
*/
#define SOCK_IPRAW4          (0x32)
#define SOCK_IPRAW           (SOCK_IPRAW4)   ///< Refer to @ref SOCK_IPRAW4.

/**
    @brief IPRAW6 SOCKETn mode
    @details @ref SOCK_IPRAW6 SOCKETn indicates SOCKETn is opened as IPv6 RAW mode.\n
            @ref _Sn_SR_ is changed from @ref SOCK_CLOSED to @ref SOCK_IPRAW6 when @ref Sn_CR_OPEN is performed with @ref Sn_MR_IPRAW6. \n
            A DATA packet can be send to or received from a peer without a connection like as @ref SOCK_UDP.\n
            Before a DATA packet is sent by @ref Sn_CR_SEND6, \n
            the ICMPv6 NS is requested to the peer specified by @ref _Sn_DIPR_ or @ref _Sn_DIP6R_.\n
            In ND(Neighbor Discovery) is processing,\n
            @ref _Sn_SR_ is still at @ref SOCK_IPRAW6 even if @ref Sn_IR_TIMEOUT is set.\n
            IPRAW6 SOCKETn can receive only the packet specified by @ref _Sn_PNR_, and it discards the others packets.\n
            If you do not have any more need to send or received a DATA packet, \n
            You can perform @ref Sn_CR_CLOSE and @ref _Sn_SR_ is changed to @ref SOCK_CLOSED.
    @note It is valid only in IPRAW6 mode such as @ref Sn_MR_IPRAW6.
    @sa _Sn_SR_, _Sn_CR_, _Sn_IR_, _Sn_IRCLR_, _Sn_MR_, _Sn_DIP6R_, _Sn_PNR_
    @sa getSn_SR, getSn_CR(), getSn_IR(), setSn_IRCLR(), setSn_CR(), getSn_MR(), setSn_MR(),
       getSn_DIP6R(), setSn_DIP6R(), getSn_PNR(), setSn_PNR()
*/
#define SOCK_IPRAW6          (0x33)

/**
    @brief MACRAW SOCKETn mode
    @details @ref SOCK_MACRAW indicates SOCKET0 is opened as MACRAW mode.\n
            @ref _Sn_SR_ is changed from @ref SOCK_CLOSED to @ref SOCK_MACRAW when @ref Sn_CR_OPEN command is ordered with @ref Sn_MR_MACRAW.\n
            MACRAW SOCKET0 can be sent or received a pure Ethernet frame packet to/from any peer.
    @note  It is valid only in SOCKET0.
    @sa _Sn_SR_, _Sn_CR_, _Sn_IR_, _Sn_IRCLR_, _Sn_MR_
    @sa getSn_SR, getSn_CR(), getSn_IR(), setSn_IRCLR(), setSn_CR(), getSn_MR(), setSn_MR(),
*/
#define SOCK_MACRAW          (0x42)

/* Sn_ESR values */
/**
    @brief SOCKETn Extended Status : TCP Mode
    @details @ref Sn_ESR_TCPM masks the TCPM bit of @ref _Sn_ESR_. \n
            The masked bit values are as following. \n
            - @ref Sn_ESR_TCPM_IPV4
            - @ref Sn_ESR_TCPM_IPV6
    @note It is useful to know the destination IP version when TCPD(@ref Sn_MR_TCPD) mode SOCKETn is operated as <b>TCP SERVER</b>.
    @sa _Sn_ESR_
    @sa getSn_ESR()
*/
#define Sn_ESR_TCPM          (1<<2)

/**
    @brief TCP SOCKETn IP version - IPv4
    @details @ref Sn_ESR_TCPM_IPV4 indicates TCP SOCKETn is operated on IPv4 .
    @sa _Sn_ESR_, Sn_ESR_TCPM_IPV6
    @sa getSn_ESR()
*/
#define Sn_ESR_TCPM_IPV4     (0<<2)

/**
    @brief TCP SOCKETn IP version - IPv6
    @details @ref Sn_ESR_TCPM_IPV6 indicates TCP SOCKETn is operated on IPv6 .
    @sa _Sn_ESR_, Sn_ESR_TCPM_IPV4
    @sa getSn_ESR()
*/
#define Sn_ESR_TCPM_IPV6     (1<<2)

/**
    @brief SOCKETn Extended Status : TCP Operation Mode
    @details @ref Sn_ESR_TCPOP masks the TCPOP bit of @ref _Sn_ESR_. The masked bit values are as following. \n
              - @ref Sn_ESR_TCPOP_SVR
              - @ref Sn_ESR_TCPOP_CLT
    @note It is useful to check TCP mode SOCKETn is operated as whether <b>TCP SERVER</b> or <b>TCP CLIENT</b>.
    @sa _Sn_ESR_
    @sa getSn_ESR()
*/
#define Sn_ESR_TCPOP         (1<<1)

/**
    @brief TCP SOCKETn Operation Mode - <b>TCP SERVER</b>
    @details @ref Sn_ESR_TCPOP_SVR indicates TCP mode SOCKET n is operated as <b>TCP SERVER</b>
    @note It is valid only in TCP mode such as @ref Sn_MR_TCP4, @ref Sn_MR_TCP6, and @ref Sn_MR_TCPD.
    @sa _Sn_ESR_, Sn_ESR_TCPOP_CLT
    @sa getSn_ESR()
*/
#define Sn_ESR_TCPOP_SVR     (0<<1)

/**
    @brief TCP SOCKETn Operation Mode - <b>TCP CLIENT</b>
    @details @ref Sn_ESR_TCPOP_SVR indicates TCP mode SOCKET n is operated as <b>TCP CLIENT</b>
    @note It is valid only in TCP mode such as @ref Sn_MR_TCP4, @ref Sn_MR_TCP6, and @ref Sn_MR_TCPD.
    @sa _Sn_ESR_, Sn_ESR_TCPOP_SVR
    @sa getSn_ESR()
*/
#define Sn_ESR_TCPOP_CLT     (1<<1)

/**
    @brief SOCKETn Extended Status : Source IPv6 Address Type
    @details @ref Sn_ESR_IP6T masks the IP6T bit of @ref _Sn_ESR_. \n
            The masked bit values are as following. \n
              - @ref Sn_ESR_IP6T_LLA
              - @ref Sn_ESR_IP6T_GUA
    @note It is useful to check whether the connected peer IP address is LLA or GUA.
    @note It is valid only in TCP mode such as @ref Sn_MR_TCP6 and @ref Sn_MR_TCPD.
    @sa _Sn_ESR_
    @sa getSn_ESR()
*/
#define Sn_ESR_IP6T          (1<<0)

/**
    @brief Source IPv6 Address Type - LLA
    @details @ref Sn_ESR_IP6T_LLA indicates the source IPv6 Address is used as @ref _LLAR_
    @note It is valid only in TCP mode such as @ref Sn_MR_TCP6, and @ref Sn_MR_TCPD.
    @sa _Sn_ESR_, Sn_ESR_IP6T_GUA, _LLAR_
    @sa getSn_ESR(), getLLAR(), setLLAR()
*/
#define Sn_ESR_IP6T_LLA      (0<<0)

/**
    @brief Source IPv6 Address Type - LLA
    @details @ref Sn_ESR_IP6T_GUA indicates the source IPv6 Address is used as @ref _GUAR_
    @note It is valid only in TCP mode such as @ref Sn_MR_TCP6, and @ref Sn_MR_TCPD.
    @sa _Sn_ESR_, Sn_ESR_IP6T_LLA, _GUAR_
    @sa getSn_ESR(), getGUAR(), setGUAR()
*/
#define Sn_ESR_IP6T_GUA      (1<<0)

/* Sn_MR2 values */
/**
    @brief Destination Hardware Address Mode
    @details @ref Sn_MR2_DHAM masks the DHAM bit of @ref _Sn_MR2_.\n
            The masked bit values are as following.
              - @ref Sn_MR2_DHAM_AUTO
              - @ref Sn_MR2_DHAM_MANUAL
    @sa _Sn_MR2_
    @sa getSn_MR2(), setSn_MR2()
*/
#define Sn_MR2_DHAM          (1<<1)

/**
    @brief Destination Hardware Address Mode - AUTO
    @details @ref Sn_MR2_DHAM_AUTO sets the destination hardware address as the address acquired by ARP-process.
    @sa _Sn_MR2_, Sn_MR_DHAM_MANUAL, NETMR_DHAS
    @sa getSn_MR2(), setSn_MR2(), getNETMR(), setNETMR()
*/
#define Sn_MR2_DHAM_AUTO     (0<<1)

/**
    @brief Destination Hardware Address Mode - MANUAL
    @details @ref Sn_MR2_DHAM_MANUAL sets the destination hardware address as @ref _Sn_DHAR_.
    @sa _Sn_MR2_, Sn_MR_DHAM_MANUAL, NETMR_DHAS
    @sa getSn_MR2(), setSn_MR2(), getNETMR(), setNETMR()
*/
#define Sn_MR2_DHAM_MANUAL   (1<<1)

/**
    @brief Force ARP
    @details @ref Sn_MR2_FARP force to perform the ARP-process for acquiring the destination hardware address, before data communication\n
            0 : Normal \n
            1 : Force ARP
     - In TCP mode such as @ref Sn_MR_TCP4, @ref Sn_MR_TCP6, and @ref Sn_MR_TCPD
       If SOCKETn is operated as <b>TCP SERVER</b>, It sets the destination hardware address as the address
       acquired by the forced ARP-process before sending SYN/ACK packet.
     - In UDP mode such as @ref Sn_MR_UDP4, @ref Sn_MR_UDP6, and @ref Sn_MR_UDPD
       It sets the destination hardware address as the address acquired by the forced ARP-process whenever @ref Sn_CR_SEND or @ref Sn_CR_SEND6.
    @note When @ref Sn_MR2_DHAM_MANUAL and @ref Sn_MR2_FARP = '1', It sets the destination hardware address as @ref _Sn_DHAR_ even if the ARP-process is forced.
*/
#define Sn_MR2_FARP          (1<<0)


/*----------------------------For PHY Control-------------------------------*/

/**
    @ingroup Common_register_group_W6300
    @brief Basic Mode Control Register of Ethernet PHY [RW][0x3100]
    @details @ref PHYRAR_BMCR can be controlled by MDC/MDIO controller of @ref _WIZCHIP_. \n
            Each bit of @ref PHYRAR_BMCR is defined as the following.
    <table>
      <tr> <td>15</td>  <td>14</td> <td>13</td>  <td>12 </td>  <td>11</td>  <td>10  </td> <td>9  </td> <td>8  </td> <td>7   </td>  <td>6 ~ 0  </td> </tr>
      <tr> <td>RST</td> <td>LB</td> <td>SPD</td> <td>ANE</td> <td>PWDN</td> <td>ISOL</td> <td>RAN</td> <td>DPX</td> <td>COLT</td> <td>Reserved</td> </tr>
    </table>
     - @ref BMCR_RST
     - @ref BMCR_LB
     - @ref BMCR_SPD
     - @ref BMCR_ANE
     - @ref BMCR_PWDN
     - @ref BMCR_ISOL : Not supported.
     - @ref BMCR_REAN
     - @ref BMCR_DPX
     - @ref BMCR_COLT

    @note Its some bits have the same function as @ref _PHYCR0_ and @ref _PHYCR1_.\n
         It can control the Ethernet PHY with software, while @ref _PHYCR0_ \n
         and @ref _PHYCR1_ can control the Ethernet PHY with hardware.

    @sa PHYRAR_BMSR, _PHYRAR_, _PHYDIR_, _PHYDOR_, _PHYACR_, _PHYCR0_, _PHYCR1_
    @sa getPHYRAR(), setPHYRAR(), wiz_mdio_read(), wiz_mdio_write()
*/
#define PHYRAR_BMCR          (0x00)

//Basic mode status register, basic register
/**
    @ingroup Common_register_group_W6300
    @brief Basic Mode Status Register of Ethernet PHY [RO][0x7809]
    @details @ref PHYRAR_BMSR gets the status of Ethernet PHY through MDC/MDIO controller of @ref _WIZCHIP_. \n
            Each bit of @ref PHYRAR_BMSR is defined as the following.
    <table>
      <tr> <td>15</td> <td>14</td> <td>13</td> <td>12</td> <td>11</td> <td>10~7</td>
           <td>6 </td> <td>5 </td> <td>4 </td> <td>3 </td> <td>2 </td> <td>1   </td> <td>0</td> </tr>
      <tr> <td>100_T4  </td> <td>100_FDX     </td> <td>100_HDX    </td> <td>10_FDX     </td> <td>10_HDX       </td> <td>Reserved</td> <td>MF_SUP</td>
           <td>ANG_COMP</td> <td>REMOTE_FAULT</td> <td>ANG_ABILITY</td> <td>LINK_STATUS</td> <td>JABBER_DETECT</td> <td>EXT_CAPA</td> </tr>
    </table>
     - @ref BMSR_100_T4        : Not supported. Always 0
     - @ref BMSR_100_FDX
     - @ref BMSR_100_HDX
     - @ref BMSR_10_FDX
     - @ref BMSR_10_HDX
     - @ref BMSR_MF_SUP        : Not supported. Always 0.
     - @ref BMSR_AN_COMP
     - @ref BMSR_REMOTE_FAULT  : Not supported. Always 0.
     - @ref BMSR_AN_ABILITY
     - @ref BMSR_LINK_STATUS
     - @ref BMSR_JABBER_DETECT
     - @ref BMSR_EXT_CAPA      : Always 1. If you need a extended register information, send e-mail to support@wiznet.io

    @note Its some bits have the same function as @ref _PHYSR_.
    @sa PHYRAR_BMCR, _PHYRAR_, _PHYDIR_, _PHYDOR_, _PHYACR_, _PHYCR0_, _PHYCR1_
    @sa getPHYRAR(), setPHYRAR(), wiz_mdio_read(), wiz_mdio_write()
*/
#define PHYRAR_BMSR          (0x01)


/********************/
/* BMCR & BMSR Bit definitions  */
/********************/

/*For BMCR register*/
/**
    @brief Ethernet PHY S/W Reset.
    @details 0 - Normal operation \n
            1 - Software reset
    @sa PHYRAR_BMCR, PHYCR1_RST
    @sa getPHYRAR_BMCR(), setPHYRAR_BMCR()
*/
#define BMCR_RST             (1<<15)

/**
    @brief Ethernet PHY Loopback.
    @details 0 - Normal Operation \n
            1 - Loopback Enable
    @sa PHYRAR_BMCR
    @sa getPHYRAR_BMCR(), setPHYRAR_BMCR()
*/
#define BMCR_LB              (1<<14)  ///< Loopback. 0 - Noraml operation, 1 - Loopback enabled

/**
    @brief Ethernet PHY Speed
    @details 0 - 10  Mbps \n
            1 - 100 Mbps
    @sa PHYCR_BMCR, PHYCR0_100F, PHYCR0_100H, PHYCR0_10F, PHYCR0_10H
    @sa getPHYRAR_BMCR(), setPHYRAR_BMCR(), setPHYCR0()
*/
#define BMCR_SPD             (1<<13)

/**
    @brief Ethernet PHY Auto-Negotiation
    @details 0 - Disable \n
            1 - Enable
    @note When it is set, @ref BMCR_SPD and @ref BMCR_DPX is ignored
    @sa PHYCR_BMCR, PHYCR0_AUTO
    @sa getPHYRAR_BMCR(), setPHYRAR_BMCR(), setPHYCR0()
*/
#define BMCR_ANE             (1<<12)

/**
    @brief Ethernet PHY Power Down Mode
    @details 0 - Normal Operation \n
            1 - Power Down mode
    @sa PHYCR_BMCR, PHYCR0_PWDN
    @sa getPHYRAR_BMCR(), setPHYRAR_BMCR(), setPHYCR0()
*/
#define BMCR_PWDN            (1<<11)  ///< Power-down mode

/**
    @brief Ethernet PHY Isolation Mode
    @details 0 - Normal Operation \n
            1 - Isolation Mode
    @ Don't set it to '1'. It is not supported.
    @sa PHYCR_BMCR
    @sa getPHYRAR_BMCR(), setPHYRAR_BMCR()
*/
#define BMCR_ISOL            (1<<10)

/**
    @brief Ethernet PHY Restart Auto-Negotiation
    @details 0 - Normal Operation \n
            1 - Restart Auto-Negotiation
    @sa PHYCR_BMCR
    @sa getPHYRAR_BMCR(), setPHYRAR_BMCR()
*/
#define BMCR_REAN            (1<<9)

/**
    @brief Ethernet PHY Duplex
    @details 0 - Half-Duplex \n
            1 - Full-Duplex
    @sa PHYCR_BMCR, PHYCR0_100F, PHYCR0_100H, PHYCR0_10F, PHYCR0_10H
    @sa getPHYRAR_BMCR(), setPHYRAR_BMCR(), setPHYCR0()
*/
#define BMCR_DPX             (1<<8)

/**
    @brief Ethernet PHY Collision Test
    @details 0 - Normal Operation \n
            1 - Collision Test
    @sa PHYCR_BMCR
    @sa getPHYRAR_BMCR(), setPHYRAR_BMCR()
*/
#define BMCR_COLT            (1<<7)

/*For BMSR register*/

/**
    @brief Ethernet PHY 100 Base-T4 capable
    @details @ref BMSR_100_T4 is always 0.
    @note It is not supported.
    @sa PHYCR_BMSR
    @sa getPHYRAR_BMSR()
*/
#define BMSR_100_T4          (1<<15)

/**
    @brief Ethernet PHY 100 Base-TX Full-Duplex capable
    @details @ref BMSR_100_FDX is always 1.
    @sa PHYCR_BMSR
    @sa getPHYRAR_BMSR()
*/
#define BMSR_100_FDX         (1<<14)

/**
    @brief Ethernet PHY 100 Base-TX Half-Duplex capable
    @details @ref BMSR_100_HDX is always 1.
    @sa PHYCR_BMSR
    @sa getPHYRAR_BMSR()
*/
#define BMSR_100_HDX         (1<<13)

/**
    @brief Ethernet PHY 10 Base-T Full-Duplex capable
    @details @ref BMSR_10_FDX is always 1.
    @sa PHYCR_BMSR
    @sa getPHYRAR_BMSR()
*/
#define BMSR_10_FDX          (1<<12)

/**
    @brief Ethernet PHY 10 Base-T Half-Duplex capable
    @details @ref BMSR_10_HDX is always 1.
    @sa PHYCR_BMSR
    @sa getPHYRAR_BMSR()
*/
#define BMSR_10_HDX          (1<<11)

/**
    @brief Ethernet PHY Management Frame preamble suppression
    @details @ref BMSR_MF_SUP is always 0.
    @note It is not supported
    @sa PHYCR_BMSR
    @sa getPHYRAR_BMSR()
*/
#define BMSR_MF_SUP          (1<<6)

/**
    @brief Ethernet PHY Auto-Negotiation Complete
    @details @ref BMSR_MF_SUP indicates the status of auto-negotiation. \n
            0 - Auto-negotiation process is not completed \n
            1 - Auto-negotiation process is completed
    @sa PHYCR_BMSR
    @sa getPHYRAR_BMSR()
*/
#define BMSR_AN_COMP         (1<<5)

/**
    @brief Ethernet PHY Remote Fault
    @details @ref BMSR_REMOTE_FAULT is always 0.
    @note It is not supported
    @sa PHYCR_BMSR
    @sa getPHYRAR_BMSR()
*/
#define BMSR_REMOTE_FAULT    (1<<4)

/**
    @brief Ethernet PHY Auto-Negotiation Ability
    @details @ref BMSR_AN_ABILITY is always 1.
    @sa PHYCR_BMSR
    @sa getPHYRAR_BMSR()
*/
#define BMSR_AN_ABILITY      (1<<3)

/**
    @brief Ethernet PHY Link Status
    @details @ref BMSR_LINK_STATUS indicates the status of link. \n
            0 - Link is not established
            1 - Valid link is established
    @sa PHYCR_BMSR
    @sa getPHYRAR_BMSR()
*/
#define BMSR_LINK_STATUS     (1<<2)

/**
    @brief Ethernet PHY Jabber Detect
    @details @ref BMSR_JABBER_DETECT indicates the status of auto-negotiation. \n
            0 - Jabber condition is not detected\n
            1 - Jabber condition is detected
    @sa PHYCR_BMSR
    @sa getPHYRAR_BMSR()
*/
#define BMSR_JABBER_DETECT   (1<<1)

/**
    @brief Ethernet PHY Extended capability
    @details @ref BMSR_EXT_CAPA indicates the extended register capability. \n
            0 - Only basic registers are capable\n
            1 - Extended registers are capable
    @sa PHYCR_BMSR
    @sa getPHYRAR_BMSR()
*/
#define BMSR_EXT_CAPA        (1<<0)


/**
    @brief Enter a critical section
    @details It is provided to protect your shared code and hardware resources against interference. \n
     - Non-OS environment
       It can be just implemented by disabling whole interrupt.
     - OS environment
       You can replace it to critical section API supported by OS.

    @note It is callback function that can be replaced it with your code, by calling @ref reg_wizchip_cris_cbfunc().
    @sa WIZCHIP_READ(), WIZCHIP_WRITE(), WIZCHIP_READ_BUF(), WIZCHIP_WRITE_BUF()
    @sa WIZCHIP_CRITICAL_EXIT(), reg_wizchip_cris_cbfunc()
*/
#define WIZCHIP_CRITICAL_ENTER()       WIZCHIP.CRIS._enter()


/**
    @brief Enter a critical section
    @details It exits the protected code and hardware resources against interference. \n
     - Non-OS environment
       It can be just implemented by enabling whole interrupt.\n
     - OS environment
       You can replace it to critical section API supported by OS.

    @note It is callback function that can be replaced it with your code, by calling @ref reg_wizchip_cris_cbfunc().
    @sa WIZCHIP_READ(), WIZCHIP_WRITE(), WIZCHIP_READ_BUF(), WIZCHIP_WRITE_BUF()
    @sa WIZCHIP_CRITICAL_EXIT(), reg_wizchip_cris_cbfunc()
*/
#define WIZCHIP_CRITICAL_EXIT()        WIZCHIP.CRIS._exit()



////////////////////////
// Basic I/O Function //
////////////////////////
//
//
/**
    @ingroup Basic_IO_function_W6300
    @brief It reads 1 byte value from a register.
    @param AddrSel Register address
    @return The value of register
    @sa WIZCHIP_READ_BUF(), reg_wizchip_bus_cbfunc(), reg_wizchip_spi_cbfunc(), WIZCHIP_WRITE()
*/
uint8_t WIZCHIP_READ(uint32_t AddrSel);

/**
    @ingroup Basic_IO_function_W6300
    @brief It writes 1 byte value to a register.
    @param AddrSel Register address
    @param wb Write data
    @return void
    @sa WIZCHIP_WRITE_BUF(), reg_wizchip_bus_cbfunc(), reg_wizchip_spi_cbfunc(), WIZCHIP_READ()
*/
void WIZCHIP_WRITE(uint32_t AddrSel, uint8_t wb);

/**
    @ingroup Basic_IO_function_W6300
    @brief It reads sequentail data from registers.
    @param AddrSel Register address
    @param pBuf Pointer buffer to read data
    @param len Data length
    @return void
    @sa WIZCHIP_WRITE_BUF(), reg_wizchip_bus_cbfunc(), reg_wizchip_spi_cbfunc(), WIZCHIP_READ()
*/
void WIZCHIP_READ_BUF(uint32_t AddrSel, uint8_t* pBuf, datasize_t len);

/**
    @ingroup Basic_IO_function_W6300
    @brief It writes sequential data to registers.
    @param AddrSel Register address
    @param pBuf Pointer buffer to write data
    @param len Data length
    @return void
    @sa WIZCHIP_READ_BUF(), reg_wizchip_bus_cbfunc(), reg_wizchip_spi_cbfunc(), WIZCHIP_WRITE()
*/
void WIZCHIP_WRITE_BUF(uint32_t AddrSel, uint8_t* pBuf, datasize_t len);



/////////////////////////////////
// Common Register IO function //
/////////////////////////////////
/**
    @addtogroup Common_register_access_function_W6300
    @{
*/

#define getRTL() \
        WIZCHIP_READ(_RTL_)

#define getCIDR() \
        ((((uint16_t)WIZCHIP_READ(_CIDR_)| (((WIZCHIP_READ(_RTL_))&0x0F) << 1)) << 8) + WIZCHIP_READ(WIZCHIP_OFFSET_INC(_CIDR_,1)))

#define getVER() \
        ((((uint16_t)WIZCHIP_READ(_VER_)) << 8) + WIZCHIP_READ(WIZCHIP_OFFSET_INC(_VER_,1)))

#define getSYSR() \
        WIZCHIP_READ(_SYSR_)

#define getSYCR0() \
        WIZCHIP_READ(_SYCR0_)

#define setSYCR0(sycr0) \
        WIZCHIP_WRITE(_SYCR0_, (sycr0))

#define getSYCR1() \
        WIZCHIP_READ(_SYCR1_)

#define setSYCR1(sycr1) \
        WIZCHIP_WRITE(_SYCR1_, (sycr1))

#define getTCNTR() \
        ((((uint16_t)WIZCHIP_READ(_TCNTR_)) << 8) + WIZCHIP_READ(WIZCHIP_OFFSET_INC(_TCNTR_,1)))

#define setTCNTRCLR(tcntrclr) \
        WIZCHIP_WRITE(_TCNTRCLR_,(tcntrclr))

#define getIR() \
        WIZCHIP_READ(_IR_)

#define getSIR() \
        WIZCHIP_READ(_SIR_)

#define getSLIR() \
        WIZCHIP_READ(_SLIR_)

#define setIMR(imr) \
        WIZCHIP_WRITE(_IMR_,(imr))

#define getIMR() \
        WIZCHIP_READ(_IMR_)

#define setIRCLR(irclr) \
        WIZCHIP_WRITE(_IRCLR_,(irclr))
#define setIR(ir)                setIRCLR(ir)

#define setSIMR(simr) \
        WIZCHIP_WRITE(_SIMR_,(simr))

#define getSIMR() \
        WIZCHIP_READ(_SIMR_)

#define setSLIMR(slimr) \
        WIZCHIP_WRITE(_SLIMR_,(slimr))

#define getSLIMR() \
        WIZCHIP_READ(_SLIMR_)

#define setSLIRCLR(slirclr) \
        WIZCHIP_WRITE(_SLIRCLR_,(slirclr))
#define setSLIR(slir)            setSLIRCLR(slir)

#define setSLPSR(slpsr) \
        WIZCHIP_WRITE(_SLPSR_,(slpsr))

#define getSLPSR() \
        WIZCHIP_READ(_SLPSR_)

#define setSLCR(slcr) \
        WIZCHIP_WRITE(_SLCR_,(slcr))

#define getSLCR()      \
        WIZCHIP_READ(_SLCR_)

#define getPHYSR() \
        WIZCHIP_READ(_PHYSR_)

#define setPHYRAR(phyrar) \
        WIZCHIP_WRITE(_PHYRAR_,(phyrar))

#define getPHYRAR() \
        WIZCHIP_READ(_PHYRAR_)

#define setPHYDIR(phydir)  \
        do{                                                                       \
           WIZCHIP_WRITE(WIZCHIP_OFFSET_INC(_PHYDIR_,1), (uint8_t)((phydir)>>8)); \
           WIZCHIP_WRITE(_PHYDIR_, (uint8_t)(phydir));                            \
        }while(0);

#define getPHYDOR() \
        ((((uint16_t)WIZCHIP_READ(WIZCHIP_OFFSET_INC(_PHYDOR_,1))) << 8) + WIZCHIP_READ(_PHYDOR_))

#define setPHYACR(phyacr) \
        WIZCHIP_WRITE(_PHYACR_,(phyacr))

#define getPHYACR() \
        WIZCHIP_READ(_PHYACR_)

#define setPHYDIVR(phydivr) \
        WIZCHIP_WRITE(_PHYDIVR_,(phydivr))

#define getPHYDIVR() \
        WIZCHIP_READ(_PHYDIVR_)

#define setPHYCR0(phycr0) \
        WIZCHIP_WRITE(_PHYCR0_,(phycr0))

#define setPHYCR1(phycr1) \
        WIZCHIP_WRITE(_PHYCR1_,(phycr1))

#define getPHYCR1() \
        WIZCHIP_READ(_PHYCR1_)

#define setNET4MR(net4mr) \
        WIZCHIP_WRITE(_NET4MR_,(net4mr))

#define setNET6MR(net6mr) \
        WIZCHIP_WRITE(_NET6MR_,(net6mr))

#define setNETMR(netmr) \
        WIZCHIP_WRITE(_NETMR_,(netmr))

#define setNETMR2(netmr2) \
        WIZCHIP_WRITE(_NETMR2_,(netmr2))

#define getNET4MR() \
        WIZCHIP_READ(_NET4MR_)

#define getNET6MR() \
        WIZCHIP_READ(_NET6MR_)

#define getNETMR() \
        WIZCHIP_READ(_NETMR_)

#define getNETMR2() \
        WIZCHIP_READ(_NETMR2_)

#define setPTMR(ptmr) \
        WIZCHIP_WRITE(_PTMR_, (ptmr))

#define getPTMR() \
        WIZCHIP_READ(_PTMR_)

#define setPMNR(pmnr) \
        WIZCHIP_WRITE(_PMNR_, (pmnr))

#define getPMNR() \
        WIZCHIP_READ(_PMNR_)

#define setPHAR(phar) \
        WIZCHIP_WRITE_BUF(_PHAR_,(phar),6)

#define getPHAR(phar) \
        WIZCHIP_READ_BUF(_PHAR_,(phar),6)

#define setPSIDR(psidr) \
        do{                                                                \
            WIZCHIP_WRITE(_PSIDR_,(uint8_t)((psidr) >> 8));                \
            WIZCHIP_WRITE(WIZCHIP_OFFSET_INC(_PSIDR_,1),(uint8_t)(psidr)); \
        }while(0);

#define getPSIDR() \
        ((((uint16_t)WIZCHIP_READ(_PSIDR_)) << 8) + WIZCHIP_READ(WIZCHIP_OFFSET_INC(_PSIDR_,1)))

#define setPMRUR(pmrur) \
        do{                                                                \
            WIZCHIP_WRITE(_PMRUR_,(uint8_t)((pmrur) >> 8));                \
            WIZCHIP_WRITE(WIZCHIP_OFFSET_INC(_PMRUR_,1),(uint8_t)(pmrur)); \
        }while(0);

#define getPMRUR() \
        ((((uint16_t)WIZCHIP_READ(_PMRUR_)) << 8) + WIZCHIP_READ(WIZCHIP_OFFSET_INC(_PMRUR_,1)))

#define setSHAR(shar) \
        WIZCHIP_WRITE_BUF(_SHAR_,(shar),6)

#define getSHAR(shar) \
        WIZCHIP_READ_BUF(_SHAR_,(shar),6)

#define setGAR(gar) \
        WIZCHIP_WRITE_BUF(_GAR_,(gar),4)

#define getGAR(gar) \
        WIZCHIP_READ_BUF(_GAR_,(gar),4)

#define setGA4R(ga4r)            setGAR(ga4r)
#define getGA4R(ga4r)            getGAR(ga4r)

#define setSUBR(subr) \
        WIZCHIP_WRITE_BUF(_SUBR_,(subr),4)

#define getSUBR(subr) \
        WIZCHIP_READ_BUF(_SUBR_,(subr),4)

#define setSUB4R(sub4r)          setSUBR(sub4r)
#define getSUB4R(sub4r)          getSUBR(sub4r)

#define setSIPR(sipr) \
        WIZCHIP_WRITE_BUF(_SIPR_,(sipr),4)

#define getSIPR(sipr) \
        WIZCHIP_READ_BUF(_SIPR_,(sipr),4)

#define setLLAR(llar) \
        WIZCHIP_WRITE_BUF(_LLAR_,(llar),16)

#define getLLAR(llar) \
        WIZCHIP_READ_BUF(_LLAR_,(llar),16)

#define setGUAR(guar) \
        WIZCHIP_WRITE_BUF(_GUAR_,(guar),16)

#define getGUAR(guar) \
        WIZCHIP_READ_BUF(_GUAR_,(guar),16)

#define setSUB6R(sub6r) \
        WIZCHIP_WRITE_BUF(_SUB6R_,(sub6r),16)

#define getSUB6R(sub6r) \
        WIZCHIP_READ_BUF(_SUB6R_,(sub6r),16)

#define setGA6R(ga6r) \
        WIZCHIP_WRITE_BUF(_GA6R_,(ga6r),16)

#define getGA6R(ga6r) \
        WIZCHIP_READ_BUF(_GA6R_,(ga6r),16)

#define setSLDIPR(sldipr) \
        WIZCHIP_WRITE_BUF(_SLDIPR_,(sldipr),4)
#define setSLDIP4R(sldip4r)      setSLDIPR((sldip4r))

#define getSLDIPR(sldipr) \
        WIZCHIP_READ_BUF(_SLDIPR_,(sldipr),4)
#define getSLDIP4R(sldip4r)      getSLDIPR((sldip4r))

#define setSLDIP6R(sldip6r) \
        WIZCHIP_WRITE_BUF(_SLDIP6R_, (sldip6r),16)

#define getSLDIP6R(sldip6r) \
        WIZCHIP_READ_BUF(_SLDIP6R_,(sldip6r),16)

#define getSLDHAR(sldhar) \
        WIZCHIP_READ_BUF(_SLDHAR_,(sldhar),6)

#define setPINGIDR(pingidr) \
        do{                                                                    \
            WIZCHIP_WRITE(_PINGIDR_,(uint8_t)((pingidr)>>8));                  \
            WIZCHIP_WRITE(WIZCHIP_OFFSET_INC(_PINGIDR_,1),(uint8_t)(pingidr)); \
        }while(0);

#define getPINGIDR() \
        (((int16_t)(WIZCHIP_READ(_PINGIDR_) << 8)) + WIZCHIP_READ(WIZCHIP_OFFSET_INC(_PINGIDR_,1)))

#define setPINGSEQR(pingseqr) \
        do{                                                                      \
            WIZCHIP_WRITE(_PINGSEQR_,(uint8_t)((pingseqr)>>8));                  \
            WIZCHIP_WRITE(WIZCHIP_OFFSET_INC(_PINGSEQR_,1),(uint8_t)(pingseqr)); \
        }while(0);

#define getPINGSEQR() \
        (((int16_t)(WIZCHIP_READ(_PINGSEQR_) << 8)) + WIZCHIP_READ(WIZCHIP_OFFSET_INC(_PINGSEQR_,1)))

#define getUIPR(uipr) \
        WIZCHIP_READ_BUF(_UIPR_, (uipr), 4)

#define getUIP4R(uip4r)          getUIPR(uip4r)

#define getUPORTR() \
        ((((uint16_t)WIZCHIP_READ(_UPORTR_)) << 8) + WIZCHIP_READ(WIZCHIP_OFFSET_INC(_UPORTR_,1)))

#define getUPORT4R()             getUPORTR()

#define getUIP6R(uip6r) \
        WIZCHIP_READ_BUF(_UIP6R_,(uip6r),16)

#define getUPORT6R(uport6r) \
        ((((uint16_t)WIZCHIP_READ(_UPORT6R_)) << 8) + WIZCHIP_READ(WIZCHIP_OFFSET_INC(_UPORT6R_,1)))

#define setINTPTMR(intptmr) \
        do{                                                                     \
            WIZCHIP_WRITE(_INTPTMR_,(uint8_t)((intptmr) >> 8));                 \
            WIZCHIP_WRITE(WIZCHIP_OFFSET_INC(_INTPTMR_,1),(uint8_t)(intptmr));  \
       }while(0);

#define getINTPTMR() \
        ((((uint16_t)WIZCHIP_READ(_INTPTMR_)) << 8) + WIZCHIP_READ(WIZCHIP_OFFSET_INC(_INTPTMR_,1)))

#define getPLR() \
        WIZCHIP_READ(_PLR_)

#define getPFR() \
        WIZCHIP_READ(_PFR_)

#define getVLTR() \
        ( (((uint32_t)WIZCHIP_READ(_VLTR_)) << 24) +                       \
          (((uint32_t)WIZCHIP_READ(WIZCHIP_OFFSET_INC(_VLTR_,1))) << 16) + \
          (((uint32_t)WIZCHIP_READ(WIZCHIP_OFFSET_INC(_VLTR_,2))) << 16) + \
          (((uint32_t)WIZCHIP_READ(WIZCHIP_OFFSET_INC(_VLTR_,3))) << 16) )

#define getPLTR() \
        ( (((uint32_t)WIZCHIP_READ(_PLTR_)) << 24) +                       \
          (((uint32_t)WIZCHIP_READ(WIZCHIP_OFFSET_INC(_PLTR_,1))) << 16) + \
          (((uint32_t)WIZCHIP_READ(WIZCHIP_OFFSET_INC(_PLTR_,2))) << 16) + \
          (((uint32_t)WIZCHIP_READ(WIZCHIP_OFFSET_INC(_PLTR_,3))) << 16) )

#define getPAR(par) \
        WIZCHIP_READ_BUF(_PAR_, (par), 16)

#define setICMP6BLKR(icmp6blkr) \
        WIZCHIP_WRITE(_ICMP6BLKR_,(icmp6blkr))

#define getICMP6BLKR() \
        WIZCHIP_READ(_ICMP6BLKR_)

#define setCHPLCKR(chplckr) \
        WIZCHIP_WRITE(_CHPLCKR_, (chplckr))

#define getCHPLCKR() \
        ((getSYSR() & SYSR_CHPL) >> 7)

#define CHIPLOCK()      setCHPLCKR(0xFF)
#define CHIPUNLOCK()    setCHPLCKR(0xCE)

#define setNETLCKR(netlckr) \
        WIZCHIP_WRITE(_NETLCKR_, (netlckr))

#define getNETLCKR() \
        ((getSYSR() & SYSR_NETL) >> 6)

#define NETLOCK()      setNETLCKR(0xC5)
#define NETUNLOCK()    setNETLCKR(0x3A)

#define setPHYLCKR(phylckr) \
        WIZCHIP_WRITE(_PHYLCKR_,(phylckr))

#define getPHYLCKR() \
        ((getSYSR() & SYSR_PHYL) >> 5)

#define PHYLOCK()      setPHYLCKR(0xFF)
#define PHYUNLOCK()    setPHYLCKR(0x53)

#define setRTR(rtr) \
        do{                                                            \
            WIZCHIP_WRITE(_RTR_,(uint8_t)((rtr)>>8));                  \
            WIZCHIP_WRITE(WIZCHIP_OFFSET_INC(_RTR_,1),(uint8_t)(rtr)); \
        }while(0);

#define getRTR() \
        ((((uint16_t)WIZCHIP_READ(_RTR_)) << 8) + WIZCHIP_READ(WIZCHIP_OFFSET_INC(_RTR_,1)))

#define setRCR(rcr) \
        WIZCHIP_WRITE(_RCR_,(rcr))

#define getRCR() \
        WIZCHIP_READ(_RCR_)

#define setSLRTR(slrtr) \
        do{                                                                \
            WIZCHIP_WRITE(_SLRTR_,(uint8_t)((slrtr)>>8));                  \
            WIZCHIP_WRITE(WIZCHIP_OFFSET_INC(_SLRTR_,1),(uint8_t)(slrtr)); \
        }while(0);

#define getSLRTR() \
        ((((uint16_t)WIZCHIP_READ(_SLRTR_)) << 8) + WIZCHIP_READ(WIZCHIP_OFFSET_INC(_SLRTR_,1)))

#define setSLRCR(slrcr) \
        WIZCHIP_WRITE(_SLRCR_,(slrcr))

#define getSLRCR() \
        WIZCHIP_READ(_SLRCR_)

#define setSLHOPR(slhopr) \
        WIZCHIP_WRITE(_SLHOPR_,(slhopr))

#define getSLHOPR() \
        WIZCHIP_READ(_SLHOPR_)
/**
    @}
*/


////////////////////////////////////
// SOCKETn  register I/O function //
////////////////////////////////////
/**
    @addtogroup Socket_register_access_function_W6300
    @{
*/
#define setSn_MR(sn,mr) \
        WIZCHIP_WRITE(_Sn_MR_(sn),(mr))
#define getSn_MR(sn) \
        WIZCHIP_READ(_Sn_MR_(sn))

#define setSn_PSR(sn,psr) \
        WIZCHIP_WRITE(_Sn_PSR_(sn),(psr))
#define getSn_PSR(sn) \
        WIZCHIP_READ(_Sn_PSR_(sn))

#define setSn_CR(sn,cr) \
        WIZCHIP_WRITE(_Sn_CR_(sn),(cr))
#define getSn_CR(sn) \
        WIZCHIP_READ(_Sn_CR_(sn))

#define getSn_IR(sn) \
        WIZCHIP_READ(_Sn_IR_(sn))

#define setSn_IMR(sn,imr) \
        WIZCHIP_WRITE(_Sn_IMR_(sn),(imr))
#define getSn_IMR(sn) \
        WIZCHIP_READ(_Sn_IMR_(sn))

#define setSn_IRCLR(sn,irclr) \
        WIZCHIP_WRITE(_Sn_IRCLR_(sn),(irclr))
#define setSn_IR(sn,ir)          setSn_IRCLR(sn,(ir))

#define getSn_SR(sn) \
        WIZCHIP_READ(_Sn_SR_(sn))

#define getSn_ESR(sn) \
        WIZCHIP_READ(_Sn_ESR_(sn))

#define setSn_PNR(sn,pnr) \
        WIZCHIP_WRITE(_Sn_PNR_(sn),(pnr))
#define setSn_NHR(sn,nhr)        setSn_PNR(_Sn_PNR_(sn),(nhr))

#define getSn_PNR(sn) \
        WIZCHIP_READ(_Sn_PNR_(sn))
#define getSn_NHR(sn)            getSn_PNR(sn)

#define setSn_TOSR(sn,tosr) \
        WIZCHIP_WRITE(_Sn_TOSR_(sn),(tosr))
#define getSn_TOSR(sn) \
        WIZCHIP_READ(_Sn_TOSR_(sn))
#define getSn_TOS(sn)   getSn_TOSR(sn)    ///< For compatible ioLibrar
#define setSn_TOS(sn,tos)  setSn_TOSR(sn,tos)   ///< For compatible ioLibrar


#define setSn_TTLR(sn,ttlr) \
        WIZCHIP_WRITE(_Sn_TTLR_(sn),(ttlr))
#define getSn_TTLR(sn) \
        WIZCHIP_READ(_Sn_TTLR_(sn))
#define setSn_TTL(sn,ttl)       setSn_TTLR(sn,ttl)   ///< For compatible ioLibrary
#define getSn_TTL(sn)           getSn_TTLR(sn)   ///< For compatible ioLibrary

#define setSn_HOPR(sn,hopr)      setSn_TTLR(sn),(ttlr))
#define getSn_HOPR(sn)           getSn_TTLR(sn)

#define setSn_FRGR(sn,frgr) \
        do{                                                                      \
            WIZCHIP_WRITE(_Sn_FRGR_(sn),(uint8_t)((frgr)>>8));                   \
            WIZCHIP_WRITE(WIZCHIP_OFFSET_INC(_Sn_FRGR_(sn),1),(uint8_t)(frgr));  \
        }while(0);
#define getSn_FRGR(sn,frgr) \
        ((((uint16_t)WIZCHIP_READ(_Sn_FRGR_(sn))) << 8) + WIZCHIP_READ(WIZCHIP_OFFSET_INC(_Sn_FRGR_(sn),1)))

#define setSn_MSSR(sn,mssr) \
        do{                                                                      \
            WIZCHIP_WRITE(_Sn_MSSR_(sn),(uint8_t)((mssr)>>8));                   \
            WIZCHIP_WRITE(WIZCHIP_OFFSET_INC(_Sn_MSSR_(sn),1),(uint8_t)(mssr));  \
        }while(0);
#define getSn_MSSR(sn) \
        ((((uint16_t)WIZCHIP_READ(_Sn_MSSR_(sn))) << 8) + WIZCHIP_READ(WIZCHIP_OFFSET_INC(_Sn_MSSR_(sn),1)))

#define setSn_PORTR(sn,portr) \
        do{                                                                       \
            WIZCHIP_WRITE(_Sn_PORTR_(sn),(uint8_t)((portr)>>8));                  \
            WIZCHIP_WRITE(WIZCHIP_OFFSET_INC(_Sn_PORTR_(sn),1),(uint8_t)(portr)); \
        }while(0);
#define getSn_PORTR(sn) \
        ((((uint16_t)WIZCHIP_READ(_Sn_PORTR_(sn))) << 8) + WIZCHIP_READ(WIZCHIP_OFFSET_INC(_Sn_PORTR_(sn),1)))

#define setSn_DHAR(sn,dhar) \
        WIZCHIP_WRITE_BUF(_Sn_DHAR_(sn),(dhar),6)
#define getSn_DHAR(sn,dhar) \
        WIZCHIP_READ_BUF(_Sn_DHAR_(sn),(dhar),6)

#define setSn_DIPR(sn,dipr) \
        WIZCHIP_WRITE_BUF(_Sn_DIPR_(sn),(dipr),4)
#define getSn_DIPR(sn,dipr) \
        WIZCHIP_READ_BUF(_Sn_DIPR_(sn),(dipr),4)

#define setSn_DIP4R(sn,dipr)      setSn_DIPR(sn,(dipr))
#define getSn_DIP4R(sn,dipr)      getSn_DIPR(sn,(dipr))

#define setSn_DIP6R(sn,dip6r) \
        WIZCHIP_WRITE_BUF(_Sn_DIP6R_(sn),(dip6r),16)
#define getSn_DIP6R(sn,dip6r) \
        WIZCHIP_READ_BUF(_Sn_DIP6R_(sn),(dip6r),16)

#define setSn_DPORTR(sn,dportr) \
        do{                                                                         \
            WIZCHIP_WRITE(_Sn_DPORTR_(sn),(uint8_t)((dportr)>>8));                  \
            WIZCHIP_WRITE(WIZCHIP_OFFSET_INC(_Sn_DPORTR_(sn),1),(uint8_t)(dportr)); \
        }while(0);


#define getSn_DPORTR(sn) \
        ((((uint16_t)WIZCHIP_READ(_Sn_DPORTR_(sn))) << 8) + WIZCHIP_READ(WIZCHIP_OFFSET_INC(_Sn_DPORTR_(sn),1)))

#define getSn_DPORT(sn) getSn_DPORTR(sn)
#define setSn_DPORT(sn,dportr)  setSn_DPORTR(sn,dportr)


#define setSn_MR2(sn,mr2) \
        WIZCHIP_WRITE(_Sn_MR2_(sn),(mr2))
#define getSn_MR2(sn) \
        WIZCHIP_READ(_Sn_MR2_(sn))

#define setSn_RTR(sn,rtr) \
        do{                                                                   \
            WIZCHIP_WRITE(_Sn_RTR_(sn),(uint8_t)((rtr)>>8));                  \
            WIZCHIP_WRITE(WIZCHIP_OFFSET_INC(_Sn_RTR_(sn),1),(uint8_t)(rtr)); \
        }while(0);
#define getSn_RTR(sn) \
        ((((uint16_t)WIZCHIP_READ(_Sn_RTR_(sn))) << 8) + WIZCHIP_READ(WIZCHIP_OFFSET_INC(_Sn_RTR_(sn),1)))

#define setSn_RCR(sn,rcr) \
        WIZCHIP_WRITE(_Sn_RCR_(sn),(rcr))
#define getSn_RCR(sn) \
        WIZCHIP_READ(_Sn_RCR_(sn))

#define setSn_KPALVTR(sn,kpalvtr) \
        WIZCHIP_WRITE(_Sn_KPALVTR_(sn),(kpalvtr))
#define getSn_KPALVTR(sn) \
        WIZCHIP_READ(_Sn_KPALVTR_(sn))

#define setSn_TX_BSR(sn, tmsr) \
        WIZCHIP_WRITE(_Sn_TX_BSR_(sn),(tmsr))
#define setSn_TXBUF_SIZE(sn, tmsr)      setSn_TX_BSR(sn,(tmsr))

#define getSn_TX_BSR(sn) \
        WIZCHIP_READ(_Sn_TX_BSR_(sn))
#define getSn_TXBUF_SIZE(sn)            getSn_TX_BSR(sn)

#define getSn_TxMAX(sn) \
	  (getSn_TX_BSR(sn) << 10)

uint16_t getSn_TX_FSR(uint8_t sn);

#define getSn_TX_RD(sn) \
        ((((uint16_t)WIZCHIP_READ(_Sn_TX_RD_(sn))) << 8) + WIZCHIP_READ(WIZCHIP_OFFSET_INC(_Sn_TX_RD_(sn),1)))

#define setSn_TX_WR(sn,txwr) \
        do{                                                                       \
            WIZCHIP_WRITE(_Sn_TX_WR_(sn), (uint8_t)((txwr)>>8));                  \
            WIZCHIP_WRITE(WIZCHIP_OFFSET_INC(_Sn_TX_WR_(sn),1), (uint8_t)(txwr)); \
        }while(0);
#define getSn_TX_WR(sn) \
        (((uint16_t)WIZCHIP_READ(_Sn_TX_WR_(sn)) << 8) + WIZCHIP_READ(WIZCHIP_OFFSET_INC(_Sn_TX_WR_(sn),1)))

#define setSn_RX_BSR(sn,rmsr) \
        WIZCHIP_WRITE(_Sn_RX_BSR_(sn),(rmsr))
#define setSn_RXBUF_SIZE(sn,rmsr)      setSn_RX_BSR(sn,(rmsr))

#define getSn_RX_BSR(sn) \
        WIZCHIP_READ(_Sn_RX_BSR_(sn))
#define getSn_RXBUF_SIZE(sn)           getSn_RX_BSR(sn)

#define getSn_RxMAX(sn) \
        (getSn_RX_BSR(sn) <<10)

uint16_t getSn_RX_RSR(uint8_t s);

#define setSn_RX_RD(sn,rxrd) \
        do{                                                                        \
            WIZCHIP_WRITE(_Sn_RX_RD_(sn), (uint8_t)((rxrd)>>8));                   \
            WIZCHIP_WRITE(WIZCHIP_OFFSET_INC(_Sn_RX_RD_(sn),1), (uint8_t)(rxrd)) ; \
        }while(0);

#define getSn_RX_RD(sn) \
        (((uint16_t)WIZCHIP_READ(_Sn_RX_RD_(sn)) << 8) + WIZCHIP_READ(WIZCHIP_OFFSET_INC(_Sn_RX_RD_(sn),1)))

#define getSn_RX_WR(sn) \
        (((uint16_t)WIZCHIP_READ(_Sn_RX_WR_(sn)) << 8) + WIZCHIP_READ(WIZCHIP_OFFSET_INC(_Sn_RX_WR_(sn),1)))
/**
    @}
*/


/////////////////////////////////////
// Sn_TXBUF & Sn_RXBUF IO function //
/////////////////////////////////////
/**
    @ingroup Basic_IO_function_W6300
    @brief It saves data to be sent in the SOCKETn TX buffer.
    @details This function reads first @ref _Sn_TX_WR_ \n
            and starts to copy <i>wizdata</i> from @ref _Sn_TX_WR_ address of SOCKETn TX buffer as many as <i>len</i>.\n
            After it is completed to copy <i><wizdata</i>, \n
            It increases @ref _Sn_TX_WR_ as many as <i>len</i>.
    @param sn SOCKETn. It should be <b>0 ~ @ref _WIZCHIP_SOCK_NUM_</b>.
    @param wizdata Pointer buffer to write data
    @param len Data length
    @sa wiz_recv_data()
*/
void wiz_send_data(uint8_t sn, uint8_t *wizdata, uint16_t len);

/**
    @ingroup Basic_IO_function_W6300
    @brief It reads the received data from the SOCKETn RX buffer and copies the data to your system memory specified by <i>wizdata</i>.
    @details This function reads first @ref _Sn_RX_RD_ \n
            and starts to copy the received data to <i>wizdata</i> as many as <i>len</i>.\n
            After it is completed to copy the received data, \n
            It increases @ref _Sn_RX_RD_ as many as <i>len</i>.
    @param sn SOCKETn. It should be <b>0 ~ @ref _WIZCHIP_SOCK_NUM_</b>.
    @param wizdata Pointer buffer to read data
    @param len Data length
    @sa wiz_send_data()
*/
void wiz_recv_data(uint8_t sn, uint8_t *wizdata, uint16_t len);

/**
    @ingroup Basic_IO_function_W6300
    @brief It discards the received data in the SOCKETn RX buffer.
    @details This function discards the received data by increasing @ref _Sn_RX_RD_ as manay as <i>len</i> without coping the data.
    @param sn SOCKETn. It should be <b>0 ~ @ref _WIZCHIP_SOCK_NUM_</b>.
    @param len Data length
*/
void wiz_recv_ignore(uint8_t sn, uint16_t len);

#if 1
// 20231019 taylor
/**
    @ingroup Special_function_W6300
    @brief Delay function
    @details Delay function using internal 100us timer of the W6300
    @param (uint32_t)ms Time to delay in milliseconds.
*/
void wiz_delay_ms(uint32_t ms);
#endif

/// @cond DOXY_APPLY_CODE
#if (_PHY_IO_MODE_ == _PHY_IO_MODE_MII_)
/// @endcond
/**
    @ingroup Special_function_W6300
    @brief Write data to the PHY via MDC/MDIO interface.
    @details Write command data to the PHY via MDC/MDIO interface.
    @param phyregaddr Address of the PHY register. It should be @ref PHYRAR_BMCR, @ref PHYRAR_BMSR, and etc.
    @param var Data to write to the PHY register. Please refer to the bit definitions of the BMCR and BMSR register.
    @note In order to use it, You should define @ref _PHY_IO_MODE_ to @ref _PHY_IO_MODE_MII_.
*/
void wiz_mdio_write(uint8_t phyregaddr, uint16_t var);

/**
    @ingroup Special_function_W6300
    @brief Read data from the PHY via MDC/MDIO interface.
    @details Read command or status data from the PHY via MDC/MDIO interface.
    @param phyregaddr Address of the PHY register. It should be @ref PHYRAR_BMCR, @ref PHYRAR_BMSR, and etc.
    @return The value of the PHY register
    @note In order to use it, You should define @ref _PHY_IO_MODE_ to @ref _PHY_IO_MODE_MII_.
*/
uint16_t wiz_mdio_read(uint8_t phyregaddr);
/// @cond DOXY_APPLY_CODE
#endif
/// @endcond

/// @cond DOXY_APPLY_CODE
#endif  // _WIZCHIP_ == 6100
/// @endcond


#ifdef __cplusplus
}
#endif


#endif //_W6300_H_