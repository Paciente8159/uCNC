//*****************************************************************************
//
//! \file socket.c
//! \brief SOCKET APIs Implements file.
//! \details SOCKET APIs like as Berkeley Socket APIs.
//! \version 1.0.3
//! \date 2013/10/21
//! \par  Revision history
//!       <2015/02/05> Notice
//!        The version history is not updated after this point.
//!        Download the latest version directly from GitHub. Please visit the our GitHub repository for ioLibrary.
//!        >> https://github.com/Wiznet/ioLibrary_Driver
//!       <2014/05/01> V1.0.3. Refer to M20140501
//!         1. Implicit type casting -> Explicit type casting.
//!         2. replace 0x01 with PACK_REMAINED in recvfrom()
//!         3. Validation a destination ip in connect() & sendto():
//!            It occurs a fatal error on converting unint32 address if uint8* addr parameter is not aligned by 4byte address.
//!            Copy 4 byte addr value into temporary uint32 variable and then compares it.
//!       <2013/12/20> V1.0.2 Refer to M20131220
//!                    Remove Warning.
//!       <2013/11/04> V1.0.1 2nd Release. Refer to "20131104".
//!                    In sendto(), Add to clear timeout interrupt status (Sn_IR_TIMEOUT)
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
#include "socket.h"

//M20150401 : Typing Error
//#define SOCK_ANY_PORT_NUM  0xC000;
#define SOCK_ANY_PORT_NUM  0xC000

static uint16_t sock_any_port = SOCK_ANY_PORT_NUM;
static uint16_t sock_io_mode = 0;
static uint16_t sock_is_sending = 0;

static uint16_t sock_remained_size[_WIZCHIP_SOCK_NUM_] = {0, 0,};

//M20150601 : For extern decleation
//static uint8_t  sock_pack_info[_WIZCHIP_SOCK_NUM_] = {0,};
uint8_t  sock_pack_info[_WIZCHIP_SOCK_NUM_] = {0,};
//

#if _WIZCHIP_ == 5200
static uint16_t sock_next_rd[_WIZCHIP_SOCK_NUM_] = {0,};
#endif

//A20150601 : For integrating with W5300
#if _WIZCHIP_ == 5300
uint8_t sock_remained_byte[_WIZCHIP_SOCK_NUM_] = {0,}; // set by wiz_recv_data()
#endif


#define CHECK_SOCKNUM()   \
   do{                    \
      if(sn >= _WIZCHIP_SOCK_NUM_) return SOCKERR_SOCKNUM;   \
   }while(0);             \

#define CHECK_SOCKMODE(mode)  \
   do{                     \
      if((getSn_MR(sn) & 0x0F) != mode) return SOCKERR_SOCKMODE;  \
   }while(0);              \

#define CHECK_TCPMODE()                                           \
   do{                                                            \
      if((getSn_MR(sn) & 0x03) != 0x01) return SOCKERR_SOCKMODE;  \
   }while(0);

#define CHECK_SOCKINIT()   \
   do{                     \
      if((getSn_SR(sn) != SOCK_INIT)) return SOCKERR_SOCKINIT; \
   }while(0);              \

#define CHECK_SOCKDATA()   \
   do{                     \
      if(len == 0) return SOCKERR_DATALEN;   \
   }while(0);              \
//teddy 240122
#if _WIZCHIP_ == W6100 || _WIZCHIP_ == W6300
#define CHECK_TCPMODE()                                           \
   do{                                                            \
      if((getSn_MR(sn) & 0x03) != 0x01) return SOCKERR_SOCKMODE;  \
   }while(0);

#define CHECK_UDPMODE()                                           \
   do{                                                            \
      if((getSn_MR(sn) & 0x03) != 0x02) return SOCKERR_SOCKMODE;  \
   }while(0);

#define CHECK_IPMODE()                                            \
   do{                                                            \
      if((getSn_MR(sn) & 0x07) != 0x03) return SOCKERR_SOCKMODE;  \
   }while(0);

#define CHECK_DGRAMMODE()                                         \
   do{                                                            \
      if(getSn_MR(sn) == Sn_MR_CLOSED) return SOCKERR_SOCKMODE;   \
      if((getSn_MR(sn) & 0x03) == 0x01) return SOCKERR_SOCKMODE;  \
   }while(0);

#define CHECK_IPZERO(addr, addrlen)                                  \
   do{                                                               \
      uint16_t ipzero= 0;                                            \
      for(uint8_t i=0; i<addrlen; i++)  ipzero += (uint16_t)addr[i]; \
      if (ipzero == 0) return SOCKERR_IPINVALID;                     \
   }while(0);


#endif


#if (_WIZCHIP_ > W5500)
#define IPV6_AVAILABLE
#endif

#if 1


#define Sn_MR_TCP4           (Sn_MR_TCP)   ///< Refer to @ref Sn_MR_TCP.
#define Sn_MR_UDP4           (Sn_MR_UDP)   ///< Refer to @ref Sn_MR_UDP
#define Sn_MR_IPRAW4         (Sn_MR_IPRAW)   ///< Refer to @ref Sn_MR_IPRAW.   
#define Sn_MR_TCP6           (0x09)
#define Sn_MR_UDP6           (0x0A) //0x1010
#define Sn_MR_IPRAW6         (0x0B) //0x1011
#define Sn_MR_TCPD           (0x0D)
#define Sn_MR_UDPD           (0x0E)



#endif


#if 0 // By lihan  
static uint8_t addrlenTEST = -1 ;

void setAddrlen_W6x00(uint8_t num) {
    addrlenTEST = num;
}

uint8_t  checkAddrlen_W6x00() {
    //if (addrlenTEST < 0 )
    if ((addrlenTEST != 4)  && (addrlenTEST != 16)) {
        perror("Error: addrlen is not initialized");
    } else {
        printf("addrlenTEST %d \r\n", addrlenTEST) ;
    }
    return addrlenTEST;
}

inline void inline_setAddrlen_W6x00(uint8_t num) {
#if (_WIZCHIP_ == 6100) || (_WIZCHIP_ == 6300)
    setAddrlen_W6x00(num);
#endif
}

inline uint8_t inline_CheckAddrlen_W6x00(void) {
#if (_WIZCHIP_ == 6100) || (_WIZCHIP_ == 6300)
    return  checkAddrlen_W6x00();
#else
    return 4;
#endif
}
#endif




