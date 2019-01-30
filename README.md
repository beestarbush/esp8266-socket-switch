### ESP8266 socket switch

This software is written for the SonOff WiFi wireless switches. When a switch is loaded with this software, it is able to use the regular functionality, but it is also possible to switch it via the MQTT protocol. This means that the switch will connect to a specified WiFi network, connect to a broker and then it is able to send and receive data via the publish/subscribe mechanism. This can be combined with home automation systems like HomeAssistant, OpenHAB etc.

The software is fairly simple and the aim for this simplicity is to create a stable working switch. To achieve this stability there is chosen for a static IP address because when testing is was more stable than DHCP on a WiFi network. The switch is periodially (30 seconds) checking the connection to the WiFi network or to the MQTT broker. When the connection is lost, it will reset itself and will try to reinitialize the connections. This reinitialization process will stop until a connection is established.

The switches can be configured in software at the top of the file, once flashed it will keep these settings. In a future release a configuration update will be possible via WiFi. Below the configuration parameters are shown and a description is given, even as a example configuration.

#### Hardware specific
The software contains a few hardware specific parameters. This is for the button, relay and the LED on the switch. These settings are fixed for SonOff switches, but can be changed for other ESP8266 devices.
- #define BUTTON          0
- #define RELAY           12 
- #define LED             13

#### Network specific settings
A several network specific settings needs to specified. First of all it needs the WiFi SSID and password to connect to the wireless network. This parameters can be configured at the defines WIFI_SSID and WIFI_PASS, note that these strings must be between quotes as shown in the example below.
 - #define WIFI_SSID       "example_wifi_ssid"
 - #define WIFI_PASS       "password_of_wifi_network"
 
Furthermore, it is recommended to use a static IP address as explained before. This can be configured in the IP address objects as shown in the example below. If you would like to use DHCP, comment out these parameters and comment out the line Wifi.config(); in the InitialiseNetwork() method.
 - IPAddress gIpAddress      (192, 168, 1, 231);
 - IPAddress gSubnetMask     (255, 255, 255, 0);
 - IPAddress gDefaultGateway (192, 168, 1, 1);
 - IPAddress gDnsServer      (192, 168, 1, 1);

#### Mqtt specific settings
At last the MQTT settings needs to be configured. The client ID, MQTT broker server address, MQTT broker server port, topic and the credentials are configuration parameters. An example configuration is given below.
 - #define MQTT_CLIENT     "ESP8266_Switch_01"
 - #define MQTT_SERVER     "192.168.1.2"
 - #define MQTT_PORT       8883
 - #define MQTT_TOPIC      "/home/testtopic/01"
 - #define MQTT_USER       "mqtt_broker_user"
 - #define MQTT_PASS       "mqtt_broker_password"
 
#### Device selection
When flashing the device, make sure that the correct board is selected. For regular SonOff switches use the Generic ESP8266 module and for the SonOff Touch switches use the Generic ESP8285 module.
