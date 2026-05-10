#include <DuinoClaw.h>
#include <Arduino.h>

#include "OpenAI.h"

static const char * SYSTEM_MESSAGE = 
"You are an assistant running on an ESP32 with very limited memory.\n"
"Keep all responses short, simple, and concise.\n"
"Avoid unnecessary details. Use plain language.";

static const char * TAG = "DuinoClaw";

static const uint8_t PROMPT_REQ_FLAG = BIT0; // Set it when want to send 
static const uint8_t PROCEESS_FINISH_FLAG = BIT1; // Set it after responses for call ResponsesCallback on main task

String prompt_message;
bool responses_ok = false;
String responses_message;

static void DuinoClawLoopTask(void * args) ;

DuinoClaw::DuinoClaw() {

}

LLM_Provider_t DuinoClaw::getProvider() {
    return this->provider;
}

LLM_Model_t DuinoClaw::getModel() {
    return this->model;
}

const char * DuinoClaw::getModelId() {
    return model_id[(int) this->model];
}

const char * DuinoClaw::getAPIKey() {
    return this->api_key;
}

typedef struct {
    EventGroupHandle_t eventHandle;
    OpenAI * llm;
} DuinoClawLoopTaskArgs_t;

void DuinoClaw::begin(LLM_Provider_t provider, LLM_Model_t model, const char * api_key) {
    this->provider = provider;
    this->model = model;
    this->api_key = api_key;

    this->eventHandle = xEventGroupCreate();
    
    // TODO: use provider to make object
    this->llm = new OpenAI(this->getModelId(), api_key);

    String system_message = String(SYSTEM_MESSAGE);
    this->llm->addSystemMessage(system_message);

    static DuinoClawLoopTaskArgs_t task_args;
    task_args.eventHandle = this->eventHandle;
    task_args.llm = llm;

    /* xTaskCreateUniversal(
        DuinoClawLoopTask, "DuinoClaw", DUINO_CLAW_TASK_STACK_SIZE, &task_args, 
        5, &duinoClawTaskHandle, DUINO_CLAW_TASK_CORE_ID); */
}

void DuinoClaw::loop() {
    { // ---------
        uint32_t bits = xEventGroupWaitBits(eventHandle, PROMPT_REQ_FLAG, pdTRUE, pdTRUE, 0);
        if (bits & PROMPT_REQ_FLAG) {
            responses_message = llm->getResponses(prompt_message, &responses_ok);
            // xEventGroupSetBits(eventHandle, PROCEESS_FINISH_FLAG);
            if (responsesCallback) {
                responsesCallback(responses_ok, responses_message);
            }
        }
    } // ---------

    /* uint32_t bits = xEventGroupWaitBits(eventHandle, PROCEESS_FINISH_FLAG, pdTRUE, pdTRUE, 0);
    if (bits & PROCEESS_FINISH_FLAG) {
        if (responsesCallback) {
            responsesCallback(responses_ok, responses_message);
        }
    } */
}

String DuinoClaw::prompt(String message, bool wait) {
    prompt_message = message;
    xEventGroupSetBits(eventHandle, PROMPT_REQ_FLAG);
    if (wait) {
        /* uint32_t bits = xEventGroupWaitBits(eventHandle, PROCEESS_FINISH_FLAG, pdTRUE, pdTRUE, 60000 / portTICK_PERIOD_MS); // max wait 1 mins
        if (bits & PROCEESS_FINISH_FLAG) {
            if (responsesCallback) {
                responsesCallback(responses_ok, responses_message);
            }
            return responses_message;
        } else {
            ESP_LOGE(TAG, "wait process finish timeout");
        } */
        responses_message = llm->getResponses(prompt_message, &responses_ok);

        return responses_message;
    }

    return String();
}

ToolItem_t * tool_first = NULL;
static ToolItem_t * tool_last = NULL;

void DuinoClaw::registerTool(Tool * tool) {
    ToolItem_t * item = (ToolItem_t *) malloc(sizeof(ToolItem_t));
    if (!item) {
        ESP_LOGE(TAG, "register tool failed");
        return;
    }
    item->tool = tool;
    item->next = NULL;

    if (!tool_first) {
        tool_first = item;
    }
    if (tool_last) {
        tool_last->next = item;
    }
    tool_last = item;
}

void DuinoClawLoopTask(void * args) {
    DuinoClawLoopTaskArgs_t * task_args = (DuinoClawLoopTaskArgs_t *) args;
    EventGroupHandle_t eventHandle = task_args->eventHandle;
    OpenAI * llm = task_args->llm;

    while(1) {
        uint32_t bits = xEventGroupWaitBits(eventHandle, PROMPT_REQ_FLAG, pdTRUE, pdTRUE, portMAX_DELAY);
        if (bits & PROMPT_REQ_FLAG) {
            responses_message = llm->getResponses(prompt_message, &responses_ok);
            xEventGroupSetBits(eventHandle, PROCEESS_FINISH_FLAG);
        }
    }
    vTaskDelete(NULL);
}

DuinoClaw Claw;
