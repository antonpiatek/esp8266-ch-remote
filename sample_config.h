/*
 * Config settings - copy to new file called "config.h" and edit
 */
// Wifi settings
#define WIFI_SSID "my-wifi"
#define WIFI_PASS "My Password"

// broker IP/host
#define BROKER "10.0.0.1"
// broker password or blank
#define BROKER_PASS ""

// override default timeout from PubSubClient lib
#define MQTT_KEEPALIVE 60

// Relay pins on board
#define BUTTON_1_PIN D5
#define BUTTON_2_PIN D6

// LED pins on board
#define LED_1_PIN D7
#define LED_2_PIN D8

//Topics to pub/sub to
#define TOPIC_1_STATUS "Root/channel_1/status"
#define TOPIC_2_STATUS "Root/channel_2/status"
#define TOPIC_1_CONTROL "Root/channel_1"
#define TOPIC_2_CONTROL "Root/channel_2"
