#include <SoftwareSerial.h>
#include <Regexp.h>
SoftwareSerial s(D7, D8);
#include "FirebaseESP8266.h"
#include <ESP8266WiFi.h>
#include <WiFiClient.h>

#include "FirebaseESP8266.h"
#define FIREBASE_HOST "arduinostt-default-rtdb.firebaseio.com/"
#define FIREBASE_AUTH "fojJvtabeTdOrQew2H5dMHRShux32aqlpoHhn2qG"

String path = "5";

FirebaseData firebaseData;
FirebaseData firebaseDataGet;

FirebaseJson json1;

const char* ssid = "LAB 307";
const char* password = "lab307307";

static String t;
static String velo;
static int tT;
static int veloT;
#include <Servo.h>

Servo myservo;  // khởi tạo đối tượng Servo với tên gọi là myservo
// bạn có thể tạo tối đa 8 đối tượng Servo

int angle = 145;   // initial angle  for servo
int angleStep = 1;  // biến pos dùng để lưu tọa độ các Servo

void setup() {
  // put your setup code here, to run once:
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.waitForConnectResult() != WL_CONNECTED) {
    WiFi.begin(ssid, password);
  }


  s.begin(9600);

  myservo.attach(12);  // attach (đính) servo ở chân digital D6
  // bạn có thể thay bằng bất cứ chân digital/analog nào khác
  pinMode(D3, INPUT_PULLUP);
  pinMode(D4, INPUT_PULLUP);

  Serial.begin(9600);
  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
  Firebase.reconnectWiFi(true);

  firebaseData.setBSSLBufferSize(1024, 1024);

  firebaseData.setResponseSize(1024);
  json1.clear().add(("angle"), angle);
  json1.add(("velo"), velo);
  Firebase.updateNode(firebaseData, path , json1);
}
String data;

void loop() {
  // put your main code here, to run repeatedly:
  if (s.available() > 0)
  {
    data = s.readString();

    MatchState ms;
    MatchState ms1;
    // what we are searching (the target)
    char buf [100];
    data.toCharArray(buf, 100 );
    ms.Target (buf);  // set its address
    ms1.Target(buf);
    char result = ms.Match ("[v]");
    char result1 = ms1.Match ("[t]");
    velo = data.substring(ms.MatchStart + 1, ms1.MatchStart);
    t = data.substring(ms1.MatchStart+1);
    
    Serial.println(t);
    if (velo != "") {
      json1.clear().add(("stt"), true);
      json1.add(("time"), t);
      json1.add(("velo"), velo);
      Firebase.updateNode(firebaseData, path , json1);
    }

    Firebase.updateNode(firebaseData, path , json1);
    if (data == "f") {
      json1.add(("stt"), false);
      Firebase.updateNode(firebaseData, path , json1);
    }
  }
  if (Firebase.getInt(firebaseDataGet, path + "/angle")) {
    angle = firebaseDataGet.intData();
    myservo.write(angle);
  }
  while (digitalRead(D3) == LOW) {
    if (angle >= 30 && angle <= 145) {
      angle = angle - angleStep;
      if (angle < 30) {
        angle = 30;
      } else {
        myservo.write(angle);
        json1.add(("angle"), angle);
        Firebase.updateNode(firebaseData, path , json1);
      }
    }
    delay(50);
  }

  while (digitalRead(D4) == LOW) {
    if (angle >= 30 && angle <= 145) {
      angle = angle + angleStep;
      if (angle > 145) {
        angle = 145;
      } else {
        myservo.write(angle);
        json1.add(("angle"), angle);
        Firebase.updateNode(firebaseData, path , json1);
      }
    }
    delay(50);
  }
}
