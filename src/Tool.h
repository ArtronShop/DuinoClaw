#ifndef __TOOL__H_
#define __TOOL__H_

#include <Arduino.h>
#include <ArduinoJson.h>

typedef std::function<String(JsonObject)> ToolFunction;

class Tool {
    public:
        typedef enum {
            TYPE_STRING,
            TYPE_NUMBER,
            TYPE_INTEGER,
        } DataType_t;

        typedef struct {
            const char * name;
            const char * description;
            DataType_t type;
            bool required;
            void * next;
        } ToolPropertyItem_t;

        Tool(const char * name, const char * description) ;
        void addProperty(const char * name, const char * description, DataType_t type, bool required) ;
        void onCall(ToolFunction fn) ;

        const char * getName() { return this->name; };
        const char * getDescription() { return this->description; };
        ToolPropertyItem_t * getPropertyFirst() { return this->property_first; };
        String call(JsonObject arguments);
    
    private:
        const char * name = NULL;
        const char * description = NULL;

        ToolPropertyItem_t * property_first = NULL;
        ToolPropertyItem_t * property_last = NULL;
        
        ToolFunction fn = NULL;

};

#endif
