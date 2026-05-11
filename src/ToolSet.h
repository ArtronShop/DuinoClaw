#ifndef __TOOL_SET_H__
#define __TOOL_SET_H__

#include "Tool.h"

class ToolSet {
    public:
        typedef struct {
            Tool * tool;
            void * next;
        } ToolSetItem_t;

        ToolSetItem_t * getFirst() { return this->first; };

    protected:
        void addTool(Tool * tool);

    private:
        ToolSetItem_t * first = NULL;
        ToolSetItem_t * last = NULL;
};

#endif
