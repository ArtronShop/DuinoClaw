#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>

#include <DuinoClaw.h>
#include <UniversalTelegramBot.h>

static const char * TAG = "Main";

// WiFi Configs
const char * ssid = " -- WiFi Name --";
const char * password = "-- WiFi Password --";

// OpenAI API Key
const char * api_key = "-- OpenAI Key --";

// ---- Telegram
// Telegram BOT Token (Get from Botfather)
#define BOT_TOKEN "-- Telegram Bot Token --"

const unsigned long BOT_MTBS = 1000; // mean time between scan messages

WiFiClientSecure secured_client;
UniversalTelegramBot bot(BOT_TOKEN, secured_client);
unsigned long bot_lasttime;          // last time messages' scan has been done
String last_chat_id;
// ---

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
    if (last_chat_id.length() > 0) {
      bot.sendMessage(last_chat_id, message);
    }
  } else {
    ESP_LOGE(TAG, "response error with '%s'", message.c_str());
    if (last_chat_id.length() > 0) {
      bot.sendMessage(last_chat_id, "error with " + message);
    }
  }
}


void handleNewMessages(int numNewMessages) {
  for (int i = 0; i < numNewMessages; i++) {
    String chat_id = bot.messages[i].chat_id;
    String text = bot.messages[i].text;

    String from_name = bot.messages[i].from_name;
    if (from_name == "") {
      from_name = "Guest";
    }

    if (text == "/start") {
      bot.sendMessage(chat_id, "Hello, This is ESP32 running DuinoClaw");
    } else {
      bot.sendMessage(chat_id, "Processing...");
      last_chat_id = chat_id;

      Claw.prompt(text);
    }
  }
}

void setup() {
  Serial.begin(115200);
  Serial.setDebugOutput(true);

  ESP_LOGI(TAG, "WiFi Connect...");
  WiFi.begin(ssid, password);
  secured_client.setCACert(TELEGRAM_CERTIFICATE_ROOT); // Add root certificate for api.telegram.org
  while(!WiFi.isConnected()) {
    delay(50);
  }
  ESP_LOGI(TAG, "WiFi Connected !");

  ESP_LOGI(TAG, "Retrieving time....");
  configTime(0, 0, "pool.ntp.org"); // get UTC time via NTP
  time_t now = time(nullptr);
  while (now < 24 * 3600) {
    delay(100);
    now = time(nullptr);
  }
  ESP_LOGI(TAG, "Now: %d", now);

  // Add set_gpio_output tool
  static Tool set_gpio_output_tool("set_gpio_output_level", "Call this for set GPIO to LOW (0) or HIGH (1)");
  set_gpio_output_tool.addProperty("pin", "GPIO pin", Tool::TYPE_INTEGER, true);
  set_gpio_output_tool.addProperty("level", "Digital logic / value, 0=LOW, 1=HIGH", Tool::TYPE_INTEGER, true);
  set_gpio_output_tool.onCall(set_gpio_output);
  Claw.registerTool(&set_gpio_output_tool);

  Claw.onResponses(responses_cb);
  Claw.begin(OPEN_AI, GPT_5_4_MINI, api_key);

  // Claw.prompt("Plz set GPIO 5 to HIGH");
}

void loop() {
  Claw.loop();

  if ((millis() - bot_lasttime) >= BOT_MTBS) {
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    while (numNewMessages) {
      ESP_LOGI(TAG, "got response");
      handleNewMessages(numNewMessages);
      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }

    bot_lasttime = millis();
  }
  delay(5);
}
