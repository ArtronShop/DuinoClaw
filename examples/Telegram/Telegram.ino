#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <DuinoClaw.h>

// Built-in tools
#include <Tools/GetCurrentTimeTool.h>
#include <Tools/GPIOTool.h>
#include <Tools/WiFiTool.h>

#include <UniversalTelegramBot.h>

static const char * TAG = "Main";

// WiFi credentials
const char * ssid = "-- WiFi Name --";
const char * password = "-- WiFi Password --";

// OpenAI API key — get from https://platform.openai.com/api-keys
const char * api_key = "-- OpenAI Key --";

// Telegram bot token — get from @BotFather on Telegram
#define BOT_TOKEN "-- Telegram Bot Token --"

WiFiClientSecure secured_client;
UniversalTelegramBot bot(BOT_TOKEN, secured_client);
unsigned long bot_lasttime;

// Stores the chat_id of the last sender so the AI response goes to the right chat
String last_chat_id;

// Process incoming Telegram messages
void handleNewMessages(int numNewMessages) {
  for (int i = 0; i < numNewMessages; i++) {
    String chat_id = bot.messages[i].chat_id;
    String text = bot.messages[i].text;

    if (text == "/start") {
      bot.sendMessage(chat_id, "Hello! I am your ESP32 AI assistant. Send me a message to get started.");
    } else {
      // Prevent sending a new prompt while the AI is still responding
      if (Claw.isProcessing()) {
        bot.sendMessage(chat_id, "Please wait, I am still processing the previous request.");
        continue;
      }
      bot.sendMessage(chat_id, "Processing...");
      last_chat_id = chat_id;
      Claw.prompt(text); // Send the user message to the AI
    }
  }
}

// Called when the AI finishes responding — sends the reply back to Telegram
void responses_cb(bool ok, String message) {
  if (last_chat_id.length() == 0) {
    return;
  }
  if (ok) {
    bot.sendMessage(last_chat_id, message);
  } else {
    ESP_LOGE(TAG, "Error: %s", message.c_str());
    bot.sendMessage(last_chat_id, "Error: " + message);
  }
}

void setup() {
  Serial.begin(115200);

  // Connect to WiFi
  ESP_LOGI(TAG, "WiFi Connect...");
  WiFi.begin(ssid, password);
  secured_client.setCACert(TELEGRAM_CERTIFICATE_ROOT); // Required for Telegram TLS
  while (!WiFi.isConnected()) {
    delay(50);
  }
  ESP_LOGI(TAG, "WiFi Connected!");

  // Sync time via NTP — required for TLS certificate validation
  ESP_LOGI(TAG, "Retrieving time...");
  configTime(0, 0, "pool.ntp.org");
  time_t now = time(nullptr);
  while (now < 24 * 3600) {
    delay(100);
    now = time(nullptr);
  }
  ESP_LOGI(TAG, "Time synced");

  // Register built-in tools so the AI can use them
  static GetCurrentTimeTool time_tool(7); // GMT+7
  Claw.registerTool(&time_tool);
  static GPIOTool gpio_tool;
  Claw.registerTool(&gpio_tool);
  static WiFiTool wifi_tool;
  Claw.registerTool(&wifi_tool);

  Claw.onResponses(responses_cb);

  // Start the AI agent
  Claw.begin(OPEN_AI, GPT_5_4_MINI, api_key);
  ESP_LOGI(TAG, "Bot ready");
}

void loop() {
  // Must be called every loop to dispatch AI responses on the main task
  Claw.loop();

  // Poll Telegram for new messages every 1 second
  if ((millis() - bot_lasttime) >= 1000) {
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    while (numNewMessages) {
      ESP_LOGI(TAG, "Got %d new message(s)", numNewMessages);
      handleNewMessages(numNewMessages);
      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }
    bot_lasttime = millis();
  }

  delay(5);
}
