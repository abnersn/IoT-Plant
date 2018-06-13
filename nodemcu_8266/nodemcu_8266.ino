#include <ESP8266WiFi.h>
#include <PubSubClient.h>

// Configurações do WiFi
const char *ssid = "OnePlus";
const char *password = "siwj28Hdo";
WiFiClient espClient;

// Configurações do servidor MQTT
const char* mqttServer = "m13.cloudmqtt.com";
const int mqttPort = 13015;
const char* mqttUser = "tgrisrwn";
const char* mqttPassword = "GCLhF8pDVQsd";
PubSubClient client(espClient);

#define NUMPINS 5
char toggle_outputs[] = "11110";
int toggle_pins[] = {16, 0, 2, 12, 13};

String serialRead;

// Função auxiliar para localizar comandos numa string
String getValue(String data, char separator, int index)
{
    int found = 0;
    int strIndex[] = { 0, -1 };
    int maxIndex = data.length() - 1;

    for (int i = 0; i <= maxIndex && found <= index; i++) {
        if (data.charAt(i) == separator || i == maxIndex) {
            found++;
            strIndex[0] = strIndex[1] + 1;
            strIndex[1] = (i == maxIndex) ? i+1 : i;
        }
    }
    return found > index ? data.substring(strIndex[0], strIndex[1]) : "";
}


// Callback do servidor MQTT
void callback(char* topic, byte* payload, unsigned int length) {
  // Composição da mensagem
  String msg;
  for (int i = 0; i < length; i++) {
    char c = (char)payload[i];
    msg += c;
  }
  // Serial.println(msg);
  String topicString = String(topic);
  String msgString = String(msg);

  // TÓPICO TOGGLE
  if (topicString.equals("TIMER")) {
    toggle_outputs[4] = '1';
    Serial.println(toggle_outputs);
    delay(5000);
    toggle_outputs[4] = '0';
    Serial.println(toggle_outputs);
  }
  if (topicString.equals("TOGGLE")) {
    client.publish("ACK", "Received toggle command");
    String pin = getValue(msgString, '_', 0);
    String action = getValue(msgString, '_', 1);
    // Configura o vetor de saída dos pinos conforme o comando recebido
    if (pin.equals("pin1")) {
      if (action.equals("toggle")) {
        if (toggle_outputs[0] == '1'){
          toggle_outputs[0] = '0';
        } else {
          toggle_outputs[0] = '1';
        }
      } if (action.equals("on")) {
        toggle_outputs[0] = '1';
      } if (action.equals("off")) {
        toggle_outputs[0] = '0';
      }
    } else if (pin.equals("pin2")) {
      if (action.equals("toggle")) {
        if (toggle_outputs[1] == '1'){
          toggle_outputs[1] = '0';
        } else {
          toggle_outputs[1] = '1';
        }
      } if (action.equals("on")) {
        toggle_outputs[1] = '1';
      } if (action.equals("off")) {
        toggle_outputs[1] = '0';
      }
    } else if (pin.equals("pin3")) {
      if (action.equals("toggle")) {
        if (toggle_outputs[2] == '1'){
          toggle_outputs[2] = '0';
        } else {
          toggle_outputs[2] = '1';
        }
      } if (action.equals("on")) {
        toggle_outputs[2] = '1';
      } if (action.equals("off")) {
        toggle_outputs[2] = '0';
      }
    } else if (pin.equals("pin4")) {
      if (action.equals("toggle")) {
        if (toggle_outputs[3] == '1'){
          toggle_outputs[3] = '0';
        } else {
          toggle_outputs[3] = '1';
        }
      } if (action.equals("on")) {
        toggle_outputs[3] = '1';
      } if (action.equals("off")) {
        toggle_outputs[3] = '0';
      }
    } else if (pin.equals("all")) {
      if (action.equals("toggle")) {
        for (int i = 0; i < NUMPINS; i++) {
          if (toggle_outputs[i] == '1'){
            toggle_outputs[i] = '0';
          } else {
            toggle_outputs[i] = '1';
          }        
        }
      } if (action.equals("on")) {
        for (int i = 0; i < NUMPINS; i++) {
          toggle_outputs[i] = '1';          
        }
      } if (action.equals("off")) {
        for (int i = 0; i < NUMPINS; i++) {
          toggle_outputs[i] = '0';          
        }
      }
      toggle_outputs[4] = '0';
    }

    // LEDs
    for (int i = 0; i < NUMPINS; i++) {
      if (toggle_outputs[i] == '0') {
        digitalWrite(toggle_pins[i], LOW); 
      } else {
        digitalWrite(toggle_pins[i], HIGH); 
      }
               
    }
    Serial.println(toggle_outputs);
  }
}

void setup() {
  Serial.begin(115200);
  
  WiFi.begin(ssid, password);
 
  // Define pinos toggle
  for (int i = 0; i < NUMPINS; i++) {
    pinMode(toggle_pins[i], OUTPUT);          
  }

  while(WiFi.status() != WL_CONNECTED){
    delay(1000);
  }

  client.setServer(mqttServer, mqttPort);
  client.setCallback(callback);

  while(!client.connected()){
    if (!client.connect("node_mcu_abner", mqttUser, mqttPassword )) {
      delay(2000);
    }
  }

  client.subscribe("TOGGLE");
  client.subscribe("TIMER");
  Serial.println(toggle_outputs);
}

void loop() {
  client.loop();

  if (Serial.available()) {
    while(Serial.available()) {
      serialRead += char(Serial.read());
    }
    char msg[40];
    serialRead.toCharArray(msg, 40);
    client.publish("SENSORS", msg);
    serialRead = "";
  }
}
