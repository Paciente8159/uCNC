//*****************************************************************************
//
//! \file socket.h
//! \brief SOCKET APIs Header file.
//! \details SOCKET APIs like as berkeley socket api.
//! \version 1.0.2
//! \date 2013/10/21
//! \par  Revision history
//!       <2015/02/05> Notice
//!        The version history is not updated after this point.
//!        Download the latest version directly from GitHub. Please visit the our GitHub repository for ioLibrary.
//!        >> https://github.com/Wiznet/ioLibrary_Driver
//!       <2014/05/01> V1.0.2. Refer to M20140501
//!         1. Modify the comment : SO_REMAINED -> PACK_REMAINED
//!         2. Add the comment as zero byte udp data reception in getsockopt().
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
    @defgroup WIZnet_socket_APIs 1. WIZnet socket APIs
    @brief WIZnet socket APIs are based on Berkeley socket APIs,  thus it has much similar name and interface.
          But there is a little bit of difference.
    @details
    <b> Comparison between WIZnet and Berkeley SOCKET APIs </b>
    <table>
      <tr>   <td><b>API</b></td> <td><b>WIZnet</b></td> <td><b>Berkeley</b></td>   </tr>
      <tr>   <td>socket()</td> <td>O</td> <td>O</td>   </tr>
      <tr>   <td><b>bind()</b></td> <td>X</td> <td>O</td>   </tr>
      <tr>   <td><b>listen()</b></td> <td>O</td> <td>O</td>   </tr>
      <tr>   <td><b>connect()</b></td> <td>O</td> <td>O</td>   </tr>
      <tr>   <td><b>accept()</b></td> <td>X</td> <td>O</td>   </tr>
      <tr>   <td><b>recv()</b></td> <td>O</td> <td>O</td>    </tr>
      <tr>   <td><b>send()</b></td> <td>O</td> <td>O</td>   </tr>
      <tr>   <td><b>recvfrom()</b></td> <td>O</td> <td>O</td>   </tr>
      <tr>   <td><b>sendto()</b></td> <td>O</td> <td>O</td>    </tr>
      <tr>   <td><b>closesocket()</b></td> <td>O<br>close() & disconnect()</td> <td>O</td>   </tr>
    </table>
    There are @b bind() and @b accept() functions in @b Berkeley SOCKET API but,
    not in @b WIZnet SOCKET API. Because socket() of WIZnet is not only creating a SOCKET but also binding a local port number,
    and listen() of WIZnet is not only listening to connection request from client but also accepting the connection request. \n
    When you program "TCP SERVER" with Berkeley SOCKET API, you can use only one listen port.
    When the listen SOCKET accepts a connection request from a client, it keeps listening.
    After accepting the connection request, a new SOCKET is created and the new SOCKET is used in communication with the client. \n
    Following figure shows network flow diagram by Berkeley SOCKET API.
    @image html Berkeley_SOCKET.jpg "<Berkeley SOCKET API>"
    But, When you program "TCP SERVER" with WIZnet SOCKET API, you can use as many as 8 listen SOCKET with same port number. \n
    Because there's no accept() in WIZnet SOCKET APIs, when the listen SOCKET accepts a connection request from a client,
    it is changed in order to communicate with the client.
    And the changed SOCKET is not listening any more and is dedicated for communicating with the client. \n
    If there're many listen SOCKET with same listen port number and a client requests a connection,
    the SOCKET which has the smallest SOCKET number accepts the request and is changed as communication SOCKET. \n
    Following figure shows network flow diagram by WIZnet SOCKET API.
    @image html WIZnet_SOCKET.jpg "<WIZnet SOCKET API>"