int8_t socket(uint8_t sn, uint8_t protocol, uint16_t port, uint8_t flag) {

    uint8_t taddr[16];
    uint16_t local_port = 0;
    CHECK_SOCKNUM();
    switch (protocol & 0x0F) {
#ifdef IPV6_AVAILABLE
    case Sn_MR_TCP4 :
        getSIPR(taddr);
        CHECK_IPZERO(taddr, 4);
        break;
    case Sn_MR_TCP6 :
        getLLAR(taddr);
        CHECK_IPZERO(taddr, 16);
        //getGUAR(taddr);
        //CHECK_IPZERO(taddr, 16);
        break;
    case Sn_MR_TCPD :
        getSIPR(taddr);
        CHECK_IPZERO(taddr, 4);
        getLLAR(taddr);
        CHECK_IPZERO(taddr, 16);
        //getGUAR(taddr);
        //CHECK_IPZERO(taddr, 16);
        break;
#else
    case Sn_MR_TCP : {
        //M20150601 : Fixed the warning - taddr will never be NULL
        /*
            uint8_t taddr[4];
            getSIPR(taddr);
        */
        uint32_t taddr;
        getSIPR((uint8_t*)&taddr);
        if (taddr == 0) {
            return SOCKERR_SOCKINIT;
        }
        break;
    }
#endif
    case Sn_MR_UDP :
    case Sn_MR_UDP6 :
    case Sn_MR_UDPD :
    case Sn_MR_MACRAW :
    case Sn_MR_IPRAW4 :
    case Sn_MR_IPRAW6 :
        break;
#if ( _WIZCHIP_ < 5200 )
    case Sn_MR_PPPoE :
        break;
#endif
    default :
        return SOCKERR_SOCKMODE;
    }
    //M20150601 : For SF_TCP_ALIGN & W5300
    //if((flag & 0x06) != 0) return SOCKERR_SOCKFLAG;
    if ((flag & 0x04) != 0) {
        return SOCKERR_SOCKFLAG;
    }
#if _WIZCHIP_ == 5200
    if (flag & 0x10) {
        return SOCKERR_SOCKFLAG;
    }
#endif

    if (flag != 0) {
        switch (protocol) {

#ifdef IPV6_AVAILABLE
        case Sn_MR_MACRAW:
            if ((flag & (SF_DHA_MANUAL | SF_FORCE_ARP)) != 0) {
                return SOCKERR_SOCKFLAG;
            }
            break;
        case Sn_MR_TCP4:
        case Sn_MR_TCP6:
        case Sn_MR_TCPD:
            if ((flag & (SF_MULTI_ENABLE | SF_UNI_BLOCK)) != 0) {
                return SOCKERR_SOCKFLAG;
            }
            break;
        case Sn_MR_IPRAW4:
        case Sn_MR_IPRAW6:
            if (flag != 0) {
                return SOCKERR_SOCKFLAG;
            }
            break;
#else
        case Sn_MR_TCP:
            //M20150601 :  For SF_TCP_ALIGN & W5300
#if _WIZCHIP_ == 5300
            if ((flag & (SF_TCP_NODELAY | SF_IO_NONBLOCK | SF_TCP_ALIGN)) == 0) {
                return SOCKERR_SOCKFLAG;
            }
#else
            if ((flag & (SF_TCP_NODELAY | SF_IO_NONBLOCK)) == 0) {
                return SOCKERR_SOCKFLAG;
            }
#endif

            break;
        case Sn_MR_UDP:
            if (flag & SF_IGMP_VER2) {
                if ((flag & SF_MULTI_ENABLE) == 0) {
                    return SOCKERR_SOCKFLAG;
                }
            }
#if _WIZCHIP_ == 5500
            if (flag & SF_UNI_BLOCK) {
                if ((flag & SF_MULTI_ENABLE) == 0) {
                    return SOCKERR_SOCKFLAG;
                }
            }
#endif
            break;

#endif

        default:
            break;
        }
    }
    close(sn);
    //M20150601
#if _WIZCHIP_ == 5300
    setSn_MR(sn, ((uint16_t)(protocol | (flag & 0xF0))) | (((uint16_t)(flag & 0x02)) << 7));
#else
    setSn_MR(sn, (protocol | (flag & 0xF0)));
#endif
#ifdef IPV6_AVAILABLE
    setSn_MR2(sn, flag & 0x03);
#endif
    if (!port) {
        port = sock_any_port++;
        if (sock_any_port == 0xFFF0) {
            sock_any_port = SOCK_ANY_PORT_NUM;
        }
    }
    setSn_PORTR(sn, port);
    setSn_CR(sn, Sn_CR_OPEN);
    while (getSn_CR(sn));
    //A20150401 : For release the previous sock_io_mode
    sock_io_mode &= ~(1 << sn);
    //
#ifndef IPV6_AVAILABLE
    sock_io_mode |= ((flag & SF_IO_NONBLOCK) << sn);
#else
    sock_io_mode |= ((flag & (SF_IO_NONBLOCK >> 3)) << sn);
#endif
    sock_is_sending &= ~(1 << sn);
    sock_remained_size[sn] = 0;
    //M20150601 : repalce 0 with PACK_COMPLETED
    //sock_pack_info[sn] = 0;
    sock_pack_info[sn] = PACK_COMPLETED;//PACK_COMPLETED //TODO::need verify:LINAN 20250421
    //
    while (getSn_SR(sn) == SOCK_CLOSED);
    return (int8_t)sn;
}

int8_t close(uint8_t sn) {
    CHECK_SOCKNUM();
    //A20160426 : Applied the erratum 1 of W5300
#if   (_WIZCHIP_ == 5300)
    //M20160503 : Wrong socket parameter. s -> sn
    //if( ((getSn_MR(s)& 0x0F) == Sn_MR_TCP) && (getSn_TX_FSR(s) != getSn_TxMAX(s)) )
    if (((getSn_MR(sn) & 0x0F) == Sn_MR_TCP) && (getSn_TX_FSR(sn) != getSn_TxMAX(sn))) {
        uint8_t destip[4] = {0, 0, 0, 1};
        // TODO
        // You can wait for completing to sending data;
        // wait about 1 second;
        // if you have completed to send data, skip the code of erratum 1
        // ex> wait_1s();
        //     if (getSn_TX_FSR(s) == getSn_TxMAX(s)) continue;
        //
        //M20160503 : The socket() of close() calls close() itself again. It occures a infinite loop - close()->socket()->close()->socket()-> ~
        //socket(s,Sn_MR_UDP,0x3000,0);
        //sendto(s,destip,1,destip,0x3000); // send the dummy data to an unknown destination(0.0.0.1).
        setSn_MR(sn, Sn_MR_UDP);
        setSn_PORTR(sn, 0x3000);
        setSn_CR(sn, Sn_CR_OPEN);
        while (getSn_CR(sn) != 0);
        while (getSn_SR(sn) != SOCK_UDP);
        sendto(sn, destip, 1, destip, 0x3000); // send the dummy data to an unknown destination(0.0.0.1).
    };
#endif
    setSn_CR(sn, Sn_CR_CLOSE);
    /* wait to process the command... */
    while (getSn_CR(sn));
    /* clear all interrupt of SOCKETn. */
    setSn_IR(sn, 0xFF);
    //A20150401 : Release the sock_io_mode of socket n.
    sock_io_mode &= ~(1 << sn);
    //
    sock_is_sending &= ~(1 << sn);
    sock_remained_size[sn] = 0;
    sock_pack_info[sn] = PACK_NONE;
    while (getSn_SR(sn) != SOCK_CLOSED);
    return SOCK_OK;
}

