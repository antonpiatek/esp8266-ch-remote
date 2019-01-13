#include "config.h"
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <Wire.h> // Enable this line if using Arduino Uno, Mega, etc.
#include <Adafruit_GFX.h>
#include "Adafruit_LEDBackpack.h"
#include <LiquidCrystal_I2C_ESP.h> https://github.com/blueinnotech/LiquidCrystal_I2C_ESP
#include <ArduinoJson.h>

#ifndef WIFI_SSID
  #error "WIFI_SSID must be defined in config"
#endif

#ifndef WIFI_PASS
  #error "WIFI_PASS must be defined in config"
#endif

#ifndef BROKER
  #error "BROKER must be defined in config"
#endif

#ifndef BROKER_PASS
  #error "BROKER_PASS must be defined in config"
#endif

#ifndef BUTTON_1_PIN
  #error "BUTTON_1_PIN must be defined in config"
#endif

#ifndef BUTTON_2_PIN
  #error "BUTTON_2_PIN must be defined in config"
#endif

#ifndef TOPIC_1_STATUS
  #error "TOPIC_1_STATUS must be defined in config"
#endif

#ifndef TOPIC_2_STATUS
  #error "TOPIC_2_STATUS must be defined in config"
#endif

#ifndef TOPIC_1_CONTROL
  #error "TOPIC_1_CONTROL must be defined in config"
#endif

#ifndef TOPIC_2_CONTROL
  #error "TOPIC_2_CONTROL must be defined in config"
#endif

// Main program
WiFiClient espClient;
PubSubClient client(espClient); 
String clientId;

String topic1_pub;
String topic2_pub;
String topic_data_sub;//TODO #def and config

bool status_1=false;
bool status_2=false;

volatile int button_1_trigger = 0;
volatile int button_2_trigger = 0;

unsigned long debounceTime=0;
unsigned long debounceThreshold=200; //TODO CONST/CONFIG

Adafruit_7segment matrix = Adafruit_7segment();

StaticJsonDocument<200> doc; // size estimator: arduinojson.org/v6/assistant

//TODO CONST/CONFIG address
LiquidCrystal_I2C lcd(0x3F,20,2);  // set the LCD address to 0x3F for a 16 chars and 2 line display, sda/scl are d2/d1


void setup() {

  pinMode(LED_1_PIN, OUTPUT);
  pinMode(LED_2_PIN, OUTPUT);
  //and off
  digitalWrite(LED_1_PIN, true);
  digitalWrite(LED_2_PIN, true);

  pinMode(BUTTON_1_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(BUTTON_1_PIN), button1, CHANGE);
  pinMode(BUTTON_2_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(BUTTON_2_PIN), button2, CHANGE);

  Serial.begin(9600);
  
  log("init...");
  pinMode(LED_BUILTIN, OUTPUT);

  //set wifi
  Serial.println();
  Serial.print("WIFI configured to ");
  Serial.println(WIFI_SSID);
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  topic1_pub = String(TOPIC_1_CONTROL);
  topic2_pub = String(TOPIC_2_CONTROL);
  topic_data_sub = String("test/data");

  Serial.println("TOPIC1 pub: "+topic1_pub);
  Serial.println("TOPIC2 pub: "+topic2_pub);
  Serial.println("topic data sub: "+topic_data_sub);
  
  //set mqtt
  clientId = WiFi.macAddress();
  client.setServer(BROKER, 1883);
  client.setCallback(callback);

  matrix.begin(0x70);//TODO #def in config
  matrix.clear();
  matrix.writeDisplay();
  
  lcd.init();                      // initialize the lcd 
  lcd.backlight();
  lcd.setCursor(0,0);
  lcd.print("Initialising...");
  checkComms();

  log("ready");
  
  
  client.loop();
}



