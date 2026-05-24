#include <Arduino.h>
#include <WiFi.h>
#include <DuinoClaw.h>

// WiFi credentials
const char * ssid = "-- WiFi Name --";
const char * password = "-- WiFi Password --";

// OpenAI/ThaiLLM API key — get from https://platform.openai.com/api-keys, https://playground.thaillm.or.th/
const char * api_key = "-- OpenAI/ThaiLLM Key --";

// GPIO pins for RGB LED
#define PIN_LED_RED   25
#define PIN_LED_GREEN 26
#define PIN_LED_BLUE  27

// Enum values for the LED color tool (string enum)
static const char * COLORS[] = { "red", "green", "blue", "off" };

// Enum values for the buzzer tool (integer enum: 0=off, 1=on)
static const long BUZZER_STATES[] = { 0, 1 };

void setup() {
  Serial.begin(115200);

  pinMode(PIN_LED_RED, OUTPUT);
  pinMode(PIN_LED_GREEN, OUTPUT);
  pinMode(PIN_LED_BLUE, OUTPUT);

  // Connect to WiFi
  Serial.println();
  Serial.print("WiFi Connecting");
  WiFi.begin(ssid, password);
  while (!WiFi.isConnected()) {
    delay(500);
    Serial.print(".");
  }
  Serial.println(" Connected !");

  // --- Custom Tool 1: LED color (string enum) ---
  // addEnumProperty restricts the AI to only send values in the COLORS array
  static Tool led_tool("set_led_color", "Set the RGB LED color");
  led_tool.addEnumProperty("color", "LED color to display", COLORS, 4, true);
  led_tool.onCall([](JsonObject args) -> String {
    String color = args["color"].as<String>();

    Serial.print("'set_led_color' call with color=");
    Serial.println(color);

    digitalWrite(PIN_LED_RED, color == "red" ? HIGH : LOW);
    digitalWrite(PIN_LED_GREEN, color == "green" ? HIGH : LOW);
    digitalWrite(PIN_LED_BLUE, color == "blue" ? HIGH : LOW);
    return "LED set to " + color;
  });
  Claw.registerTool(&led_tool);

  // --- Custom Tool 2: Buzzer (integer enum) ---
  // addEnumProperty with long[] restricts state to 0 or 1 only
  static Tool buzzer_tool("set_buzzer", "Turn the buzzer on or off");
  buzzer_tool.addProperty("pin", "GPIO pin of the buzzer", Tool::TYPE_INTEGER, true);
  buzzer_tool.addEnumProperty("state", "0=off, 1=on", BUZZER_STATES, 2, true);
  buzzer_tool.onCall([](JsonObject args) -> String {
    int pin = args["pin"].as<int>();
    int state = args["state"].as<int>();

    Serial.print("'buzzer_tool' call with pin=");
    Serial.print(pin);
    Serial.print(", state=");
    Serial.println(state);

    pinMode(pin, OUTPUT);
    digitalWrite(pin, state);
    return state ? "Buzzer on" : "Buzzer off";
  });
  Claw.registerTool(&buzzer_tool);

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
  Claw.prompt("Set the LED to blue and turn the buzzer on pin 32 off");
}

void loop() {
  // Must be called every loop to dispatch AI responses on the main task
  Claw.loop();
  delay(5);
}
