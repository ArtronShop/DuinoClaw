#ifndef __LLM_H__
#define __LLM_H__

#include <Arduino.h>

typedef enum {
    TYPE_MESSAGE,
    TYPE_FUNCTION_CALL,
    TYPE_FUNCTION_CALL_OUTPUT,
} ConversationType_t;

typedef enum {
    ROLE_SYSTEM = 0,
    ROLE_USER,
    ROLE_ASSISTANT,
    ROLE_NONE,
} ConversationRole_t;

typedef struct __attribute__((__packed__)) {
    uint8_t type;

    union {
        // Message
        struct {
            uint8_t role;
            char * content;
        };

        // Function Call
        struct {
            char * name;
            char * arguments;
            char * req_call_id;
        };

        // Function Call Output
        struct {
            char * call_id;
            char * output;
        };
    };

    void * next;
} ConversationItem_t;

class LLM {
    public:
        virtual String getResponses(String prompt, bool * ok = NULL) ;
        bool addSystemMessage(String& content) ;

    protected:
        ConversationItem_t * conversation_first_item = NULL;
        ConversationItem_t * conversation_last_item = NULL;
        size_t conversation_item_count = 0;

        bool putConversation(ConversationType_t, ConversationRole_t, String& content_or_name_or_call_id, String& arguments_or_output, String& call_id) ;
        bool putConversationMessage(ConversationRole_t, String&) ;
        bool putConversationFunctionCall(String& name, String& arguments, String& call_id) ;
        bool putConversationFunctionCallOutput(String& call_id, String& output) ;

    private:
        

};

#endif