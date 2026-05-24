#include <Arduino.h>
#include <WiFi.h>
#include <DuinoClaw.h>

// Built-in tools
#include <Tools/GetCurrentTimeTool.h>
#include <Tools/GPIOTool.h>
#include <Tools/WiFiTool.h>

// WiFi credentials
const char * ssid = "-- WiFi Name --";
const char * password = "-- WiFi Password --";

// OpenAI/ThaiLLM API key — get from https://platform.openai.com/api-keys, https://playground.thaillm.or.th/
const char * api_key = "-- OpenAI/ThaiLLM Key --";

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

  // Register built-in tools so the AI can use them
  static GetCurrentTimeTool time_tool(7); // GMT+7
  Claw.registerTool(&time_tool);
  static GPIOTool gpio_tool;
  Claw.registerTool(&gpio_tool);
  static WiFiTool wifi_tool;
  Claw.registerTool(&wifi_tool);

  // Start the AI agent
  Claw.begin(OPEN_AI, GPT_5_4_MINI, api_key);

  // Start interactive console on Serial.
  // Type a message and press Enter to send it to the AI.
  // The AI response will be printed back to Serial automatically.
  Claw.startConsole(Serial);
}

void loop() {
  // Must be called every loop to dispatch AI responses on the main task
  Claw.loop();
  delay(5);
}
