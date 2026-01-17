//*****************************************************************************
//
//! \file W6300.c
//! \brief W6300 HAL Implements file.
//! \version 1.0.0
//! \date 2019/01/01
//! \par  Revision history
//!       <2019/01/01> 1st Release
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

#include "w6300.h"


#if 0
#define _WIZCHIP_SPI_VDM_OP_    0x00
#define _WIZCHIP_SPI_FDM_LEN1_  0x01
#define _WIZCHIP_SPI_FDM_LEN2_  0x02
#define _WIZCHIP_SPI_FDM_LEN4_  0x03
#endif
//
// If you want to use SPI FDM mode, Feel free contact to WIZnet.
// http://forum.wiznet.io
//

#if _WIZCHIP_ == 6300
////////////////////////////////////////////////////////////////////////////////////////


#define _W6300_SPI_OP_          _WIZCHIP_SPI_VDM_OP_
#define _W6300_SPI_READ_                  (0x00 << 5)        ///< SPI interface Read operation in Control Phase
#define _W6300_SPI_WRITE_                 (0x01 << 5)        ///< SPI interface Write operation in Control Phase

//////////////////////////////////////////////////
void WIZCHIP_WRITE(uint32_t AddrSel, uint8_t wb) {

    uint8_t opcode = 0;
    uint16_t ADDR = 0;

    WIZCHIP_CRITICAL_ENTER();
    WIZCHIP.CS._select();

#if (_WIZCHIP_IO_MODE_ & 0xff00) & _WIZCHIP_IO_MODE_BUS_
    uint8_t tAD[4];
    tAD[0] = (uint8_t)((AddrSel & 0x00FF0000) >> 16);
    tAD[1] = (uint8_t)((AddrSel & 0x0000FF00) >> 8);
    tAD[2] = (uint8_t)(AddrSel & 0x000000ff);
    tAD[3] = wb;
    WIZCHIP.IF.BUS._write_data_buf(IDM_AR0, tAD, 4, 1);
#else //w6300 QSPI MODE
    opcode = (uint8_t)((AddrSel & 0x000000FF) | (_W6300_SPI_WRITE_) | (_WIZCHIP_QSPI_MODE_));
    ADDR = (uint16_t)((AddrSel & 0x00ffff00) >> 8);
    WIZCHIP.IF.QSPI._write_qspi(opcode, ADDR, &wb, 1);
#endif

    WIZCHIP.CS._deselect();
    WIZCHIP_CRITICAL_EXIT();
}

uint8_t  WIZCHIP_READ(uint32_t AddrSel) {
    //uint8_t ret;
    uint8_t ret[2] = {0,};
    uint8_t opcode = 0;
    uint16_t ADDR = 0;

    WIZCHIP_CRITICAL_ENTER();
    WIZCHIP.CS._select();

#if (_WIZCHIP_IO_MODE_ & 0xff00) & _WIZCHIP_IO_MODE_BUS_
    uint8_t tAD[3];
    tAD[0] = (uint8_t)((AddrSel & 0x00FF0000) >> 16);
    tAD[1] = (uint8_t)((AddrSel & 0x0000FF00) >> 8);
    tAD[2] = (uint8_t)(AddrSel & 0x000000ff);
    WIZCHIP.IF.BUS._write_data_buf(IDM_AR0, tAD, 3, 1);
    ret[0] = WIZCHIP.IF.BUS._read_data(IDM_DR);
#else
    opcode = (uint8_t)((AddrSel & 0x000000FF) | (_W6300_SPI_READ_) | (_WIZCHIP_QSPI_MODE_));
    ADDR = (uint16_t)((AddrSel & 0x00ffff00) >> 8);
    WIZCHIP.IF.QSPI._read_qspi(opcode, ADDR, ret, 1);
#endif
    WIZCHIP.CS._deselect();
    WIZCHIP_CRITICAL_EXIT();
    return ret[0];
}


void WIZCHIP_WRITE_BUF(uint32_t AddrSel, uint8_t* pBuf, datasize_t len) {

    uint8_t opcode = 0;
    uint16_t ADDR = 0;

    WIZCHIP_CRITICAL_ENTER();
    WIZCHIP.CS._select();

#if (_WIZCHIP_IO_MODE_ & 0xff00) & _WIZCHIP_IO_MODE_BUS_
    uint8_t tAD[3];
    tAD[0] = (uint8_t)((AddrSel & 0x00FF0000) >> 16);
    tAD[1] = (uint8_t)((AddrSel & 0x0000FF00) >> 8);
    tAD[2] = (uint8_t)(AddrSel & 0x000000ff);
    WIZCHIP.IF.BUS._write_data_buf(IDM_AR0, tAD, 3, 1);
    WIZCHIP.IF.BUS._write_data_buf(IDM_DR, pBuf, len, 0);
#else
    opcode = (uint8_t)((AddrSel & 0x000000FF) | (_W6300_SPI_WRITE_) | (_WIZCHIP_QSPI_MODE_));
    ADDR = (uint16_t)((AddrSel & 0x00ffff00) >> 8);
    WIZCHIP.IF.QSPI._write_qspi(opcode, ADDR, pBuf, len);//by_lihan
    //qspi_write_buf(opcode, ADDR, pBuf, len);
    WIZCHIP.CS._deselect();
    WIZCHIP_CRITICAL_EXIT();
#endif
}