void checkComms(){
    if(WiFi.status() != WL_CONNECTED){
        while (WiFi.status() != WL_CONNECTED) {
            Serial.print("waiting for wifi, rc= ");
            Serial.println(WiFi.status());
            digitalWrite(LED_BUILTIN, HIGH); 
            delay(500);
            //TODO while connecting flash led
            Serial.print(".");
            digitalWrite(LED_BUILTIN, LOW); 
            delay(500);
        }
        Serial.println("");
        Serial.println("WiFi connected");
        Serial.print("IP address: ");
        Serial.println(WiFi.localIP());
    }
    if(!client.connected()){
        Serial.print("MQTT connecting...");
        while (!client.connected()) {
            digitalWrite(LED_BUILTIN, HIGH); 
            Serial.print(".");
            if (client.connect(clientId.c_str(), "use-token-auth", BROKER_PASS)) {
                Serial.println("connected");
                client.subscribe(topic_data_sub.c_str());
                digitalWrite(LED_BUILTIN, LOW); 
            }else {
                Serial.print("failed, rc=");
                Serial.println(client.state());
            }
        }
    }
    digitalWrite(LED_BUILTIN, LOW); 
}

uint16_t counter = 0;

void loop() {
  checkComms();
  client.loop();

  //Check button status and set leds
  if(button_1_trigger > 0){
    if(millis() - debounceTime > debounceThreshold){
      //status_1 = !status_1;
      publish(topic1_pub);
    }
    button_1_trigger=0;
    debounceTime = millis();
  }
  if(button_2_trigger > 0){
    if(millis() - debounceTime > debounceThreshold){
      //status_2 = !status_2;
      publish(topic2_pub);
    }
    button_2_trigger=0;
    debounceTime = millis();
  }
 
  delay(10);

}

void log(String l){
  Serial.println("# "+l);
}


void callback(char* topic, byte* payload, unsigned int length) {
  //TODO: CALLBACK should be setting a buffer and returning, then main loop dealing with data as then we don't block
  
  char strPayload[length];
  strncpy(strPayload, (char*)payload, length);
  strPayload[length]=NULL;
  
  Serial.print("Message (");
  Serial.print(length);
  Serial.print(") arrived on ");
  Serial.print(topic);
  Serial.print(": '");
  Serial.print((char*)strPayload);
  Serial.println("'");
  
  if(strcmp(topic,topic_data_sub.c_str()) == 0){
    handle_data_callback(strPayload);
    return;
  }
  
  Serial.println("unknown topic");
  
}


void button1(){
  button_1_trigger++;
}

void button2(){
  button_2_trigger++;
}

void publish(String topic){
  Serial.println("publishing on "+topic);
  client.publish(topic.c_str(), "1", true);
}

void handle_data_callback(char* payload){
  Serial.println("parsing json callback");
  
  DeserializationError error = deserializeJson(doc, payload);
  if (error) {
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(error.c_str());
    return;
  }

  int leds_0 = doc["leds"][0]; // 1
  int leds_1 = doc["leds"][1]; // 0

  // negate status as pulldown to turn on LED
  digitalWrite(LED_1_PIN, !leds_0);
  digitalWrite(LED_2_PIN, !leds_1);

  //7seg
  const char* js7seg = doc["7seg"];
  if(js7seg != NULL){
    Serial.print("7seg: ");
    char s7seg[] = "0000";
    strncpy(s7seg,js7seg,4);
    Serial.println(s7seg);
    //Convert (unsafe) char to int  and send 
    matrix.writeDigitNum(0, s7seg[0]-'0');
    matrix.writeDigitNum(1, s7seg[1]-'0');
    matrix.writeDigitNum(3, s7seg[2]-'0');
    matrix.writeDigitNum(4, s7seg[3]-'0');
    matrix.writeDisplay();
  }

  // lcd
  const char* lcd_0 = doc["lcd"][0]; // "1234567890123456"
  const char* lcd_1 = doc["lcd"][1]; // "1234567890123456"
  if(lcd_0 != NULL || lcd_1 != NULL){
    lcd.clear();
  }
  if(lcd_0 != NULL){
    lcd.setCursor(0,0);  
    lcd.print(lcd_0);
  }
  if(lcd_1 != NULL){
    lcd.setCursor(0,1);  
    lcd.print(lcd_1);
  }
  
}


