#include <Arduino.h>          // Include Arduino core library
#include <WiFi.h>             // Include WiFi library for ESP32
#include <WiFiManager.h>      // Include WiFiManager library for easy WiFi and MQTT parameter configuration
#include <MQTT.h>             // Include MQTT library for ESP32
#include <DHT.h>              // Include DHT library for temperature and humidity sensor

const int dht_pin = 41;       // Pin for DHT sensor

// DHT sensor configuration
#define DHTTYPE DHT11         // DHT 11 sensor type
DHT dht(dht_pin, DHTTYPE);    // Create DHT object from DHT Sensor library

// MQTT parameters - will be configurable through WiFiManager
char mqtt_broker[40] = "iotgw.local";               // Default MQTT broker IP
char mqtt_port[6] = "1883";                         // Default MQTT port                 
char mqtt_client_id[40] = "DHTSensor";              // Default MQTT client ID
char mqtt_username[40] = "test_user";               // Default MQTT username
char mqtt_password[40] = "test_password";           // Default MQTT password
char mqtt_topic_temp[40] = "DHTSensor/temperature"; // Default MQTT topic for temperature
char mqtt_topic_humid[40] = "DHTSensor/humidity";   // Default MQTT topic for humidity

WiFiManager wifiManager;                           // Create an instance of WiFiManager for WiFi and MQTT configuration     

WiFiClient net;                                    // Create a WiFi client for MQTT connection                
MQTTClient mqtt;                                   // Create an MQTT client for communication with the broker                

bool should_save_config = false;                    // Flag to indicate if configuration should be saved

// Callback notifying us of the need to save config; taken from WiFiManager example
void save_config_callback() {                     
  Serial.println("Should save config");             // Debug message indicating config save request
  should_save_config = true;                        // Set flag to true to save config    
}

