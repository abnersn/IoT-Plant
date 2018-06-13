#include "DHT.h"

#define SENSOR_LUZ A0
#define SENSOR_DHT A1
#define SENSOR_SOLO A2

#define BOMBA 7

DHT dht(SENSOR_DHT, DHT11);

char sensorsStatus[] = "11110";
String sensorsValues;

void serialFlush(){
  while(Serial.available() > 0) {
    char t = Serial.read();
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(BOMBA, OUTPUT);
}

void loop() {
  if (sensorsStatus[4] == '1') {
    digitalWrite(BOMBA, HIGH);
    delay(5000);
    digitalWrite(BOMBA, LOW);
  }
  sensorsValues = "";
  if (sensorsStatus[0] == '1') {
    float dht_u = dht.readHumidity();
    if (!isnan(dht_u)) {
      sensorsValues += String(dht_u);  
    } else {
      sensorsValues += "nan";
    }
  } else {
    sensorsValues += "off";
  }
  
  sensorsValues += "_";
  
  if (sensorsStatus[1] == '1') {
    float dht_t = dht.readTemperature();
    if (!isnan(dht_t)) {
      sensorsValues += String(dht_t);  
    } else {
      sensorsValues += "nan";
    }
  } else {
    sensorsValues += "off";
  }

  sensorsValues += "_";
  if (sensorsStatus[2] == '1') {
    int luz = analogRead(SENSOR_LUZ);
    sensorsValues += String(luz);
  } else {
    sensorsValues += "off";
  }
  
  sensorsValues += "_";
  if (sensorsStatus[3] == '1') {
    int solo = analogRead(SENSOR_SOLO);
    sensorsValues += String(solo);
  } else {
    sensorsValues += "off";
  }

  Serial.println(sensorsValues);
  if (Serial.available()) {
    // Lê 5 caracteres da entrada serial,
    // mas só atualiza se o caractere for '0' ou '1'.
    for (int i = 0; i < 5; i++) {
      char c = char(Serial.read());
      if (c == '1' || c == '0') {
        sensorsStatus[i] = c;
      } else {
        sensorsStatus[i] = '1';
      }
    }
    // Limpa o buffer do serial
    serialFlush();
  }
  delay(1000);
}
