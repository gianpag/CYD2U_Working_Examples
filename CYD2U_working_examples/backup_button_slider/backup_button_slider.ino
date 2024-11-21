/* 
 * This assumes you already have LVGL working with Arduino/ESP32
  - Arduino   
    - C:\Users\kolap\Documents\Arduino
  - EEZ Studio 
    - C:\Users\kolap\eez-projects
    - C:\Users\kolap\Documents\Arduino\EEZ_studio_projects
  - Square Line
    - C:\Users\kolap\Documents\SquareLine\tutorials
    - C:\Users\kolap\Documents\SquareLine\tutorials\Export_UI_files   
    - C:\Users\kolap\Documents\Arduino\SQ_line_CYD
 1. Create UI with Squareline
  - Remember to set both screen width/height and Depth=16 bits as used by ESP32
  - File > Project settings set the following
    - Project Export Root
    - UI Files Export Path (This will contain files you will need to copy over to Arduino project)
      - Remember this location as you will need to copy the contents.
  - Export the UI code
    - Export > Export UI Files
      - This will create files in UI Files Export Path you created above
  - Create a new Arduino project 
  - Open an existing one
  - Copy all UI Files to the Arduino project root
  - In Arduino setup() add:
    - ui_init(); 
    - Run the program
 */

/*
 * This script initializes and runs a UI created using the LVGL library on an ESP32.
 * Make sure to have LVGL, TFT_eSPI, and XPT2046_Touchscreen libraries installed.
 */
#include <WiFi.h> // Use <ESP8266WiFi.h> for ESP8266
#include <WebSocketsClient.h>
#include <ArduinoJson.h>
#include <lvgl.h>
#include "ui.h"
#include <TFT_eSPI.h>
#include <XPT2046_Touchscreen.h>
#include <vars.h>

// WIFI config
const char* ssid = "wifi_ssid";
const char* password = "wifi_pass";

// Home Assistant config
const char* homeAssistantIP = "192.168.x.x"; // Replace with your Home Assistant IP
const int homeAssistantPort = 8123;
const char* accessToken = "xxxxxxxxxxxx"; // Replace with your Home Assistant token
const char* lightEntityId = "light.light_entity_name"; // Replace with your light entity ID

// WebSocket config
WebSocketsClient webSocket;

// Touch Screen pins for ESP32
#define XPT2046_IRQ 36
#define XPT2046_MOSI 32
#define XPT2046_MISO 39
#define XPT2046_CLK 25
#define XPT2046_CS 33

// Screen dimensions
#define SCREEN_WIDTH 240
#define SCREEN_HEIGHT 320

// inversion of color 
#define TFT_INVERSION_OFF
#define TFT_RGB_ORDER TFT_BGR
#define ST7789_DRIVER

// Touchscreen configuration and variables
SPIClass touchscreenSPI = SPIClass(VSPI);
XPT2046_Touchscreen touchscreen(XPT2046_CS, XPT2046_IRQ);
int x, y, z;

// LVGL draw buffer size (1/10th of the screen size, typically efficient)
#define DRAW_BUF_SIZE (SCREEN_HEIGHT * SCREEN_WIDTH / 10 * (LV_COLOR_DEPTH / 8))
uint32_t draw_buf[DRAW_BUF_SIZE / 4];

// Function to log messages if LVGL logging is enabled
#if LV_USE_LOG != 0
void log_print(lv_log_level_t level, const char *buf) {
  LV_UNUSED(level);
  Serial.println(buf);
  Serial.flush();
}
#endif

// Touchscreen read function
void touchscreen_read(lv_indev_t *indev, lv_indev_data_t *data) {
  if (touchscreen.tirqTouched() && touchscreen.touched()) {
    TS_Point p = touchscreen.getPoint();

    // Map touch coordinates to screen dimensions
    x = map(p.x, 200, 3700, 1, SCREEN_WIDTH);
    y = map(p.y, 240, 3800, 1, SCREEN_HEIGHT);
    z = p.z;

    data->state = LV_INDEV_STATE_PRESSED;
    data->point.x = x;
    data->point.y = y;
  } else {
    data->state = LV_INDEV_STATE_RELEASED;
  }
}

// Home Assistant lights integration
bool lightOn = false; // Track light state
bool previousLightState = false;
bool currentLightState = false;

void toggleLight() {
  currentLightState = get_light_state();
  
  if (currentLightState != previousLightState) {
    if (currentLightState) {
      turnOnLight();
    } else {
      turnOffLight();
    }
    previousLightState = currentLightState;
  } 
}

