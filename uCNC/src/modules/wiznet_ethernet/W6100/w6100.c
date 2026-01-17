//*****************************************************************************
//
//! \file w6100.c
//! \brief W6100 HAL Implements file.
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

#include "w6100.h"



#define _WIZCHIP_SPI_VDM_OP_    0x00
#define _WIZCHIP_SPI_FDM_LEN1_  0x01
#define _WIZCHIP_SPI_FDM_LEN2_  0x02
#define _WIZCHIP_SPI_FDM_LEN4_  0x03
//
// If you want to use SPI FDM mode, Feel free contact to WIZnet.
// http://forum.wiznet.io
//

#if _WIZCHIP_ == 6100
////////////////////////////////////////////////////////////////////////////////////////


#define _W6100_SPI_OP_          _WIZCHIP_SPI_VDM_OP_

//////////////////////////////////////////////////
void WIZCHIP_WRITE(uint32_t AddrSel, uint8_t wb) {
    uint8_t tAD[4];
    tAD[0] = (uint8_t)((AddrSel & 0x00FF0000) >> 16);
    tAD[1] = (uint8_t)((AddrSel & 0x0000FF00) >> 8);
    tAD[2] = (uint8_t)(AddrSel & 0x000000ff);
    tAD[3] = wb;

    WIZCHIP_CRITICAL_ENTER();
    WIZCHIP.CS._select();

#if( (_WIZCHIP_IO_MODE_ == _WIZCHIP_IO_MODE_SPI_VDM_))
    tAD[2] |= (_W6100_SPI_WRITE_ | _W6100_SPI_OP_);
    if (!WIZCHIP.IF.SPI._write_burst) {	// byte operation
        WIZCHIP.IF.SPI._write_byte(tAD[0]);
        WIZCHIP.IF.SPI._write_byte(tAD[1]);
        WIZCHIP.IF.SPI._write_byte(tAD[2]);
        WIZCHIP.IF.SPI._write_byte(tAD[3]);
    } else {
        WIZCHIP.IF.SPI._write_burst(tAD, 4);
    }
#elif ( (_WIZCHIP_IO_MODE_ == _WIZCHIP_IO_MODE_BUS_INDIR_) )
#if 1
    // 20231103 taylor
    WIZCHIP.IF.BUS._write_data_buf(IDM_AR0, tAD, 4, 1);
#else
    WIZCHIP.IF.BUS._write_data(IDM_AR0, tAD, 4, 1);
#endif
#else
#error "Unknown _WIZCHIP_IO_MODE_ in W6100. !!!"
#endif

    WIZCHIP.CS._deselect();
    WIZCHIP_CRITICAL_EXIT();
}

uint8_t  WIZCHIP_READ(uint32_t AddrSel) {
    uint8_t ret;
    uint8_t tAD[3];
    tAD[0] = (uint8_t)((AddrSel & 0x00FF0000) >> 16);
    tAD[1] = (uint8_t)((AddrSel & 0x0000FF00) >> 8);
    tAD[2] = (uint8_t)(AddrSel & 0x000000ff);

    WIZCHIP_CRITICAL_ENTER();
    WIZCHIP.CS._select();

#if( (_WIZCHIP_IO_MODE_ ==  _WIZCHIP_IO_MODE_SPI_VDM_))
    tAD[2] |= (_W6100_SPI_READ_ | _W6100_SPI_OP_);
    if (!WIZCHIP.IF.SPI._read_burst || !WIZCHIP.IF.SPI._write_burst) {	// byte operation
        WIZCHIP.IF.SPI._write_byte(tAD[0]);
        WIZCHIP.IF.SPI._write_byte(tAD[1]);
        WIZCHIP.IF.SPI._write_byte(tAD[2]);
    } else {
        WIZCHIP.IF.SPI._write_burst(tAD, 3);
    }
    ret = WIZCHIP.IF.SPI._read_byte();
#elif ( (_WIZCHIP_IO_MODE_ == _WIZCHIP_IO_MODE_BUS_INDIR_) )
    WIZCHIP.IF.BUS._write_data_buf(IDM_AR0, tAD, 3, 1);
    ret = WIZCHIP.IF.BUS._read_data(IDM_DR);
#else
#error "Unknown _WIZCHIP_IO_MODE_ in W6100. !!!"
#endif

    WIZCHIP.CS._deselect();
    WIZCHIP_CRITICAL_EXIT();
    return ret;
}

