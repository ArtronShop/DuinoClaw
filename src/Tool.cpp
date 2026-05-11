#include "Tool.h"

static const char * TAG = "Tool";

Tool::Tool(const char * name, const char * description) : name(name), description(description) {
    // ----
}

void Tool::addProperty(const char * name, const char * description, DataType_t type, bool required) {
    ToolPropertyItem_t * item = (ToolPropertyItem_t *) malloc(sizeof(ToolPropertyItem_t));
    if (!item) {
        ESP_LOGE(TAG, "malloc property of '%s' failed", this->name);
        return;
    }
    item->name = name;
    item->description = description;
    item->type = type;
    item->required = required;
    item->enum_values = NULL;
    item->enum_count = 0;
    item->next = NULL;

    if (!property_first) {
        property_first = item;
    }
    if (property_last) {
        property_last->next = item;
    }
    property_last = item;
}

void Tool::addEnumProperty(const char * name, const char * description, DataType_t type, const char ** enum_values, uint8_t enum_count, bool required) {
    ToolPropertyItem_t * item = (ToolPropertyItem_t *) malloc(sizeof(ToolPropertyItem_t));
    if (!item) {
        ESP_LOGE(TAG, "malloc property of '%s' failed", this->name);
        return;
    }
    item->name = name;
    item->description = description;
    item->type = type;
    item->required = required;
    item->enum_values = enum_values;
    item->enum_count = enum_count;
    item->next = NULL;

    if (!property_first) {
        property_first = item;
    }
    if (property_last) {
        property_last->next = item;
    }
    property_last = item;
}

void Tool::onCall(ToolFunction fn) {
    this->fn = fn;
}

String Tool::call(JsonObject arguments) {
    if (this->fn) {
        return this->fn(arguments);
    }

    return String();
}