void turnOnLight() {
  sendWebSocketCommand("turn_on");
  //lightOn = true;
  //set_light_state(true);
}

void turnOffLight() {
  sendWebSocketCommand("turn_off");
  //lightOn = false;
  //set_light_state(false);
}


// ChatGPT suggested Slider command for dimming light
void set_brightness(int brightness) {
    if (webSocket.isConnected()) {
        StaticJsonDocument<200> doc;
        doc["id"] = millis();
        doc["type"] = "call_service";
        doc["domain"] = "light";
        doc["service"] = "turn_on";

        JsonObject serviceData = doc.createNestedObject("service_data");
        serviceData["entity_id"] = lightEntityId;
        serviceData["brightness"] = brightness;

        String jsonString;
        serializeJson(doc, jsonString);
        webSocket.sendTXT(jsonString);
        Serial.println("Sent brightness: " + String(brightness));
    } else {
        Serial.println("WebSocket not connected");
    }
}

void sendWebSocketCommand(const char* command) {
  if (webSocket.isConnected()) {
    StaticJsonDocument<200> doc;
    doc["id"] = millis();
    doc["type"] = "call_service";
    doc["domain"] = "light";
    doc["service"] = command;

    JsonObject serviceData = doc.createNestedObject("service_data");
    serviceData["entity_id"] = lightEntityId;

    String jsonString;
    serializeJson(doc, jsonString);
    webSocket.sendTXT(jsonString);
    //Serial.println("Sent: " + jsonString);
  } else {
    Serial.println("WebSocket not connected");
  }
}

void webSocketEvent(WStype_t type, uint8_t * payload, size_t length) {
  switch(type) {
    case WStype_CONNECTED:
      Serial.println("Connected to Home Assistant");
      // Authenticate with Home Assistant
      authenticate();
      break;
    case WStype_DISCONNECTED:
      Serial.println("Disconnected from Home Assistant");
      break;
    case WStype_TEXT:
      //Serial.printf("Received message: %s\n", payload);
      break;
  }
}

void authenticate() {
  StaticJsonDocument<100> doc;
  doc["type"] = "auth";
  doc["access_token"] = accessToken;

  String authString;
  serializeJson(doc, authString);
  webSocket.sendTXT(authString);
  Serial.println("Authentication sent to Home Assistant");
}

// Setup function
void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, password);

  // Wait for WiFi connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");

  // Configure WebSocket
  String url = String("/api/websocket");
  webSocket.begin(homeAssistantIP, homeAssistantPort, url);
  webSocket.onEvent(webSocketEvent);

  //ChatGPT suggestion for Slider dimmer initial setup
  // Set initial slider value (e.g., 50% brightness)
  //lv_slider_set_value(objects.obj2, 128, LV_ANIM_OFF);

  // Print LVGL version info
  String LVGL_Arduino = String("LVGL Library Version: ") + lv_version_major() + "." + lv_version_minor() + "." + lv_version_patch();
  Serial.println(LVGL_Arduino);

  // Initialize LVGL
  lv_init();
  
  // Register print function for debugging (if logging is enabled)
  #if LV_USE_LOG != 0
  lv_log_register_print_cb(log_print);
  #endif

  // Initialize SPI for the touchscreen
  touchscreenSPI.begin(XPT2046_CLK, XPT2046_MISO, XPT2046_MOSI, XPT2046_CS);
  touchscreen.begin(touchscreenSPI);
  touchscreen.setRotation(2); // Set touchscreen rotation if necessary

  // Initialize the TFT display using TFT_eSPI
  lv_display_t *disp;
  disp = lv_tft_espi_create(SCREEN_WIDTH, SCREEN_HEIGHT, draw_buf, sizeof(draw_buf));
  lv_display_set_rotation(disp, LV_DISPLAY_ROTATION_270);

  // Initialize the input device (touchscreen) for LVGL
  lv_indev_t *indev = lv_indev_create();
  lv_indev_set_type(indev, LV_INDEV_TYPE_POINTER);
  lv_indev_set_read_cb(indev, touchscreen_read);

  // Initialize the UI
  ui_init();
}

// Main loop function
void loop() {
  webSocket.loop();
  lv_task_handler(); // Handle LVGL tasks
  lv_tick_inc(5);    // Update LVGL tick count
  toggleLight();
  delay(5);          // Delay to control update rate
}
