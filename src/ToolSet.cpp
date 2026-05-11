#include "ToolSet.h"

#include <Arduino.h>

static const char * TAG = "ToolSet";

void ToolSet::addTool(Tool * tool) {
    ToolSetItem_t * item = (ToolSetItem_t *) malloc(sizeof(ToolSetItem_t));
    if (!item) {
        ESP_LOGE(TAG, "malloc failed");
        return;
    }
    item->tool = tool;
    item->next = NULL;

    if (!first) {
        first = item;
    }
    if (last) {
        last->next = item;
    }
    last = item;
}
