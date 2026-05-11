#ifndef __WIFI_TOOL_H__
#define __WIFI_TOOL_H__

#include "ToolSet.h"

class WiFiTool : public ToolSet {
    Tool get_ip{"wifi_get_ip", "Get the device WiFi IP address"};
    Tool get_rssi{"wifi_get_rssi", "Get the WiFi signal strength in dBm"};
    Tool get_ssid{"wifi_get_ssid", "Get the connected WiFi network name (SSID)"};
    Tool get_mac{"wifi_get_mac", "Get the device WiFi MAC address"};

    public:
        WiFiTool();
};

#endif
