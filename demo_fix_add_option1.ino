TaskHandle_t Task1;
TaskHandle_t Task2;

#include <WiFi.h>
#include <FirebaseESP32.h>
#include <Wire.h>
#include <SH1106.h>

#define FIREBASE_HOST "arduinostt-default-rtdb.firebaseio.com"
#define FIREBASE_AUTH "fojJvtabeTdOrQew2H5dMHRShux32aqlpoHhn2qG"
#define readPin 34
#define ledPin 12

uint16_t velo = 0;
uint16_t calibVelo = 0;
int counter = 0;
uint16_t time2 = 0;
bool stt = false;
String path = "1";

SH1106 display(0x3C, 21, 22);
void setup() {

  disableCore0WDT();
  disableCore1WDT();
  pinMode(ledPin, OUTPUT);
  pinMode(5, OUTPUT);
  //create a task that will be executed in the Task1code() function, with priority 1 and executed on core 0
  xTaskCreatePinnedToCore(
    Task1code,   /* Task function. */
    "Task1",     /* name of task. */
    40000,       /* Stack size of task */
    NULL,        /* parameter of the task */
    1,           /* priority of the task */
    &Task1,      /* Task handle to keep track of created task */
    0);          /* pin task to core 0 */

  //create a task that will be executed in the Task2code() function, with priority 1 and executed on core 1
  xTaskCreatePinnedToCore(
    Task2code,   /* Task function. */
    "Task2",     /* name of task. */
    30000,       /* Stack size of task */
    NULL,        /* parameter of the task */
    1,           /* priority of the task */
    &Task2,      /* Task handle to keep track of created task */
    1);          /* pin task to core 1 */

}

//Task1code: blinks an LED every 1000 ms
void Task1code( void * pvParameters ) {

  static unsigned long time1 = 0 ;
  unsigned long timeTemp1 = 0 ;
  uint16_t veloTemp = 0;
  uint16_t state = 0;
  uint8_t hour = 0;
  uint8_t minute = 0;
  unsigned long period;
  uint16_t arr[5] = { 0, 0, 0, 0, 0 };
  vTaskDelay(100);
  // khai bao man hinh
  display.init();
  display.flipScreenVertically();
  display.setFont(ArialMT_Plain_16);
  display.setTextAlignment(TEXT_ALIGN_CENTER);
  display.drawString(64, 20, "CONNECTED...");
  display.display();
  vTaskDelay(1000);

  time1 = millis();
  state = analogRead(readPin);
  for (;;) {
    if ((analogRead(readPin) - state) > 80) {
      digitalWrite(ledPin, HIGH);
      period = millis() - time1;
      velo = (uint16_t)averageArr((uint16_t)(60000 / (uint16_t)period), arr);

      counter++;
      time2 = (8500 - counter) / (velo);
      hour = time2 / 60;
      minute = time2 - hour * 60;
      graphics(veloTemp, String(hour) + " h " + String(minute) + "p");

      time1 = millis();

      if (counter % 5 == 0) {
        veloTemp = velo;
      }
//      if (veloTemp > 300) {
//        digitalWrite(5, HIGH);
//      }
//      else {
//        digitalWrite(5, LOW);
//      }
        if(velo < calibVelo -20 || velo > calibVelo + 20) {
          digitalWrite(5,HIGH);
        }
        else{
          digitalWrite(5,LOW);
        }
      vTaskDelay(120);
      digitalWrite(ledPin, LOW);
      if (((analogRead(readPin) > (200 + state)) || analogRead(readPin) < (state - 200)) && counter % 5 ==0) {
        state = analogRead(readPin);
      }
    }
    if (analogRead(readPin) <  state - 500) {
      state = analogRead(readPin);
    }

    if (millis() - time1 > 5000 && counter != 0 ) {
      stt = false;
      display.setTextAlignment(TEXT_ALIGN_CENTER);
      display.drawString(64, 0, "Warning");
      display.display();

      if ( (unsigned long)(millis() - timeTemp1) > 300 )
      {
        if ( digitalRead(5) == LOW )
        {
          digitalWrite(5, HIGH);
        } else {
          digitalWrite(5, LOW );
        }
        timeTemp1 = millis();
      }
    }
    else {
      digitalWrite(5, LOW);
      stt = true;
      display.clear();
    }
  }
  
}


void Task2code( void * pvParameters ) {
  const char *ssid = "LAB 307";
  const char *password = "lab307307";
  FirebaseData firebaseData;
  FirebaseData firebaseDataGet;
  FirebaseJson json1;
  uint16_t time3 = 0;
  WiFi.mode(WIFI_STA);

  while (WiFi.waitForConnectResult() != WL_CONNECTED) {
    WiFi.begin(ssid, password);
  }

  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
  Firebase.reconnectWiFi(true);

  firebaseData.setResponseSize(1024);
  json1.clear().add(("count"), counter);
  Firebase.updateNode(firebaseData, path , json1);
  for(;;){
  if (millis() - time3 > 5000) {
    json1.add(("count"), counter);
    json1.add(("time"), time2);
    json1.add(("velo"), velo);
    Firebase.updateNode(firebaseData, path , json1);
    json1.clear();
    Firebase.getInt(firebaseDataGet, path + "/calibVelo");
    calibVelo = firebaseDataGet.intData();
    time3 = millis();
  }
  }
}
void loop () {

}

void graphics(uint16_t velo, String time2) {
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

uint16_t averageArr(uint16_t a, uint16_t* arr) {
  for (byte i = 4; i > 0; i--) {
    arr[i] = arr[i - 1];
  }
  arr[0] = a;
  uint16_t sum = 0;
  for (byte j = 0; j < 5; j++) {
    sum += arr[j];
  }
  return (uint16_t)sum / (uint16_t)5;
}