void WIZCHIP_READ_BUF(uint32_t AddrSel, uint8_t* pBuf, datasize_t len) {

    uint8_t ret;
    uint8_t opcode = 0;
    uint16_t ADDR = 0;

    WIZCHIP_CRITICAL_ENTER();
    WIZCHIP.CS._select();

#if _WIZCHIP_IO_MODE_ & _WIZCHIP_IO_MODE_BUS_
    uint8_t tAD[3];
    tAD[0] = (uint8_t)((AddrSel & 0x00FF0000) >> 16);
    tAD[1] = (uint8_t)((AddrSel & 0x0000FF00) >> 8);
    tAD[2] = (uint8_t)(AddrSel & 0x000000ff);
    WIZCHIP.IF.BUS._write_data_buf(IDM_AR0, tAD, 3, 1);
    WIZCHIP.IF.BUS._read_data_buf(IDM_DR, pBuf, len, 0);
#else
    opcode = (uint8_t)((AddrSel & 0x000000FF) | (_W6300_SPI_READ_) | (_WIZCHIP_QSPI_MODE_));
    ADDR = (uint16_t)((AddrSel & 0x00ffff00) >> 8);
    WIZCHIP.IF.QSPI._read_qspi(opcode, ADDR, pBuf, len);//by_lihan
    //qspi_read_buf(opcode, ADDR, pBuf, len);
    WIZCHIP.CS._deselect();
    WIZCHIP_CRITICAL_EXIT();
#endif
}

uint16_t getSn_TX_FSR(uint8_t sn) {
    uint16_t prev_val = -1, val = 0;
    do {
        prev_val = val;
        val = WIZCHIP_READ(_Sn_TX_FSR_(sn));
        val = (val << 8) + WIZCHIP_READ(WIZCHIP_OFFSET_INC(_Sn_TX_FSR_(sn), 1));
    } while (val != prev_val);
    return val;
}

uint16_t getSn_RX_RSR(uint8_t sn) {
    uint16_t prev_val = -1, val = 0;
    do {
        prev_val = val;
        val = WIZCHIP_READ(_Sn_RX_RSR_(sn));
        val = (val << 8) + WIZCHIP_READ(WIZCHIP_OFFSET_INC(_Sn_RX_RSR_(sn), 1));
    } while (val != prev_val);
    return val;
}

void wiz_send_data(uint8_t sn, uint8_t *wizdata, uint16_t len) {
    uint16_t ptr = 0;
    uint32_t addrsel = 0;
    ptr = getSn_TX_WR(sn);
    addrsel = ((uint32_t)ptr << 8) + WIZCHIP_TXBUF_BLOCK(sn);
    WIZCHIP_WRITE_BUF(addrsel, wizdata, len);
    ptr += len;
    setSn_TX_WR(sn, ptr);
}

#if 0
#define ETHERNET_BUF_MAX_SIZE_TEMP (1024 * 32 )
void wiz_recv_data(uint8_t sn, uint8_t *wizdata, uint16_t len) {
    uint16_t ptr = 0;
    uint32_t addrsel = 0;
    if (len == 0) {
        return;
    }
    ptr = getSn_RX_RD(sn);

    if (ptr + len > 0xFFFF) {
        addrsel = ((uint32_t)ptr << 8) + WIZCHIP_RXBUF_BLOCK(sn);
        uint16_t size = 0xFFFF - ptr;
        WIZCHIP_READ_BUF(addrsel, wizdata, size);
        wizdata += size;
        size = len - size;
        addrsel = WIZCHIP_RXBUF_BLOCK(sn);
        WIZCHIP_READ_BUF(addrsel, wizdata, size);
    } else {
        addrsel = ((uint32_t)ptr << 8) + WIZCHIP_RXBUF_BLOCK(sn);
        WIZCHIP_READ_BUF(addrsel, wizdata, len);
    }

    ptr += len;
    ptr %= 0xFFFF ;

    setSn_RX_RD(sn, ptr);
}


#else
void wiz_recv_data(uint8_t sn, uint8_t *wizdata, uint16_t len) {
    uint16_t ptr = 0;
    uint32_t addrsel = 0;
    if (len == 0) {
        return;
    }
    ptr = getSn_RX_RD(sn);
    addrsel = ((uint32_t)ptr << 8) + WIZCHIP_RXBUF_BLOCK(sn);
    WIZCHIP_READ_BUF(addrsel, wizdata, len);
    ptr += len;
    setSn_RX_RD(sn, ptr);
}
#endif
void wiz_recv_ignore(uint8_t sn, uint16_t len) {
    setSn_RX_RD(sn, getSn_RX_RD(sn) + len);
}

#if 1
// 20231019 taylor
void wiz_delay_ms(uint32_t milliseconds) {
    uint32_t i;
    for (i = 0 ; i < milliseconds ; i++) {
        //Write any values to clear the TCNTCLKR register
        setTCNTRCLR(0xff);

        // Wait until counter register value reaches 10.(10 = 1ms : TCNTR is 100us tick counter register)
        while (getTCNTR() < 0x0a) {}
    }
}
#endif

/// @cond DOXY_APPLY_CODE
#if (_PHY_IO_MODE_ == _PHY_IO_MODE_MII_)
/// @endcond
void wiz_mdio_write(uint8_t phyregaddr, uint16_t var) {
    setPHYRAR(phyregaddr);
    setPHYDIR(var);
    setPHYACR(PHYACR_WRITE);
    while (getPHYACR()); //wait for command complete
}

uint16_t wiz_mdio_read(uint8_t phyregaddr) {
    setPHYRAR(phyregaddr);
    setPHYACR(PHYACR_READ);
    while (getPHYACR()); //wait for command complete
    return getPHYDOR();
}
/// @cond DOXY_APPLY_CODE
#endif
/// @endcond

////////////////////////////////////////////////////////////////////////////////////////
#endif
