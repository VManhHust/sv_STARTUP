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
int potValue = 0;

int state = 0;
int velo = 0;
int veloTemp = 0;
//bool a = true;
int calibVelo = 0;
int process = 0;
int counter = 0;
int counter1 = 0;
bool stt = false , stt1 = false;
static unsigned long time1 = 0 ;
unsigned long timeTemp1 = 0 ;
int time2 = 0;
int hour = 0;
int minute = 0;
unsigned long period;
String path = "1";
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

   // khai bao man hinh
  display.init();
  display.flipScreenVertically();
  display.setFont(ArialMT_Plain_16);
  display.setTextAlignment(TEXT_ALIGN_CENTER);
  display.drawString(64, 20, "CONNECTED...");

  display.display();
  vTaskDelay(1000);
  display.clear();

 
  
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
    
    display.clear();
    display.setTextAlignment(TEXT_ALIGN_CENTER);
    display.drawString(64, 15, "Warning");
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

    graphics(veloTemp, String(hour) + " h " + String(minute) + "p");
    
    time1 = millis();
    

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
    if(counter % 5 == 0){
      json1.add(("count"), counter);
      json1.add(("time"), time2);
      json1.add(("velo"),velo);
      Firebase.updateNode(firebaseData, path , json1);
      
    }
      Firebase.getInt(firebaseDataGet, path + "/calibVelo");
      calibVelo = firebaseDataGet.intData();

    {
      Serial.println(calibVelo);
      delay(1000);
      }
      if(velo > calibVelo + 30 || velo < calibVelo - 30) {
        digitalWrite(5,HIGH);
      }else {
        digitalWrite(5,LOW);
      }
        
      
      
  }
  
}

void loop() {

}

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
