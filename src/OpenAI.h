#ifndef __LLM_OPEN_AI_H__
#define __LLM_OPEN_AI_H__

#include <stdio.h>
#include "LLM.h"

class OpenAI : public LLM {
    protected:
        const char * api_endpoint = "https://api.openai.com/v1/responses";
        bool thai_llm = false;

    private:
        const char * model = NULL;
        const char * api_key = NULL;

    public:
        OpenAI(const char * model, const char * api_key);
        String getResponses(String prompt, bool * ok = NULL);
        
};


#endif
