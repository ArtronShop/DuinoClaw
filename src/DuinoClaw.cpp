#include <DuinoClaw.h>
#include <Arduino.h>

#include "OpenAI.h"
#include "ToolSet.h"

static const char * TAG = "DuinoClaw";

static const uint8_t PROMPT_REQ_FLAG = BIT0; // Set it when want to send 
static const uint8_t PROCEESS_FINISH_FLAG = BIT1; // Set it after responses for call ResponsesCallback on main task
static const uint8_t PROCEESSING_FLAG = BIT2; // Set when start and clear after finish

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

void DuinoClaw::begin(LLM_Provider_t provider, LLM_Model_t model, const char * api_key) {
    this->provider = provider;
    this->model = model;
    this->api_key = api_key;

    this->eventHandle = xEventGroupCreate();
    
    // TODO: use provider to make object
    this->llm = new OpenAI(this->getModelId(), api_key);

    String system_message = String(this->system_message);
    this->llm->addSystemMessage(system_message);
}

String DuinoClaw::prompt(String message, bool wait) {
    prompt_message = message;
    xEventGroupSetBits(eventHandle, PROMPT_REQ_FLAG | PROCEESSING_FLAG);
    if (wait) {
        responses_message = llm->getResponses(prompt_message, &responses_ok);

        return responses_message;
    }

    return String();
}

bool DuinoClaw::isProcessing() {
    return xEventGroupGetBits(eventHandle) & PROCEESSING_FLAG;
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

void DuinoClaw::registerTool(ToolSet * toolset) {
    ToolSet::ToolSetItem_t * item = toolset->getFirst();
    while (item) {
        registerTool(item->tool);
        item = (ToolSet::ToolSetItem_t *) item->next;
    }
}

void DuinoClaw::startConsole(Stream & consoleStream) {
    consoleStream.setTimeout(100);
    this->consoleStream = &consoleStream;

    consoleStream.println();
    consoleStream.println();
    consoleStream.println("DuinoClaw Console Start");
    consoleStream.println();
    consoleStream.print("> ");

    xTaskCreate([](void * consoleStream) {
        Stream * console = (Stream *) consoleStream;

        while(1) {
            if (Claw.isProcessing()) {
                delay(50);
                continue;
            }

            String line = "";
            while(console->available()) {
                char c = console->read();
                if (c == '\r') {
                    continue;
                }
                if (c == '\n') {
                    console->println();

                    Claw.prompt(line);

                    console->println("Processing...");

                    line = "";
                    continue;
                }
                console->write(c);
                line += c;
            }

            delay(10);
        }
        vTaskDelete(NULL);
    }, "ConsoleReadTask", 1 * 1024, &consoleStream, 5, &consoleReadTaskHandle);
}

void DuinoClaw::loop() {
    {
        uint32_t bits = xEventGroupWaitBits(eventHandle, PROMPT_REQ_FLAG, pdTRUE, pdTRUE, 0);
        if (bits & PROMPT_REQ_FLAG) {
            responses_message = llm->getResponses(prompt_message, &responses_ok);
            xEventGroupClearBits(eventHandle, PROCEESSING_FLAG);
            if (responsesCallback) {
                responsesCallback(responses_ok, responses_message);
            }
            if (this->consoleStream) {
                this->consoleStream->println(responses_message);
                this->consoleStream->print("> ");
            }
        }
    }
}

DuinoClaw Claw;
