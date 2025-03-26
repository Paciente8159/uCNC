#include <Arduino.h>
#include "Ethernet.h"
#include "EthernetServer.h"

byte mac[] = {
		0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED};

#ifndef TELNET_PORT
#define TELNET_PORT 23
#endif

#ifndef WEBSERVER_PORT
#define WEBSERVER_PORT 80
#endif

#ifndef WEBSOCKET_PORT
#define WEBSOCKET_PORT 8080
#endif

#ifndef WEBSOCKET_MAX_CLIENTS
#define WEBSOCKET_MAX_CLIENTS 2
#endif

EthernetServer web_server(WEBSERVER_PORT);
EthernetServer telnet_server(TELNET_PORT);
EthernetClient server_client;

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

	void ethernet_loop()
	{
		switch (Ethernet.maintain())
		{
		case 1:
			// renewed fail
			DBGMSG("Error: renewed fail");
			ethernet_status = ETHERNET_DISCONNECTED;
			break;

		case 2:
			// renewed success
			DBGMSG("Renewed success");
			ethernet_status = ETHERNET_CONNECTED;
			break;

		case 3:
			// rebind fail
			DBGMSG("Error: rebind fail");
			ethernet_status = ETHERNET_DISCONNECTED;
			break;

		case 4:
			// rebind success
			DBGMSG("Rebind success");
			ethernet_status = ETHERNET_CONNECTED;
			break;

		default:
			// nothing happened
			break;
		}
	}

	bool ethernet_clientok(void)
	{
		static uint32_t next_info = 30000;
		static bool connected = false;
		uint8_t str[128];

		if ((ethernet_status == ETHERNET_DISCONNECTED))
		{
			connected = false;
			if (next_info > millis())
			{
				return false;
			}
			next_info = millis() + 30000;
			proto_info("Disconnected from Ethernet");
			return false;
		}

		if (!connected)
		{
			connected = true;
			proto_info("Connected to Ethernet");
			uint32_t ip = Ethernet.localIP();
			proto_info("IP>%I", ip);
		}

		if(!server_client){
			server_client = telnet_server.available();
			if(server_client){
				server_client.println("[MSG:New client connected]");
			}
			return false;
		}
		else
		{
			return server_client.connected();
		}
		return false;
	}

	void ethernet_dotasks()
	{
		switch (ethernet_status)
		{
		case ETHERNET_OFF:
			ethernet_init();
			break;
		case ETHERNET_DISCONNECTED:
			// ethernet_init();
			break;
		case ETHERNET_CONNECTED:
			ethernet_loop();
			break;
		}
	}
}