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

// OpenAI API key — get from https://platform.openai.com/api-keys
const char * api_key = "-- OpenAI Key --";

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

  // Override the default system message to give the AI a specific role.
  // Call this before Claw.begin().
  Claw.setSystemMessage(
    "You are a smart home controller running on an ESP32. "
    "You can control GPIO pins and read sensor data. "
    "Keep all responses short and in plain text only. "
    "Never use markdown formatting."
  );

  // Register built-in tools so the AI can use them
  static GetCurrentTimeTool time_tool(7); // GMT+7
  Claw.registerTool(&time_tool);
  static GPIOTool gpio_tool;
  Claw.registerTool(&gpio_tool);
  static WiFiTool wifi_tool;
  Claw.registerTool(&wifi_tool);

  // Called when the AI finishes responding
  Claw.onResponses([](bool ok, String message) {
    if (ok) {
      Serial.print("Responses: ");
    } else {
      Serial.print("ERROR: ");
    }
    Serial.println(message);
  });

  // Start the AI agent
  Claw.begin(OPEN_AI, GPT_5_4_MINI, api_key);

  // Send the first prompt
  Claw.prompt("What time is it and what is the WiFi signal strength?");
}

void loop() {
  // Must be called every loop to dispatch AI responses on the main task
  Claw.loop();
  delay(5);
}