int8_t listen(uint8_t sn) {
    CHECK_SOCKNUM();
    CHECK_TCPMODE();
    CHECK_SOCKINIT();
    setSn_CR(sn, Sn_CR_LISTEN);
    while (getSn_CR(sn));
    while (getSn_SR(sn) != SOCK_LISTEN) {
        close(sn);
        return SOCKERR_SOCKCLOSED;
    }
    return SOCK_OK;
}
//int8_t connect (uint8_t sn, uint8_t * addr, uint16_t port )
int8_t connect_W5x00(uint8_t sn, uint8_t * addr, uint16_t port) {
    // printf(" W5x00 - connect - addrlen = %d \r\n" , 4 );
    // #ifdef IPV6_AVAILABLE
    // TODO :define how to work, when IPV6_AVAILABLE is defined
    // #endif
    return connect_IO_6(sn, addr, port, 4);
}

int8_t connect_W6x00(uint8_t sn, uint8_t * addr, uint16_t port, uint8_t addrlen) {
    // printf(" W6x00 - connect - addrlen = %d \r\n" , addrlen );
    // #ifdef IPV6_AVAILABLE
    // TODO :define how to work, when IPV6_AVAILABLE is defined
    // #endif
    return connect_IO_6(sn, addr, port, addrlen);
}

static int8_t connect_IO_6(uint8_t sn, uint8_t * addr, uint16_t port, uint8_t addrlen) {

    // printf(" connect - addrlen = %d \r\n" , addrlen );

    CHECK_SOCKNUM();
    CHECK_TCPMODE(); // same macro " CHECK_SOCKMODE(Sn_MR_TCP);"
    CHECK_SOCKINIT();

#ifdef IPV6_AVAILABLE
    CHECK_IPZERO(addr, addrlen);
#else
    //M20140501 : For avoiding fatal error on memory align mismatched
    //if( *((uint32_t*)addr) == 0xFFFFFFFF || *((uint32_t*)addr) == 0) return SOCKERR_IPINVALID;
    {
        uint32_t taddr;
        taddr = ((uint32_t)addr[0] & 0x000000FF);
        taddr = (taddr << 8) + ((uint32_t)addr[1] & 0x000000FF);
        taddr = (taddr << 8) + ((uint32_t)addr[2] & 0x000000FF);
        taddr = (taddr << 8) + ((uint32_t)addr[3] & 0x000000FF);
        if (taddr == 0xFFFFFFFF || taddr == 0) {
            return SOCKERR_IPINVALID;
        }
    }
#endif

    if (port == 0) {
        return SOCKERR_PORTZERO;
    }

    setSn_DPORTR(sn, port);

    if (addrlen == 16) {   // addrlen=16, Sn_MR_TCP6(1001), Sn_MR_TCPD(1101))
#ifdef IPV6_AVAILABLE
        if (getSn_MR(sn) & 0x08) {
            setSn_DIP6R(sn, addr);
            setSn_CR(sn, Sn_CR_CONNECT6);
        } else
#endif
            return SOCKERR_SOCKMODE;
    } else {       // addrlen=4, Sn_MR_TCP4(0001), Sn_MR_TCPD(1101)
        if (getSn_MR(sn) == Sn_MR_TCP6) {
            return SOCKERR_SOCKMODE;
        }
        setSn_DIPR(sn, addr);
        //setSn_DPORT(sn,port); //TODO::need verify:LINAN 20250421
        setSn_CR(sn, Sn_CR_CONNECT);
    }
    while (getSn_CR(sn));
    if (sock_io_mode & (1 << sn)) {
        return SOCK_BUSY;
    }
    while (getSn_SR(sn) != SOCK_ESTABLISHED) {
        if (getSn_IR(sn) & Sn_IR_TIMEOUT) {
            setSn_IR(sn, Sn_IR_TIMEOUT);
            return SOCKERR_TIMEOUT;
        }

        if (getSn_SR(sn) == SOCK_CLOSED) {
            return SOCKERR_SOCKCLOSED;
        }
    }

    return SOCK_OK;
}

int8_t disconnect(uint8_t sn) {
    CHECK_SOCKNUM();
    CHECK_TCPMODE();
    if (getSn_SR(sn) != SOCK_CLOSED) {
        setSn_CR(sn, Sn_CR_DISCON);
        /* wait to process the command... */
        while (getSn_CR(sn));
        sock_is_sending &= ~(1 << sn);
        if (sock_io_mode & (1 << sn)) {
            return SOCK_BUSY;
        }
        while (getSn_SR(sn) != SOCK_CLOSED) {
            if (getSn_IR(sn) & Sn_IR_TIMEOUT) {
                close(sn);
                return SOCKERR_TIMEOUT;
            }
        }
    }
    return SOCK_OK;
}


