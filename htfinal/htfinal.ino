#include "MPU9250.h"

#include <ArduinoJson.h>
MPU9250 mpu;
#include "EspMQTTClient.h"
const int capacity = JSON_OBJECT_SIZE(3);
StaticJsonDocument<capacity> doc;

EspMQTTClient client(
  "...",
  "..."
  "...",  // MQTT Broker server ip
  //"MQTTUsername",   // Can be omitted if not needed
  //"MQTTPassword",   // Can be omitted if not needed
  "ClientESP",     // Client name that uniquely identify your device
  1883              // The MQTT port, default to 1883. this line can be omitted
);
void setup()
{
    Serial.begin(115200);

    Wire.begin();

    delay(2000);
    mpu.setup();
    mpu.calibrateAccelGyro();
    mpu.calibrateMag();

    mpu.printCalibration();
     client.enableDebuggingMessages(); // Enable debugging messages sent to serial output
  client.enableHTTPWebUpdater(); // Enable the web updater. User and password default to values of MQTTUsername and MQTTPassword. These can be overrited with enableHTTPWebUpdater("user", "password").
  client.enableLastWillMessage("ClientESP/lastwill", "I am going offline");  // You can activate the retain flag by setting the third parameter to true
}

// This function is called once everything is connected (Wifi and MQTT)
// WARNING : YOU MUST IMPLEMENT IT IF YOU USE EspMQTTClient
void onConnectionEstablished()
{
  // Subscribe to "mytopic/test" and display received message to Serial
  client.subscribe("espin/test", [](const String & payload) {
    Serial.println(payload);
  });

  // Publish a message to "mytopic/test"
  client.publish("espin/test", "This is a message"); // You can activate the retain flag by setting the third parameter to true

  // Execute delayed instructions
  client.executeDelayed(5 * 1000, []() {
    client.publish("espin/test", "This is a message sent 5 seconds later");
  });
}

void loop()
{
    static uint32_t prev_ms = millis();
    if ((millis() - prev_ms) > 16)
    {
        mpu.update();
        mpu.print();

        Serial.print("roll  (x-forward (north)) : ");
        Serial.println(mpu.getRoll());
        Serial.print("pitch (y-right (east))    : ");
        Serial.println(mpu.getPitch());
        Serial.print("yaw   (z-down (down))     : ");
        Serial.println(mpu.getYaw());
        
    //doc["accx"].set(int(0));
    //doc["accy"].set(int(0));
    //doc["accz"].set(int(0));
    
    doc["gyrox"].set(int(mpu.getPitch()));
    doc["gyroy"].set(int(mpu.getRoll()));
    doc["gyroz"].set(int(mpu.getYaw()));
    
    //doc["magx"].set(int(0));
    //doc["magy"].set(int(0));
    //doc["magz"].set(int(0));
    char buffer[512];
 size_t n = serializeJson(doc, buffer);
client.publish("espin",buffer);
  client.loop();
        prev_ms = millis();
    }
}
