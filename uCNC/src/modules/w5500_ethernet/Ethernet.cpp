/* Copyright 2018 Paul Stoffregen
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this
 * software and associated documentation files (the "Software"), to deal in the Software
 * without restriction, including without limitation the rights to use, copy, modify,
 * merge, publish, distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
 * INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
 * PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
 * OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include <Arduino.h>
#include "Ethernet.h"
#include "utility/w5100.h"
#include "Dhcp.h"

IPAddress EthernetClass::_dnsServerAddress;
DhcpClass* EthernetClass::_dhcp = NULL;

int EthernetClass::begin(uint8_t *mac, unsigned long timeout, unsigned long responseTimeout)
{
	static DhcpClass s_dhcp;
	_dhcp = &s_dhcp;

	// Initialise the basic info
	if (W5100.init() == 0) return 0;
	W5500_SPI_START();
	W5100.setMACAddress(mac);
	W5100.setIPAddress(IPAddress(0,0,0,0).raw_address());
	W5500_SPI_STOP();

	// Now try to get our config info from a DHCP server
	int ret = _dhcp->beginWithDHCP(mac, timeout, responseTimeout);
	if (ret == 1) {
		// We've successfully found a DHCP server and got our configuration
		// info, so set things accordingly
		W5500_SPI_START();
		W5100.setIPAddress(_dhcp->getLocalIp().raw_address());
		W5100.setGatewayIp(_dhcp->getGatewayIp().raw_address());
		W5100.setSubnetMask(_dhcp->getSubnetMask().raw_address());
		W5500_SPI_STOP();
		_dnsServerAddress = _dhcp->getDnsServerIp();
		socketPortRand(micros());
	}
	return ret;
}

void EthernetClass::begin(uint8_t *mac, IPAddress ip)
{
	// Assume the DNS server will be the machine on the same network as the local IP
	// but with last octet being '1'
	IPAddress dns = ip;
	dns[3] = 1;
	begin(mac, ip, dns);
}

void EthernetClass::begin(uint8_t *mac, IPAddress ip, IPAddress dns)
{
	// Assume the gateway will be the machine on the same network as the local IP
	// but with last octet being '1'
	IPAddress gateway = ip;
	gateway[3] = 1;
	begin(mac, ip, dns, gateway);
}

void EthernetClass::begin(uint8_t *mac, IPAddress ip, IPAddress dns, IPAddress gateway)
{
	IPAddress subnet(255, 255, 255, 0);
	begin(mac, ip, dns, gateway, subnet);
}

void EthernetClass::begin(uint8_t *mac, IPAddress ip, IPAddress dns, IPAddress gateway, IPAddress subnet)
{
	if (W5100.init() == 0) return;
	W5500_SPI_START();
	W5100.setMACAddress(mac);
	W5100.setIPAddress(ip.raw_address());
	W5100.setGatewayIp(gateway.raw_address());
	W5100.setSubnetMask(subnet.raw_address());
	W5500_SPI_STOP();
	_dnsServerAddress = dns;
}

void EthernetClass::init(uint8_t sspin)
{
}

EthernetLinkStatus EthernetClass::linkStatus()
{
	switch (W5100.getLinkStatus()) {
		case UNKNOWN:  return Unknown;
		case LINK_ON:  return LinkON;
		case LINK_OFF: return LinkOFF;
		default:       return Unknown;
	}
}

EthernetHardwareStatus EthernetClass::hardwareStatus()
{
	switch (W5100.getChip()) {
		case 51: return EthernetW5100;
		case 52: return EthernetW5200;
		case 55: return EthernetW5500;
		default: return EthernetNoHardware;
	}
}

int EthernetClass::maintain()
{
	int rc = DHCP_CHECK_NONE;
	if (_dhcp != NULL) {
		// we have a pointer to dhcp, use it
		rc = _dhcp->checkLease();
		switch (rc) {
		case DHCP_CHECK_NONE:
			//nothing done
			break;
		case DHCP_CHECK_RENEW_OK:
		case DHCP_CHECK_REBIND_OK:
			//we might have got a new IP.
			W5500_SPI_START();
			W5100.setIPAddress(_dhcp->getLocalIp().raw_address());
			W5100.setGatewayIp(_dhcp->getGatewayIp().raw_address());
			W5100.setSubnetMask(_dhcp->getSubnetMask().raw_address());
			W5500_SPI_STOP();
			_dnsServerAddress = _dhcp->getDnsServerIp();
			break;
		default:
			//this is actually an error, it will retry though
			break;
		}
	}
	return rc;
}


void EthernetClass::MACAddress(uint8_t *mac_address)
{
	W5500_SPI_START();
	W5100.getMACAddress(mac_address);
	W5500_SPI_STOP();
}

IPAddress EthernetClass::localIP()
{
	IPAddress ret;
	W5500_SPI_START();
	W5100.getIPAddress(ret.raw_address());
	W5500_SPI_STOP();
	return ret;
}

IPAddress EthernetClass::subnetMask()
{
	IPAddress ret;
	W5500_SPI_START();
	W5100.getSubnetMask(ret.raw_address());
	W5500_SPI_STOP();
	return ret;
}

IPAddress EthernetClass::gatewayIP()
{
	IPAddress ret;
	W5500_SPI_START();
	W5100.getGatewayIp(ret.raw_address());
	W5500_SPI_STOP();
	return ret;
}

void EthernetClass::setMACAddress(const uint8_t *mac_address)
{
	W5500_SPI_START();
	W5100.setMACAddress(mac_address);
	W5500_SPI_STOP();
}

void EthernetClass::setLocalIP(const IPAddress local_ip)
{
	W5500_SPI_START();
	IPAddress ip = local_ip;
	W5100.setIPAddress(ip.raw_address());
	W5500_SPI_STOP();
}

void EthernetClass::setSubnetMask(const IPAddress subnet)
{
	W5500_SPI_START();
	IPAddress ip = subnet;
	W5100.setSubnetMask(ip.raw_address());
	W5500_SPI_STOP();
}

void EthernetClass::setGatewayIP(const IPAddress gateway)
{
	W5500_SPI_START();
	IPAddress ip = gateway;
	W5100.setGatewayIp(ip.raw_address());
	W5500_SPI_STOP();
}

void EthernetClass::setRetransmissionTimeout(uint16_t milliseconds)
{
	if (milliseconds > 6553) milliseconds = 6553;
	W5500_SPI_START();
	W5100.setRetransmissionTime(milliseconds * 10);
	W5500_SPI_STOP();
}

void EthernetClass::setRetransmissionCount(uint8_t num)
{
	W5500_SPI_START();
	W5100.setRetransmissionCount(num);
	W5500_SPI_STOP();
}










EthernetClass Ethernet;
