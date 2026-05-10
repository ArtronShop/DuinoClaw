#ifndef __TIME_TOOL_H__
#define __TIME_TOOL_H__

#include "Tool.h"

class GetCurrentTimeTool : public Tool {
    public:
        GetCurrentTimeTool(int timezone = +7, const char * ntp_server1 = "pool.ntp.org", const char * ntp_server2 = NULL, const char * ntp_server3 = NULL) ;

        static String get_current_time(JsonObject) ;
};

#endif
