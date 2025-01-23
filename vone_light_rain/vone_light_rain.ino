#include "VOneMqttClient.h"

// Define LED pins
int led_pins[] = {5, 6, 9}; // Pins for LEDs

// Define sensor pins
int ir_sensor = 41;  // IR sensor pin
int rain_sensor = 4; // Rain sensor pin

// V-One device ID 
const char* IRSensor = "3c01e9b6-a5cf-4832-84fc-bf956f7011bf";
const char* RainSensor = "ca8cefee-9035-41db-b5de-306f296fb6e1"; 
const char* LED1 = "f1467398-829c-445c-840b-7648fcb30d65";
const char* LED2 = "d6226679-a5f7-495b-be10-ecda91201f77";
const char* LED3 = "b7f67c0e-59f6-415c-8761-883db2181143";


// Create an instance of VOneMqttClient
VOneMqttClient voneClient;

// Last message time for telemetry data
unsigned long lastMsgTime = 0;

// Setup Wi-Fi
void setup_wifi() {
    delay(10);
    Serial.println();
    Serial.print("Connecting to ");
    Serial.println(WIFI_SSID);

    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }

    Serial.println("");
    Serial.println("Wi-Fi connected");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
}

void setup() {
    Serial.begin(9600); // Start Serial Monitor

    // Connect to Wi-Fi
    setup_wifi();

    // Set LED pins as OUTPUT for ESP32
   for (int i = 0; i < 3; i++) {
        pinMode(led_pins[i], OUTPUT);
    }

    // Set sensor pins
    pinMode(ir_sensor, INPUT);  // IR sensor for motion detection
    pinMode(rain_sensor, INPUT); // Rain sensor

    // Setup V-One MQTT Client
    voneClient.setup();
}

void loop() {
    int ir_status = digitalRead(ir_sensor);   // Read IR sensor status
    int rain_status = analogRead(rain_sensor); // Read Rain sensor value

    // Print sensor values to Serial Monitor
    Serial.print("IR Sensor Value: ");
    Serial.print(ir_status);
    Serial.print(" | Rain Sensor Value: ");
    Serial.println(rain_status);

    // Light Control Logic
    if (rain_status < 3000) { // Rain detected
        for (int i = 0; i < 3; i++) {
            analogWrite(led_pins[i], 255); // Full brightness
        }
        if (ir_status == LOW) {
            Serial.println("Rain and motion detected: Lights ON (Full Brightness)");
        } else {
            Serial.println("Rain detected: Lights ON (Full Brightness)");
        }
    } else if (ir_status == LOW) { // No rain, Motion detected
        for (int i = 0; i < 3; i++) {
            analogWrite(led_pins[i], 255); // Full brightness
        }
        Serial.println("Motion detected: Lights ON (Full Brightness)");
    } else { // No rain, No motion
        for (int i = 0; i < 3; i++) {
            analogWrite(led_pins[i], 100); // Dimmed
        }
        Serial.println("No motion, No rain: Lights Dimmed");
    }

    // Send sensor data to V-One
    if (!voneClient.connected()) {
        voneClient.reconnect();
    }
    voneClient.loop();

    unsigned long cur = millis();
    if (cur - lastMsgTime > 5000) {  // Send data every 5 seconds
        lastMsgTime = cur;

        // Create a JSON payload for telemetry data
        JSONVar payloadObject;
        payloadObject["Obstacle"] = ir_status;

        JSONVar payloadobject;
        payloadObject["Raining"] = rain_status;
 
        // Publish the telemetry data to V-One for both sensors
        voneClient.publishTelemetryData(IRSensor, payloadObject);
        voneClient.publishTelemetryData(RainSensor, payloadObject);

        Serial.println("Telemetry data sent to V-One.");
    }

    delay(1000); // Wait for 1 second before the next loop
}