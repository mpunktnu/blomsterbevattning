/*
 * Watering system for flowers
 * 
 * Created by Erik Karlsson @ m.nu
 * 
 * The script supports up to 4 flowers (this is a limit in the Z-unos number of analog pins)
 * 
 * To add additional flowers, do the following:
 * 1. Change the numberOfFlowers to correct number
 * 2. Uncomment the setup channels (one for the humidity and one for the motor)
 * 
 * NOTE:
 * Automatic mode is always disabled after power cycle, regardless of switch state. 
 * To change this behaviour set parameter 90 to 1.
 * 
 */

#define LED_PIN     13

//Define number of flowers to water
#define numberOfFlowers  4

//Define humidity pins
#define HUM_1           A1
#define HUM_2           A2
#define HUM_3           A3
#define HUM_4           A0 //Use A0 last, since it is also interupt pin
#define HUM_power        9

//Define pump pins
#define PUMP_1          10
#define PUMP_2          11
#define PUMP_3          12
#define PUMP_4          14 //dont use 13 since it is the built in led

//Setup parameters
//param 64 and up
word updateInterval;          //regular interval for updating sensors. In Minutes
word updateIntervalWatering;  //interval when pumping in seconds
word automationMode;          //select watering logic
word minRead;                 //Calibrated min reading
word maxRead;                 //Calibrated max reading
word pumpTimer;               //When using automode 1, for how long to pump

//param 70 and up
word START_PUMP_1;
word START_PUMP_2;
word START_PUMP_3;
word START_PUMP_4;

//param 80 and up
word STOP_PUMP_1;
word STOP_PUMP_2;
word STOP_PUMP_3;
word STOP_PUMP_4;

//param 90 and up
word AUTOMATION;
word maxWatering;

int START[4];
int STOP[4]; 

int SENSORS[] = {HUM_1, HUM_2, HUM_3, HUM_4};
int PUMPS[] = {PUMP_1, PUMP_2, PUMP_3, PUMP_4};

int LASTSENSORVALUES[numberOfFlowers];
int LASTPUMPVALUES[numberOfFlowers];

long secondsPassedInBigLoop = 0;

ZUNO_SETUP_SLEEPING_MODE(ZUNO_SLEEPING_MODE_ALWAYS_AWAKE);
ZUNO_SETUP_CFGPARAMETER_HANDLER(config_parameter_changed);

//make sure sensors are the first channels. Else reports won't work correct.
ZUNO_SETUP_CHANNELS(ZUNO_SENSOR_MULTILEVEL_HUMIDITY(getHum1), 
                    ZUNO_SENSOR_MULTILEVEL_HUMIDITY(getHum2),
                    ZUNO_SENSOR_MULTILEVEL_HUMIDITY(getHum3),
                    ZUNO_SENSOR_MULTILEVEL_HUMIDITY(getHum4),
                    ZUNO_SWITCH_BINARY(getPumpOne, setPumpOne), 
                    ZUNO_SWITCH_BINARY(getPumpTwo, setPumpTwo),
                    ZUNO_SWITCH_BINARY(getPumpThree, setPumpThree),
                    ZUNO_SWITCH_BINARY(getPumpFour, setPumpFour),
                    ZUNO_SWITCH_BINARY(getAutomation, setAutomation));