#if 1
int32_t send(uint8_t sn, uint8_t * buf, uint16_t len) {
    uint8_t tmp = 0;
    uint16_t freesize = 0;
    /*
        The below codes can be omitted for optmization of speed
    */
    //CHECK_SOCKNUM();
    //CHECK_TCPMODE(Sn_MR_TCP4);
    /************/
#ifndef IPV6_AVAILABLE
    CHECK_SOCKNUM();
    CHECK_SOCKMODE(Sn_MR_TCP);
    CHECK_SOCKDATA();
    tmp = getSn_SR(sn);
    if (tmp != SOCK_ESTABLISHED && tmp != SOCK_CLOSE_WAIT) {
        return SOCKERR_SOCKSTATUS;
    }
    if (sock_is_sending & (1 << sn)) {
        tmp = getSn_IR(sn);
        if (tmp & Sn_IR_SENDOK) {
            setSn_IR(sn, Sn_IR_SENDOK);
            //M20150401 : Typing Error
            //#if _WZICHIP_ == 5200
#if _WIZCHIP_ == 5200
            if (getSn_TX_RD(sn) != sock_next_rd[sn]) {
                setSn_CR(sn, Sn_CR_SEND);
                while (getSn_CR(sn));
                return SOCK_BUSY;
            }
#endif
            sock_is_sending &= ~(1 << sn);
        } else if (tmp & Sn_IR_TIMEOUT) {
            close(sn);
            return SOCKERR_TIMEOUT;
        } else {
            return SOCK_BUSY;
        }
    }
#endif
    freesize = getSn_TxMAX(sn);
    if (len > freesize) {
        len = freesize;    // check size not to exceed MAX size.
    }
    while (1) {
        freesize = (uint16_t)getSn_TX_FSR(sn);
        tmp = getSn_SR(sn);
        if ((tmp != SOCK_ESTABLISHED) && (tmp != SOCK_CLOSE_WAIT)) {
            if (tmp == SOCK_CLOSED) {
                close(sn);
            }
            return SOCKERR_SOCKSTATUS;
        }
        if ((sock_io_mode & (1 << sn)) && (len > freesize)) {
            return SOCK_BUSY;    //TODO::need verify:LINAN 20250421
        }
        // if( sock_io_mode & (1<<sn) ) return SOCK_BUSY;  //TODO::need verify:LINAN 20250421
        if (len <= freesize) {
            break;
        }
    }
    wiz_send_data(sn, buf, len);
#if _WIZCHIP_ == 5200
    sock_next_rd[sn] = getSn_TX_RD(sn) + len;
#endif

#if _WIZCHIP_ == 5300
    setSn_TX_WRSR(sn, len);
#endif
    if (sock_is_sending & (1 << sn)) {
        while (!(getSn_IR(sn) & Sn_IR_SENDOK)) {
            tmp = getSn_SR(sn);
            if ((tmp != SOCK_ESTABLISHED) && (tmp != SOCK_CLOSE_WAIT)) {
                if ((tmp == SOCK_CLOSED) || (getSn_IR(sn) & Sn_IR_TIMEOUT)) {
                    close(sn);
                }
                return SOCKERR_SOCKSTATUS;
            }
            if (sock_io_mode & (1 << sn)) {
                return SOCK_BUSY;
            }
        }
        setSn_IR(sn, Sn_IR_SENDOK);
    }
    setSn_CR(sn, Sn_CR_SEND);

    while (getSn_CR(sn));  // wait to process the command...
    sock_is_sending |= (1 << sn);

    return len;
}
#else //for speed optimization, by lihan
int32_t send(uint8_t sn, uint8_t * buf, uint16_t len) {
    uint8_t tmp = 0;
    uint16_t freesize = 0;

    // tx_bufferSize / 4

    //if (len > 4096) len = 4096; // check size not to exceed MAX size.//
    //if (len > 8192) len = 8192; // check size not to exceed MAX siz
    //if (len > 16384) len = 16384; // check size not to exceed MAX size.
    //if (len > 32768) len = 32768; // check size not to exceed MAX size.
#define __FREESIZE__(i)  1024 * i
#define __FREESIZE__Value 8
    if (len > __FREESIZE__(__FREESIZE__Value)) {
        len = __FREESIZE__(__FREESIZE__Value);    // check size not to exceed MAX size.//tse
    }

    while (1) {
        freesize = (uint16_t)getSn_TX_FSR(sn);
        if (len <= freesize) {
            break;
        }
    }
    wiz_send_data(sn, buf, len);
    setSn_CR(sn, Sn_CR_SEND);

    while (getSn_CR(sn));  // wait to process the command...
    sock_is_sending |= (1 << sn);

    return len;
}
#endif
int32_t recv(uint8_t sn, uint8_t * buf, uint16_t len) { //lihan
    uint8_t  tmp = 0;
    uint16_t recvsize = 0;
    /*
        The below codes can be omitted for optmization of speed
    */
    //A20150601 : For integarating with W5300
#if   _WIZCHIP_ == 5300
    uint8_t head[2];
    uint16_t mr;
#endif
    //
    CHECK_SOCKNUM();
    CHECK_SOCKMODE(Sn_MR_TCP);
    CHECK_SOCKDATA();

    recvsize = getSn_RxMAX(sn);
    if (recvsize < len) {
        len = recvsize;
    }

    //A20150601 : For Integrating with W5300
#if _WIZCHIP_ == 5300
    //sock_pack_info[sn] = PACK_COMPLETED;    // for clear
    if (sock_remained_size[sn] == 0) {
#endif
        //
        while (1) {
            recvsize = (uint16_t)getSn_RX_RSR(sn);
            tmp = getSn_SR(sn);
            if (tmp != SOCK_ESTABLISHED) {
                if (tmp == SOCK_CLOSE_WAIT) {
                    if (recvsize != 0) {
                        break;
                    } else if (getSn_TX_FSR(sn) == getSn_TxMAX(sn)) {
                        close(sn);
                        return SOCKERR_SOCKSTATUS;
                    }
                } else {
                    close(sn);
                    return SOCKERR_SOCKSTATUS;
                }
            }
#ifdef IPV6_AVAILABLE
            if (recvsize != 0) {
                break;
            }
            if (sock_io_mode & (1 << sn)) {
                return SOCK_BUSY;
            }
#else
            if (sock_io_mode & (1 << sn)) {
                return SOCK_BUSY;
            }
            if (recvsize != 0) {
                break;
            }
#endif
        };
#if _WIZCHIP_ == 5300
    }
#endif

    //A20150601 : For integrating with W5300
#if _WIZCHIP_ == 5300
    if ((sock_remained_size[sn] == 0) || (getSn_MR(sn) & Sn_MR_ALIGN)) {
        mr = getMR();
        if ((getSn_MR(sn) & Sn_MR_ALIGN) == 0) {
            wiz_recv_data(sn, head, 2);
            if (mr & MR_FS) {
                recvsize = (((uint16_t)head[1]) << 8) | ((uint16_t)head[0]);
            } else {
                recvsize = (((uint16_t)head[0]) << 8) | ((uint16_t)head[1]);
            }
            sock_pack_info[sn] = PACK_FIRST;
        }
        sock_remained_size[sn] = recvsize;
    }
    if (len > sock_remained_size[sn]) {
        len = sock_remained_size[sn];
    }
    recvsize = len;
    if (sock_pack_info[sn] & PACK_FIFOBYTE) {
        *buf = sock_remained_byte[sn];
        buf++;
        sock_pack_info[sn] &= ~(PACK_FIFOBYTE);
        recvsize -= 1;
        sock_remained_size[sn] -= 1;
    }
    if (recvsize != 0) {
        wiz_recv_data(sn, buf, recvsize);
        setSn_CR(sn, Sn_CR_RECV);
        while (getSn_CR(sn));
    }
    sock_remained_size[sn] -= recvsize;
    if (sock_remained_size[sn] != 0) {
        sock_pack_info[sn] |= PACK_REMAINED;
        if (recvsize & 0x1) {
            sock_pack_info[sn] |= PACK_FIFOBYTE;
        }
    } else {
        sock_pack_info[sn] = PACK_COMPLETED;
    }
    if (getSn_MR(sn) & Sn_MR_ALIGN) {
        sock_remained_size[sn] = 0;
    }
    //len = recvsize;
#else
    if (recvsize < len) {
        len = recvsize;
    }
    wiz_recv_data(sn, buf, len);
    setSn_CR(sn, Sn_CR_RECV);
    while (getSn_CR(sn));
#endif

    //M20150409 : Explicit Type Casting
    //return len;
    return (int32_t)len;
}


