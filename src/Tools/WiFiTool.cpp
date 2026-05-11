#include "WiFiTool.h"

#include <WiFi.h>

WiFiTool::WiFiTool() {
    get_ip.onCall([](JsonObject) -> String {
        return WiFi.localIP().toString();
    });

    get_rssi.onCall([](JsonObject) -> String {
        return String(WiFi.RSSI()) + " dBm";
    });

    get_ssid.onCall([](JsonObject) -> String {
        return WiFi.SSID();
    });

    get_mac.onCall([](JsonObject) -> String {
        return WiFi.macAddress();
    });

    addTool(&get_ip);
    addTool(&get_rssi);
    addTool(&get_ssid);
    addTool(&get_mac);
}
