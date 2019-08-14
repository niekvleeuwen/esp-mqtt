/*
  ESP8266 MQTT
  Niek van Leeuwen 0967267

  Ik maak gebruik van de PubSub client
  https://pubsubclient.knolleary.net/api.html
*/
#include <ESP8266WiFi.h>
#include <PubSubClient.h>

const char* ssid = "";
const char* WifiPassword = ""; 
const char* mqtt_server = ""; 
const char* topic_esp1="TI/0971051"; //in dit kanaal publiceert de ESP
const char* topic_esp2="TI/0967267"; //naar dit kanaal luistert de ESP
const char* MQTTusername = ""; 
const char* MQTTpassword = ""; 
const String clientId = "0967267-esp"; //de naam waarmee de ESP zich bij de broker identificeert
const byte led = D2; //pin waar je je led op aansluit
const byte knop = D1; //pin waar je de knop op aansluit
long lastMsg = 0;
char msg[50];

WiFiClient espClient; //initialiseer een WIFI client
PubSubClient client(espClient); //initialiseer een PubSubclient met als parameter de aangemaakte WifiClient

//deze functie verzorgt de wifi verbinding
void setup_wifi() {
  delay(10);

  Serial.println();
  Serial.print("Verbinden met ");
  Serial.println(ssid);
  WiFi.begin(ssid, WifiPassword);

  //wachten totdat de ESP verbonden is
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.println("Bezig met verbinden....");
  }
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Bericht aangekomen: ");
  Serial.println(topic);
  
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  
  Serial.println();

  //zet de LED aan/uit op basis van het ontvangen karakter
  if ((char)payload[0] == '0') {
    digitalWrite(led, LOW);  
  } else if ((char)payload[0] == '1') {
    digitalWrite(led, HIGH); 
  }
}

void reconnect() {
  //loop totdat de ESP verbonden is
  while (!client.connected()) {
    Serial.print("Proberen te verbinden met de MQTT server...");
    //Proberen te vervinden met het gegeven client ID
    if (client.connect(clientId.c_str(), MQTTusername, MQTTpassword)) {
      Serial.println("Verbonden");
      //verstuur een welkomsbericht in beide kanalen
      client.publish(topic_esp1, "Ik ben ESP 1");
      client.publish(topic_esp2, "Ik ben ESP 1");
      //de ESP abonneert op de het topic van de andere ESP
      client.subscribe(topic_esp2);
    } else {
      Serial.print("Mislukt, rc=");
      Serial.print(client.state());
      Serial.println(" opnieuw proberen over 2 seconden");
      //twee seconden wachten op een nieuwe poging
      delay(2000);
    }
  }
}

void setup() {
  pinMode(led, OUTPUT); //initialiseer de led als output
  pinMode(knop, INPUT); //initialiseer de knop als input
  Serial.begin(115200); 
  setup_wifi();
  client.setServer(mqtt_server, 1883); //gebruikt poort 1883 (zelfde port als de MQTT broker)
  client.setCallback(callback); //deze functie wordt aangeroepen als er een bericht binnenkomt
}

void loop() {
  //checken of de ESP verbonden is met de MQTT server
  if (!client.connected()) {
    reconnect(); //als de ESP niet meer verbonden is met de server proberen opnieuw te verbinden
  }
  client.loop(); //loop moet aangroepen worden om de ESP de binnenkomende berichten te laten verwerken en de verbinding met de server te behouden

  long now = millis();
  //lees om de halve seconde de staat van de knop
  if (now - lastMsg > 500) {
    lastMsg = now;
    if(digitalRead(knop) == HIGH){
      client.publish(topic_esp1, "1"); //verstuur een 1 als de knop is ingedrukt
    }else{
      client.publish(topic_esp1, "0"); //verstuur een 0 als de knop niet is ingedrukt
    }
  }
}
