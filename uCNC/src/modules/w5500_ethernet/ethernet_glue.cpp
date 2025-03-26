#include <Arduino.h>
#include "Ethernet.h"
#include "EthernetServer.h"

byte mac[] = {
		0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED};

EthernetServer webserver(80);

extern "C"
{
#include "../../cnc.h"

#define ETHERNET_OFF 0
#define ETHERNET_DISCONNECTED 1
#define ETHERNET_CONNECTED 2

	static uint8_t ethernet_status;

	void ethernet_init()
	{
		if (ethernet_status == ETHERNET_OFF)
		{
			if (Ethernet.begin(mac) == 0)
			{
				DBGMSG("Failed to configure Ethernet using DHCP\n");
				if (Ethernet.hardwareStatus() == EthernetNoHardware)
				{
					ethernet_status = ETHERNET_OFF;
					proto_info("Ethernet shield was not found");
				}
				else if (Ethernet.linkStatus() == LinkOFF)
				{
					ethernet_status = ETHERNET_DISCONNECTED;
					proto_info("Ethernet cable is not connected");
				}
				else
				{
					ethernet_status = ETHERNET_CONNECTED;
					proto_info("Ethernet connected");
					uint32_t ip = Ethernet.localIP();
					proto_info("IP>%I", ip);
				}
			}
		}
	}
}