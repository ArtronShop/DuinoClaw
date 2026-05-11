#include <Arduino.h>
#include <WiFi.h>
#include <DuinoClaw.h>

static const char * TAG = "Main";

// WiFi Configs
const char * ssid = " -- WiFi Name --";
const char * password = "-- WiFi Password --";

// OpenAI API Key
const char * api_key = "-- OpenAI Key --";

void responses_cb(bool ok, String message) {
  if (ok) {
    ESP_LOGV(TAG, "response ok, message=%s", message.c_str());
  } else {
    ESP_LOGE(TAG, "response error with '%s'", message.c_str());
  }
}

void setup() {
  Serial.begin(115200);
  Serial.setDebugOutput(true);

  // WiFi Connact
  ESP_LOGI(TAG, "WiFi Connect...");
  WiFi.begin(ssid, password);
  while(!WiFi.isConnected()) {
    delay(50);
  }
  ESP_LOGI(TAG, "WiFi Connected !");

  Claw.onResponses(responses_cb);
  Claw.begin(OPEN_AI, GPT_5_4_MINI, api_key);

  Claw.prompt("Hello, how are you?");
}

void loop() {
  Claw.loop();
  delay(5);
}
