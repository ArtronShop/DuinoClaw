#include "OpenAI.h"

#include <Arduino.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

#include "ToolRegister.h"

static const char * TAG = "OpenAI";

OpenAI::OpenAI(const char * model, const char * api_key) : model(model), api_key(api_key) {
    // ---
}

void stringEncode(String& in) {
    in.replace("\"", "\\\"");
    in.replace("\r", "\\r");
    in.replace("\n", "\\n");
}

String OpenAI::getResponses(String prompt, bool * ok) {
    bool end = false;
    String responses = "";

    putConversationMessage(ROLE_USER, prompt);

    do {
        HTTPClient http;

        ESP_LOGV(TAG, "[HTTP] begin...\n");

        http.begin(this->api_endpoint);
        http.addHeader("Content-Type", "application/json");
        {
            String value = "Bearer " + String(api_key);
            http.addHeader("Authorization", value);
        }

        ESP_LOGV(TAG, "[HTTP] POST...");
        
        int httpCode;
        {
            String payload = "{";
            payload += "  \"model\": \"" + String(this->model) + "\",";
            payload += "  \"input\": [";

            ConversationItem_t *item = conversation_first_item;
            while(item) {
                if (item->type == TYPE_MESSAGE) {
                    String role;
                    if (item->role == ROLE_SYSTEM) {
                        role = "system";
                    } else if (item->role == ROLE_USER) {
                        role = "user";
                    } else if (item->role == ROLE_ASSISTANT) {
                        role = "assistant";
                    }
                    String content = String(item->content);
                    stringEncode(content);
                    payload += "    {\"role\":\"" + role + "\",\"content\":\"" + content + "\"}";
                } else if (item->type == TYPE_FUNCTION_CALL) {
                    String name = item->name;
                    String arguments = item->arguments;
                    stringEncode(arguments);
                    String call_id = item->req_call_id;
                    payload += "    {\"type\":\"function_call\",\"name\":\"" + name + "\",\"arguments\":\"" + arguments + "\",\"call_id\":\"" + call_id + "\"}";
                } else if (item->type == TYPE_FUNCTION_CALL_OUTPUT) {
                    String call_id = item->call_id;
                    String output = item->output;
                    stringEncode(output);
                    payload += "    {\"type\":\"function_call_output\",\"call_id\":\"" + call_id + "\",\"output\":\"" + output + "\"}";
                }
                if (item->next != NULL) { // Not last item ?
                    payload += ",";
                }
                item = (ConversationItem_t *) item->next;
            }

            // payload += "    {\"role\":\"system\",\"content\":\"You are an assistant running on an ESP32 with very limited memory.\\nKeep all responses short, simple, and concise.\\nAvoid unnecessary details. Use plain language.\"},";
            // payload += "    {\"role\":\"user\",\"content\":\"Hello, plz set GPIO pin 5 to LOW\"}";
            // payload += ,{"role":"assistant","content":"Nice to meet you, Ton!"}
            // payload += ,{"role":"user","content":"What is my name?"}
            payload += "  ],";
            payload += "  \"tools\": [";
            ToolItem_t * tool_item = tool_first;
            while (tool_item) {
                Tool * tool = tool_item->tool;
                String name = tool->getName();
                String description = tool->getDescription();
                Tool::ToolPropertyItem_t * property_item = tool->getPropertyFirst();
                String property_required = "";
                
                payload += "  {";
                payload += "    \"type\": \"function\",";
                payload += "    \"name\": \"" + name + "\",";
                payload += "    \"description\": \"" + description + "\",";
                payload += "    \"parameters\": {";
                payload += "      \"type\": \"object\",";
                payload += "      \"properties\": {";
                while(property_item) {
                    String property_name = property_item->name;
                    String property_description = property_item->description;
                    stringEncode(property_description);
                    String property_type = "string";
                    if (property_item->type == Tool::TYPE_STRING) {
                        property_type = "string";
                    } else if (property_item->type == Tool::TYPE_NUMBER) {
                        property_type = "number";
                    } else if (property_item->type == Tool::TYPE_INTEGER) {
                        property_type = "integer";
                    }

                    payload += "        \"" + property_name + "\": {";
                    payload += "          \"description\": \"" + property_description + "\",";
                    payload += "          \"type\": \"" + property_type + "\"";
                    if (property_item->enum_values && property_item->enum_count > 0) {
                        payload += ",";
                        payload += "          \"enum\": [";
                        if (property_item->type == Tool::TYPE_STRING) {
                            const char ** str_enum = (const char **) property_item->enum_values;
                            for (uint8_t e = 0; e < property_item->enum_count; e++) {
                                payload += "\"";
                                payload += str_enum[e];
                                payload += "\"";
                                if (e < property_item->enum_count - 1) payload += ",";
                            }
                        } else {
                            const long * long_enum = (const long *) property_item->enum_values;
                            for (uint8_t e = 0; e < property_item->enum_count; e++) {
                                payload += long_enum[e];
                                if (e < property_item->enum_count - 1) payload += ",";
                            }
                        }
                        payload += "]";
                    }
                    payload += "        }";
                    if (property_item->next != NULL) {
                        payload += ",";
                    }

                    if (property_item->required) {
                        if (property_required.length() > 0) {
                            property_required += ",";
                        }
                        property_required += "\"" + property_name + "\"";
                    }

                    property_item = (Tool::ToolPropertyItem_t * ) property_item->next;
                }
                payload += "      },";
                payload += "      \"required\": [" + property_required + "]";
                payload += "    }";
                payload += "  }";
                if (tool_item->next != NULL) {
                    payload += ",";
                }
                tool_item = (ToolItem_t *) tool_item->next;
            }
            payload += "  ]";
            payload += "}";
            ESP_LOGV(TAG, "Payload: %s", payload.c_str());
            httpCode = http.POST(payload);
        }
        if (httpCode > 0) {
            ESP_LOGV(TAG, "[HTTP] POST... code: %d", httpCode);
            String payload = http.getString();
            ESP_LOGV(TAG, "Payload: %s", payload.c_str());
            if (httpCode == HTTP_CODE_OK) {
                JsonDocument doc;
                deserializeJson(doc, payload);

                if (doc["output"].is<JsonArray>()) {
                    int len = doc["output"].size();
                    for (uint32_t i=0;i<len;i++) {
                        if (doc["output"][i]["type"].isNull()) {
                            ESP_LOGE(TAG, "output[%d].type' is invaild", i);
                            continue;
                        }

                        String type = doc["output"][i]["type"].as<String>();
                        if (type == "message") { // message reply
                            String role = doc["output"][i]["role"].as<String>();
                            if (role == "assistant") {
                                if (!doc["output"][i]["content"][0]["text"].isNull()) {
                                    String content = doc["output"][i]["content"][0]["text"].as<String>();
                                    this->putConversationMessage(ROLE_ASSISTANT, content);

                                    responses = content;
                                    end = true;
                                    if (ok) {
                                        *ok = true;
                                    }
                                } else {
                                    ESP_LOGE(TAG, "output[%d].content[0].text' is null", i);
                                }
                            } else {
                                ESP_LOGW(TAG, "OpenAI reply role invaild : %s", role.c_str());
                            }
                        } else if (type == "function_call") { // call tool
                            if ((!doc["output"][i]["name"].isNull()) && (!doc["output"][i]["arguments"].isNull()) && (!doc["output"][i]["call_id"].isNull())) {
                                String name = doc["output"][i]["name"].as<String>();
                                const char * name_cstr = name.c_str();
                                String arguments = doc["output"][i]["arguments"].as<String>();
                                String call_id = doc["output"][i]["call_id"].as<String>();
                                this->putConversationFunctionCall(name, arguments, call_id);

                                JsonDocument doc_arguments;
                                deserializeJson(doc_arguments, arguments);
                                JsonObject arguments_json = doc_arguments.as<JsonObject>();

                                String output = "Error unknow";

                                // Find tool by name
                                ToolItem_t * item = tool_first;
                                while (item) {
                                    if (strcmp(item->tool->getName(), name_cstr) == 0) {
                                        output = item->tool->call(arguments_json);
                                        break;
                                    }
                                    item = (ToolItem_t *) item->next;
                                }
                                doc_arguments.clear(); // free
                                
                                // After call tool
                                this->putConversationFunctionCallOutput(call_id, output);
                            } else {
                                ESP_LOGW(TAG, "OpenAI reply function_call invaild : missing name or arguments or call_id");

                                end = true;
                                if (ok) {
                                    *ok = false;
                                }
                            }
                        }
                    }
                } else {
                    ESP_LOGE(TAG, "'output' is invaild");

                    end = true;
                    if (ok) {
                        *ok = false;
                    }
                }
            } else {
                ESP_LOGV(TAG, "[HTTP] POST... failed, error: %s", http.errorToString(httpCode).c_str());

                end = true;
                if (ok) {
                    *ok = false;
                }
            }
        } else {
            ESP_LOGV(TAG, "[HTTP] POST... failed, error: %s", http.errorToString(httpCode).c_str());

            end = true;
            if (ok) {
                *ok = false;
            }
        }

        http.end();
    } while(!end);

    return responses;
}
