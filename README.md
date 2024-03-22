In this project, were used:

1. Solar energy panel;
2. 3d printing upon drawing in the AutoCAD;
3. Microcontroller ESP-WROOM-32 (Arduino IDE);
4. Sensors to measure humidity, temperature and luminosity;
5. Other components (resistors, step-up, TP4056, Arduino etc.);
6. Soldering on a PCB Circuit Board.

I learned a lot with my team through problem solving about one of my favorite subjects: renewable energy. On doing so, the project was highly graded and could be used in my applications in real life. The most exciting part, after making it all work, was making it a real low energy gadget, more information are available in the .pdf file.

Code in English:

// Library for communication with the Ubidots platform
#include <UbidotsEsp32Mqtt.h>
// Library for interaction with the sensors
#include "DHT.h"
// Library for entering hibernation mode. It's the 
// 'deep sleep' mode, but with 30x lower power consumption (5 uA) and with
// consumption 9200x lower compared to normal mode (46 mA).
#include <esp_sleep.h>

// DHT sensor parameters 
#define DHTTYPE DHT11  
uint8_t DHTPIN = 4;
DHT dht(DHTPIN, DHTTYPE);

// Light sensor parameter
uint8_t analogPin = 35;           

// Ubidots and Wi-Fi connection settings
const char *UBIDOTS_TOKEN = "BBFF-lKcfF3UNLvvQCGrZyzJ1XlXnhskj3S";
const char *WIFI_SSID = "Patrick";      
const char *WIFI_PASS = "kikao1234";     
const char *DEVICE_LABEL = "DHT11-ESP32-C";
const char *VARIABLE_LABEL_1 = "temperature"; 
const char *VARIABLE_LABEL_2 = "humidity";
const char *VARIABLE_LABEL_3 = "light"; 
// Publishes data 1 second after exiting hibernation mode
// or every second if this mode was not enabled.
const int PUBLISH_FREQUENCY = 1000; 
// This timer counts the time in ms to perform actions in the
// required time.
unsigned long timer; 
Ubidots ubidots(UBIDOTS_TOKEN);

// Important real-time callback function for MQTT messages. 
void callback(char *topic, byte *payload, unsigned int length)
{
  // When a new MQTT message arrives, this function is called
  // and prints a message.
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  // Iterates each byte of the MQTT message content and 
  // displays on the Serial Monitor.
  for (int i = 0; i < length; i++)
  {
    // Transcribe each byte of the content into a character
    // so that we can understand it on the Serial Monitor.
    Serial.print((char)payload[i]);
  }
  // New line in the Serial Monitor for better understanding.
  Serial.println();
}

// Here, ESP32 initializes serial communication, 
// the DHT sensor and the Ubidots connection, configuring 
// a callback function, performing 
// the essential delays and initializing the  
// timer with the current count in ms.
void setup()
{
  Serial.begin(115200);     
  dht.begin();
  delay(1000);
  ubidots.connectToWifi(WIFI_SSID, WIFI_PASS);
  ubidots.setCallback(callback);
  ubidots.setup();
  ubidots.reconnect();
  timer = millis();
}

void loop()
{
  // Automatically reconnects to the platform
  // if not connected
  if (!ubidots.connected())
  {
    ubidots.reconnect();
  }
  // Since time is passing, it counts to "pull"
  // the data after 1s, but, of course, if it's not in
  // hibernation mode, then it takes 11s, for example.
  if (millis() - timer >= PUBLISH_FREQUENCY) 
  {
  // Sensor reading
  float u = dht.readHumidity();
  float t = dht.readTemperature();
  float l = analogRead(analogPin);
  // Connection between sensors and platform
  // Ubidots.
  ubidots.add(VARIABLE_LABEL_1, t);
  ubidots.add(VARIABLE_LABEL_2, u);
  ubidots.add(VARIABLE_LABEL_3, l);
  
  // Publishing data
  ubidots.publish(DEVICE_LABEL);

  // To find out from the Serial Monitor if
  // passing data to the cloud.
  Serial.println("Temperature: " + String(t));
  Serial.println("Humidity: " + String(u));
  Serial.println("Luminosity: " + String(l));

  // Best reading on M. Serial
  Serial.println("-----------------------------------------");
  // Timer reset
  timer = millis();

  // Delay added to hibernation mode, because it was giving an error
  delay(100); 

  // Checks Wi-Fi signal connection before entering hibernation mode
  if (ubidots.connected() && WiFi.status() == WL_CONNECTED) {
    // Configure and enter hibernation mode
    esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_SLOW_MEM, ESP_PD_OPTION_OFF);
    esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_FAST_MEM, ESP_PD_OPTION_OFF);
    esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_PERIPH, ESP_PD_OPTION_OFF);
    esp_sleep_enable_timer_wakeup(10000000); // Sleep for 10 seconds
    esp_deep_sleep_start();
  } else {
    // Wi-Fi unstable, but in a loop until it works and becomes stable.
    Serial.println("Connection unstable, working on it...");
  }
  }
  // Strategic pause of 100 ms
  // for some tasks to run
  // run asynchronously.
  delay(100); 
  ubidots.loop();
}
