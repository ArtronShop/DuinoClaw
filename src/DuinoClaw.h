#ifndef __DUINO_CLAW_H__
#define __DUINO_CLAW_H__

#include <Arduino.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

#include "LLM.h"
#include "OpenAI.h"
#include "Tool.h"
#include "ToolRegister.h"

#define DUINO_CLAW_TASK_STACK_SIZE (64 * 1024)
#define DUINO_CLAW_TASK_CORE_ID (1)

typedef enum {
    OPEN_AI,
} LLM_Provider_t;

typedef enum {
    GPT_5_5 = 0, // GPT-5.5,
    GPT_5_4, // GPT-5.4
    GPT_5_4_MINI, // GPT-5.4 mini
} LLM_Model_t;

class DuinoClaw {
    public:
        DuinoClaw();

        void begin(LLM_Provider_t, LLM_Model_t, const char*) ;
        void loop() ; // for callback can run on main task

        LLM_Provider_t getProvider() ;
        LLM_Model_t getModel() ;
        const char * getModelId() ;
        const char * getAPIKey() ;

        String prompt(String message, bool wait = false);

        typedef std::function<void(bool, String)> ResponsesCallback;
        void onResponses(ResponsesCallback cb) { responsesCallback = cb; };

        // Tools
        void registerTool(Tool * tool) ;

    private:
        LLM_Provider_t provider = OPEN_AI;
        LLM_Model_t model = GPT_5_4_MINI;
        const char * api_key = NULL;

        const char * model_id[3] = { "gpt-5.5", "gpt-5.4", "gpt-5.4-mini" };

        OpenAI * llm = NULL;
        
        EventGroupHandle_t eventHandle = NULL;

        TaskHandle_t duinoClawTaskHandle = NULL;
        ResponsesCallback responsesCallback = NULL;

};

extern DuinoClaw Claw;

#endif