void setup() { // This function will run once
  Serial.begin();

  //UpdateInterval
  zunoLoadCFGParam(64, &updateInterval);
  if (updateInterval > 65000){ //parameters default to a high value. If user never touched the value set a correct, default, value
    updateInterval = 45;
    }

  //UpdateInterval when watering
  zunoLoadCFGParam(65, &updateIntervalWatering);
  if (updateIntervalWatering > 65000){
    updateIntervalWatering = 30;
    }

  //Automation mode
  zunoLoadCFGParam(66, &automationMode);
  if (automationMode > 65000){
    automationMode = 2;
    }

  //Min read (max percent)
  zunoLoadCFGParam(67, &minRead);
  if (minRead > 65000){
    minRead = 200;
    }

  //Max read (min percent)
  zunoLoadCFGParam(68, &maxRead);
  if (maxRead > 65000){
    maxRead = 900;
    }

  //For how long to pump
  zunoLoadCFGParam(69, &pumpTimer);
  if (pumpTimer > 65000){
    pumpTimer = 5;
    }

  //Start pump 1
  zunoLoadCFGParam(70, &START_PUMP_1);
  if (START_PUMP_1 > 65000){
    START_PUMP_1 = 35;
    }

  //Start pump 2
  zunoLoadCFGParam(71, &START_PUMP_2);
  if (START_PUMP_2 > 65000){
    START_PUMP_2 = 35;
    }

  //Start pump 3
  zunoLoadCFGParam(72, &START_PUMP_3);
  if (START_PUMP_3 > 65000){
    START_PUMP_3 = 35;
    }

  //Start pump 4
  zunoLoadCFGParam(73, &START_PUMP_4);
  if (START_PUMP_4 > 65000){
    START_PUMP_4 = 35;
    }

  //STOP_PUMP_1
  zunoLoadCFGParam(80, &STOP_PUMP_1);
  if (STOP_PUMP_1 > 65000){
    STOP_PUMP_1 = 50;
    }

  //STOP_PUMP_2
  zunoLoadCFGParam(81, &STOP_PUMP_2);
  if (STOP_PUMP_2 > 65000){
    STOP_PUMP_2 = 50;
    }

  //STOP_PUMP_3
  zunoLoadCFGParam(82, &STOP_PUMP_3);
  if (STOP_PUMP_3 > 65000){
    STOP_PUMP_3 = 50;
    }

  //STOP_PUMP_4
  zunoLoadCFGParam(83, &STOP_PUMP_4);
  if (STOP_PUMP_4 > 65000){
    STOP_PUMP_4 = 50;
    }

  //AUTOMATION
  zunoLoadCFGParam(90, &AUTOMATION);
  if (AUTOMATION > 65000){
    AUTOMATION = 1;
    }

  //Max time for watering
  zunoLoadCFGParam(91, &maxWatering);
  if (maxWatering > 65000){
    maxWatering = 10;
    }

  //Start values for watering in percent for each motor. 0 is off.
  START[0] = int(START_PUMP_1);
  START[1] = int(START_PUMP_2);
  START[2] = int(START_PUMP_3);
  START[3] = int(START_PUMP_4); 

  //Stop values for watering in percent for each motor. 0 is off.
  STOP[0] = int(STOP_PUMP_1);
  STOP[1] = int(STOP_PUMP_2);
  STOP[2] = int(STOP_PUMP_3);
  STOP[3] = int(STOP_PUMP_4); 

  pinMode(LED_PIN, OUTPUT); // set LED pin as output

  pinMode(HUM_power, OUTPUT); //Set power output
  digitalWrite(HUM_power, LOW); //Set power to off

  for(int i = 0; i < numberOfFlowers ; i++){
    pinMode(SENSORS[i], INPUT); //init all sensors
    pinMode(PUMPS[i], OUTPUT); //init all pumps
    digitalWrite(PUMPS[i], HIGH); //turn off the motors
    LASTSENSORVALUES[i] = 0; //init sensor readings
    LASTPUMPVALUES[i] = 0; //init pump readings
    }
  
}

//When dry, pump for XX seconds then stop
void autoMode_1(int i, int currentValue){
  digitalWrite(PUMPS[i], LOW); //start the pump
  LASTPUMPVALUES[i] = 1;
  zunoSendReport(i + 1 + numberOfFlowers); //report new pump status
  Serial.println("Started pump because the dirt is dry");
  
  int secondsInLoop = 0;
  int readInterval = 0;

  //instead of just doing a delay we loop so the pumping can be interrupted!
  while(getHumSensor(LASTSENSORVALUES[i]) < STOP[i] && LASTPUMPVALUES[i] == 1 && AUTOMATION == 1 && secondsInLoop < maxWatering * 60 && secondsInLoop / 60 < pumpTimer ){    
    if(readInterval >= updateIntervalWatering){
      read_humidity_sensor(SENSORS[i], i);
      readInterval = 0;
      }
    readInterval += 1;
    secondsInLoop += 1;
    delay(1000);
    }

  digitalWrite(PUMPS[i], HIGH); //stop the pump
  LASTPUMPVALUES[i] = 0;
  zunoSendReport(i + 1 + numberOfFlowers); //report new pump status
  
  Serial.println("Stopped pump");
  }

