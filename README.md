# DuinoClaw

Embed an AI agent into your Arduino/ESP32 projects with ease.

DuinoClaw lets your ESP32 talk to an LLM (OpenAI), maintain conversation history, and execute custom tools (functions) — all running as a background FreeRTOS task.

## Requirements

- ESP32 (Arduino framework via PlatformIO)
- Libraries: `ArduinoJson`

## Installation

Add to your `platformio.ini`:

```ini
lib_deps =
    ArduinoJson
    https://github.com/ArtronShop/DuinoClaw
```

## Quick Start

```cpp
#include <Arduino.h>
#include <WiFi.h>
#include <DuinoClaw.h>

// Tool callback — runs when AI decides to call the tool
String set_gpio_output(JsonObject arguments) {
    int pin   = arguments["pin"].as<int>();
    int level = arguments["level"].as<int>();
    digitalWrite(pin, level);
    return "OK";
}

void responses_cb(bool ok, String message) {
    if (ok) Serial.println("AI: " + message);
    else    Serial.println("Error: " + message);
}

void setup() {
    Serial.begin(115200);

    WiFi.begin("ssid", "password");
    while (!WiFi.isConnected()) delay(50);

    // Register tool
    static Tool gpio_tool("set_gpio_output_level", "Set a GPIO pin to LOW (0) or HIGH (1)");
    gpio_tool.addProperty("pin",   "GPIO pin number",              Tool::TYPE_INTEGER, true);
    gpio_tool.addProperty("level", "Logic level: 0=LOW, 1=HIGH",  Tool::TYPE_INTEGER, true);
    gpio_tool.onCall(set_gpio_output);
    Claw.registerTool(&gpio_tool);

    Claw.onResponses(responses_cb);
    Claw.begin(OPEN_AI, GPT_5_4_MINI, "sk-...");

    Claw.prompt("Set GPIO 5 to HIGH");
}

void loop() {
    Claw.loop(); // required for callbacks to run on the main task
}
```

## API

### `Claw.begin(provider, model, api_key)`

Initialize and start the background AI task.

| Parameter | Type | Description |
|---|---|---|
| `provider` | `LLM_Provider_t` | `OPEN_AI` |
| `model` | `LLM_Model_t` | `GPT_5_5`, `GPT_5_4`, `GPT_5_4_MINI` |
| `api_key` | `const char*` | Your OpenAI API key |

### `Claw.prompt(message)`

Send a message to the AI. Runs asynchronously; result is delivered via `onResponses`.

### `Claw.onResponses(callback)`

Register a callback for AI responses.

```cpp
Claw.onResponses([](bool ok, String message) {
    // ok    = false if an error occurred
    // message = AI reply text
});
```

### `Claw.loop()`

Must be called in `loop()` to dispatch response callbacks on the main Arduino task.

### Tool Registration

```cpp
Tool myTool("tool_name", "Tool description");
myTool.addProperty("param", "Description", Tool::TYPE_INTEGER, /* required */ true);
myTool.onCall([](JsonObject args) -> String {
    // read args["param"].as<int>() etc.
    return "result";
});
Claw.registerTool(&myTool);
```

**Property types:** `Tool::TYPE_STRING`, `Tool::TYPE_NUMBER`, `Tool::TYPE_INTEGER`

## License

MIT — ArtronShop Co., Ltd.
