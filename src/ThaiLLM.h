#ifndef __THAI_LLM_H__
#define __THAI_LLM_H__

#include "OpenAI.h"

class ThaiLLM : public OpenAI {
    public:
        ThaiLLM(const char * model, const char * api_key) : OpenAI(model, api_key) {
            api_endpoint = "http://thaillm.or.th/api/v1/chat/completions";
            thai_llm = true;
        }
};

#endif
