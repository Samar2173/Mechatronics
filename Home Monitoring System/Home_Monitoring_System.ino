  #include <WiFi.h>
#include <PubSubClient.h>
#include "DHT.h"

#define DHTPIN 4 
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

#define WIFISSID " ---- " // Put your WifiSSID here
#define PASSWORD " ---- " // Put your wifi password here
#define TOKEN " ---- " // Put your Ubidots' TOKEN
#define MQTT_CLIENT_NAME " ---- " // MQTT client Name, please enter your own 8-12 alphanumeric character ASCII string;
//it should be a random and unique ascii string and different from all other devices

/****************************************
* Define Constants
****************************************/
#define VARIABLE_LABEL_SUBSCRIBE "light" // Assing the variable label
#define VARIABLE_LABEL1 "temp" // Assing the variable label
#define VARIABLE_LABEL2 "humid" // Assing the variable label
#define DEVICE_LABEL "esp32_shop_automation" 

#define Light 22

char mqttBroker[]  = "industrial.api.ubidots.com";
char payload[1000];
char topic[150];
char topic1[150];
char topic2[150];
char topicSubscribe[100];

// Space to store values to send
char str_Temp[10];
char str_humidity[10];

/****************************************
* Auxiliar Functions
****************************************/
WiFiClient ubidots;
PubSubClient client(ubidots);

void callback(char* topic, byte* payload, unsigned int length) {
    char p[length + 1];
    memcpy(p, payload, length);
    p[length] = NULL;
    String message(p);
    Serial.println(message);
  
    if (message == "0.0") {
    digitalWrite(Light, HIGH);
    Serial.println("if");
    Serial.println("");
    } 
    else {
    digitalWrite(Light, LOW);
    Serial.println("else");
    Serial.println("");
    }
  
    Serial.write(payload, length);
    Serial.println();
}
void reconnect() {
// Loop until we're reconnected
    while (!client.connected()) {
        Serial.println("Attempting MQTT connection...");
        // Attemp to connect
            if (client.connect(MQTT_CLIENT_NAME, TOKEN, "")) {
            Serial.println("Connected");
            } else {
            Serial.print("Failed, rc=");
            Serial.print(client.state());
            Serial.println(" try again in 2 seconds");
            // Wait 2 seconds before retrying
            delay(2000);
        }
    }
}
/****************************************
* Main Functions
****************************************/
void setup() {
    Serial.begin(115200);
    dht.begin();
    WiFi.begin(WIFISSID, PASSWORD);
    // Assign the pin as OUTPUT
    pinMode(Light, OUTPUT);
    pinMode(23, OUTPUT);
    pinMode(5, OUTPUT);
    
    Serial.println();
    Serial.print("Wait for WiFi...");
    while (WiFi.status() != WL_CONNECTED) {
        Serial.print(".");
        digitalWrite(23, HIGH);
        delay(250);
        digitalWrite(23, LOW);
        delay(2500);
}
    Serial.println("");
    digitalWrite(23, LOW);
    Serial.println("WiFi Connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
    client.setServer(mqttBroker, 1883);
    client.setCallback(callback);

    sprintf(topicSubscribe, "/v1.6/devices/%s/%s/lv", DEVICE_LABEL, VARIABLE_LABEL_SUBSCRIBE);
    client.subscribe(topicSubscribe);
}
void loop() {
    if (!client.connected()) {
    reconnect();
    client.subscribe(topicSubscribe);
  }

    float h = dht.readHumidity();
    float t = dht.readTemperature();
    if (isnan(h) || isnan(t)) {
        Serial.println("Failed to read from DHT sensor!");
        return;
        }
    Serial.print("Humidity: ");
    Serial.print(h);
    Serial.print(" %\t");
    Serial.print("Temperature: ");
    Serial.print(t);
    Serial.println(" *C ");
    Serial.println();

    if(t > 24.50) {
      digitalWrite(5, HIGH);
      delay(250);
      digitalWrite(5, LOW);
    }
    
    dtostrf(t, 4, 2, str_Temp);
    dtostrf(h, 4, 2, str_humidity);

    sprintf(topic1, "%s%s", "/v1.6/devices/", DEVICE_LABEL);
    sprintf(payload, "%s", ""); 
    sprintf(payload, "{\"%s\":", VARIABLE_LABEL1); 
    sprintf(payload, "%s {\"value\": %s}}", payload, str_Temp); 
    //Serial.println("Publishing temperature to Ubidots Cloud");
    client.publish(topic1, payload);

    sprintf(topic2, "%s%s", "/v1.6/devices/", DEVICE_LABEL);
    sprintf(payload, "%s", ""); 
    sprintf(payload, "{\"%s\":", VARIABLE_LABEL2); 
    sprintf(payload, "%s {\"value\": %s}}", payload, str_humidity); 
    //Serial.println("Publishing humidity to Ubidots Cloud");
    client.publish(topic2, payload);

    client.loop();
    delay(1000);
}