// Function to configure and connect WiFi using WiFiManager
void setup_wifi() {
  Serial.println("Setting up WiFi connection...");    // Debug message indicating WiFi setup start
  
  // Set callback that gets called when connecting to previous WiFi fails, and enters Access Point mode; taken from WEifimanager example
  wifiManager.setSaveConfigCallback(save_config_callback);

  // Add custom parameters for MQTT config
  WiFiManagerParameter custom_mqtt_broker("mqtt_broker", "MQTT Broker IP", mqtt_broker, 40);                // Custom parameter for MQTT broker IP
  WiFiManagerParameter custom_mqtt_port("mqtt_port", "MQTT Port", mqtt_port, 6);                            // Custom parameter for MQTT port
  WiFiManagerParameter custom_mqtt_client("mqtt_client", "MQTT Client ID", mqtt_client_id, 40);             // Custom parameter for MQTT client ID
  WiFiManagerParameter custom_mqtt_username("mqtt_username", "MQTT Username", mqtt_username, 40);           // Custom parameter for MQTT username
  WiFiManagerParameter custom_mqtt_password("mqtt_password", "MQTT Password", mqtt_password, 40);           // Custom parameter for MQTT password
  WiFiManagerParameter custom_mqtt_topic_temp("mqtt_topic_temp", "Temperature Topic", mqtt_topic_temp, 40); // Custom parameter for temperature topic
  WiFiManagerParameter custom_mqtt_topic_humid("mqtt_topic_humid", "Humidity Topic", mqtt_topic_humid, 40); // Custom parameter for humidity topic
  
  // Add all parameters
  wifiManager.addParameter(&custom_mqtt_broker);          // Add custom MQTT broker parameter
  wifiManager.addParameter(&custom_mqtt_port);            // Add custom MQTT port parameter
  wifiManager.addParameter(&custom_mqtt_client);          // Add custom MQTT client ID parameter
  wifiManager.addParameter(&custom_mqtt_username);        // Add custom MQTT username parameter
  wifiManager.addParameter(&custom_mqtt_password);        // Add custom MQTT password parameter
  wifiManager.addParameter(&custom_mqtt_topic_temp);      // Add custom temperature topic parameter
  wifiManager.addParameter(&custom_mqtt_topic_humid);     // Add custom humidity topic parameter

  // Fetches SSID and password and tries to connect; from WEifimanager Example
  // If it does not connect, it starts an access point with the specified name
  if (!wifiManager.autoConnect("ESP32_DHT")) {             // Start access point with name "ESP32_DHT"
    Serial.println("Failed to connect and hit timeout");   // Debug message indicating connection failure
    delay(3000);                                           // Wait for 3 seconds before restarting                       
    ESP.restart();                                         // Restart ESP32                 
  }

  Serial.println("WiFi connected successfully!");                        // Debug message indicating successful connection
  Serial.printf("IP address: %s\n", WiFi.localIP().toString().c_str());  // Print local IP address

  if (should_save_config) {                                              // Check if config should be saved
    strcpy(mqtt_broker, custom_mqtt_broker.getValue());                  // Copy MQTT broker IP
    strcpy(mqtt_port, custom_mqtt_port.getValue());                      // Copy MQTT port                  
    strcpy(mqtt_client_id, custom_mqtt_client.getValue());               // Copy MQTT client ID     
    strcpy(mqtt_username, custom_mqtt_username.getValue());              // Copy MQTT username      
    strcpy(mqtt_password, custom_mqtt_password.getValue());              // Copy MQTT password   
    strcpy(mqtt_topic_temp, custom_mqtt_topic_temp.getValue());          // Copy MQTT topics 
    strcpy(mqtt_topic_humid, custom_mqtt_topic_humid.getValue());        // Copy MQTT topics
    
    Serial.println("Parameters saved:");                          // Debug message indicating parameters saved
    Serial.printf("MQTT Broker: %s\n", mqtt_broker);              // Print MQTT broker IP
    Serial.printf("MQTT Port: %s\n", mqtt_port);                  // Print MQTT port
    Serial.printf("MQTT Client ID: %s\n", mqtt_client_id);        // Print MQTT client ID
    Serial.printf("MQTT Username: %s\n", mqtt_username);          // Print MQTT username
    Serial.printf("Temperature Topic: %s\n", mqtt_topic_temp);    // Print MQTT topic for temperature
    Serial.printf("Humidity Topic: %s\n", mqtt_topic_humid);      // Print MQTT topic for humidity
  }
}

// Function to connect to MQTT broker
void connect_mqtt() {       
  Serial.print("Connecting to MQTT broker...");                              // Debug message indicating MQTT connection start                                              
  mqtt.begin(mqtt_broker, atoi(mqtt_port), net);                              // Initialize MQTT client with broker IP and port, using WiFi connection	net
  mqtt.setOptions(120, true, 1000);                                           // set MQTT options keepAlive, cleanSession, timeout        
  
  int attempts = 0;                                                  // Initialize connection attempts counter                  
  bool connected = false;                                            // Initialize connection status flag             
  
  while (!connected && attempts < 5) {                               // Try to connect to MQTT broker with a maximum of 5 attempts
    connected = mqtt.connect(mqtt_client_id, mqtt_username, mqtt_password);     // Connect to MQTT broker with client ID, username, and password
    if (!connected) {                                                           // If connection fails
      Serial.print(".");                                                        // Print dot to indicate connection attempt
      delay(1000);                                                              // Wait for 1 second before retrying
      attempts++;                                                               // Increment connection attempts counter
    }
  }
  
  if (connected) {
    Serial.println("Connected to MQTT broker!");                          // Debug message indicating successful connection
  } else {
    Serial.println("Failed to connect to MQTT broker!");                  // Debug message indicating connection failure
    Serial.printf("Last error: %d\n", mqtt.lastError());                 // Print last error code; Error codes can be found at https://github.com/256dpi/lwmqtt/blob/master/include/lwmqtt.h#L15
  }
}