//Pump when dry, until wet
void autoMode_2(int i, int currentValue){
  digitalWrite(PUMPS[i], LOW); //start the pump
  LASTPUMPVALUES[i] = 1;
  zunoSendReport(i + 1 + numberOfFlowers);
  Serial.println("Started pump because the dirt is dry");

  int secondsInLoop = 0;
  int readInterval = 0;

  //instead of just doing a delay we loop so the pumping can be interrupted!
  while (getHumSensor(LASTSENSORVALUES[i]) < STOP[i] && LASTPUMPVALUES[i] == 1 && AUTOMATION == 1 && secondsInLoop < maxWatering * 60){ //while the dirt is dry or motor has not been turned off
    Serial.println(secondsInLoop);
    if(readInterval >= updateIntervalWatering){
      read_humidity_sensor(SENSORS[i], i);
      readInterval = 0;
      }
    readInterval += 1;
    secondsInLoop += 1;
    delay(1000);
  }
  
  digitalWrite(PUMPS[i], HIGH); //stop the pump
  LASTPUMPVALUES[i] = 0;
  zunoSendReport(i + 1 + numberOfFlowers);
  Serial.println("Stopped pump because the dirt is wet");
  }

void printDebug(int i, int currentValue){
   Serial.print("Parameter 64: ");
   Serial.println(updateInterval);
   Serial.print("Parameter 65: ");
   Serial.println(updateIntervalWatering);
   Serial.print("Parameter 66: ");
   Serial.println(automationMode);
   Serial.print("Parameter 67: ");
   Serial.println(minRead);
   Serial.print("Parameter 68: ");
   Serial.println(maxRead);
   Serial.print("Parameter 69: ");
   Serial.println(pumpTimer);
   Serial.print("Parameter 70: ");
   Serial.println(START_PUMP_1);
   Serial.print("Parameter 71: ");
   Serial.println(START_PUMP_2);
   Serial.print("Parameter 72: ");
   Serial.println(START_PUMP_3);
   Serial.print("Parameter 73: ");
   Serial.println(START_PUMP_4);
   Serial.print("Parameter 80: ");
   Serial.println(STOP_PUMP_1);
   Serial.print("Parameter 81: ");
   Serial.println(STOP_PUMP_2);
   Serial.print("Parameter 82: ");
   Serial.println(STOP_PUMP_3);
   Serial.print("Parameter 83: ");
   Serial.println(STOP_PUMP_4);
   Serial.print("Parameter 90: ");
   Serial.println(AUTOMATION);
   Serial.print("Parameter 91: ");
   Serial.println(maxWatering);

   Serial.print("Value of sensor ");
   Serial.print(i + 1);
   Serial.print(": ");
   Serial.println(currentValue); 

   if (i == 0){
        Serial.print("Converted to percent: ");
        Serial.println(getHum1());
      } else {
        Serial.print("Converted to percent: ");
        Serial.println(getHum2());
      }

  Serial.print("Pin no: ");
  Serial.println(SENSORS[i]);
  }

void loop() { // This function will run in a loop

  if (secondsPassedInBigLoop == 0 || secondsPassedInBigLoop / 60 >= updateInterval){
    for(int i=0;i < numberOfFlowers;i++){ //Loop over all flowers
       
        int currentValue = read_humidity_sensor(SENSORS[i], i);
        
        printDebug(i, currentValue);
  
        int percentValue = getHumSensor(LASTSENSORVALUES[i]);
        
        if (START[i] > 0 && percentValue < STOP[i] && automationMode == 1 && AUTOMATION == 1){ //activate autoMode 1
            autoMode_1(i, currentValue);
          }else if (START[i] > 0 && percentValue < START[i] && automationMode == 2 && AUTOMATION == 1){ //activate autoMode 2
            autoMode_2(i, currentValue);
          }
      } 
      secondsPassedInBigLoop = 0;
   }
   
 delay(1000);
 secondsPassedInBigLoop += 1;  
}

// ################### Control if automation is on or off #####################
byte getAutomation(){
  return AUTOMATION;
}

byte setAutomation(byte newVal){
  if (newVal > 0){
    AUTOMATION = 1;
  } else {
      AUTOMATION = 0;
    }
}

// ################### Pump setter/getter functions #####################
byte getPumpOne() {
  return LASTPUMPVALUES[0];
}

byte getPumpTwo() {
  return LASTPUMPVALUES[1];
}