int32_t sendto_W5x00(uint8_t sn, uint8_t * buf, uint16_t len, uint8_t * addr, uint16_t port) {
    //static int32_t sendto_IO_6(uint8_t sn, uint8_t * buf, uint16_t len, uint8_t * addr, uint16_t port)
    // printf("sendto_W5x00\r\n" ) ;
    return sendto_IO_6(sn,   buf,  len,   addr,  port, 4);
}

int32_t sendto_W6x00(uint8_t sn, uint8_t * buf, uint16_t len, uint8_t * addr, uint16_t port, uint8_t addrlen) {
    // printf("sendto_W6x00\r\n" ) ;
    //static int32_t sendto_IO_6(uint8_t sn, uint8_t * buf, uint16_t len, uint8_t * addr, uint16_t port)
    return sendto_IO_6(sn,  buf,  len,   addr,  port, addrlen);
}

static int32_t sendto_IO_6(uint8_t sn, uint8_t * buf, uint16_t len, uint8_t * addr, uint16_t port, uint8_t addrlen) {
    uint8_t tmp = 0;
    uint8_t tcmd = Sn_CR_SEND;
    uint16_t freesize = 0;
    uint32_t taddr;

    /*
        The below codes can be omitted for optmization of speed
    */
    CHECK_SOCKNUM();
    //CHECK_DGRAMMODE();
    /************/
    switch (getSn_MR(sn) & 0x0F) {
    case Sn_MR_UDP:
    case Sn_MR_MACRAW:
    //         break;
    //   #if ( _WIZCHIP_ < 5200 )
    case Sn_MR_IPRAW:
    case Sn_MR_IPRAW6:
        break;
    //   #endif
    default:
        return SOCKERR_SOCKMODE;
    }
    tmp = getSn_MR(sn);
    if (tmp != Sn_MR_MACRAW) {
        if (addrlen == 16) {    // addrlen=16, Sn_MR_UDP6(1010), Sn_MR_UDPD(1110)), IPRAW6(1011)
#ifdef IPV6_AVAILABLE
            if (tmp & 0x08) {
                setSn_DIP6R(sn, addr);
                tcmd = Sn_CR_SEND6;
            } else
#endif
                return SOCKERR_SOCKMODE;
        } else if (addrlen == 4) { // addrlen=4, Sn_MR_UDP4(0010), Sn_MR_UDPD(1110), IPRAW4(0011)
            if (tmp == Sn_MR_UDP6 || tmp == Sn_MR_IPRAW6) {
                return SOCKERR_SOCKMODE;
            }
            setSn_DIPR(sn, addr);
            tcmd = Sn_CR_SEND;
        } else {
            return SOCKERR_IPINVALID;
        }
    }
    if ((tmp & 0x03) == 0x02) { // Sn_MR_UPD4(0010), Sn_MR_UDP6(1010), Sn_MR_UDPD(1110)
        if (port) {
            setSn_DPORTR(sn, port);
        } else {
            return SOCKERR_PORTZERO;
        }
    }
#ifndef IPV6_AVAILABLE
    CHECK_SOCKDATA();
    //M20140501 : For avoiding fatal error on memory align mismatched
    //if(*((uint32_t*)addr) == 0) return SOCKERR_IPINVALID;
    //{
    //uint32_t taddr;
    taddr = ((uint32_t)addr[0]) & 0x000000FF;
    taddr = (taddr << 8) + ((uint32_t)addr[1] & 0x000000FF);
    taddr = (taddr << 8) + ((uint32_t)addr[2] & 0x000000FF);
    taddr = (taddr << 8) + ((uint32_t)addr[3] & 0x000000FF);
    //}
    //
    //if(*((uint32_t*)addr) == 0) return SOCKERR_IPINVALID;
    if ((taddr == 0) && ((getSn_MR(sn)&Sn_MR_MACRAW) != Sn_MR_MACRAW)) {
        return SOCKERR_IPINVALID;
    }
    if ((port  == 0) && ((getSn_MR(sn)&Sn_MR_MACRAW) != Sn_MR_MACRAW)) {
        return SOCKERR_PORTZERO;
    }
    tmp = getSn_SR(sn);
    //#if ( _WIZCHIP_ < 5200 )
    if ((tmp != SOCK_MACRAW) && (tmp != SOCK_UDP) && (tmp != SOCK_IPRAW)) {
        return SOCKERR_SOCKSTATUS;
    }
    //#else
    //   if(tmp != SOCK_MACRAW && tmp != SOCK_UDP) return SOCKERR_SOCKSTATUS;
    //#endif

    setSn_DIPR(sn, addr);
    setSn_DPORT(sn, port);
#endif

    freesize = getSn_TxMAX(sn);
    if (len > freesize) {
        len = freesize;    // check size not to exceed MAX size.
    }

    while (1) {
        freesize = getSn_TX_FSR(sn);
        if (getSn_SR(sn) == SOCK_CLOSED) {
            return SOCKERR_SOCKCLOSED;
        }
        if ((sock_io_mode & (1 << sn)) && (len > freesize)) {
            return SOCK_BUSY;
        }
        if (len <= freesize) {
            break;
        }
    };
    wiz_send_data(sn, buf, len);

#if _WIZCHIP_ < 5500   //M20150401 : for WIZCHIP Errata #4, #5 (ARP errata)
    getSIPR((uint8_t *)&taddr);
    if (taddr == 0) {
        getSUBR((uint8_t*)&taddr);
        setSUBR((uint8_t*)"\x00\x00\x00\x00");
    } else {
        taddr = 0;
    }
#endif

#ifdef IPV6_AVAILABLE
    setSn_CR(sn, tcmd);
#else
    //A20150601 : For W5300
#if _WIZCHIP_ == 5300
    setSn_TX_WRSR(sn, len);
#endif
    //
    setSn_CR(sn, Sn_CR_SEND);
#endif
    /* wait to process the command... */
    while (getSn_CR(sn));
    while (1) {
        tmp = getSn_IR(sn);
        if (tmp & Sn_IR_SENDOK) {
            setSn_IR(sn, Sn_IR_SENDOK);
            break;
        }
        //M:20131104
        //else if(tmp & Sn_IR_TIMEOUT) return SOCKERR_TIMEOUT;
        else if (tmp & Sn_IR_TIMEOUT) {
            setSn_IR(sn, Sn_IR_TIMEOUT);
            //M20150409 : Fixed the lost of sign bits by type casting.
            //len = (uint16_t)SOCKERR_TIMEOUT;
            //break;
#if _WIZCHIP_ < 5500   //M20150401 : for WIZCHIP Errata #4, #5 (ARP errata)
            if (taddr) {
                setSUBR((uint8_t*)&taddr);
            }
#endif
            return SOCKERR_TIMEOUT;
        }
        ////////////
    }
#if _WIZCHIP_ < 5500   //M20150401 : for WIZCHIP Errata #4, #5 (ARP errata)
    if (taddr) {
        setSUBR((uint8_t*)&taddr);
    }
#endif
    //M20150409 : Explicit Type Casting
    //return len;
    return (int32_t)len;
}