// Read and publish DHT data
void read_and_publish_dht() {                                                      
  float temperature = dht.readTemperature();                            // Read temperature in Celsius
  float humidity = dht.readHumidity();                                  // Read humidity in percentage                     
  
  if (isnan(temperature) || isnan(humidity)) {                // If any reading failed
    Serial.println("Failed to read from DHT sensor!");        // Debug message indicating read failure
    

    delay(1000);                                              // Wait for 1 second before retrying
    temperature = dht.readTemperature();                      // Retry reading temperature
    humidity = dht.readHumidity();                            // Retry reading humidity
  }
  
  if (!isnan(temperature) && !isnan(humidity)) {              // If readings are valid                    
    Serial.printf("Temperature: %.1f°C, Humidity: %.1f%%\n", temperature, humidity);  // Print temperature and humidity values
    
    if (WiFi.status() == WL_CONNECTED && mqtt.connected()) {                          // Check if WiFi and MQTT are connected
      char temp_str[10];                                                              // Buffer for temperature string, 10 characters are sufficient
      char humid_str[10];                                                             // Buffer for humidity string, 10 characters are sufficient   
      snprintf(temp_str, sizeof(temp_str), "%.1f", temperature);                      // Convert temperature to string with 1 decimal place
      snprintf(humid_str, sizeof(humid_str), "%.1f", humidity);                       // Convert humidity to string with 1 decimal place
      
      bool temp_published = mqtt.publish(mqtt_topic_temp, temp_str, false, 2);       // Publish temperature to MQTT topic with QoS 2 (topic, payload, retained, QoS)
      bool humid_published = mqtt.publish(mqtt_topic_humid, humid_str, false, 2);     // Publish humidity to MQTT topic with QoS 2 (topic, payload, retained, QoS)
      
      if (temp_published && humid_published) {                         // If both messages were published successfully
        Serial.println("DHT data published to MQTT with QoS 2");      // Debug message indicating successful publish
      } else {
        Serial.println("Failed to publish some MQTT messages");       // Debug message indicating publish failure
      }
    }
  }
}


void setup() {
  Serial.begin(115200);
  delay(1000);                                                // Longer delay to ensure serial is ready and more stable           
  Serial.println("Starting setup...");                        // Debug message indicating setup start

  Serial.println("Initializing DHT sensor...");               // Debug message indicating DHT sensor initialization
  dht.begin();                                                // Initialize DHT sensor
  Serial.println("DHT sensor initialized");                   // Debug message indicating DHT sensor initialization success

  Serial.println("Starting WiFi setup...");                   // Debug message indicating WiFi setup start
  setup_wifi();                                               // Call function to configure and connect WiFi using WiFiManager
  
  if (WiFi.status() == WL_CONNECTED) {                        // Check if WiFi is connected
    Serial.println("WiFi connected, setting up MQTT...");     // Debug message indicating MQTT setup start
    connect_mqtt();                                           // Call function to connect to MQTT broker
  }
  
  Serial.println("Setup completed successfully.");            // Debug message indicating setup completion
}

void loop() {                        
  unsigned long now = millis();

  if (WiFi.status() != WL_CONNECTED) {                             // If WiFi is not connected
    Serial.println("WiFi disconnected. Reconnecting...");          // Debug message indicating WiFi reconnection
    setup_wifi();                                                  // Call function to configure and connect WiFi using WiFiManager
  }
  
  if (WiFi.status() == WL_CONNECTED && !mqtt.connected()) {        // If WiFi is connected but MQTT is not connected
    Serial.println("MQTT disconnected. Reconnecting...");          // Debug message indicating MQTT reconnection
    connect_mqtt();                                                // Call function to connect to MQTT broker
  }

  static unsigned long lastDHTRead = 0;                          // Variable to store last DHT read time
  if (now - lastDHTRead > 10000) {                               // If 10 seconds have passed since last DHT read
    lastDHTRead = now;                                           // Update last DHT read time
    read_and_publish_dht();                                      // Call function to read and publish DHT data
  }

  delay(100);                                 // Delay for 100 milliseconds to prevent CPU overload
}