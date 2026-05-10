#ifndef __TOOL_REGISTER_H__
#define __TOOL_REGISTER_H__

#include "Tool.h"

typedef struct {
    Tool * tool;
    void * next;
} ToolItem_t;


extern ToolItem_t * tool_first;

#endif