int32_t recvfrom_W5x00(uint8_t sn, uint8_t * buf, uint16_t len, uint8_t * addr, uint16_t *port) {
    //int32_t recvfrom_IO_6(uint8_t sn, uint8_t * buf, uint16_t len, uint8_t * addr, uint16_t *port)
    // printf("recvfrom_W5x00\r\n" ) ;
    uint8_t addrlen = 4; //M20150601 : For W5300
    uint8_t *dummy = &addrlen;
    return recvfrom_IO_6(sn,   buf,  len,   addr,  port, dummy);
}

int32_t recvfrom_W6x00(uint8_t sn, uint8_t * buf, uint16_t len, uint8_t * addr, uint16_t *port, uint8_t *addrlen) {
    // printf("recvfrom_W6x00\r\n" ) ;
    //int32_t recvfrom_IO_6(uint8_t sn, uint8_t * buf, uint16_t len, uint8_t * addr, uint16_t *port)
    return recvfrom_IO_6(sn,  buf,  len,   addr,  port, addrlen);
}
static int32_t recvfrom_IO_6(uint8_t sn, uint8_t * buf, uint16_t len, uint8_t * addr, uint16_t *port, uint8_t *addrlen) { //TODO : WILL BE IMPROVED
    //M20150601 : For W5300
#if _WIZCHIP_ == 5300
    uint16_t mr;
    uint16_t mr1;
#else
    uint8_t  mr;
#endif
    //
    uint8_t  head[8];
    uint16_t pack_len = 0;

    /*
        The below codes can be omitted for optmization of speed
    */
    CHECK_SOCKNUM();
    //CHECK_DGRAMMODE();
    //CHECK_SOCKDATA();
    /************/
    //CHECK_SOCKMODE(Sn_MR_UDP);
    //A20150601
#if _WIZCHIP_ == 5300
    mr1 = getMR();
#endif

    switch ((mr = getSn_MR(sn)) & 0x0F) {
    case Sn_MR_UDP:
    case Sn_MR_IPRAW:
    case Sn_MR_IPRAW6:
    case Sn_MR_MACRAW:
        break;
#if ( _WIZCHIP_ < 5200 )
    case Sn_MR_PPPoE:
        break;
#endif
    default:
        return SOCKERR_SOCKMODE;
    }
    CHECK_SOCKDATA();
    if (sock_remained_size[sn] == 0) {
        while (1) {
            pack_len = getSn_RX_RSR(sn);
            if (getSn_SR(sn) == SOCK_CLOSED) {
                return SOCKERR_SOCKCLOSED;
            }
#ifndef IPV6_AVAILABLE
            if ((sock_io_mode & (1 << sn)) && (pack_len == 0)) {
                return SOCK_BUSY;
            }
            if (pack_len != 0) {
                break;
            }
#else
            if (pack_len != 0) {
                sock_pack_info[sn] = PACK_NONE;
                break;
            }
            if (sock_io_mode & (1 << sn)) {
                return SOCK_BUSY;
            }
#endif
        };
    }
#ifdef IPV6_AVAILABLE
    /* First read 2 bytes of PACKET INFO in SOCKETn RX buffer*/
    wiz_recv_data(sn, head, 2);
    setSn_CR(sn, Sn_CR_RECV);
    while (getSn_CR(sn));
    pack_len = head[0] & 0x07;
    pack_len = (pack_len << 8) + head[1];
#endif
    //D20150601 : Move it to bottom
    // sock_pack_info[sn] = PACK_COMPLETED;
    switch (mr & 0x07) {
    case Sn_MR_UDP4 :
    case Sn_MR_UDP6:
    case Sn_MR_UDPD:
#ifdef IPV6_AVAILABLE
        if (addr == 0) {
            return SOCKERR_ARG;
        }

        sock_pack_info[sn] = head[0] & 0xF8;

        if (sock_pack_info[sn] & PACK_IPv6) {
            *addrlen = 16 ;
        } else {
            *addrlen = 4 ;
        }
        wiz_recv_data(sn, addr, *addrlen);
        setSn_CR(sn, Sn_CR_RECV);

        while (getSn_CR(sn));

#else
        if (sock_remained_size[sn] == 0) {
            wiz_recv_data(sn, head, 8);
            setSn_CR(sn, Sn_CR_RECV);
            while (getSn_CR(sn));
            // read peer's IP address, port number & packet length
            //A20150601 : For W5300
#if _WIZCHIP_ == 5300
            if (mr1 & MR_FS) {
                addr[0] = head[1];
                addr[1] = head[0];
                addr[2] = head[3];
                addr[3] = head[2];
                *port = head[5];
                *port = (*port << 8) + head[4];
                sock_remained_size[sn] = head[7];
                sock_remained_size[sn] = (sock_remained_size[sn] << 8) + head[6];
            } else {
#endif
                addr[0] = head[0];
                addr[1] = head[1];
                addr[2] = head[2];
                addr[3] = head[3];
                *port = head[4];
                *port = (*port << 8) + head[5];
                sock_remained_size[sn] = head[6];
                sock_remained_size[sn] = (sock_remained_size[sn] << 8) + head[7];
#if _WIZCHIP_ == 5300
            }
#endif
            sock_pack_info[sn] = PACK_FIRST;
        }
        if (len < sock_remained_size[sn]) {
            pack_len = len;
        } else {
            pack_len = sock_remained_size[sn];
        }
        //A20150601 : For W5300
        len = pack_len;
#if _WIZCHIP_ == 5300
        if (sock_pack_info[sn] & PACK_FIFOBYTE) {
            *buf++ = sock_remained_byte[sn];
            pack_len -= 1;
            sock_remained_size[sn] -= 1;
            sock_pack_info[sn] &= ~PACK_FIFOBYTE;
        }
#endif
        //
        // Need to packet length check (default 1472)
        //
        wiz_recv_data(sn, buf, pack_len); // data copy.
#endif
        break;
    case Sn_MR_MACRAW :
        if (sock_remained_size[sn] == 0) {
#ifndef IPV6_AVAILABLE
            wiz_recv_data(sn, head, 2);
            setSn_CR(sn, Sn_CR_RECV);
            while (getSn_CR(sn));
#endif
            // read peer's IP address, port number & packet length
            sock_remained_size[sn] = head[0];
            sock_remained_size[sn] = (sock_remained_size[sn] << 8) + head[1] - 2;
#if _WIZCHIP_ == W5300
            if (sock_remained_size[sn] & 0x01) {
                sock_remained_size[sn] = sock_remained_size[sn] + 1 - 4;
            } else {
                sock_remained_size[sn] -= 4;
            }
#endif
            if (sock_remained_size[sn] > 1514) {
                close(sn);
                return SOCKFATAL_PACKLEN;
            }
            sock_pack_info[sn] = PACK_FIRST;
        }
        if (len < sock_remained_size[sn]) {
            pack_len = len;
        } else {
            pack_len = sock_remained_size[sn];
        }
        wiz_recv_data(sn, buf, pack_len);
        break;
    //#if ( _WIZCHIP_ < 5200 )
    case Sn_MR_IPRAW6:
    case Sn_MR_IPRAW4 :
        if (sock_remained_size[sn] == 0) {
#ifndef IPV6_AVAILABLE
            wiz_recv_data(sn, head, 6);
            setSn_CR(sn, Sn_CR_RECV);
            while (getSn_CR(sn));
            addr[0] = head[0];
            addr[1] = head[1];
            addr[2] = head[2];
            addr[3] = head[3];
            sock_remained_size[sn] = head[4];
            //M20150401 : For Typing Error
            //sock_remaiend_size[sn] = (sock_remained_size[sn] << 8) + head[5];
            sock_remained_size[sn] = (sock_remained_size[sn] << 8) + head[5];
            sock_pack_info[sn] = PACK_FIRST;
            //
            // Need to packet length check
            //
            if (len < sock_remained_size[sn]) {
                pack_len = len;
            } else {
                pack_len = sock_remained_size[sn];
            }
            wiz_recv_data(sn, buf, pack_len); // data copy.
#else
            if (*addr == 0) {
                return SOCKERR_ARG;
            }
            sock_pack_info[sn] = head[0] & 0xF8;
            if (sock_pack_info[sn] & PACK_IPv6) {
                *addrlen = 16;
            } else {
                *addrlen = 4;
            }
            wiz_recv_data(sn, addr, *addrlen);
            setSn_CR(sn, Sn_CR_RECV);
            while (getSn_CR(sn));

#endif
        }
        break;
    default:
        wiz_recv_ignore(sn, pack_len); // data copy.
        sock_remained_size[sn] = pack_len;
        break;
    }
#ifdef IPV6_AVAILABLE
    sock_remained_size[sn] = pack_len;
    sock_pack_info[sn] |= PACK_FIRST;
    if ((getSn_MR(sn) & 0x03) == 0x02) { // Sn_MR_UDP4(0010), Sn_MR_UDP6(1010), Sn_MR_UDPD(1110)
        /* Read port number of PACKET INFO in SOCKETn RX buffer */
        if (port == 0) {
            return SOCKERR_ARG;
        }
        wiz_recv_data(sn, head, 2);
        *port = (((((uint16_t)head[0])) << 8) + head[1]);
        setSn_CR(sn, Sn_CR_RECV);
        while (getSn_CR(sn));
    }

    if (len < sock_remained_size[sn]) {
        pack_len = len;
    } else {
        pack_len = sock_remained_size[sn];
    }
    wiz_recv_data(sn, buf, pack_len);
    setSn_CR(sn, Sn_CR_RECV);
    /* wait to process the command... */
    while (getSn_CR(sn)) ;

    sock_remained_size[sn] -= pack_len;
    if (sock_remained_size[sn] != 0) {
        sock_pack_info[sn] |= PACK_REMAINED;
    } else {
        sock_pack_info[sn] |= PACK_COMPLETED;
    }

#else
    setSn_CR(sn, Sn_CR_RECV);
    /* wait to process the command... */
    while (getSn_CR(sn)) ;
    sock_remained_size[sn] -= pack_len;
    //M20150601 :
    //if(sock_remained_size[sn] != 0) sock_pack_info[sn] |= 0x01;
    if (sock_remained_size[sn] != 0) {
        sock_pack_info[sn] |= PACK_REMAINED;
#if _WIZCHIP_ == 5300
        if (pack_len & 0x01) {
            sock_pack_info[sn] |= PACK_FIFOBYTE;
        }
#endif
    } else {
        sock_pack_info[sn] = PACK_COMPLETED;
    }
#if _WIZCHIP_ == 5300
    pack_len = len;
#endif
    //
    //M20150409 : Explicit Type Casting
    //return pack_len;
#endif
    return (int32_t)pack_len;
}


