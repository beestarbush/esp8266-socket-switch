# esp8266-socket-switch

# Configurable parameters
#### Hardware specific
 - define BUTTON          0                                    // (Don't Change for Sonoff) 
 - define RELAY           12                                   // (Don't Change for Sonoff) 
 - define LED             13                                   // (Don't Change for Sonoff) 

#### Wifi specific settings
 - define WIFI_SSID       "##YOUR_SSID##"
 - define WIFI_PASS       "##YOUR_WIFI_PASS"

#### IP specific settings, comment this if you would like to use DHCP.
 - IPAddress gIpAddress      (192, 168, 1, 231); </br>
 - IPAddress gSubnetMask     (255, 255, 255, 0); </br>
 - IPAddress gDefaultGateway (192, 168, 1, 1); </br>
 - IPAddress gDnsServer      (192, 168, 1, 1); </br>

#### Mqtt specific settings
 - define MQTT_CLIENT     "##YOUR_MQTT_CLIENT_ID##"
 - define MQTT_SERVER     "##YOUR_MQTT_BROKER_IP##"
 - define MQTT_PORT       ##YOUR_MQTT_BROKER_PORT##
 - define MQTT_TOPIC      "##YOUR_MQTT_TOPIC_PER_SWITCH##"
 - define MQTT_USER       "##YOUR_MQTT_USER##"
 - define MQTT_PASS       "##YOUR_MQTT_PASSWORD##"