void WIZCHIP_WRITE_BUF(uint32_t AddrSel, uint8_t* pBuf, uint16_t len) {
    uint8_t tAD[3];
    uint16_t i = 0;

    tAD[0] = (uint8_t)((AddrSel & 0x00FF0000) >> 16);
    tAD[1] = (uint8_t)((AddrSel & 0x0000FF00) >> 8);
    tAD[2] = (uint8_t)(AddrSel & 0x000000ff);

    WIZCHIP_CRITICAL_ENTER();
    WIZCHIP.CS._select();

#if((_WIZCHIP_IO_MODE_ == _WIZCHIP_IO_MODE_SPI_VDM_))
    tAD[2] |= (_W6100_SPI_WRITE_ | _W6100_SPI_OP_);
    if (!WIZCHIP.IF.SPI._write_burst) {	// byte operation
        WIZCHIP.IF.SPI._write_byte(tAD[0]);
        WIZCHIP.IF.SPI._write_byte(tAD[1]);
        WIZCHIP.IF.SPI._write_byte(tAD[2]);
        for (i = 0; i < len; i++) {
            WIZCHIP.IF.SPI._write_byte(pBuf[i]);
        }
    } else {
        WIZCHIP.IF.SPI._write_burst(tAD, 3);
        WIZCHIP.IF.SPI._write_burst(pBuf, len);
    }
#elif ( (_WIZCHIP_IO_MODE_ == _WIZCHIP_IO_MODE_BUS_INDIR_) )
    WIZCHIP.IF.BUS._write_data_buf(IDM_AR0, tAD, 3, 1);
    WIZCHIP.IF.BUS._write_data_buf(IDM_DR, pBuf, len, 0);
#else
#error "Unknown _WIZCHIP_IO_MODE_ in W6100. !!!!"
#endif

    WIZCHIP.CS._deselect();
    WIZCHIP_CRITICAL_EXIT();
}

void WIZCHIP_READ_BUF(uint32_t AddrSel, uint8_t* pBuf, uint16_t len) {
    uint8_t tAD[3];
    uint16_t i;
    tAD[0] = (uint8_t)((AddrSel & 0x00FF0000) >> 16);
    tAD[1] = (uint8_t)((AddrSel & 0x0000FF00) >> 8);
    tAD[2] = (uint8_t)(AddrSel & 0x000000ff);

    WIZCHIP_CRITICAL_ENTER();
    WIZCHIP.CS._select();

#if((_WIZCHIP_IO_MODE_ == _WIZCHIP_IO_MODE_SPI_VDM_))
    tAD[2] |= (_W6100_SPI_READ_ | _W6100_SPI_OP_);
    if (!WIZCHIP.IF.SPI._read_burst || !WIZCHIP.IF.SPI._write_burst) {	// byte operation
        WIZCHIP.IF.SPI._write_byte(tAD[0]);
        WIZCHIP.IF.SPI._write_byte(tAD[1]);
        WIZCHIP.IF.SPI._write_byte(tAD[2]);
        for (i = 0; i < len; len++) {
            pBuf[i] = WIZCHIP.IF.SPI._read_byte();
        }
    } else {
        WIZCHIP.IF.SPI._write_burst(tAD, 3);
        WIZCHIP.IF.SPI._read_burst(pBuf, len);
    }

#elif ( (_WIZCHIP_IO_MODE_ == _WIZCHIP_IO_MODE_BUS_INDIR_) )
    WIZCHIP.IF.BUS._write_data_buf(IDM_AR0, tAD, 3, 1);
    WIZCHIP.IF.BUS._read_data_buf(IDM_DR, pBuf, len, 0);
#else
#error "Unknown _WIZCHIP_IO_MODE_ in W6100. !!!!"
#endif
    WIZCHIP.CS._deselect();
    WIZCHIP_CRITICAL_EXIT();
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