int8_t  ctlsocket(uint8_t sn, ctlsock_type cstype, void* arg) {
    uint8_t tmp = 0;
    CHECK_SOCKNUM();
    tmp = *((uint8_t*)arg);
    switch (cstype) {
    case CS_SET_IOMODE:
        if (tmp == SOCK_IO_NONBLOCK) {
            sock_io_mode |= (1 << sn);
        } else if (tmp == SOCK_IO_BLOCK) {
            sock_io_mode &= ~(1 << sn);
        } else {
            return SOCKERR_ARG;
        }
        break;
    case CS_GET_IOMODE:
        //M20140501 : implict type casting -> explict type casting
        //*((uint8_t*)arg) = (sock_io_mode >> sn) & 0x0001;
        *((uint8_t*)arg) = (uint8_t)((sock_io_mode >> sn) & 0x0001);
        //
        break;
    case CS_GET_MAXTXBUF:
        *((uint16_t*)arg) = getSn_TxMAX(sn);
        break;
    case CS_GET_MAXRXBUF:
        *((uint16_t*)arg) = getSn_RxMAX(sn);
        break;
    case CS_CLR_INTERRUPT:
        if (tmp > SIK_ALL) {
            return SOCKERR_ARG;
        }
        setSn_IR(sn, tmp);
        break;
    case CS_GET_INTERRUPT:
        *((uint8_t*)arg) = getSn_IR(sn);
        break;
#if _WIZCHIP_ != 5100
    case CS_SET_INTMASK:
        if (tmp > SIK_ALL) {
            return SOCKERR_ARG;
        }
        setSn_IMR(sn, tmp);
        break;
    case CS_GET_INTMASK:
        *((uint8_t*)arg) = getSn_IMR(sn);
        break;
#endif
#ifdef IPV6_AVAILABLE
    case CS_SET_PREFER:
        if ((tmp & 0x03) == 0x01) {
            return SOCKERR_ARG;
        }
        setSn_PSR(sn, tmp);
        break;
    case CS_GET_PREFER:
        *(uint8_t*) arg = getSn_PSR(sn);
        break;
#endif
    default:
        return SOCKERR_ARG;
    }
    return SOCK_OK;
}

