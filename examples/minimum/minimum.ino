#include <Arduino.h>
#include <WiFi.h>
#include <DuinoClaw.h>

// Build'in Tools (option)
#include <Tools/GetCurrentTimeTool.h>

static const char * TAG = "Main";

// WiFi Configs
const char * ssid = " -- WiFi Name --";
const char * password = "-- WiFi Password --";

// OpenAI API Key
const char * api_key = "-- OpenAI Key --";

String set_gpio_output(JsonObject arguments) {
  int pin = arguments["pin"].as<int>();
  int level = arguments["level"].as<int>();
  ESP_LOGV(TAG, "Call 'set_gpio_output' cb with pin: %d, level: %d", pin, level);

  pinMode(pin, OUTPUT);
  digitalWrite(pin, level);

  return "OK";
}

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

  // Add get_current_time tool
  static GetCurrentTimeTool get_current_time_tool(7 /* Timezone GMT+7 */);
  Claw.registerTool(&get_current_time_tool);

  // Add set_gpio_output tool
  static Tool set_gpio_output_tool("set_gpio_output_level", "Call this for set GPIO to LOW (0) or HIGH (1)");
  set_gpio_output_tool.addProperty("pin", "GPIO pin", Tool::TYPE_INTEGER, true);
  set_gpio_output_tool.addProperty("level", "Digital logic / value, 0=LOW, 1=HIGH", Tool::TYPE_INTEGER, true);
  set_gpio_output_tool.onCall(set_gpio_output);
  Claw.registerTool(&set_gpio_output_tool);

  Claw.onResponses(responses_cb);
  Claw.begin(OPEN_AI, GPT_5_4_MINI, api_key);

  Claw.prompt("Plz set GPIO 5 to HIGH");
}

void loop() {
  Claw.loop();
  delay(5);
}
