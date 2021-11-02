TaskHandle_t Task1;
TaskHandle_t Task2;


#include "FirebaseESP32.h"
#include <WiFi.h>
#include <WiFiClient.h>

#include <Wire.h>
#include <SH1106.h>
#include <SoftwareSerial.h>
SoftwareSerial s(16, 13);

int process = 0;
int counter = 0;
static int counter1 = 0;
int state;
int stateVelo = 0;
int arr[5] = { 0, 0, 0, 0, 0 };
static int velocity = 0;
static int veloTemp = 0;
bool stt = false , stt1 = false;
static unsigned long time1 = 0 ;
unsigned long timeTemp1 = 0 ;
int time2 = 0;
int hour = 0;
int minute = 0;
unsigned long period;

SH1106 display(0x3C, 33, 36);
#define D5 14
#define D6 12



#define FIREBASE_HOST "fir-ac4d1-default-rtdb.firebaseio.com"
#define FIREBASE_AUTH "4eC30gYjL3a47h9bl5QpeVDJfVdDO9bnCLUsstSL"

String path = "1";
FirebaseData firebaseData;
FirebaseData firebaseDataGet;
FirebaseJson json1;

const char *ssid = "LAB 307";
const char *password = "lab307307";


static String t;
static String velo;

void setup() {
  Serial.begin(115200);
  disableCore0WDT();
  disableCore1WDT();
  //create a task that will be executed in the Task1code() function, with priority 1 and executed on core 0
  xTaskCreatePinnedToCore(
    Task1code,   /* Task function. */
    "Task1",     /* name of task. */
    20000,       /* Stack size of task */
    NULL,        /* parameter of the task */
    1,           /* priority of the task */
    &Task1,      /* Task handle to keep track of created task */
    0);          /* pin task to core 0 */

  //create a task that will be executed in the Task2code() function, with priority 1 and executed on core 1
  xTaskCreatePinnedToCore(
    Task2code,   /* Task function. */
    "Task2",     /* name of task. */
    10000,       /* Stack size of task */
    NULL,        /* parameter of the task */
    1,           /* priority of the task */
    &Task2,      /* Task handle to keep track of created task */
    1);          /* pin task to core 1 */

}











//Task1code: blinks an LED every 1000 ms
void graphics(int velo, String time2) {
  display.clear();
  display.flipScreenVertically();
  display.drawLine(0, 30, 128, 30);
  display.setFont(ArialMT_Plain_16);
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.drawString(0, 5, "V:");
  display.setFont(ArialMT_Plain_16);
  display.setTextAlignment(TEXT_ALIGN_RIGHT);
  display.drawString(110, 5, String(velo) + " g/p");

  display.setFont(ArialMT_Plain_16);
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.drawString(0, 40, "T:");

  display.setTextAlignment(TEXT_ALIGN_RIGHT);
  display.setFont(ArialMT_Plain_16);
  display.drawString(110, 40, time2);
  
  display.display();
}
int averageArr(int a, int* arr) {
  for (int i = 4; i > 0; i--) {
    arr[i] = arr[i - 1];
  }
  arr[0] = a;
  int sum = 0;
  for (int j = 0; j < 5; j++) {
    sum += arr[j];
  }
  return (int)sum / (int)5;
}
void drawProgressBarDemo() {
  display.clear();
  process = (counter / 5) % 101;
  // draw the progress bar
  display.drawProgressBar(0, 32, 120, 10, process);

  // draw the percentage as String
  display.setTextAlignment(TEXT_ALIGN_CENTER);
  display.drawString(64, 15, String(process) + "%");
  display.display();
  counter++;
  delay(5);
}

void Task1code( void * pvParameters ) {
  Serial.print("Task1 running on core ");
  Serial.println(xPortGetCoreID());
   // khai bao chan tin hieu, va dat bien so sanh
  pinMode(A0, INPUT);
  pinMode(D5, OUTPUT);
  pinMode(D6, OUTPUT);
  state = analogRead(A0);
  Serial.begin(115200);

  // khai bao man hinh
  display.init();
  display.flipScreenVertically();
  display.setFont(ArialMT_Plain_16);
  display.setTextAlignment(TEXT_ALIGN_CENTER);
  display.drawString(64, 20, "CONNECTED...");

  display.display();

  delay(1000);
  display.clear();
  
  //   anation, hieu ung
    while (1) {
      drawProgressBarDemo();
      if (process == 100) {
        break;
      }
    }
  display.clear();
  
  display.flipScreenVertically();
  display.drawString(64, 20, "START");
  display.display();
  display.clear();
  s.begin(9600);

  for (;;) {
      if (millis() - time1 > 5000 & counter1 != 0) {

    stt = false;
    if (stt1 == false) {
      s.print('f');
    }
    stt1 = true;
    display.clear();
    display.setTextAlignment(TEXT_ALIGN_CENTER);
    display.drawString(64, 15, "Warning");
    display.display();
    if ( (unsigned long)(millis() - timeTemp1) > 300 )
    {
        if ( digitalRead(D6) == LOW )
        {
            digitalWrite(D6, HIGH);
        } else {
            digitalWrite(D6, LOW );
        }
        timeTemp1 = millis();
    }
  }
  else {
    digitalWrite(D6, LOW);
    stt = true;
    stt1 = false;
  }
  
  if ((analogRead(A0) - state) > 20) {
    digitalWrite(D5, HIGH);
    period = millis() - time1;
    velocity = (int)averageArr((int)(60000 / (int)period), arr);
    counter1++;

    time2 = (8500 - counter1) / (velocity);
    hour = time2 / 60;
    minute = time2 - hour * 60;

    graphics(veloTemp, String(hour) + " h " + String(minute) + "p");
    time1 = millis();
    if (counter1 % 5 == 0) {
      veloTemp = velocity;
      s.print("v" + String(veloTemp) + "t" + String(time2));
    }
    if (veloTemp > 200) {
      digitalWrite(D6, HIGH);
    }
    else {
      digitalWrite(D6, LOW);
    }
    delay(100);
    digitalWrite(D5, LOW);
  }
}

}



















//Task2code: blinks an LED every 700 ms
void Task2code( void * pvParameters ) {
  Serial.print("Task2 running on core ");
  Serial.println(xPortGetCoreID());
  WiFi.begin(ssid, password);
  //create a task that will be executed in the Task2code() function, with priority 1 and executed on core 1

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.println("Connecting to WiFi..");
  }
  Serial.println(WiFi.localIP());
  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
  Firebase.reconnectWiFi(true);
  json1.clear().add(("t"), t);
  json1.add(("velo"), velo);
  Firebase.updateNode(firebaseData, path , json1);

  String data;

  for (;;) {
    
    if (velo != "") {
      json1.clear().add(("stt"), true);
      json1.add(("t"), t);
      json1.add(("velo"), velo);
      Firebase.updateNode(firebaseData, path , json1);
    }

    Firebase.updateNode(firebaseData, path , json1);
    if (data == "f") {
      json1.add(("stt"), false);
      Firebase.updateNode(firebaseData, path , json1);
    }
  }


  
}

void loop() {

}
