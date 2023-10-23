#include <Arduino.h>

#include <WiFi.h>
#include <WiFiMulti.h>
#include <WiFiClientSecure.h>

#include <ArduinoJson.h>

#include <WebSocketsClient.h>
#include <SocketIOclient.h>

#include <Adafruit_NeoPixel.h>

#include <Servo.h>

#include "config.h"

#define NEO_PIN 14
#define NEO_COUNT 8
#define SERVO_PIN 13

#define SERVO_CLOSED 80
#define SERVO_OPEN 40
#define BRIGHTNESS_CLOSED 10
#define BRIGHTNESS_OPEN 70
#define MOVEMENT_DELAY 100

// #define SERIAL_DEBUG

SocketIOclient socketIO;

Adafruit_NeoPixel strip = Adafruit_NeoPixel(NEO_COUNT, NEO_PIN, NEO_GRB + NEO_KHZ800);

Servo servo;

int servoPosition = SERVO_CLOSED;
uint32_t color = 0xFFFFFF;
TaskHandle_t taskHandle = NULL;

int servoToBrightness(int servo) {
    int brightness = map(servo, SERVO_CLOSED, SERVO_OPEN, 0, 100);
    brightness = brightness * brightness;
    brightness = map(brightness, 0, 100*100, BRIGHTNESS_CLOSED, BRIGHTNESS_OPEN);
    return brightness;
}

void moveFlower(void* parameter) {
    for (int i = servoPosition; i >= SERVO_OPEN; i--) {
        servoPosition = i;
        servo.write(servoPosition);
        for (int k = 0; k < NEO_COUNT; k++) {
            strip.setPixelColor(k, color);
        }
        strip.setBrightness(servoToBrightness(servoPosition));
        strip.show();
        delay(MOVEMENT_DELAY);
    }

    for (int i = servoPosition; i <= SERVO_CLOSED; i++) {
        servoPosition = i;
        servo.write(servoPosition);
        for (int k = 0; k < NEO_COUNT; k++) {
            strip.setPixelColor(k, color);
        }
        strip.setBrightness(servoToBrightness(servoPosition));
        strip.show();
        delay(MOVEMENT_DELAY);
    }

    taskHandle = NULL;
    vTaskDelete(NULL);
}

void socketIOEvent(socketIOmessageType_t type, uint8_t * payload, size_t length) {
    switch(type) {
        case sIOtype_DISCONNECT:
            #ifdef SERIAL_DEBUG
            Serial.printf("[IOc] Disconnected!\n");
            #endif
            break;
        case sIOtype_CONNECT:
            #ifdef SERIAL_DEBUG
            Serial.printf("[IOc] Connected to url: %s\n", payload);
            #endif

            // join default namespace (no auto join in Socket.IO V3)
            socketIO.send(sIOtype_CONNECT, "/");
            break;
        case sIOtype_EVENT:
        {
            char * sptr = NULL;
            int id = strtol((char *)payload, &sptr, 10);
            #ifdef SERIAL_DEBUG
            Serial.printf("[IOc] get event: %s id: %d\n", payload, id);
            #endif
            if(id) {
                payload = (uint8_t *)sptr;
            }
            DynamicJsonDocument doc(1024);
            DeserializationError error = deserializeJson(doc, payload, length);
            if(error) {
                #ifdef SERIAL_DEBUG
                Serial.print(F("deserializeJson() failed: "));
                Serial.println(error.c_str());
                #endif
                return;
            }

            String eventName = doc[0];
            #ifdef SERIAL_DEBUG
            Serial.printf("[IOc] event name: %s\n", eventName.c_str());
            #endif

            if (eventName == "color") {
                color = doc[1];
                #ifdef SERIAL_DEBUG
                Serial.printf("Decoded color: %i\n", color);
                #endif
                if (taskHandle != NULL) {
                    vTaskDelete(taskHandle);
                    taskHandle = NULL;
                }
                xTaskCreate(
                    moveFlower,
                    "move flower",
                    2000,
                    NULL,
                    1,
                    &taskHandle
                );
            }

            // Message Includes a ID for a ACK (callback)
            if(id) {
                // creat JSON message for Socket.IO (ack)
                DynamicJsonDocument docOut(1024);
                JsonArray array = docOut.to<JsonArray>();

                // add payload (parameters) for the ack (callback function)
                JsonObject param1 = array.createNestedObject();
                param1["now"] = millis();

                // JSON to String (serializion)
                String output;
                output += id;
                serializeJson(docOut, output);

                // Send event
                socketIO.send(sIOtype_ACK, output);
            }
        }
            break;
        case sIOtype_ACK:
            #ifdef SERIAL_DEBUG
            Serial.printf("[IOc] get ack: %u\n", length);
            #endif
            break;
        case sIOtype_ERROR:
            #ifdef SERIAL_DEBUG
            Serial.printf("[IOc] get error: %u\n", length);
            #endif
            break;
        case sIOtype_BINARY_EVENT:
            #ifdef SERIAL_DEBUG
            Serial.printf("[IOc] get binary: %u\n", length);
            #endif
            break;
        case sIOtype_BINARY_ACK:
            #ifdef SERIAL_DEBUG
            Serial.printf("[IOc] get binary ack: %u\n", length);
            #endif
            break;
    }
}

void setup() {
    #ifdef SERIAL_DEBUG
    Serial.begin(115200);
    Serial.setDebugOutput(true);
    Serial.println();
    Serial.println();
    Serial.println();
    #endif

    for(uint8_t t = 4; t > 0; t--) {
          #ifdef SERIAL_DEBUG
          Serial.printf("[SETUP] BOOT WAIT %d...\n", t);
          Serial.flush();
          #endif
          delay(1000);
    }

    WiFi.begin(SSID, PASSWORD);

    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      #ifdef SERIAL_DEBUG
      Serial.println("Connecting to WiFi..");
      #endif
    }

    String ip = WiFi.localIP().toString();
    #ifdef SERIAL_DEBUG
    Serial.printf("[SETUP] WiFi Connected %s\n", ip.c_str());
    #endif

    // server address, port and URL
    socketIO.begin(SERVER_IP, SERVER_PORT, "/socket.io/?EIO=4");

    // event handler
    socketIO.onEvent(socketIOEvent);

    strip.begin();
    for (int k = 0; k < NEO_COUNT; k++) {
        strip.setPixelColor(k, color);
    }
    strip.setBrightness(BRIGHTNESS_CLOSED);
    strip.show();

    servo.attach(SERVO_PIN);
    servo.write(servoPosition);
}

void loop() {
    socketIO.loop();
}