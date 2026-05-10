#include "GetCurrentTimeTool.h"

GetCurrentTimeTool::GetCurrentTimeTool(int timezone, const char * ntp_server1, const char * ntp_server2, const char * ntp_server3)
    : Tool("get_current_time", "Call this when you need to know what time or date it is. ISO 8601 format.")
{
    configTime(timezone * 3600, 0, ntp_server1, ntp_server2, ntp_server3);

    Tool::onCall(GetCurrentTimeTool::get_current_time);
}

String GetCurrentTimeTool::get_current_time(JsonObject args) {
    struct tm timeinfo;
    if (!getLocalTime(&timeinfo)) {
        return "ERROR";
    }

    char buf[30];
    // %z → +0700
    strftime(buf, sizeof(buf), "%Y-%m-%dT%H:%M:%S%z", &timeinfo);

    // Convart +0700 → +07:00
    String iso = String(buf);
    if (iso.length() >= 5) {
        iso = iso.substring(0, iso.length() - 2) + ":" + iso.substring(iso.length() - 2);
    }

    return iso;
}
