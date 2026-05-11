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

void Tool::addEnumProperty(const char * name, const char * description, DataType_t type, void * enum_values, uint8_t enum_count, bool required) {
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

void Tool::addEnumProperty(const char * name, const char * description, const char ** enum_values, uint8_t enum_count, bool required) {
    addEnumProperty(name, description, TYPE_STRING, (void *) enum_values, enum_count, required);
}

void Tool::addEnumProperty(const char * name, const char * description, const long * enum_values, uint8_t enum_count, bool required) {
    addEnumProperty(name, description, TYPE_INTEGER, (void *) enum_values, enum_count, required);
}

void Tool::onCall(ToolFunction fn) {
    this->fn = fn;
}

String Tool::call(JsonObject arguments) {
    ToolPropertyItem_t * prop = property_first;
    while (prop) {
        JsonVariant val = arguments[prop->name];

        // 1. Required check
        if (prop->required && val.isNull()) {
            return String("ERROR: missing required argument '") + prop->name + "'";
        }

        if (!val.isNull()) {
            // 2. Type check
            bool type_ok = false;
            if (prop->type == TYPE_STRING) type_ok = val.is<const char *>();
            if (prop->type == TYPE_NUMBER) type_ok = val.is<double>();
            if (prop->type == TYPE_INTEGER) type_ok = val.is<long>();

            if (!type_ok) {
                const char * expected = (prop->type == TYPE_STRING) ? "string"
                                      : (prop->type == TYPE_NUMBER) ? "number"
                                      : "integer";
                return String("ERROR: argument '") + prop->name + "' must be " + expected;
            }

            // 3. Enum check
            if (prop->enum_values && prop->enum_count > 0) {
                bool found = false;
                String allowed = "";
                if (prop->type == TYPE_STRING) {
                    const char ** str_enum = (const char **) prop->enum_values;
                    const char * val_str = val.as<const char *>();
                    for (uint8_t i = 0; i < prop->enum_count; i++) {
                        if (i > 0) allowed += ", ";
                        allowed += str_enum[i];
                        if (strcmp(val_str, str_enum[i]) == 0) found = true;
                    }
                } else {
                    const long * long_enum = (const long *) prop->enum_values;
                    long val_long = val.as<long>();
                    for (uint8_t i = 0; i < prop->enum_count; i++) {
                        if (i > 0) allowed += ", ";
                        allowed += long_enum[i];
                        if (val_long == long_enum[i]) found = true;
                    }
                }
                if (!found) {
                    return String("ERROR: argument '") + prop->name + "' must be one of: " + allowed;
                }
            }
        }

        prop = (ToolPropertyItem_t *) prop->next;
    }

    if (this->fn) {
        return this->fn(arguments);
    }

    return String();
}
