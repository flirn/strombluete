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

SocketIOclient socketIO;

Adafruit_NeoPixel strip = Adafruit_NeoPixel(NEO_COUNT, NEO_PIN, NEO_GRB + NEO_KHZ800);

Servo servo;

void socketIOEvent(socketIOmessageType_t type, uint8_t * payload, size_t length) {
    switch(type) {
        case sIOtype_DISCONNECT:
            Serial.printf("[IOc] Disconnected!\n");
            break;
        case sIOtype_CONNECT:
            Serial.printf("[IOc] Connected to url: %s\n", payload);

            // join default namespace (no auto join in Socket.IO V3)
            socketIO.send(sIOtype_CONNECT, "/");
            break;
        case sIOtype_EVENT:
        {
            char * sptr = NULL;
            int id = strtol((char *)payload, &sptr, 10);
            Serial.printf("[IOc] get event: %s id: %d\n", payload, id);
            if(id) {
                payload = (uint8_t *)sptr;
            }
            DynamicJsonDocument doc(1024);
            DeserializationError error = deserializeJson(doc, payload, length);
            if(error) {
                Serial.print(F("deserializeJson() failed: "));
                Serial.println(error.c_str());
                return;
            }

            String eventName = doc[0];
            Serial.printf("[IOc] event name: %s\n", eventName.c_str());

            if (eventName == "color") {
              uint32_t color = doc[1];
              /**/
              int servoStart = 70;
              for (int i = 0; i < 50; i++) {
                for (int i = 0; i < NEO_COUNT; i++) {
                  strip.setPixelColor(i, color);
                }
                strip.setBrightness(i);
                strip.show();
                servo.write(servoStart-i);
                delay(100);
              }
              /**/
             /*
              servo.write(60);
              delay(100);
              servo.write(50);
              delay(100);
              servo.write(40);
              delay(100);
              servo.write(30);
              delay(100);
              servo.write(20);
              delay(2000);
              servo.write(30);
              delay(100);
              servo.write(40);
              delay(100);
              servo.write(50);
              delay(100);
              servo.write(60);
              delay(100);
              servo.write(70);
              */

              /**/
              for (int i = 50; i >= 0; i--) {
                for (int i = 0; i < NEO_COUNT; i++) {
                  strip.setPixelColor(i, color);
                }
                strip.setBrightness(i);
                strip.show();
                servo.write(servoStart-i);
                delay(100);
              }
              /**/
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
            Serial.printf("[IOc] get ack: %u\n", length);
            break;
        case sIOtype_ERROR:
            Serial.printf("[IOc] get error: %u\n", length);
            break;
        case sIOtype_BINARY_EVENT:
            Serial.printf("[IOc] get binary: %u\n", length);
            break;
        case sIOtype_BINARY_ACK:
            Serial.printf("[IOc] get binary ack: %u\n", length);
            break;
    }
}

void setup() {
    Serial.begin(115200);
    Serial.setDebugOutput(true);

    Serial.println();
    Serial.println();
    Serial.println();

      for(uint8_t t = 4; t > 0; t--) {
          Serial.printf("[SETUP] BOOT WAIT %d...\n", t);
          Serial.flush();
          delay(1000);
      }

    WiFi.begin(SSID, PASSWORD);

    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.println("Connecting to WiFi..");
    }

    String ip = WiFi.localIP().toString();
    Serial.printf("[SETUP] WiFi Connected %s\n", ip.c_str());

    // server address, port and URL
    socketIO.begin(SERVER_IP, SERVER_PORT, "/socket.io/?EIO=4");

    // event handler
    socketIO.onEvent(socketIOEvent);

    strip.begin();
    strip.setBrightness(50);
    strip.show();

    servo.attach(SERVO_PIN);
}

void loop() {
    socketIO.loop();
}