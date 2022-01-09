TaskHandle_t Task1;
TaskHandle_t Task2;

#include <WiFi.h>
#include <FirebaseESP32.h>
#include <Wire.h>
#include <SH1106.h>

#define FIREBASE_HOST "fir-ac4d1-default-rtdb.firebaseio.com"
#define FIREBASE_AUTH "4eC30gYjL3a47h9bl5QpeVDJfVdDO9bnCLUsstSL"
#define readPin 34
#define ledPin 12
int potValue = 0;

int state = 0;
int velo = 0;
int veloTemp = 0;
int counter = 0;
int counter1 = 0;
bool stt = false , stt1 = false;
static unsigned long time1 = 0 ;
unsigned long timeTemp1 = 0 ;
int time2 = 0;
int hour = 0;
int minute = 0;
unsigned long period;
String path = "test";
int arr[5] = { 0, 0, 0, 0, 0 };
FirebaseData firebaseData;
FirebaseData firebaseDataGet;
FirebaseJson json1;

const char *ssid = "LAB 307";
const char *password = "lab307307";

SH1106 display(0x3C, 21, 22);

void setup() {

  disableCore0WDT();
  disableCore1WDT();
  //create a task that will be executed in the Task1code() function, with priority 1 and executed on core 0
  xTaskCreatePinnedToCore(
    Task1code,   /* Task function. */
    "Task1",     /* name of task. */
    30000,       /* Stack size of task */
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
  Serial.begin(115200);
  pinMode(ledPin,OUTPUT);
  pinMode(5,OUTPUT);
  time1 = millis();
  state = analogRead(readPin);
  for (;;) {
    
     if (millis() - time1 > 5000 & counter != 0) {
    stt = false;
    if (stt1 == false) {
  
    }
    stt1 = true;
   
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
    stt1 = false;
  }
  
    if ((analogRead(readPin) - state) > 20) {
      digitalWrite(ledPin,HIGH);
      period = millis() - time1;
      velo = (int)averageArr((int)(60000 / (int)period), arr);

      counter++;
    time2 = (8500 - counter) / (velo);
    hour = time2 / 60;
    minute = time2 - hour * 60;
    time1 = millis();
    
//      digitalWrite(ledPin,LOW);
      

    if (counter % 5 == 0) {
      veloTemp = velo;
    }
    if (veloTemp > 200) {
      digitalWrite(5, HIGH);
    }
    else {
      digitalWrite(5, LOW);
    }
     vTaskDelay(100);
    digitalWrite(ledPin, LOW);
    
    }
    
  }
}

//Task2code: blinks an LED every 700 ms
void Task2code( void * pvParameters ) {
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.waitForConnectResult() != WL_CONNECTED) {
    WiFi.begin(ssid, password);
  }
  WiFi.begin(ssid, password);
    Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
  Firebase.reconnectWiFi(true);

  firebaseData.setResponseSize(1024);
  json1.clear().add(("count"), counter);
  Firebase.updateNode(firebaseData, path , json1);
  for(;;){
    Serial.println(counter);
    if(counter % 5 == 0){
      json1.add(("count"), counter);
      json1.add(("time"), time2);
      json1.add(("velo"),velo);
      Firebase.updateNode(firebaseData, path , json1);
      
    }
  }
  
}

void loop() {

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