byte getPumpThree() {
  return LASTPUMPVALUES[1];
}

byte getPumpFour() {
  return LASTPUMPVALUES[1];
}

void setPumpOne(byte newVal) {
  if (newVal > 0) {
    digitalWrite(PUMPS[0], LOW);
  } else {
    digitalWrite(PUMPS[0], HIGH);
  }
  LASTPUMPVALUES[0] = newVal;
}

void setPumpTwo(byte newVal) {
  if (newVal > 0) {
    digitalWrite(PUMPS[1], LOW);
  } else {
    digitalWrite(PUMPS[1], HIGH);
  }
  LASTPUMPVALUES[1] = newVal;
}

void setPumpThree(byte newVal) {
  if (newVal > 0) {
    digitalWrite(PUMPS[2], LOW);
  } else {
    digitalWrite(PUMPS[2], HIGH);
  }
  LASTPUMPVALUES[2] = newVal;
}

void setPumpFour(byte newVal) {
  if (newVal > 0) {
    digitalWrite(PUMPS[3], LOW);
  } else {
    digitalWrite(PUMPS[3], HIGH);
  }
  LASTPUMPVALUES[3] = newVal;
}

// ################### Humidity sensor functions #####################
byte getHumSensor(int value) {
  //used to control scale of outputs
  int newMax = 100; //Max output
  int newMin = 0;   //min output

  float readRange = int(maxRead)-int(minRead);
  float newRange = newMax-newMin;

  int tempVar = (int)(((value-int(minRead))*(newRange/readRange)+newMin)-newMax)*-1; //convert measured range to %

  if (tempVar > 100){
    tempVar = 100;
    Serial.println("Value rounded to 100. Please check min/max readings");
    } else if (tempVar < 0){
      tempVar = 0;
      Serial.println("Value rounded to 0. Please check min/max readings");
      }
  return tempVar;
}

byte getHum1(void) {
  return (byte)getHumSensor(LASTSENSORVALUES[0]);
}

byte getHum2(void) {
  return (byte)getHumSensor(LASTSENSORVALUES[1]);
}

byte getHum3(void) {
  return (byte)getHumSensor(LASTSENSORVALUES[2]);
}

byte getHum4(void) {
  return (byte)getHumSensor(LASTSENSORVALUES[3]);
}

//Reads the humidity of a sensor
int read_humidity_sensor(int pin, int i) { 
  digitalWrite(HUM_power, HIGH); //power on the sensor
  delay(500); //wait a little
  int currentValue = analogRead(pin); //read the analog value of the sensor
  digitalWrite(HUM_power, LOW); //turn off the sensor
  
  if (currentValue != LASTSENSORVALUES[i]){ //only update value if necessary
    updateHumidityValue(i, currentValue);       
  }
  return currentValue; //return the value
}

//Updates the stored humidity value
void updateHumidityValue(int i, int currentValue) {
  Serial.print("Saving updated value and updating controller on node "); 
  Serial.println(i + 1);
  LASTSENSORVALUES[i] = currentValue;
  zunoSendReport(i + 1); //sensors are the first channels. So i + 1 will always be the correct report
}

// ################### Param changes #####################
void config_parameter_changed(byte param, word * value) {
    if(param == 64) { // The first user-defined parameter 
      updateInterval = *value;  
    }
    if(param == 65) {
      updateIntervalWatering = *value;  
    }
    if(param == 66) {
      automationMode = *value;  
    }
    if(param == 67) {
      minRead = *value;  
    }
    if(param == 68) {
      maxRead = *value;  
    }
    if(param == 69) {
      pumpTimer = *value;  
    }
    if(param == 70) {
      START_PUMP_1 = *value;  
    }
    if(param == 71) {
      START_PUMP_2 = *value;  
    }
    if(param == 72) {
      START_PUMP_3 = *value;  
    }
    if(param == 73) {
      START_PUMP_4 = *value;  
    }
    if(param == 80) {
      STOP_PUMP_1 = *value;  
    }
    if(param == 81) {
      STOP_PUMP_2 = *value;  
    }
    if(param == 82) {
      STOP_PUMP_3 = *value;  
    }
    if(param == 83) {
      STOP_PUMP_4 = *value;  
    }
    if(param == 90) {
      AUTOMATION = *value;  
    }
    if(param == 91) {
      maxWatering = *value;  
    }
}
