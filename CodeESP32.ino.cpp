#include <WiFi.h>
#include <PubSubClient.h>
#include <OneWire.h>
#include <DallasTemperature.h>


#define BUILTIN_LED 2
#define LED_PIN 18
#define LDR_PIN 36
#define ONE_WIRE_BUS 19
// Update these with values suitable for your network.

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

const char* ssid = "YOUR_SSID";
const char* password = "YOUR_PASSWORD";
const char* mqtt_server = "broker.shiftr.io";
hw_timer_t * timer = NULL;
uint8_t flag = 0;

WiFiClient espClient;
PubSubClient client(espClient);

void IRAM_ATTR onTimer(){
  flag = 1;
}

void setup_wifi() {

  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  randomSeed(micros());

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
  
  // Switch on the LED if an 1 was received as first character
  if ((char)payload[0] == '1') {
    digitalWrite(LED_PIN, HIGH);   // Turn the LED on (Note that LOW is the voltage level
  } 
  else {
    if((char)payload[0] == '0'){
          digitalWrite(LED_PIN, LOW);  // Turn the LED off by making the voltage HIGH
    }
    else{

    }
  }
    delay(50);
    int analog_LDR=0;
    analog_LDR = analogRead(LDR_PIN);
    Serial.print("LDR Value:  ");
    Serial.println(analog_LDR);
    if(analog_LDR < 3700){
      client.publish("inState","1");
    }
    else {
      client.publish("inState","0");
    }

}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP32";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str(),"mqtt_integration_app","71224b33c9cee434")) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.subscribe("outState");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void setup() {
  pinMode(BUILTIN_LED, OUTPUT);     // Initialize the BUILTIN_LED pin as an output
  pinMode(LED_PIN,OUTPUT);
  digitalWrite(LED_PIN,LOW);
  sensors.begin();
  timer = timerBegin(0, 80, true);
  timerAttachInterrupt(timer, &onTimer, true);
  timerAlarmWrite(timer, 10000000, true);
  timerAlarmEnable(timer);
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}

void loop() {

  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  if(flag==1){
  //Serial.print("Requesting temperatures...");
  sensors.requestTemperatures();
  //Serial.println(sensors.getTempCByIndex(0));
  client.publish("inMonitor",String(sensors.getTempCByIndex(0)).c_str());
  flag=0;
  }
}
