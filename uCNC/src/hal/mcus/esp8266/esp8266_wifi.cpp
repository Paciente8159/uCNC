//#include "Arduino.h"
#include "../../../../cnc_config.h"
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <Hash.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPUpdateServer.h>
#include <WiFiManager.h>
#include <string.h>
#include <stdint.h>
#include <stdio.h>

#ifndef WIFI_BUFFER
#define WIFI_BUFFER 128
#endif

#ifndef WIFI_PORT
#define WIFI_PORT 23
#endif

#ifndef WIFI_USER
#define WIFI_USER "admin"
#endif

#ifndef WIFI_PASS
#define WIFI_PASS "pass"
#endif

ESP8266WebServer httpServer(80);
ESP8266HTTPUpdateServer httpUpdater;
const char *update_path = "/firmware";
const char *update_username = WIFI_USER;
const char *update_password = WIFI_PASS;
#define MAX_SRV_CLIENTS 1
WiFiServer server(WIFI_PORT);
WiFiClient serverClient;
WiFiManager wifiManager;

extern "C"
{
        void esp8266_wifi_init(void)
        {
#ifdef WIFI_DEBUG
                Serial.begin(115200);
                wifiManager.setDebugOutput(true);
#else
                wifiManager.setDebugOutput(false);
#endif
                wifiManager.autoConnect("ESP8266");
#ifdef WIFI_DEBUG
                Serial.println("[MSG: WiFi manager up]");
#endif
#ifdef WIFI_DEBUG
                Serial.println("[MSG: Setup page @ 192.168.4.1]");
#endif
                server.setNoDelay(true);
                httpUpdater.setup(&httpServer, update_path, update_username, update_password);
                httpServer.begin();
                WiFi.setSleepMode(WIFI_NONE_SLEEP);

#ifdef WIFI_DEBUG
                Serial.println("[MSG: WiFi server us up]");
#endif
        }

        void esp8266_wifi_read(void (*read_callback)(unsigned char))
        {
                if (server.hasClient())
                {
#ifdef WIFI_DEBUG
                        Serial.println("[MSG: client waiting]");
#endif
                        if (serverClient && serverClient.connected())
                        {
#ifdef WIFI_DEBUG
                                Serial.println("[MSG: kill old client]");
#endif
                                serverClient.stop();
                        }

#ifdef WIFI_DEBUG
                        Serial.println("[MSG: client started]");
#endif
                }
                else if (serverClient && serverClient.connected())
                {
                        size_t rxlen = serverClient.available();
                        if (rxlen > 0)
                        {
#ifdef WIFI_DEBUG
                                Serial.println("[MSG: client has data]");
#endif
                                uint8_t sbuf[rxlen];
                                serverClient.readBytes(sbuf, rxlen);
                                for (uint8_t i = 0; i < rxlen; i++)
                                {
                                        if (read_callback)
                                        {
                                                read_callback((unsigned char)sbuf[i]);
                                        }
                                        yield();
                                }
                        }
                }
        }

        void esp8266_wifi_write(char *buff, uint8_t len)
        {
                if (server.hasClient())
                {
#ifdef WIFI_DEBUG
                        Serial.println("[MSG: client waiting]");
#endif
                        if (serverClient && serverClient.connected())
                        {
#ifdef WIFI_DEBUG
                                Serial.println("[MSG: kill old client]");
#endif
                                serverClient.stop();
                        }

#ifdef WIFI_DEBUG
                        Serial.println("[MSG: client started]");
#endif
                }
                else if (serverClient && serverClient.connected())
                {
#ifdef WIFI_DEBUG
                        Serial.println("[MSG: sent data to client]");
#endif
                        serverClient.write(buff, (size_t)len);
                }
        }
}
