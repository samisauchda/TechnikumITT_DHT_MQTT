import paho.mqtt.client as mqtt                     # For MQTT client operations
import csv                                          # For CSV file operations
import datetime                                     # For date and time operations
import time                                         # For time operations
import os                                           # For file operations                                         

MQTT_BROKER = "iotgw.local"                                             # MQTT broker address; cal also be an IP address
MQTT_PORT = 1883                                                        # MQTT broker port; default is 1883
CLIENT_ID = "DHTClient2"                                                 # Unique client ID for this script; needs to be different from other clients
USERNAME = "test_user"                                                  # MQTT username
PASSWORD = "test_password"                                              # MQTT password
TOPIC_TEMPERATURE = "DHTSensor/temperature"                             # Topic for temperature readings
TOPIC_HUMIDITY = "DHTSensor/humidity"                                   # Topic for humidity readings
QOS_LEVEL = 2                                                           # Quality of Service level; 0 = at most once, 1 = at least once, 2 = exactly once
TIME_WINDOW = 3                                                         # Time window to consider readings as paired
CSV_FILE = "sensor_data.csv"                                            # CSV file to save the readings

def on_connect(client, userdata, flags, rc):                            
    if rc == 0:                                                         # Connection successful                                         
        print("Successfully connected to MQTT broker")                  # Print success message
        client.subscribe(TOPIC_TEMPERATURE, QOS_LEVEL)                  # Subscribe to temperature topic
        client.subscribe(TOPIC_HUMIDITY, QOS_LEVEL)                     # Subscribe to humidity topic
    else:
        print(f"Connection failed with code {rc}")                      # Print error message if connection fails

def on_message(client, userdata, msg):
    topic = msg.topic                                                   # Get the topic of the message
    value = msg.payload.decode("utf-8")                                 # Decode the message payload to string
    current_time = datetime.datetime.now()                              # Get the current date and time

    print(f"New MQTT Message: {topic} = {value}")                        # Print the topic and value
    
    date_str = current_time.strftime("%Y-%m-%d")                        # get current date as YYYY-MM-DD
    time_str = current_time.strftime("%H:%M:%S")                        # get current time as HH:MM:SS
    if topic == TOPIC_TEMPERATURE:                                      # Check if the topic is temperature
        file_exists = os.path.isfile(CSV_FILE)                          # Check if the file exists
        with open(CSV_FILE, 'a', newline='') as f:                      # Open the CSV file in append mode
            writer = csv.writer(f)                                      # Create a CSV writer object
            if not file_exists:                                         # Check if the file exists                               
                writer.writerow(["Date", "Time", "SensorID", "Temperature", "Humidity"])        # write header line to CSV file
            writer.writerow([date_str, time_str, "DHTSensor", value, ""])                       # Save the reading to the CSV file
    elif topic == TOPIC_HUMIDITY:                                                               # Check if the topic is humidity                       
        file_exists = os.path.isfile(CSV_FILE)                                                  # Check if the file exists
        with open(CSV_FILE, 'a', newline='') as f:                                              # Open the CSV file in append mode
            writer = csv.writer(f)                                                              # Create a CSV writer object
            if not file_exists:                                                                 # Check if the file exists
                writer.writerow(["Date", "Time", "SensorID", "Temperature", "Humidity"])        # write header line to CSV file
            writer.writerow([date_str, time_str, "DHTSensor", "", value])                       # Save the reading to the CSV file

client = mqtt.Client(client_id=CLIENT_ID)                               # Create a new MQTT client instance
client.username_pw_set(USERNAME, PASSWORD)                              # Set the username and password for the MQTT broker                      
client.on_connect = on_connect                                          # set the callback function for connection events
client.on_message = on_message                                          # set the callback function for incoming messages

try:
    print(f"Connecting to {MQTT_BROKER}:{MQTT_PORT}")                   # Print the broker address and port
    print(f"Monitoring temperature and humidity readings...")           # Print the topics being monitored
    print(f"Saving readings to {CSV_FILE}")                             # Print the file name where readings are saved
    
    client.connect(MQTT_BROKER, MQTT_PORT, 60)                          # Connect to the MQTT broker
    client.loop_forever()                                               # Start the loop to process incoming messages                                          
    
except Exception as e:                                                  # Handle any exceptions that occur
    print(f"Error: {e}")                                                # Print the error message