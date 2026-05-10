#include "LLM.h"

bool LLM::putConversation(ConversationType_t type, ConversationRole_t role, String& content_or_name_or_call_id, String& arguments_or_output, String& call_id) {
    // trim conversation
    if (conversation_item_count >= 20) {
        ConversationItem_t * item = conversation_first_item;
        if (item->type == TYPE_MESSAGE) {
            if (item->content) {
                free(item->content);
            }
        } else if (item->type == TYPE_FUNCTION_CALL) {
            if (item->name) {
                free(item->name);
            }
            if (item->arguments) {
                free(item->arguments);
            }
            if (item->req_call_id) {
                free(item->req_call_id);
            }
        } else if (item->type == TYPE_FUNCTION_CALL_OUTPUT) {
            if (item->call_id) {
                free(item->call_id);
            }
            if (item->output) {
                free(item->output);
            }
        }
        conversation_first_item = (ConversationItem_t *) item->next;
        if (!conversation_first_item) {
            conversation_last_item = NULL;
        }
        free(item);
        conversation_item_count--;
    }

    ConversationItem_t * item = (ConversationItem_t *) malloc(sizeof(ConversationItem_t));
    if (!item) {
        return false;
    }
    memset(item, 0, sizeof(ConversationItem_t));
    item->type = type;

    if (type == TYPE_MESSAGE) {
        item->role = role;
        size_t content_len = content_or_name_or_call_id.length();
        item->content = (char *) malloc(content_len + 1);
        if (!item->content) {
            free(item);
            return false;
        }
        memset(item->content, 0, content_len + 1);
        memcpy(item->content, content_or_name_or_call_id.c_str(), content_len);
    } else if (type == TYPE_FUNCTION_CALL) {
        { // Name
            size_t name_len = content_or_name_or_call_id.length();
            item->name = (char *) malloc(name_len + 1);
            if (!item->name) {
                free(item);
                return false;
            }
            memset(item->name, 0, name_len + 1);
            memcpy(item->name, content_or_name_or_call_id.c_str(), name_len);
        }

        { // Arguments
            size_t arguments_len = arguments_or_output.length();
            item->arguments = (char *) malloc(arguments_len + 1);
            if (!item->arguments) {
                free(item->name);
                free(item);
                return false;
            }
            memset(item->arguments, 0, arguments_len + 1);
            memcpy(item->arguments, arguments_or_output.c_str(), arguments_len);
        }

        { // Call Id
            size_t call_id_len = call_id.length();
            item->req_call_id = (char *) malloc(call_id_len + 1);
            if (!item->req_call_id) {
                free(item->name);
                free(item->arguments);
                free(item);
                return false;
            }
            memset(item->req_call_id, 0, call_id_len + 1);
            memcpy(item->req_call_id, call_id.c_str(), call_id_len);
        }
    } else if (type == TYPE_FUNCTION_CALL_OUTPUT) {
        { // Call Id
            size_t call_id_len = content_or_name_or_call_id.length();
            item->call_id = (char *) malloc(call_id_len + 1);
            if (!item->call_id) {
                free(item);
                return false;
            }
            memset(item->call_id, 0, call_id_len + 1);
            memcpy(item->call_id, content_or_name_or_call_id.c_str(), call_id_len);
        }

        { // Output
            size_t output_len = arguments_or_output.length();
            item->output = (char *) malloc(output_len + 1);
            if (!item->output) {
                free(item->call_id);
                free(item);
                return false;
            }
            memset(item->output, 0, output_len + 1);
            memcpy(item->output, arguments_or_output.c_str(), output_len);
        }
    }
    item->next = NULL;

    if (!conversation_first_item) {
        conversation_first_item = item;
    }
    if (conversation_last_item) {
        conversation_last_item->next = item;
    }
    conversation_last_item = item;

    conversation_item_count++;

    return true;
}

bool LLM::putConversationMessage(ConversationRole_t role, String& content) {
    String dummy = "";
    return this->putConversation(TYPE_MESSAGE, role, content, dummy, dummy);
}

bool LLM::putConversationFunctionCall(String& name, String& arguments, String& call_id) {
    return this->putConversation(TYPE_FUNCTION_CALL, ROLE_NONE, name, arguments, call_id);
}

bool LLM::putConversationFunctionCallOutput(String& call_id, String& output) {
    String dummy = "";
    return this->putConversation(TYPE_FUNCTION_CALL_OUTPUT, ROLE_NONE, call_id, output, dummy);
}


bool LLM::addSystemMessage(String& content) {
    return this->putConversationMessage(ROLE_SYSTEM, content);
}