int8_t  setsockopt(uint8_t sn, sockopt_type sotype, void* arg) {
    // M20131220 : Remove warning
    //uint8_t tmp;
    CHECK_SOCKNUM();
    switch (sotype) {
    case SO_TTL:
        setSn_TTL(sn, *(uint8_t*)arg);
        break;
    case SO_TOS:
        setSn_TOS(sn, *(uint8_t*)arg);
        break;
    case SO_MSS:
        setSn_MSSR(sn, *(uint16_t*)arg);
        break;
    case SO_DESTIP:
#ifdef IPV6_AVAILABLE
        if (((wiz_IPAddress *)arg)->len == 16) {
            setSn_DIP6R(sn, ((wiz_IPAddress*)arg)->ip);
        } else
#endif
            setSn_DIPR(sn, (uint8_t*)arg);
        break;
    case SO_DESTPORT:
        setSn_DPORTR(sn, *(uint16_t*)arg);
        break;
#if _WIZCHIP_ != 5100
    case SO_KEEPALIVESEND:
        CHECK_TCPMODE();
#if _WIZCHIP_ > 5200
        if (getSn_KPALVTR(sn) != 0) {
            return SOCKERR_SOCKOPT;
        }
#endif
        setSn_CR(sn, Sn_CR_SEND_KEEP);
        while (getSn_CR(sn) != 0) {
            // M20131220
            //if ((tmp = getSn_IR(sn)) & Sn_IR_TIMEOUT)
            if (getSn_IR(sn) & Sn_IR_TIMEOUT) {
                setSn_IR(sn, Sn_IR_TIMEOUT);
                return SOCKERR_TIMEOUT;
            }
        }
        break;
#if _WIZCHIP_ > 5200
    case SO_KEEPALIVEAUTO:
        CHECK_TCPMODE();
        setSn_KPALVTR(sn, *(uint8_t*)arg);
        break;
#endif
#endif
    default:
        return SOCKERR_ARG;
    }
    return SOCK_OK;
}

int8_t getsockopt(uint8_t sn, sockopt_type sotype, void* arg) {
    CHECK_SOCKNUM();
    switch (sotype) {
    case SO_FLAG:
#ifdef IPV6_AVAILABLE
        *(uint8_t*)arg = (getSn_MR(sn) & 0xF0) | (getSn_MR2(sn)) | ((uint8_t)(((sock_io_mode >> sn) & 0x0001) << 3));
#endif
        *(uint8_t*)arg = getSn_MR(sn) & 0xF0;
        break;
    case SO_TTL:
        *(uint8_t*) arg = getSn_TTL(sn);
        break;
    case SO_TOS:
        *(uint8_t*) arg = getSn_TOS(sn);
        break;
    case SO_MSS:
        *(uint16_t*) arg = getSn_MSSR(sn);
        break;
    case SO_DESTIP:
#ifdef IPV6_AVAILABLE
        CHECK_TCPMODE();
        if (getSn_ESR(sn) & TCPSOCK_MODE) { //IPv6 ?
            getSn_DIP6R(sn, ((wiz_IPAddress*)arg)->ip);
            ((wiz_IPAddress*)arg)->len = 16;
        } else {
            getSn_DIPR(sn, ((wiz_IPAddress*)arg)->ip);
            ((wiz_IPAddress*)arg)->len = 4;
        }
        break;
#else
        getSn_DIPR(sn, (uint8_t*)arg);
        break;
#endif
    case SO_DESTPORT:
        *(uint16_t*) arg = getSn_DPORTR(sn);
        break;
#if  _WIZCHIP_ > 5200
    case SO_KEEPALIVEAUTO:
        CHECK_TCPMODE();
        *(uint16_t*) arg = getSn_KPALVTR(sn);
        break;
#endif
    case SO_SENDBUF:
        *(uint16_t*) arg = getSn_TX_FSR(sn);
        break;
    case SO_RECVBUF:
        *(uint16_t*) arg = getSn_RX_RSR(sn);
        break;
    case SO_STATUS:
        *(uint8_t*) arg = getSn_SR(sn);
        break;
#ifdef IPV6_AVAILABLE
    case SO_EXTSTATUS:
        CHECK_TCPMODE();
        *(uint8_t*) arg = getSn_ESR(sn) & 0x07;
        break;
    case SO_REMAINSIZE:
        if (getSn_MR(sn) == SOCK_CLOSED) {
            return SOCKERR_SOCKSTATUS;
        }
        if (getSn_MR(sn) & 0x01) {
            *(uint16_t*)arg = getSn_RX_RSR(sn);
        } else {
            *(uint16_t*)arg = sock_remained_size[sn];
        }
        break;
    case SO_PACKINFO:
        if (getSn_MR(sn) == SOCK_CLOSED) {
            return SOCKERR_SOCKSTATUS;
        }
        if (getSn_MR(sn) & 0x01) {
            return SOCKERR_SOCKMODE;
        } else {
            *(uint8_t*)arg = sock_pack_info[sn];
        }
        break;
    case SO_MODE:
        *(uint8_t*) arg = 0x0F & getSn_MR(sn);
        break;
#else
    case SO_REMAINSIZE:
        if (getSn_MR(sn) & Sn_MR_TCP) {
            *(uint16_t*)arg = getSn_RX_RSR(sn);
        } else {
            *(uint16_t*)arg = sock_remained_size[sn];
        }
        break;
    case SO_PACKINFO  :
        //CHECK_SOCKMODE(Sn_MR_TCP);
#if _WIZCHIP_ != 5300
        if ((getSn_MR(sn) == Sn_MR_TCP)) {
            return SOCKERR_SOCKMODE;
        }
#endif
        *(uint8_t*)arg = sock_pack_info[sn];
        break;

#endif
    default:
        return SOCKERR_SOCKOPT;
    }
    return SOCK_OK;
}

#ifdef IPV6_AVAILABLE
int16_t peeksockmsg(uint8_t sn, uint8_t* submsg, uint16_t subsize) {
    uint32_t rx_ptr = 0;
    uint16_t i = 0, sub_idx = 0;

    if ((getSn_RX_RSR(sn) > 0) && (subsize > 0)) {
        rx_ptr = ((uint32_t)getSn_RX_RD(sn) << 8)  + WIZCHIP_RXBUF_BLOCK(sn);
        sub_idx = 0;
        for (i = 0; i < getSn_RX_RSR(sn) ; i++) {
            if (WIZCHIP_READ(rx_ptr) == submsg[sub_idx]) {
                sub_idx++;
                if (sub_idx == subsize) {
                    return (i + 1 - sub_idx);
                }
            } else {
                sub_idx = 0;
            }
            rx_ptr = WIZCHIP_OFFSET_INC(rx_ptr, 1);
        }
    }
    return -1;
}


#endif

