#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <Ticker.h>

extern "C" { 
  #include "user_interface.h" 
}

/*------------------------------ Configurable parameters -------------------------------*/
// Hardware specific
#define BUTTON          0                                    // (Don't Change for Sonoff)
#define RELAY           12                                   // (Don't Change for Sonoff)
#define LED             13                                   // (Don't Change for Sonoff)

// Wifi specific settings
#define WIFI_SSID       "##YOUR_SSID##"
#define WIFI_PASS       "##YOUR_WIFI_PASS"

// IP specific settings, comment this if you would like to use DHCP.
IPAddress gIpAddress      (192, 168, 1, 231);
IPAddress gSubnetMask     (255, 255, 255, 0);
IPAddress gDefaultGateway (192, 168, 1, 1);
IPAddress gDnsServer      (192, 168, 1, 1);

// Mqtt specific settings
#define MQTT_CLIENT     "##YOUR_MQTT_CLIENT_ID##"
#define MQTT_SERVER     "##YOUR_MQTT_BROKER_IP##"
#define MQTT_PORT       ##YOUR_MQTT_BROKER_PORT##
#define MQTT_TOPIC      "##YOUR_MQTT_TOPIC_PER_SWITCH##"
#define MQTT_USER       "##YOUR_MQTT_USER##"
#define MQTT_PASS       "##YOUR_MQTT_PASSWORD##"

/*--------------------------------------------------------------------------------------*/

/*---------------------------- Software version definition -----------------------------*/
#define VERSION    "\n\r Version: v3.0 (Citron)"
/*--------------------------------------------------------------------------------------*/

/*-------------------------------- Global definitions ----------------------------------*/
bool gCurrentStateChanged           = false;
bool gRequestRestart                = false;

int gcConnectionCheckerFrequency    = 1;
int gcWifiConnectionRetries         = 10;
int gcMqttConnectionRetries         = 10;

unsigned long gPreviousMillis       = 0;
unsigned long gButtonCount          = 0;

WiFiClient gWifiClient;
PubSubClient gMqttClient(gWifiClient, MQTT_SERVER, MQTT_PORT);
Ticker gButtonTimer;
/*--------------------------------------------------------------------------------------*/

void InitialiseHardware();
bool InitialiseNetwork();
bool InitialiseMqttConnection();
void PrintBoardInformation();
void OnMqttDataAvailable(const MQTT::Publish& aPublishedData);
void OnButtonTick();
void PublishCurrentState();
void ProcessLed(int aPin, int aDuration, int aAmountOfFlashes);
void ProcessSuccesfullInitialization();
void ProcessRebootRequest();
void CheckPeriodicallyForActiveConnection();
void CheckConnection();


void setup() {
    InitialiseHardware();
    PrintBoardInformation();

    if (InitialiseNetwork())
    {
        if (InitialiseMqttConnection())
        {
            ProcessSuccesfullInitialization();
        }
    }
}

void loop() {
    CheckPeriodicallyForActiveConnection();
    if (gRequestRestart)
    {
        ProcessRebootRequest();
    }
  
    if (gMqttClient.loop())
    {
        if (gCurrentStateChanged)
        {
            PublishCurrentState();
        }    
    }
}

void InitialiseHardware() {
    pinMode(LED, OUTPUT);
    pinMode(RELAY, OUTPUT);
    pinMode(BUTTON, INPUT);

    digitalWrite(LED, HIGH);
    digitalWrite(RELAY, LOW);

    /* Initialise button ticker, to prevent jitter. */
    gButtonTimer.attach(0.05, OnButtonTick);

    Serial.begin(115200);
}

bool InitialiseNetwork() {
    WiFi.mode(WIFI_STA);
    WiFi.config(gIpAddress, gDnsServer, gDefaultGateway, gSubnetMask);
    WiFi.begin(WIFI_SSID, WIFI_PASS);
    Serial.print("Connecting to: ");
    Serial.print(WIFI_SSID);
    while ((WiFi.status() != WL_CONNECTED) && gcWifiConnectionRetries--)
    {
        delay(500);
        Serial.print(" .");
    }
    Serial.println();

    if (WiFi.status() != WL_CONNECTED)
    {
        Serial.print("Unable to connect to Wifi, error code: ");
        Serial.println(WiFi.status());
        Serial.println("\n----------------------------------------------------------------\n");
        return false;
    }

    Serial.println("Device connected to Wifi.");
    Serial.print("IP Address is: ");
    Serial.println(WiFi.localIP());

    return true;
}

