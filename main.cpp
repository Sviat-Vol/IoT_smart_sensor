#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <string>
#include <SensirionI2cSht4x.h>
#include <Wire.h>
#include <tuple>
#include "secrets.h"

SensirionI2cSht4x sensor;
static uint16_t error;
static char errorMessage[64];

uint8_t sending_mode = 0;

float sending_temp_rate = 2; //temperature difference
float sending_hum_rate = 2;

float last_sent_temp = 0;
float last_sent_hum = 0;
unsigned long last_time_sent = 0;
unsigned long last_time_message = 0;

const unsigned long BOT_MTBS = 1000;

WiFiClientSecure secured_client;
UniversalTelegramBot bot(BOT_TOKEN, secured_client);

void process_messages(int new_messages);
std::tuple<float, float> check_sensor_data();


void setup() {
  Serial.begin(9600);
  delay(4000);
  Serial.println("Serial start");

  secured_client.setInsecure();
  Serial.print("Connecting to WiFi: ");
  Serial.println(WIFI_SSID);
  
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }

  Serial.println("\nWiFi connected!");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  Wire.begin(8, 9);
  Wire.setTimeOut(150);
  Serial.println("Wire start");

  sensor.begin(Wire, SHT41_I2C_ADDR_44);
  Serial.println("Sensor started");
  sensor.softReset();
  Serial.println("Sensor reser");
  delay(10);
  uint32_t serialNumber = 0;
  error = sensor.serialNumber(serialNumber);
  if(error != 0){
    Serial.print("Error trying to execute serialNumber(): ");
    errorToString(error, errorMessage, sizeof errorMessage);
    Serial.println(errorMessage);
    return;
  }
  Serial.print("serialNumber: ");
  Serial.println(serialNumber);
}

void loop() {
  float temperature = 0;
  float humidity = 0;
  if(millis() - last_time_message > BOT_MTBS){
    last_time_message = millis();

    int new_messages_num = bot.getUpdates(bot.last_message_received + 1);

    while(new_messages_num){
      Serial.println("Got new messages");
      process_messages(new_messages_num);
      new_messages_num = bot.getUpdates(bot.last_message_received + 1);
    }
  }
  if(sending_mode == 0 and millis() - last_time_sent > 60000){//add time
    float temperature = 0;
    float humidity = 0;
    std::tie(temperature, humidity) = check_sensor_data();
    //check temp and hum
    if(fabsf(temperature - last_sent_temp) > sending_temp_rate or fabsf(humidity - last_sent_hum) > sending_hum_rate){
      bot.sendMessage(CHAT_ID, "Conditions in room changed to: ");
      String update_message = "Temperature: " + String(temperature) + "°C" + ". Humidity: " + String(humidity) + "%.";
      bot.sendMessage(CHAT_ID, update_message);
      Serial.println(update_message.c_str());
      last_sent_hum = humidity;
      last_sent_temp = temperature;
      last_time_sent = millis();
    }
  }
}

void process_messages(int new_messages){
  for(int i = 0; i < new_messages; ++i){
    std::string text = bot.messages[i].text.c_str();
    Serial.println(text.c_str());
    if(text == "/command1"){  //Turn on/off screen

    }else if(text == "/command2"){  //Stop sending updates
      sending_mode = 1;
      Serial.println("Sending mode is set to 1(no regular updates)");
      bot.sendMessage(CHAT_ID, "Temperature and humidity changes will no longer be sent regularly.");
    }else if(text == "/command3"){  //Start sending updates
      sending_mode = 0;
      Serial.println("Sending mode is set to 0(get regular updates)");
      bot.sendMessage(CHAT_ID, "Temperature and humidity changes will be sent regularly.");
    }else if(text == "/command4"){  //Get live information
      Serial.println("Current temperature: ??; Humidity: ??.");
      float temperature = 0;
      float humidity = 0;
      std::tie(temperature, humidity) = check_sensor_data();
      bot.sendMessage(CHAT_ID, "Conditions in room right now: ");
      String update_message = "Temperature: " + String(temperature) + "°C" + ". Humidity: " + String(humidity) + "%.";
      bot.sendMessage(CHAT_ID, update_message);
      Serial.println(update_message.c_str());
      // read sensors and sent data and sent info to serial
    }else if(text.rfind("/command5 ", 0) == 0){  //Set temp update rate
      if(text.length() <= 10){
        Serial.println("Incorrect command argument");
        bot.sendMessage(CHAT_ID, "One argument required");
      }else{
        text.erase(0, 10);
        bool passed = 1;
        int dot_count = 0;
        for(char c : text){
          if(c == '.') dot_count++;
          if((!std::isdigit(c)  and c != '.') or  dot_count > 1){
            passed = 0;
          }
        }
        if(dot_count != 0 and text.length() == 1){
          passed = 0;
        }
        if(passed){
          sending_temp_rate = std::stod(text);
          std::string message = "Temperature sending rate is set to: " + text + "°C.";
          Serial.println(message.c_str());
          bot.sendMessage(CHAT_ID, message.c_str());
        }else{
          Serial.println("Incorrect command argument");
          bot.sendMessage(CHAT_ID, "INCORRECT ARGUMENT");
        }
      }
    }else if(text.rfind("/command6 ", 0) == 0){  //Set humidity update rate
      if(text.length() <= 10){
        Serial.println("Incorrect command argument");
        bot.sendMessage(CHAT_ID, "One argument required");
      }else{
        text.erase(0, 10); 
        bool passed = 1;
        int dot_count = 0;
        for(char c : text){
          if(c == '.') dot_count++;
          if((!std::isdigit(c)  and c != '.') or  dot_count > 1){
            passed = 0;
          }
        }
        if(dot_count != 0 and text.length() == 1){
          passed = 0;
        }
        if(passed){
          sending_hum_rate = std::stod(text);
          std::string message = "Humidity sending rate is set to: " + text + "%.";
          Serial.println(message.c_str());
          bot.sendMessage(CHAT_ID, message.c_str());
        }else{
          Serial.println("Incorrect command argument");
          bot.sendMessage(CHAT_ID, "INCORRECT ARGUMENT");
        }
      }
    }else{
      Serial.print("Unknown command:");
      Serial.println(text.c_str());
      bot.sendMessage(CHAT_ID, "Unknown command.");
    }
  }
}

std::tuple<float, float> check_sensor_data(){
  float temperature = 0;
  float humidity = 0;
  error = sensor.measureHighPrecision(temperature, humidity);
  if(error != 0){
    Serial.print("Error trying to execute serialNumber(): ");
    errorToString(error, errorMessage, sizeof errorMessage);
    Serial.println(errorMessage);
    return std::tuple<float, float>(-100, -100);
  }
  return std::tuple<float, float>(temperature, humidity);
}
