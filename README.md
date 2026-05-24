# DuinoClaw

Embed an AI agent into your Arduino/ESP32 projects with ease.

DuinoClaw lets your ESP32 talk to an LLM (OpenAI or ThaiLLM), maintain conversation history, and execute custom tools (functions). Responses are dispatched on the main Arduino task via `Claw.loop()`.

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
#include <Tools/GPIOTool.h>

void setup() {
  Serial.begin(115200);

  WiFi.begin("ssid", "password");
  while (!WiFi.isConnected()) delay(50);

  // Declare tools as static inside setup()
  static GPIOTool gpio_tool;
  Claw.registerTool(&gpio_tool);

  Claw.onResponses([](bool ok, String message) {
    Serial.println(ok ? "AI: " + message : "Error: " + message);
  });

  Claw.begin(OPEN_AI, GPT_5_4_MINI, "sk-...");
  Claw.prompt("Set GPIO 5 to HIGH");
}

void loop() {
  Claw.loop();
}
```

> **Note:** Always declare `Tool`, `ToolSet`, and built-in tool objects as `static` **inside `setup()`**, not at the global scope. Global objects are constructed before `setup()` runs, which can cause a crash because the system is not fully initialized yet.

## API

### `Claw.begin(provider, model, api_key)`

Initialize the AI agent.

| Parameter | Type | Options |
|---|---|---|
| `provider` | `LLM_Provider_t` | `OPEN_AI`, `THAI_LLM` |
| `model` | `LLM_Model_t` | See table below |
| `api_key` | `const char*` | API key for the selected provider |

**OpenAI models** (`OPEN_AI`):

| Constant | Model ID |
|---|---|
| `GPT_5_5` | `gpt-5.5` |
| `GPT_5_4` | `gpt-5.4` |
| `GPT_5_4_MINI` | `gpt-5.4-mini` |

**ThaiLLM models** (`THAI_LLM`):

| Constant | Model ID |
|---|---|
| `OPEN_THAI_GPT` | `OpenThaiGPT-ThaiLLM-8B-Instruct-v7.2` |
| `TYPHOON` | `Typhoon-S-ThaiLLM-8B-Instruct` |
| `PATHUMMA` | `Pathumma-ThaiLLM-qwen3-8b-think-3.0.0` |
| `THALLE` | `THaLLE-0.2-ThaiLLM-8B-fa` |

> **Note:** ThaiLLM uses the OpenAI-compatible Chat Completions API (`/v1/chat/completions`). Get an API key at [playground.thaillm.or.th/chat](https://playground.thaillm.or.th/chat/).

### `Claw.prompt(message, wait)`

Send a message to the AI.

- `wait = false` (default) — async, result delivered via `onResponses`
- `wait = true` — blocks until response received, returns the response string

### `Claw.onResponses(callback)`

Register a callback for AI responses.

```cpp
Claw.onResponses([](bool ok, String message) {
  // ok      = false if an error occurred
  // message = AI reply text, or error description
});
```

### `Claw.loop()`

Must be called in `loop()` to dispatch response callbacks on the main task.

### `Claw.setSystemMessage(message)`

Override the default system prompt before calling `begin()`.

```cpp
Claw.setSystemMessage("You are a helpful assistant for a smart home system.");
Claw.begin(OPEN_AI, GPT_5_4_MINI, api_key);
```

### `Claw.isProcessing()`

Returns `true` while the AI is waiting for a response.

### `Claw.startConsole(stream)`

Start an interactive console on a Stream (e.g. `Serial`). Reads lines and sends them as prompts.

```cpp
Claw.startConsole(Serial);
```

---

## Built-in Tools

Include and register any combination of built-in tools with `Claw.registerTool()`.

> **Note:** Declare all tool objects as `static` inside `setup()`. Do not declare them at the global scope.

### `GetCurrentTimeTool`

Gets the current date and time via NTP.

```cpp
#include <Tools/GetCurrentTimeTool.h>

void setup() {
  // ...
  static GetCurrentTimeTool time_tool(7 /* timezone offset */);
  Claw.registerTool(&time_tool);
}
```

| Parameter | Default | Description |
|---|---|---|
| `timezone` | `7` | UTC offset in hours |
| `ntp_server1` | `pool.ntp.org` | Primary NTP server |

### `WiFiTool`

Provides WiFi information tools.

```cpp
#include <Tools/WiFiTool.h>

void setup() {
  // ...
  static WiFiTool wifi_tool;
  Claw.registerTool(&wifi_tool);
}
```

| Tool name | Description |
|---|---|
| `wifi_get_ip` | Get device IP address |
| `wifi_get_rssi` | Get signal strength (dBm) |
| `wifi_get_ssid` | Get connected network name |
| `wifi_get_mac` | Get device MAC address |

### `GPIOTool`

Provides GPIO control and reading tools.

```cpp
#include <Tools/GPIOTool.h>

void setup() {
  // ...
  static GPIOTool gpio_tool;
  Claw.registerTool(&gpio_tool);
}
```

| Tool name | Parameters | Description |
|---|---|---|
| `gpio_digital_write` | `pin`, `level` (0/1) | Set pin HIGH or LOW |
| `gpio_digital_read` | `pin` | Read pin state (0 or 1) |
| `gpio_pwm_write` | `pin`, `duty` (0-255), `frequency` (Hz) | Set PWM output |
| `gpio_analog_read` | `pin` | Read ADC value (0-4095) |
| `gpio_pulse_in` | `pin`, `state` (0/1), `timeout_us` | Measure pulse duration |

---

## Examples

### CustomSystemMessage

Override the system prompt to give the AI a specific role.

```cpp
#include <Arduino.h>
#include <WiFi.h>
#include <DuinoClaw.h>
#include <Tools/GetCurrentTimeTool.h>
#include <Tools/GPIOTool.h>