bool InitialiseMqttConnection() {
    Serial.print("Connecting to Mqtt broker: ");
    Serial.print(MQTT_SERVER);
    delay(500);
    while (!gMqttClient.connect(MQTT::Connect(MQTT_CLIENT).set_keepalive(90).set_auth(MQTT_USER, MQTT_PASS)) && gcMqttConnectionRetries--) 
    {
        Serial.print(" .");
        delay(1000);
    }
    Serial.println();

    if (!gMqttClient.connected())
    {
        Serial.println("Unable to connect to Mqtt broker.");
        Serial.println("\n----------------------------------------------------------------\n");
        return false;
    }

    Serial.println("Device connected to Mqtt broker.");
    gMqttClient.set_callback(OnMqttDataAvailable);
    gMqttClient.subscribe(MQTT_TOPIC);

    return true;
}

void PrintBoardInformation() {
    Serial.println(VERSION);
    Serial.print("ESP ChipId: ");
    Serial.print(ESP.getChipId(), HEX);
    Serial.println();
}

void OnMqttDataAvailable(const MQTT::Publish& aPublishedData) {
    if (aPublishedData.payload_string() == "stat") 
    {
        // Currently we do not use the stat payload.
    }
    else if (aPublishedData.payload_string() == "on") 
    {
        digitalWrite(LED, LOW);
        digitalWrite(RELAY, HIGH);
    }
    else if (aPublishedData.payload_string() == "off") 
    {
        digitalWrite(LED, HIGH);
        digitalWrite(RELAY, LOW);
    }
    else if (aPublishedData.payload_string() == "reset") 
    {
        gRequestRestart = true;
    }

    gCurrentStateChanged = true;
}

void OnButtonTick() {
    if (!digitalRead(BUTTON)) 
    {
        gButtonCount++;
        return;
    }

    if (gButtonCount > 1 && gButtonCount <= 40) 
    {   
        digitalWrite(LED, !digitalRead(LED));
        digitalWrite(RELAY, !digitalRead(RELAY));
        gCurrentStateChanged = true;
    } 
    else if (gButtonCount > 40)
    {
        Serial.println("Rebooting, please wait."); 
        gRequestRestart = true;
    }

    gButtonCount = 0;
}

void PublishCurrentState() {
    if(digitalRead(LED) == LOW)
    {
        gMqttClient.publish(MQTT::Publish(MQTT_TOPIC"/stat", "on").set_retain().set_qos(1));
        Serial.println("Relay is set to on.");
    } 
    else 
    {
        gMqttClient.publish(MQTT::Publish(MQTT_TOPIC"/stat", "off").set_retain().set_qos(1));
        Serial.println("Relay is set to off.");
    }
    gCurrentStateChanged = false;
}

void ProcessLed(int aPin, int aDuration, int aAmountOfFlashes) {             
    for(int i = 0; i < aAmountOfFlashes; i++)
    {  
        digitalWrite(aPin, HIGH);        
        delay(aDuration);
        digitalWrite(aPin, LOW);
        delay(aDuration);
    }
}

void ProcessSuccesfullInitialization() {
    Serial.println("Initialization complete.");
    Serial.println("\n****************************  Activity log  ****************************\n");
    ProcessLed(LED, 40, 8);
    digitalWrite(LED, HIGH);
}

void ProcessRebootRequest() {
    ProcessLed(LED, 400, 4);
    Serial.println("Device will reboot now!");
    ESP.restart();
}

void CheckPeriodicallyForActiveConnection() {
    unsigned long lCurrentMillis = millis();

    if ((lCurrentMillis - gPreviousMillis) >= (gcConnectionCheckerFrequency * 30000)) 
    {
        gPreviousMillis = lCurrentMillis;
        CheckConnection();
    }
}

void CheckConnection() {
    if (WiFi.status() == WL_CONNECTED)
    {
        if (gMqttClient.connected()) 
        {
            Serial.println("Mqtt connection to broker is active.");
        } 
        else 
        {
            Serial.println("Mqtt connection is lost, requesting restart.");
            gRequestRestart = true;
        }
        return;
    }

    Serial.println("WiFi connection is lost, requesting restart.");
    gRequestRestart = true;
}
