/////////////////////////////////////////////////////////////////////////////
// Programmer  : Michael McGrath
// Class       : CMPEN 482W
//
// Discription : AbDisc code to swtich between crunch sessions, step
//               counting, and idle modes. Connects to the app over
//               BLE.
//
// Notes       : The companion app was made with the MIT App inventor 2 web
//               assistant. The best way to view the project is through that.
//               The BLE code is based off of Adafruit Examples, you can
//               there repo here: https://learn.adafruit.com/bluefruit-nrf52-feather-learning-guide/introduction
//
// Log         : Need to initalize time on arduino (through phone?)
//               Need proper accelerometer code (phone approach did not work)
//               Need better error checking with packet transfers (sometimes causes crashes)
//               
//////////////////////////////////////////////////////////////////////////////

// Project Includes
#include <bluefruit.h>

// Constants
////////////////////////////////////////////////////////////////////////////
const int VIB_PIN = 5; //pin numbers are for the Adafruit nRF52
const int SENSOR_PIN = 7; 
const int ACC_PIN = 9;
const int BAUD = 115200; // Set constant for baud rate

// Variables
////////////////////////////////////////////////////////////////////////////
uint16_t buff[3] = {0,0,0}; // data buffer for BLE writes
                            // buff[0] - holds the date
                            // buff[1] - holds the number of steps
                            // buff[2] - holds the number of crunch sessions
                            // both buff[1] and buff[2] are since last BLE
                            // write
int sensorAvg = 0; // variable for average sensor value
int sensorValue; // variable for immediate sensor value
int i; // index variable for loops
int f5_flag = 0; // flag from app for data write
BLEUart bleuart;

// Initialization Code Section
////////////////////////////////////////////////////////////////////////////
void setup() {
  // Enable serial debug.
  Serial.begin(BAUD); // Set BaudRate
  Serial.println("Begin Connection"); // print message for console debugging

  // Enable output.
  pinMode(VIB_PIN, OUTPUT);

  // Initialize BLE library.
  Bluefruit.begin();
  Bluefruit.setName("AbDisc");

  bleuart.begin();
  setupAdv();
  Bluefruit.Advertising.start();
  Serial.println("ble_begin done!"); // debugging print statement

  //set the initial threshold
  readSensor();
  threshold = sensorAvg;
}

// Main Loop Code Section
/////////////////////////////////////////////////////////////////////////////
void loop() {
  
  // If there is input from the app...
  while(ble_available()){
    f5_flag = ble_read();
    if(f5_flag != 0){ // and it states a refresh
      ble_write_bytes((byte*)&buff, 48); // send data
    }
  }
  // Process BLE events and reset counters
  ble_do_events();
  buff[1] = 0;
  buff[2] = 0;

mainLoop:
  readSensor();
  if(sensorAvg < 800)
    goto steps;
  else{
    buff[3] = buff[3] + 1;
    
crunchLoop:
    readSensor();
    if(sensorAvg > threshold){
      vibeTimes(1);
      vibes = vibes + 1;
      noVibes = 0;
    }
    else{
      noVibes = noVibes + 1;
      vibes = 0;
    }

    if (vibes > 2){
      vibes = 0;
      threshold = sensorAvg;
    }

    if (noVibes > 2){
      noVibes = 0;
      threshold = sensorAvg;
    }
    goto crunchLoop;
  }

steps:
  if(analogRead(ACC_PIN) < 50)
    goto idle;
  else if(analogRead(ACC_PIN > 150)
    buff[1] = buff[1] + 1;
  else
    goto steps:

/*
  if (packetbuffer[1] == 'A') {
    float x, y, z;
    x = parsefloat(packetbuffer+2);
    y = parsefloat(packetbuffer+6);
    z = parsefloat(packetbuffer+10);
    Serial.print("Accel\t");
    Serial.print(x); Serial.print('\t');
    Serial.print(y); Serial.print('\t');
    Serial.print(z); Serial.println();
  }
*/

idle:
  delay(10000);
  goto mainLoop;
  
}

// Functions Code Section
///////////////////////////////////////////////////////////////////////////////
void vibeTimes(int j) {
  // viberates j number of times
  for  (i = 0; i < j; i++) {
    digitalWrite (VIB_PIN, HIGH);
    delay (350);
    digitalWrite(VIB_PIN, LOW);
    delay (100);
  }
  return;
}

void readSensor() {
  //sets sensorAvg global variable
  sensorValue = 0;
  for (i = 0; i < 50; i++) {
    sensorValue = sensorValue + analogRead(SENSOR_PIN);
  delay(25);
  }
  sensorAvg = sensorValue / 50;
  return;
}

void setupAdv(void){
  Bluefruit.Advertising.addFlags(BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE);
  Bluefruit.Advertising.addTxPower();
  Bluefruit.Advertising.addService(bleuart);
  Bluefruit.ScanResponse.addName();
}

