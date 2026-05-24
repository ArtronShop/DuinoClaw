#include <Arduino.h>
#include <WiFi.h>
#include <DuinoClaw.h>

// WiFi Configs
const char * ssid = "-- WiFi Name --";
const char * password = "-- WiFi Password --";

// AI Provider API Key
const char * api_key = "-- OpenAI/ThaiLLM Key --";

void responses_cb(bool ok, String message) {
  if (ok) {
    Serial.print("Responses: ");
  } else {
    Serial.print("ERROR: ");
  }
  Serial.println(message);
}

void setup() {
  Serial.begin(115200);

  // Connect to WiFi
  Serial.println();
  Serial.print("WiFi Connecting");
  WiFi.begin(ssid, password);
  while (!WiFi.isConnected()) {
    delay(500);
    Serial.print(".");
  }
  Serial.println(" Connected !");

  Claw.onResponses(responses_cb);
  Claw.begin(OPEN_AI, GPT_5_4_MINI, api_key);

  Claw.prompt("Hello, how are you?");
}

void loop() {
  Claw.loop();
  delay(5);
}