*/
#ifndef _SOCKET_H_
#define _SOCKET_H_
#ifdef __cplusplus
extern "C" {
#endif

#include "wizchip_conf.h"

#define SOCKET                uint8_t  ///< SOCKET type define for legacy driver

#define SOCK_OK               1        ///< Result is OK about socket process.
#define SOCK_BUSY             0        ///< Socket is busy on processing the operation. Valid only Non-block IO Mode.
#define SOCK_FATAL            -1000    ///< Result is fatal error about socket process.

#define SOCK_ERROR            0
#define SOCKERR_SOCKNUM       (SOCK_ERROR - 1)     ///< Invalid socket number
#define SOCKERR_SOCKOPT       (SOCK_ERROR - 2)     ///< Invalid socket option
#define SOCKERR_SOCKINIT      (SOCK_ERROR - 3)     ///< Socket is not initialized or SIPR is Zero IP address when Sn_MR_TCP
#define SOCKERR_SOCKCLOSED    (SOCK_ERROR - 4)     ///< Socket unexpectedly closed.
#define SOCKERR_SOCKMODE      (SOCK_ERROR - 5)     ///< Invalid socket mode for socket operation.
#define SOCKERR_SOCKFLAG      (SOCK_ERROR - 6)     ///< Invalid socket flag
#define SOCKERR_SOCKSTATUS    (SOCK_ERROR - 7)     ///< Invalid socket status for socket operation.
#define SOCKERR_ARG           (SOCK_ERROR - 10)    ///< Invalid argument.
#define SOCKERR_PORTZERO      (SOCK_ERROR - 11)    ///< Port number is zero
#define SOCKERR_IPINVALID     (SOCK_ERROR - 12)    ///< Invalid IP address
#define SOCKERR_TIMEOUT       (SOCK_ERROR - 13)    ///< Timeout occurred
#define SOCKERR_DATALEN       (SOCK_ERROR - 14)    ///< Data length is zero or greater than buffer max size.
#define SOCKERR_BUFFER        (SOCK_ERROR - 15)    ///< Socket buffer is not enough for data communication.

#define SOCKFATAL_PACKLEN     (SOCK_FATAL - 1)     ///< Invalid packet length. Fatal Error.

#if (_WIZCHIP_ == W5100 || _WIZCHIP_ == W5100S || _WIZCHIP_ == W5200 || _WIZCHIP_ == W5300 || _WIZCHIP_ == W5500)
/*
    SOCKET FLAG
*/
#define SF_ETHER_OWN           (Sn_MR_MFEN)        ///< In @ref Sn_MR_MACRAW, Receive only the packet as broadcast, multicast and own packet
#define SF_IGMP_VER2           (Sn_MR_MC)          ///< In @ref Sn_MR_UDP with \ref SF_MULTI_ENABLE, Select IGMP version 2.   
#define SF_TCP_NODELAY         (Sn_MR_ND)          ///< In @ref Sn_MR_TCP, Use to nodelayed ack.
#define SF_MULTI_ENABLE        (Sn_MR_MULTI)       ///< In @ref Sn_MR_UDP, Enable multicast mode.

#define Sn_MR2_DHAM          (1<<1)
#define SF_DHA_MANUAL        (Sn_MR2_DHAM)
#define Sn_MR2_FARP          (1<<0)
#define SF_FORCE_ARP         (Sn_MR2_FARP)


#if _WIZCHIP_ == 5500
#define SF_BROAD_BLOCK         (Sn_MR_BCASTB)   ///< In @ref Sn_MR_UDP or @ref Sn_MR_MACRAW, Block broadcast packet. Valid only in W5500
#define SF_MULTI_BLOCK         (Sn_MR_MMB)      ///< In @ref Sn_MR_MACRAW, Block multicast packet. Valid only in W5500
#define SF_IPv6_BLOCK          (Sn_MR_MIP6B)    ///< In @ref Sn_MR_MACRAW, Block IPv6 packet. Valid only in W5500
#define SF_UNI_BLOCK           (Sn_MR_UCASTB)   ///< In @ref Sn_MR_UDP with \ref SF_MULTI_ENABLE. Valid only in W5500
#endif

//A201505 : For W5300
#if _WIZCHIP_ == 5300
#define SF_TCP_ALIGN		     0x02			   ///< Valid only \ref Sn_MR_TCP and W5300, refer to \ref Sn_MR_ALIGN
#endif

#define SF_IO_NONBLOCK           0x01              ///< Socket nonblock io mode. It used parameter in \ref socket().

/*
    UDP & MACRAW Packet Infomation
*/
#define PACK_FIRST               0x80              ///< In Non-TCP packet, It indicates to start receiving a packet. (When W5300, This flag can be applied)
#define PACK_REMAINED            0x01              ///< In Non-TCP packet, It indicates to remain a packet to be received. (When W5300, This flag can be applied)
#define PACK_COMPLETED           0x00              ///< In Non-TCP packet, It indicates to complete to receive a packet. (When W5300, This flag can be applied)
//A20150601 : For Integrating with W5300
#define PACK_FIFOBYTE            0x02              ///< Valid only W5300, It indicate to have read already the Sn_RX_FIFOR.
//
//teddy 240122

#define PACK_IPv6            (1<<7)                ///< It indicates the destination IP address of the received packet is IPv6 or IPv4.
#define PACK_IPV6_ALLNODE    (PACK_IPv6 | (1<<6))  ///< It indicates the destination IP address of the received packet is allnode multicast(broadcast) address or not.
#define PACK_IPV6_MULTI      (PACK_IPv6 | (1<<5))  ///< It indicates the destination IP address of the received packet is multicast address or not.
#define PACK_IPV6_LLA        (PACK_IPv6 | (1<<4))  ///< It indicates the destination IP address of the received packet is lla or gua.
#define PACK_NONE            (0x00)                ///< It indicates no information of a packet

#elif ((_WIZCHIP_ == 6100) || (_WIZCHIP_ == 6300))

/*
    - @ref Sn_MR_MULTI : Support UDP Multicasting
    - @ref Sn_MR_MF    : Support MAC Filter Enable
    - @ref Sn_MR_BRDB  : Broadcast Block
    - @ref Sn_MR_FPSH  : Force PSH flag
    - @ref Sn_MR_ND    : No Delay ACK flag
    - @ref Sn_MR_MC    : IGMP ver2, ver1
    - @ref Sn_MR_SMB   : Solicited Multicast Block
    - @ref Sn_MR_MMB   : IPv4 Multicast block
    - @ref Sn_MR_UNIB  : Unicast Block
    - @ref Sn_MR_MMB6  : IPv6 UDP Multicast Block </b>

    - @ref Sn_MR2_DHAM : @ref Sn_MR2_DHAM_AUTO, @ref Sn_MR2_DHAM_MANUAL
    - @ref Sn_MR_FARP
*/

/*
    SOCKET FLAG
*/
/**
    @brief In UDP mode such as @ref Sn_MR_UDP4 and @ref Sn_MR_UDP6, @ref Sn_MR_UDP6, Enable multicast mode. When @ref Sn_MR_UDP6, Enable only IPv6 Multicating.
*/
#define SF_MULTI_ENABLE      (Sn_MR_MULTI)
#define SF_ETHER_OWN         (Sn_MR_MF)       ///< In MACRAW mode such as @ref Sn_MR_MACRAW, Receive only the packet as broadcast, multicast and own packet

/**
    @brief In UDP mode such as @ref Sn_MR_UDP4, @ref Sn_MR_UDP6 and @ref Sn_MR_UDPD, or In MACRAW mode sucha as @ref Sn_MR_MACRAW, Block a broadcast packet.
*/
#define SF_BROAD_BLOCK       (Sn_MR_BRDB)
#define SF_TCP_FPSH          (Sn_MR_FPSH)       ///< In TCP mode such as @ref Sn_MR_TCP4, @ref Sn_MR_TCP6 and @ref Sn_MR_TCPD, Use to forced push flag.

#define SF_TCP_NODELAY       (Sn_MR_ND)       ///< In TCP mode such as @ref Sn_MR_TCP4, @ref Sn_MR_TCP6 and @ref Sn_MR_TCPD, Use to nodelayed ack.
#define SF_IGMP_VER2         (Sn_MR_MC)       ///< In UDP mode such as @ref Sn_MR_UDP4 with @ref SF_MULTI_ENABLE, Select IGMP version 2.   
#define SF_SOLICIT_BLOCK     (Sn_MR_SMB)      ///< In UDP mode such as @ref Sn_MR_UDP6 and @ref Sn_MR_UDPD, Block a solicited mutlicast packet.
#define SF_ETHER_MULTI4B     (Sn_MR_MMB4)     ///< In MACRAW mode such as @ref Sn_MR_MACRAW with @ref SF_MULTI_ENABLE, Block a IPv4 multicast packet. 

#define SF_UNI_BLOCK         (Sn_MR_UNIB)     ///< In UDP mdoe such as @ref Sn_MR_UDP4, @ref Sn_MR_UDP6 and @ref Sn_MR_UDPD with @ref SF_MULTI_ENABLE, Block a unicast packet. 
#define SF_ETHER_MULIT6B     (Sn_MR_MMB6)     ///< In MACRAW mode such as @ref Sn_MR_MACRAW with @ref SF_MULTI_ENABLE, Block a IPv6 multicast packet. 

/**
    @brief Force to APR.
    @details In datagram mode such as @ref Sn_MR_IPRAW4, @ref Sn_MR_IPRAW6, @ref Sn_MR_UDP4, @ref Sn_MR_UDP6, and @ref Sn_MR_UDPD,
            Force to request ARP before a packet is sent to a destination.\n
            In TCP mode such as @ref Sn_MR_TCP4, @ref Sn_MR_TCP6, and @ref Sn_MR_TCPD and <b>TCP SERVER</b> operation mode,
            Force to request ARP before SYN/ACK packet is sent to a <b>TCP CLIENT</b>. \n
            When @ref SF_DHA_MANUAL is set, the ARP is process but the destination hardware address is fixed by user.
*/
#define SF_FORCE_ARP         (Sn_MR2_FARP)

/**
    @brief The destination hardware address of packet to be transmitted is set by user through @ref _Sn_DHAR_. It is invalid in MACRAW mode such as @ref Sn_MR_MACRAW.
*/
#define SF_DHA_MANUAL        (Sn_MR2_DHAM)

#define SF_IO_NONBLOCK       (0x01 << 3)     ///< Socket nonblock io mode. It used parameter in @ref socket().

/*
    UDP, IPRAW, MACRAW Packet Infomation
*/
#define PACK_IPv6            (1<<7)                ///< It indicates the destination IP address of the received packet is IPv6 or IPv4.
#define PACK_IPV6_ALLNODE    (PACK_IPv6 | (1<<6))  ///< It indicates the destination IP address of the received packet is allnode multicast(broadcast) address or not.
#define PACK_IPV6_MULTI      (PACK_IPv6 | (1<<5))  ///< It indicates the destination IP address of the received packet is multicast address or not.
#define PACK_IPV6_LLA        (PACK_IPv6 | (1<<4))  ///< It indicates the destination IP address of the received packet is lla or gua.
#define PACK_COMPLETED       (1<<3)                ///< It indicates the read data is last in the received packet.
#define PACK_REMAINED        (1<<2)                ///< It indicates to remain data in the received packet
#define PACK_FIRST           (1<<1)                ///< It indicates the read data is first in the received packet.
#define PACK_NONE            (0x00)                ///< It indicates no information of a packet

#define SRCV6_PREFER_AUTO    (PSR_AUTO)            ///< Soruce IPv6 address is preferred to auto-selection. Refer to @ref _Sn_PSR_
#define SRCV6_PREFER_LLA     (PSR_LLA)             ///< Soruce IPv6 address is preferred to link local address. Refer to @ref _Sn_PSR_
#define SRCV6_PREFER_GUA     (PSR_GUA)             ///< Soruce IPv6 address is preferred to global unique address. Refer to @ref _Sn_PSR_

#define TCPSOCK_MODE         (Sn_ESR_TCPM)         ///< It indicates the IP version when SOCKETn is opened as TCP6 or TCPD mode.(0 - IPv4 , 1 - IPv6)
#define TCPSOCK_OP           (Sn_ESR_TCPOP)        ///< It indicates the operation mode when SOCKETn is connected.(0 - <b>TCP CLIENT</b> , 1 - <b>TCP SERVER</b>)
#define TCPSOCK_SIP          (Sn_ESR_IP6T)         ///< It indicates the source ip address type when SOCKET is connected. (0 - Link Local, 1 - Global Unique) 

/////////////////////////////
// SOCKET CONTROL & OPTION //
/////////////////////////////
#define SOCK_IO_BLOCK         0  ///< Socket Block IO Mode in @ref setsockopt().
#define SOCK_IO_NONBLOCK      1  ///< Socket Non-block IO Mode in @ref setsockopt().
#endif

/**
    @ingroup WIZnet_socket_APIs
    @brief Open a socket.
    @details Initializes the socket with 'sn' passed as parameter and open.

    @param sn Socket number. It should be <b>0 ~ @ref \_WIZCHIP_SOCK_NUM_</b>.
    @param protocol Protocol type to operate such as TCP, UDP and MACRAW.
    @param port Port number to be bined.
    @param flag Socket flags as \ref SF_ETHER_OWN, \ref SF_IGMP_VER2, \ref SF_TCP_NODELAY, \ref SF_MULTI_ENABLE, \ref SF_IO_NONBLOCK and so on.\n
               Valid flags only in W5500 : @ref SF_BROAD_BLOCK, @ref SF_MULTI_BLOCK, @ref SF_IPv6_BLOCK, and @ref SF_UNI_BLOCK.
    @sa Sn_MR

    @return @b Success : The socket number @b 'sn' passed as parameter\n
           @b Fail    :\n @ref SOCKERR_SOCKNUM     - Invalid socket number\n
                          @ref SOCKERR_SOCKMODE    - Not support socket mode as TCP, UDP, and so on. \n
                          @ref SOCKERR_SOCKFLAG    - Invaild socket flag.
*/
int8_t  socket(uint8_t sn, uint8_t protocol, uint16_t port, uint8_t flag);

/**
    @ingroup WIZnet_socket_APIs
    @brief Close a socket.
    @details It closes the socket  with @b'sn' passed as parameter.

    @param sn Socket number. It should be <b>0 ~ @ref \_WIZCHIP_SOCK_NUM_</b>.

    @return @b Success : @ref SOCK_OK \n
           @b Fail    : @ref SOCKERR_SOCKNUM - Invalid socket number
*/
int8_t  close(uint8_t sn);

/**
    @ingroup WIZnet_socket_APIs
    @brief Listen to a connection request from a client.
    @details It is listening to a connection request from a client.
    If connection request is accepted successfully, the connection is established. Socket sn is used in passive(server) mode.

    @param sn Socket number. It should be <b>0 ~ @ref \_WIZCHIP_SOCK_NUM_</b>.
    @return @b Success : @ref SOCK_OK \n
           @b Fail    :\n @ref SOCKERR_SOCKINIT   - Socket is not initialized \n
                          @ref SOCKERR_SOCKCLOSED - Socket closed unexpectedly.
*/
int8_t  listen(uint8_t sn);

//teddy 240122
/**
    @ingroup WIZnet_socket_APIs
    @brief Try to connect to a <b>TCP SERVER</b>.
    @details It sends a connection-reqeust message to the server with destination IP address and port number passed as parameter.\n
            SOCKET <i>sn</i> is used as active(<b>TCP CLIENT</b>) mode.
    @param sn SOCKET number. It should be <b>0 ~ @ref _WIZCHIP_SOCK_NUM_</b>.
    @param addr Pointer variable of destination IPv6 or IPv4 address.
    @param port Destination port number.
    @param addrlen the length of <i>addr</i>. \n <- removed
                  If addr is IPv6 address it should be 16,else if addr is IPv4 address it should be 4. Otherwize, return @ref SOCKERR_IPINVALID.
    @return Success : @ref SOCK_OK \n
           Fail    :\n @ref SOCKERR_SOCKNUM   - Invalid socket number\n
                       @ref SOCKERR_SOCKMODE  - Invalid socket mode\n
                       @ref SOCKERR_SOCKINIT  - Socket is not initialized\n
                       @ref SOCKERR_IPINVALID - Wrong server IP address\n
                       @ref SOCKERR_PORTZERO  - Server port zero\n
                       @ref SOCKERR_TIMEOUT   - Timeout occurred during request connection\n
                       @ref SOCK_BUSY         - In non-block io mode, it returns immediately\n
    @note It is valid only in TCP client mode. \n
         In block io mode, it does not return until connection is completed. \n
         In Non-block io mode(@ref SF_IO_NONBLOCK), it returns @ref SOCK_BUSY immediately.
*/
static int8_t connect_IO_6(uint8_t sn, uint8_t * addr, uint16_t port, uint8_t addrlen);
//int8_t connect(uint8_t sn, uint8_t * addr, uint16_t port, uint8_t addrlen);

/**
    @ingroup WIZnet_socket_APIs
    @brief Try to disconnect a connection socket.
    @details It sends request message to disconnect the TCP socket 'sn' passed as parameter to the server or client.
    @note It is valid only in TCP server or client mode. \n
         In block io mode, it does not return until disconnection is completed. \n
         In Non-block io mode, it return @ref SOCK_BUSY immediately. \n

    @param sn Socket number. It should be <b>0 ~ @ref \_WIZCHIP_SOCK_NUM_</b>.
    @return @b Success :   @ref SOCK_OK \n
           @b Fail    :\n @ref SOCKERR_SOCKNUM  - Invalid socket number \n
                          @ref SOCKERR_SOCKMODE - Invalid operation in the socket \n
                          @ref SOCKERR_TIMEOUT  - Timeout occurred \n
                          @ref SOCK_BUSY        - Socket is busy.
*/
int8_t  disconnect(uint8_t sn);

/**
    @ingroup WIZnet_socket_APIs
    @brief	Send data to the connected peer in TCP socket.
    @details It is used to send outgoing data to the connected socket.
    @note    It is valid only in TCP server or client mode. It can't send data greater than socket buffer size. \n
            In block io mode, It doesn't return until data send is completed - socket buffer size is greater than data. \n
            In non-block io mode, It return @ref SOCK_BUSY immediately when socket buffer is not enough. \n
    @param sn Socket number. It should be <b>0 ~ @ref \_WIZCHIP_SOCK_NUM_</b>.
    @param buf Pointer buffer containing data to be sent.
    @param len The byte length of data in buf.
    @return	@b Success : The sent data size \n
            @b Fail    : \n @ref SOCKERR_SOCKSTATUS - Invalid socket status for socket operation \n
                            @ref SOCKERR_TIMEOUT    - Timeout occurred \n
                            @ref SOCKERR_SOCKMODE 	- Invalid operation in the socket \n
                            @ref SOCKERR_SOCKNUM    - Invalid socket number \n
                            @ref SOCKERR_DATALEN    - zero data length \n
                            @ref SOCK_BUSY          - Socket is busy.
*/
int32_t send(uint8_t sn, uint8_t * buf, uint16_t len);

/**
    @ingroup WIZnet_socket_APIs
    @brief	Receive data from the connected peer.
    @details It is used to read incoming data from the connected socket.\n
            It waits for data as much as the application wants to receive.
    @note    It is valid only in TCP server or client mode. It can't receive data greater than socket buffer size. \n
            In block io mode, it doesn't return until data reception is completed - data is filled as <I>len</I> in socket buffer. \n
            In non-block io mode, it return @ref SOCK_BUSY immediately when <I>len</I> is greater than data size in socket buffer. \n

    @param sn  Socket number. It should be <b>0 ~ @ref \_WIZCHIP_SOCK_NUM_</b>.
    @param buf Pointer buffer to read incoming data.
    @param len The max data length of data in buf.
    @return	@b Success : The real received data size \n
            @b Fail    :\n
                       @ref SOCKERR_SOCKSTATUS - Invalid socket status for socket operation \n
                       @ref SOCKERR_SOCKMODE   - Invalid operation in the socket \n
                       @ref SOCKERR_SOCKNUM    - Invalid socket number \n
                       @ref SOCKERR_DATALEN    - zero data length \n
                       @ref SOCK_BUSY          - Socket is busy.
*/
int32_t recv(uint8_t sn, uint8_t * buf, uint16_t len);

/**
    @ingroup WIZnet_socket_APIs
    @brief Send datagram to the peer specifed by destination IP address and port number passed as parameter.
    @details It sends datagram data by using UDP,IPRAW, or MACRAW mode SOCKET.
    @param sn SOCKET number. It should be <b>0 ~ @ref _WIZCHIP_SOCK_NUM_</b>.
    @param buf Pointer of data buffer to be sent.
    @param len The byte length of data in buf.
    @param addr Pointer variable of destination IPv6 or IPv4 address.
    @param port Destination port number.
    @param addrlen the length of <i>addr</i>. \n
                  If addr is IPv6 address it should be 16,else if addr is IPv4 address it should be 4. Otherwize, return @ref SOCKERR_IPINVALID.
    @return Success : The real sent data size. It may be equal to <i>len</i> or small.\n
           Fail    :\n @ref SOCKERR_SOCKNUM     - Invalid SOCKET number \n
                       @ref SOCKERR_SOCKMODE    - Invalid operation in the SOCKET \n
                       @ref SOCKERR_SOCKSTATUS  - Invalid SOCKET status for SOCKET operation \n
                       @ref SOCKERR_IPINVALID   - Invalid IP address\n
                       @ref SOCKERR_PORTZERO    - Destination port number is zero\n
                       @ref SOCKERR_DATALEN     - Invalid data length \n
                       @ref SOCKERR_SOCKCLOSED  - SOCKET unexpectedly closed \n
                       @ref SOCKERR_TIMEOUT     - Timeout occurred \n
                       @ref SOCK_BUSY           - SOCKET is busy.
    @note It is valid only in @ref Sn_MR_UDP4, @ref Sn_MR_UDP6, @ref Sn_MR_UDPD, @ref Sn_MR_IPRAW4, @ref Sn_MR_IPRAW6, and @ref Sn_MR_MACRAW. \n
         In UDP mode, It can send data as many as SOCKET RX buffer size if data is greater than SOCKET TX buffer size. \n
         In IPRAW and MACRAW mode, It should send data as many as MTU(maxium transmission unit) if data is greater than MTU. That is, <i>len</i> can't exceed to MTU.
         In block io mode, It doesn't return until data send is completed.
         In non-block io mode(@ref SF_IO_NONBLOCK), It return @ref SOCK_BUSY immediately when SOCKET transimttable buffer size is not enough.
*/
//int32_t sendto(uint8_t sn, uint8_t * buf, uint16_t len, uint8_t * addr, uint16_t port, uint8_t addrlen);
static int32_t sendto_IO_6(uint8_t sn, uint8_t * buf, uint16_t len, uint8_t * addr, uint16_t port, uint8_t addrlen);

/**
    @ingroup WIZnet_socket_APIs
    @brief Receive datagram from a peer
    @details It can read a data received from a peer by using UDP, IPRAW, or MACRAW mode SOCKET.
    @param sn   SOCKET number. It should be <b>0 ~ @ref _WIZCHIP_SOCK_NUM_</b>.
    @param buf  Pointer buffer to be saved the received data.
    @param len  The max read data length. \n
               When the received packet size <= <i>len</i>, it can read data as many as the packet size. \n
               When others, it can read data as many as len and remain to the rest data of the packet.
    @param addr Pointer variable of destination IP address.\n
               It is valid only when @ref recvfrom() is first called for receiving the datagram packet.
               You can check it valid or not through @ref PACK_FIRST. You can get it through @ref getsockopt(sn, @ref SO_PACKINFO, &packinfo).\n
               In UDP4, IPRAW mode SOCKET, it should be allocated over 4bytes. \n
               In UDP6, UDPD mode SOCKET, it should be allocated over 16bytes.
    @param port Pointer variable of destination port number. \n
               It is valid only when @ref recvfrom() is first called for receiving the datagram packet, same as <i>port</i> case.
    @param addrlen The byte length of destination IP address. \n
                  It is valid only when @ref recvfrom() is first called for receiving the datagram packet, same as <i>port</i> case.\n
                  When the destination has a IPv4 address, it is set to 4. \n
                  when the destination has a IPv6 address, it is set to 16.
    @return   Success : The real received data size. It may be equal to <i>len</i> or small.\n
            Fail    : @ref SOCKERR_SOCKMODE   - Invalid operation in the socket \n
                      @ref SOCKERR_SOCKNUM    - Invalid socket number \n
                      @ref SOCKERR_ARG        - Invalid parameter such as <i>addr</i>, <i>port</i>
                      @ref SOCK_BUSY          - SOCKET is busy.
    @note It is valid only in @ref Sn_MR_UDP4, @ref Sn_MR_UDP6, @ref Sn_MR_UDPD, @ref Sn_MR_IPRAW4, @ref Sn_MR_IPRAW6, and @ref Sn_MR_MACRAW. \n
         When SOCKET is opened with @ref Sn_MR_MACRAW or When it reads the the remained data of the previous datagram packet,
         the parameters such as <i>addr</i>, <i>port</i>, <i>addrlen</i> is ignored. \n
         Also, It can read data as many as the received datagram packet size if <i>len</i> is greater than the datagram packet size. \n
         In block io mode, it doesn't return until data reception is completed. that is, it waits until any datagram packet is received in SOCKET RX buffer. \n
         In non-block io mode(@ref SF_IO_NONBLOCK), it return @ref SOCK_BUSY immediately when SOCKET RX buffer is empty. \n
*/
//int32_t recvfrom(uint8_t sn, uint8_t * buf, uint16_t len, uint8_t * addr, uint16_t *port, uint8_t *addrlen);
static int32_t recvfrom_IO_6(uint8_t sn, uint8_t * buf, uint16_t len, uint8_t * addr, uint16_t *port, uint8_t *addrlen);


/////////////////////////////
// SOCKET CONTROL & OPTION //
/////////////////////////////
#define SOCK_IO_BLOCK         0  ///< Socket Block IO Mode in @ref setsockopt().
#define SOCK_IO_NONBLOCK      1  ///< Socket Non-block IO Mode in @ref setsockopt().

/**
    @defgroup DATA_TYPE DATA TYPE
*/

/**
    @ingroup DATA_TYPE
    @brief The kind of Socket Interrupt.
    @sa Sn_IR, Sn_IMR, setSn_IR(), getSn_IR(), setSn_IMR(), getSn_IMR()
*/
typedef enum {
    SIK_CONNECTED     = (1 << 0),    ///< connected
    SIK_DISCONNECTED  = (1 << 1),    ///< disconnected
    SIK_RECEIVED      = (1 << 2),    ///< data received
    SIK_TIMEOUT       = (1 << 3),    ///< timeout occurred
    SIK_SENT          = (1 << 4),    ///< send ok
    //M20150410 : Remove the comma of last member
    //SIK_ALL           = 0x1F,        ///< all interrupt
    SIK_ALL           = 0x1F         ///< all interrupt
} sockint_kind;

/**
    @ingroup DATA_TYPE
    @brief The type of @ref ctlsocket().
*/
typedef enum {
    CS_SET_IOMODE,          ///< set socket IO mode with @ref SOCK_IO_BLOCK or @ref SOCK_IO_NONBLOCK
    CS_GET_IOMODE,          ///< get socket IO mode
    CS_GET_MAXTXBUF,        ///< get the size of socket buffer allocated in TX memory
    CS_GET_MAXRXBUF,        ///< get the size of socket buffer allocated in RX memory
    CS_CLR_INTERRUPT,       ///< clear the interrupt of socket with @ref sockint_kind
    CS_GET_INTERRUPT,       ///< get the socket interrupt. refer to @ref sockint_kind

    //teddy 240122
    //#if _WIZCHIP_ == W6100 || _WIZCHIP_ == W6300
    CS_SET_PREFER,          ///< set the preferred source IPv6 address of transmission packet.\n Refer to @ref SRCV6_PREFER_AUTO, @ref SRCV6_PREFER_LLA and @ref SRCV6_PREFER_GUA.
    CS_GET_PREFER,          ///< get the preferred source IPv6 address of transmission packet.\n Refer to @ref SRCV6_PREFER_AUTO, @ref SRCV6_PREFER_LLA and @ref SRCV6_PREFER_GUA.
    //#endif
#if _WIZCHIP_ >= 5100
    CS_SET_INTMASK,         ///< set the interrupt mask of socket with @ref sockint_kind, Not supported in W5100
    CS_GET_INTMASK          ///< get the masked interrupt of socket. refer to @ref sockint_kind, Not supported in W5100
#endif
} ctlsock_type;


/**
    @ingroup DATA_TYPE
    @brief The type of socket option in @ref setsockopt() or @ref getsockopt()
*/
typedef enum {
    SO_FLAG,           ///< Valid only in getsockopt(), For set flag of socket refer to <I>flag</I> in @ref socket().
    SO_TTL,              ///< Set TTL. @ref Sn_TTL  ( @ref setSn_TTL(), @ref getSn_TTL() )
    SO_TOS,              ///< Set TOS. @ref Sn_TOS  ( @ref setSn_TOS(), @ref getSn_TOS() )
    SO_MSS,              ///< Set MSS. @ref Sn_MSSR ( @ref setSn_MSSR(), @ref getSn_MSSR() )
    SO_DESTIP,           ///< Set the destination IP address. @ref Sn_DIPR ( @ref setSn_DIPR(), @ref getSn_DIPR() )
    SO_DESTPORT,         ///< Set the destination Port number. @ref Sn_DPORT ( @ref setSn_DPORT(), @ref getSn_DPORT() )
#if _WIZCHIP_ != 5100
    SO_KEEPALIVESEND,    ///< Valid only in setsockopt. Manually send keep-alive packet in TCP mode, Not supported in W5100
#if !( (_WIZCHIP_ == 5100) || (_WIZCHIP_ == 5200) )
    SO_KEEPALIVEAUTO, ///< Set/Get keep-alive auto transmission timer in TCP mode, Not supported in W5100, W5200
#endif
#endif
    SO_SENDBUF,          ///< Valid only in getsockopt. Get the free data size of Socekt TX buffer. @ref Sn_TX_FSR, @ref getSn_TX_FSR()
    SO_RECVBUF,          ///< Valid only in getsockopt. Get the received data size in socket RX buffer. @ref Sn_RX_RSR, @ref getSn_RX_RSR()
    SO_STATUS,           ///< Valid only in getsockopt. Get the socket status. @ref Sn_SR, @ref getSn_SR()

    //teddy 240122
    //#if _WIZCHIP_ == W6100 || _WIZCHIP_ == W6300
    SO_EXTSTATUS,        ///< Valid only in @ref getsockopt(). Get the extended TCP SOCKETn status. @ref getSn_ESR()
    SO_MODE,
    //#endif
    SO_REMAINSIZE,       ///< Valid only in getsockopt. Get the remained packet size in other then TCP mode.
    SO_PACKINFO          ///< Valid only in getsockopt. Get the packet information as @ref PACK_FIRST, @ref PACK_REMAINED, and @ref PACK_COMPLETED in other then TCP mode.
} sockopt_type;

/**
    @ingroup WIZnet_socket_APIs
    @brief Control socket.
    @details Control IO mode, Interrupt & Mask of socket and get the socket buffer information.
             Refer to @ref ctlsock_type.
    @param sn socket number
    @param cstype type of control socket. refer to @ref ctlsock_type.
    @param arg Data type and value is determined according to @ref ctlsock_type. \n
               <table>
                    <tr> <td> @b cstype </td> <td> @b data type</td><td>@b value</td></tr>
                    <tr> <td> @ref CS_SET_IOMODE \n @ref CS_GET_IOMODE </td> <td> uint8_t </td><td>@ref SOCK_IO_BLOCK @ref SOCK_IO_NONBLOCK</td></tr>
                    <tr> <td> @ref CS_GET_MAXTXBUF \n @ref CS_GET_MAXRXBUF </td> <td> uint16_t </td><td> 0 ~ 16K </td></tr>
                    <tr> <td> @ref CS_CLR_INTERRUPT \n @ref CS_GET_INTERRUPT \n @ref CS_SET_INTMASK \n @ref CS_GET_INTMASK </td> <td> @ref sockint_kind </td><td> @ref SIK_CONNECTED, etc.  </td></tr>
               </table>
    @return @b Success @ref SOCK_OK \n
            @b fail    @ref SOCKERR_ARG         - Invalid argument\n
*/
int8_t  ctlsocket(uint8_t sn, ctlsock_type cstype, void* arg);

/**
    @ingroup WIZnet_socket_APIs
    @brief set socket options
    @details Set socket option like as TTL, MSS, TOS, and so on. Refer to @ref sockopt_type.

    @param sn socket number
    @param sotype socket option type. refer to @ref sockopt_type
    @param arg Data type and value is determined according to <I>sotype</I>. \n
               <table>
                    <tr> <td> @b sotype </td> <td> @b data type</td><td>@b value</td></tr>
                    <tr> <td> @ref SO_TTL </td> <td> uint8_t </td><td> 0 ~ 255 </td> </tr>
                    <tr> <td> @ref SO_TOS </td> <td> uint8_t </td><td> 0 ~ 255 </td> </tr>
                    <tr> <td> @ref SO_MSS </td> <td> uint16_t </td><td> 0 ~ 65535 </td> </tr>
                    <tr> <td> @ref SO_DESTIP </td> <td> uint8_t[4] </td><td>  </td></tr>
                    <tr> <td> @ref SO_DESTPORT </td> <td> uint16_t </td><td> 0 ~ 65535 </td></tr>
                    <tr> <td> @ref SO_KEEPALIVESEND </td> <td> null </td><td> null </td></tr>
                    <tr> <td> @ref SO_KEEPALIVEAUTO </td> <td> uint8_t </td><td> 0 ~ 255 </td></tr>
               </table>
    @return
    - @b Success : @ref SOCK_OK \n
    - @b Fail
    - @ref SOCKERR_SOCKNUM     - Invalid Socket number \n
    - @ref SOCKERR_SOCKMODE    - Invalid socket mode \n
    - @ref SOCKERR_SOCKOPT     - Invalid socket option or its value \n
    - @ref SOCKERR_TIMEOUT     - Timeout occurred when sending keep-alive packet \n
*/
int8_t  setsockopt(uint8_t sn, sockopt_type sotype, void* arg);

/**
    @ingroup WIZnet_socket_APIs
    @brief get socket options
    @details Get socket option like as FLAG, TTL, MSS, and so on. Refer to @ref sockopt_type
    @param sn socket number
    @param sotype socket option type. refer to @ref sockopt_type
    @param arg Data type and value is determined according to <I>sotype</I>. \n
               <table>
                    <tr> <td> @b sotype </td> <td>@b data type</td><td>@b value</td></tr>
                    <tr> <td> @ref SO_FLAG </td> <td> uint8_t </td><td> @ref SF_ETHER_OWN, etc... </td> </tr>
                    <tr> <td> @ref SO_TOS </td> <td> uint8_t </td><td> 0 ~ 255 </td> </tr>
                    <tr> <td> @ref SO_MSS </td> <td> uint16_t </td><td> 0 ~ 65535 </td> </tr>
                    <tr> <td> @ref SO_DESTIP </td> <td> uint8_t[4] </td><td>  </td></tr>
                    <tr> <td> @ref SO_DESTPORT </td> <td> uint16_t </td><td>  </td></tr>
                    <tr> <td> @ref SO_KEEPALIVEAUTO </td> <td> uint8_t </td><td> 0 ~ 255 </td></tr>
                    <tr> <td> @ref SO_SENDBUF </td> <td> uint16_t </td><td> 0 ~ 65535 </td></tr>
                    <tr> <td> @ref SO_RECVBUF </td> <td> uint16_t </td><td> 0 ~ 65535 </td></tr>
                    <tr> <td> @ref SO_STATUS </td> <td> uint8_t </td><td> @ref SOCK_ESTABLISHED, etc.. </td></tr>
                    <tr> <td> @ref SO_REMAINSIZE </td> <td> uint16_t </td><td> 0~ 65535 </td></tr>
                    <tr> <td> @ref SO_PACKINFO </td> <td> uint8_t </td><td> @ref PACK_FIRST, etc... </td></tr>
               </table>
    @return
    - @b Success : @ref SOCK_OK \n
    - @b Fail
    - @ref SOCKERR_SOCKNUM     - Invalid Socket number \n
    - @ref SOCKERR_SOCKOPT     - Invalid socket option or its value \n
    - @ref SOCKERR_SOCKMODE    - Invalid socket mode \n
    @note
     The option as PACK_REMAINED and SO_PACKINFO is valid only in NON-TCP mode and after call @ref recvfrom(). \n
     When SO_PACKINFO value is PACK_FIRST and the return value of recvfrom() is zero,
     This means the zero byte UDP data(UDP Header only) received.
*/
int8_t  getsockopt(uint8_t sn, sockopt_type sotype, void* arg);

//teddy 240122
#if _WIZCHIP_ == W6100 || _WIZCHIP_ == W6300
/**
    @ingroup WIZnet_socket_APIs
    @brief Peeks a sub-message in SOCKETn RX buffer
    @details It peeks the incoming message of SOCKETn RX buffer. \n
             It can find the specified sub-message in the incoming message and
             return the length of incoming message before the sub-message. \n
             It is useful when you need to read each messages from multiple message in SOCKET RX buffer.
    @param sn SOCKET number
    @param submsg sub-message pointer to find
    @param subsize the length of <i>submsg</i>
    @return
     - Success : the length of incoming message length before the <i>submsg</i> \n
     - Fail : -1
    @note
     It is just return the length of incoming message before the found sub-message. It does not receive the message.\n
     So, after calling peeksockmsg, @ref _Sn_RX_RD_ is not changed.
*/
int16_t peeksockmsg(uint8_t sn, uint8_t* submsg, uint16_t subsize);

#endif

// void setAddrlen_W6x00( uint8_t num) ;
// uint8_t checkAddrlen_W6x00() ;

// void inline_setAddrlen_W6x00( uint8_t num);
// uint8_t inline_CheckAddrlen_W6x00( void );


#if 1 // by_Lihan

/**
     @ingroup WIZnet_socket_APIs
     @brief  by_Lihan_W5x00
*/
int8_t connect_W5x00(uint8_t sn, uint8_t * addr, uint16_t port);
/**
     @ingroup WIZnet_socket_APIs
     @brief  by_Lihan_Wx00
*/
int8_t connect_W6x00(uint8_t sn, uint8_t * addr, uint16_t port, uint8_t addrlen);





#define GET_MACRO_connect(_1, _2, _3, _4, NAME, ...) NAME
#define CHOOSE_TESTCODE_MACRO(...) GET_MACRO_connect(__VA_ARGS__, connect_4, connect_3)

/**
    // by_LIhan for overroading
    // NOTE_LIHAN: Some sections of this code are not yet fully defined.
     @note
        In case of get 3 arguments - int8_t connect_W5x00(uint8_t sn, uint8_t * addr, uint16_t port  );\n
        In case of get 4 arguments - int8_t connect_W6x00(uint8_t sn, uint8_t * addr, uint16_t port, uint8_t addrlen );
*/

#define connect(...) CHOOSE_TESTCODE_MACRO(__VA_ARGS__)(__VA_ARGS__)

//   In case of get 3 arguments
#define connect_3(sn , addr , port ) connect_W5x00(sn , addr , port)

//   In case of get 4 arguments
#define connect_4(sn , addr , port, addrlen ) connect_W6x00(sn , addr , port,addrlen)




/**
     @ingroup WIZnet_socket_APIs
     @brief  by_Lihan
*/
int32_t sendto_W5x00(uint8_t sn, uint8_t * buf, uint16_t len, uint8_t * addr, uint16_t port);
/**
     @ingroup WIZnet_socket_APIs
     @brief  by_Lihan
*/
int32_t sendto_W6x00(uint8_t sn, uint8_t * buf, uint16_t len, uint8_t * addr, uint16_t port, uint8_t addrlen);


#define GET_MACRO_sendto(_1, _2, _3, _4, _5 , _6, NAME, ...) NAME
#define CHOOSE_sendto_MACRO(...) GET_MACRO_sendto(__VA_ARGS__, sendto_6, sendto_5)

// by_LIhan for overroading
// NOTE_LIHAN: Some sections of this code are not yet fully defined.
//   In case of get 3 arguments - int8_t sendto_W5x00(uint8_t sn, uint8_t * addr, uint16_t port  );
//   In case of get 4 arguments - int8_t sendto_W6x00(uint8_t sn, uint8_t * addr, uint16_t port,uint8_t addrlen );
#define sendto(...) CHOOSE_sendto_MACRO(__VA_ARGS__)(__VA_ARGS__)

//   In case of get 3 arguments
#define sendto_5( sn,   buf,  len,  addr,  port ) sendto_W5x00( sn,   buf,  len,  addr,  port)

//   In case of get 4 arguments
#define sendto_6( sn,   buf,  len,  addr,  port, addrlen ) sendto_W6x00( sn,   buf,  len,  addr,  port, addrlen)

/**
     @ingroup WIZnet_socket_APIs
     @brief  byLihan_W5x00
*/
int32_t recvfrom_W5x00(uint8_t sn, uint8_t * buf, uint16_t len, uint8_t * addr, uint16_t *port);
/**
     @ingroup WIZnet_socket_APIs
     @brief  byLihan_Wx00
*/
int32_t recvfrom_W6x00(uint8_t sn, uint8_t * buf, uint16_t len, uint8_t * addr, uint16_t *port,  uint8_t *addrlen);


#define GET_MACRO_recvfrom(_1, _2, _3, _4, _5, _6 ,NAME, ...) NAME
#define CHOOSE_recvfrom_MACRO(...) GET_MACRO_recvfrom(__VA_ARGS__, recvfrom_6, recvfrom_5)

// by_LIhanfor overroading
// In case of get 3 arguments - int8_t recvfrom_W5x00(uint8_t sn, uint8_t * addr, uint16_t port  );
// In case of get 4 arguments - int8_t recvfrom_W6x00(uint8_t sn, uint8_t * addr, uint16_t port,uint8_t addrlen );
#define recvfrom(...) CHOOSE_recvfrom_MACRO(__VA_ARGS__)(__VA_ARGS__)

//   In case of get 3 arguments
#define recvfrom_5(sn,   buf,  len,  addr,  port ) recvfrom_W5x00(sn,   buf,  len,  addr,  port)

//   In case of get 4 arguments
#define recvfrom_6(sn,   buf,  len,  addr,  port, addrlen  ) recvfrom_W6x00(sn,   buf,  len,  addr,  port, addrlen )


#endif



#ifdef __cplusplus
}
#endif

#endif   // _SOCKET_H_