void setup() {
  Serial.begin(115200);

  WiFi.begin("ssid", "password");
  while (!WiFi.isConnected()) delay(50);

  Claw.setSystemMessage(
    "You are a smart home controller running on an ESP32. "
    "Control GPIO pins and report sensor data. "
    "Keep responses short and in plain text only."
  );

  static GetCurrentTimeTool time_tool(7);
  Claw.registerTool(&time_tool);
  static GPIOTool gpio_tool;
  Claw.registerTool(&gpio_tool);

  Claw.onResponses([](bool ok, String message) {
    if (ok) {
      Serial.println(message);
    }
  });

  Claw.begin(OPEN_AI, GPT_5_4_MINI, "sk-...");
  Claw.startConsole(Serial);
}

void loop() {
  Claw.loop();
}
```

---

### CustomTool

Create a custom tool with parameters and an enum property.

```cpp
#include <Arduino.h>
#include <WiFi.h>
#include <DuinoClaw.h>

static const char * COLORS[] = { "red", "green", "blue" };

void setup() {
  Serial.begin(115200);

  WiFi.begin("ssid", "password");
  while (!WiFi.isConnected()) delay(50);

  static Tool led_tool("set_led_color", "Set the RGB LED color");
  led_tool.addEnumProperty("color", "LED color to set", COLORS, 3, true);
  led_tool.onCall([](JsonObject args) -> String {
    String color = args["color"].as<String>();
    if (color == "red")   { digitalWrite(25, HIGH); digitalWrite(26, LOW);  digitalWrite(27, LOW);  }
    if (color == "green") { digitalWrite(25, LOW);  digitalWrite(26, HIGH); digitalWrite(27, LOW);  }
    if (color == "blue")  { digitalWrite(25, LOW);  digitalWrite(26, LOW);  digitalWrite(27, HIGH); }
    return "LED set to " + color;
  });
  Claw.registerTool(&led_tool);

  Claw.onResponses([](bool ok, String message) {
    Serial.println(ok ? message : "Error: " + message);
  });

  Claw.begin(OPEN_AI, GPT_5_4_MINI, "sk-...");
  Claw.prompt("Set the LED to blue");
}

void loop() {
  Claw.loop();
}
```

---

### Telegram Bot with All Built-in Tools

Control your ESP32 through Telegram using all available built-in tools.

```cpp
#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <DuinoClaw.h>
#include <Tools/GetCurrentTimeTool.h>
#include <Tools/WiFiTool.h>
#include <Tools/GPIOTool.h>
#include <UniversalTelegramBot.h>

#define BOT_TOKEN "your_bot_token"

WiFiClientSecure secured_client;
UniversalTelegramBot bot(BOT_TOKEN, secured_client);
String last_chat_id;

void setup() {
  Serial.begin(115200);

  WiFi.begin("your_ssid", "your_password");
  secured_client.setCACert(TELEGRAM_CERTIFICATE_ROOT);
  while (!WiFi.isConnected()) delay(50);

  // Sync time — required for Telegram TLS
  configTime(0, 0, "pool.ntp.org");
  time_t now = time(nullptr);
  while (now < 24 * 3600) { delay(100); now = time(nullptr); }

  static GetCurrentTimeTool time_tool(7);
  Claw.registerTool(&time_tool);
  static WiFiTool wifi_tool;
  Claw.registerTool(&wifi_tool);
  static GPIOTool gpio_tool;
  Claw.registerTool(&gpio_tool);

  Claw.onResponses([](bool ok, String message) {
    if (last_chat_id.length() > 0) {
      bot.sendMessage(last_chat_id, ok ? message : "Error: " + message);
    }
  });

  Claw.begin(OPEN_AI, GPT_5_4_MINI, "your_openai_api_key");
}

void loop() {
  Claw.loop();

  static unsigned long last_check = 0;
  if (millis() - last_check >= 1000) {
    int n = bot.getUpdates(bot.last_message_received + 1);
    while (n) {
      for (int i = 0; i < n; i++) {
        last_chat_id = bot.messages[i].chat_id;
        String text = bot.messages[i].text;
        if (text == "/start") {
          bot.sendMessage(last_chat_id, "Hello! I am your ESP32 AI assistant.");
        } else if (!Claw.isProcessing()) {
          bot.sendMessage(last_chat_id, "Processing...");
          Claw.prompt(text);
        }
      }
      n = bot.getUpdates(bot.last_message_received + 1);
    }
    last_check = millis();
  }
}
```

---

### ThaiLLM

Use a Thai language model instead of OpenAI. Only the `provider` and `model` constants change — everything else stays the same.

```cpp
#include <Arduino.h>
#include <WiFi.h>
#include <DuinoClaw.h>

void setup() {
  Serial.begin(115200);

  WiFi.begin("ssid", "password");
  while (!WiFi.isConnected()) delay(50);

  Claw.onResponses([](bool ok, String message) {
    Serial.println(ok ? message : "Error: " + message);
  });

  Claw.begin(THAI_LLM, OPEN_THAI_GPT, "your_thaillm_api_key");
  Claw.prompt("สวัสดีครับ");
}

void loop() {
  Claw.loop();
}
```

---

## Tool Property Types

| Type | Constant |
|---|---|
| String | `Tool::TYPE_STRING` |
| Number (float) | `Tool::TYPE_NUMBER` |
| Integer | `Tool::TYPE_INTEGER` |

## License

MIT — ArtronShop Co., Ltd.
