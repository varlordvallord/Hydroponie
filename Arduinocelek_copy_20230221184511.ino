#include <OneWire.h>
#include <FastIO.h>
#include <I2CIO.h>
#include <Wire.h> // Library for I2C communication
#include <DallasTemperature.h> //knihovna pro teplotní komunikaci
#include <LiquidCrystal_I2C.h> // Library for LCD
const int pinPo = A0; //ph pin
const int SENSOR_PIN = 11; // pin teplota
float tempCelsius;    // temperature in Celsius
OneWire oneWire(SENSOR_PIN);         // setup a oneWire instance 
DallasTemperature sensors(&oneWire); // pass oneWire to DallasTemperature library
// mega má attach interupt na 2, 3, 18, 19, 20, 21 (pins 20 & 21 are not available to use for interrupts while they are used for I2C communication)
//1 pin do GND druhý na interupt

//////////////////////////////////////////
//kalibracni kod
float CalibrationEC=1.38; //EC value of Calibration solution is s/cm
  int R1= 1000;
  int Ra=25; //Resistance of powering Pins
  //int ECPin= A0;
  int ECGround=A1;
  int ECPower =A4;
  float TemperatureCoef = 0.019;
 // const int TempProbePossitive =8;  //Temp Probe power connected to pin 9
 // const int TempProbeNegative=9;    //Temp Probe Negative connected to pin 8
//  OneWire oneWire(ONE_WIRE_BUS);// Setup a oneWire instance to communicate with any OneWire devices
 // DallasTemperature sensors(&oneWire);// Pass our oneWire reference to Dallas Temperature.
  float TemperatureFinish=0;
  float TemperatureStart=0;
  float EC=0;
  int ppm =0;
  float raw= 0;
  float Vin= 5;
  float Vdrop= 0;
  float Rc= 0;
  float K=0;
  int i=0;
  float buffer=0;
//////////////////////////////
int upButton = 13;
int downButton = 14;  // piny pro tláčítka n apřepínání menu dysplaje
int selectButton = 15;
int menu = 1; //proměnná pro přepínání v menu
#define cerhlav 22 // pin pro relé ovládající hlavní čerpadlo
#define pinHladina1 2
#define pinHladina2 3 //piny  sledující výšku hladiny
#define pinHladina3 18
// proměnná pro uložení času aktivace
unsigned long casAktivace = 0; //pro hladinu
#define cerpadlo1 12 //pin pro cerpadlo
#define led1 5 //pin pro spouštění přísvitu
#define led2 6 //pin pro spouštění přísvitu
#define led3 7 //pin pro spouštění přísvitu
#define cerphlav 8 //pin pro spuštění hlavního čerpadla
//pin 20 21 pro LCD
// Connect to LCD via I2C, default address 0x27 (A0-A2 not jumpered)
LiquidCrystal_I2C lcd = LiquidCrystal_I2C(0x27, 16, 2); // Change to (0x27,20,4) for 20x4 LCD.

void setup() {

  Serial.begin(9600); // initialize serial
  sensors.begin(); //senzory začnou operovat
  attachInterrupt(digitalPinToInterrupt(pinHladina1), preruseni1, RISING);  //hladina
  attachInterrupt(digitalPinToInterrupt(pinHladina2), preruseni2, RISING);  //hladina
  attachInterrupt(digitalPinToInterrupt(pinHladina3), preruseni3, RISING);  //hladina
  pinMode(cerpadlo1, OUTPUT); //pin pro cerpadlo
  pinMode(led1, OUTPUT); //pin pro led pasek
  pinMode(led2, OUTPUT); //pin pro led pasek
  pinMode(led3, OUTPUT); //pin pro led pasek
  pinMode(cerphlav, OUTPUT); //pin pro cerpadlo
  lcd.init();  // Initiate the LCD:
  lcd.backlight();  // Initiate the LCD backlight:
  pinMode(upButton, INPUT_PULLUP); //tlacitko pro menu
  pinMode(downButton, INPUT_PULLUP); //tlacitko pro menu
  pinMode(selectButton, INPUT_PULLUP); //tlacitko pro menu
  updateMenu(); //základní nabídka menu
    
    //kalibracni kod
    
        //kalibracni kod
     //pinMode(TempProbeNegative , OUTPUT ); //seting ground pin as output for tmp probe
     //digitalWrite(TempProbeNegative , LOW );//Seting it to ground so it can sink current
     //pinMode(TempProbePossitive , OUTPUT );//ditto but for positive
     //digitalWrite(TempProbePossitive , HIGH );
     pinMode(pinPo,INPUT);
     pinMode(ECPower,OUTPUT);//Setting pin for sourcing current
     pinMode(ECGround,OUTPUT);//setting pin for sinking current
     digitalWrite(ECGround,LOW);//We can leave the ground connected permanantly
 
     delay(100);// gives sensor time to settle
     sensors.begin();
     delay(100);
     //  Adding Digital Pin Resistance to [25 ohm] to the static Resistor 
     // Consule Read-Me for Why, or just accept it as true
     R1=(R1+Ra);
 
     //Serial.println("ElCheapo Arduino EC-PPM measurments Calibration");
     //Serial.println("By: Michael Ratcliffe  Mike@MichaelRatcliffe.com");
     //Serial.println("Free software: you can redistribute it and/or modify it under GNU ");
      //Serial.println("");
      //Serial.println("Make sure Probe and Temp Sensor are in Solution and solution is well mixed");
     //Serial.println("");
     // Serial.println("Starting Calibration: Estimated Time 60 Seconds:");
    
    
}
void loop() {

   ph();

}
void teplota() {

 sensors.requestTemperatures();
 tempCelsius = sensors.getTempCByIndex(0);

  Serial.print("Temperature: ");
  Serial.print(tempCelsius);    // print the temperature in Celsius
  Serial.print("°C");
  Serial.print("  ~  ");        // separator between Celsius and Fahrenheit
  delay(500);

}
void ph(){
    // vytvoření pomocných proměnných
  int pole[10];
  int zaloha;
  unsigned long int prumerVysl = 0;
  // načtení deseti vzorků po 10 ms do pole
  for (int i = 0; i < 10; i++) {
    pole[i] = analogRead(pinPo);
    delay(10);
  }
  // seřazení členů pole naměřených výsledků podle velikosti
  for (int i = 0; i < 9; i++) {
    for (int j = i + 1; j < 10; j++) {
      if (pole[i] > pole[j]) {
        zaloha = pole[i];
        pole[i] = pole[j];
        pole[j] = zaloha;
      }
    }
  }
  // uložení 2. až 8. výsledku do
  // proměnné, z které se vypočte průměr
  // (vynechání dvou členů pole na začátku
  // a konci pro lepší přesnost)
  for (int i = 2; i < 8; i++) {
    prumerVysl += pole[i];
  }
  // výpočet hodnoty pH z průměru
  // měření a přepočet na rozsah 0-14 pH
  float prumerPH = (float)prumerVysl * 5.0 / 1024 / 6;
  float vyslednePH = -5.70 * prumerPH + 21.34;
  Serial.print("Namerene pH: ");// vytištění výsledků po sériové lince
  Serial.println(vyslednePH);
  delay(900); // pauza 900 ms před novým měřením, celkem tedy 1s
}
void ph_kalibrace() {
  //kodpro kalibraci PH
  i=1;
  buffer=0;
  sensors.requestTemperatures();// Send the command to get temperatures
  TemperatureStart=sensors.getTempCByIndex(0); //Stores Value in Variable
 
  //************Estimates Resistance of Liquid ****************
    while(i<=10){
 
 
 
  digitalWrite(ECPower,HIGH);
  raw= analogRead(pinPo);
  raw= analogRead(pinPo);// This is not a mistake, First reading will be low
  digitalWrite(ECPower,LOW);
  buffer=buffer+raw;
  i++;
  delay(5000);
  };
  raw=(buffer/10);
 
 
 
 
  sensors.requestTemperatures();// Send the command to get temperatures
  TemperatureFinish=sensors.getTempCByIndex(0); //Stores Value in Variable
 
  //*************Compensating For Temperaure********************
  EC =CalibrationEC*(1+(TemperatureCoef*(TemperatureFinish-25.0))) ;
 
  //***************** Calculates R relating to Calibration fluid **************************
  Vdrop= (((Vin)*(raw))/1024.0);
  Rc=(Vdrop*R1)/(Vin-Vdrop);
  Rc=Rc-Ra;
  K= 1000/(Rc*EC);
 
 
 
 
  Serial.print("Calibration Fluid EC: ");
  Serial.print(CalibrationEC);
  Serial.print(" S  ");  //add units here
  Serial.print("Cell Constant K");
  Serial.print(K);
 
 
  if (TemperatureStart==TemperatureFinish){
  Serial.println("  Results are Trustworthy");
  Serial.println("  Safe To Use Above Cell Constant in Main EC code");
 
  }
  else{
  Serial.println("  Error -Wait For Temperature To settle");
 
  }
}
void preruseni1() {
  // výpis informace při detekci
  Serial.println("Dosla kapalina!!!");
}
void preruseni3() {
  // výpis informace při detekci
  Serial.println("Dosla kapalina!!!");

}
void preruseni2() {
  // výpis informace při detekci
  Serial.println("Dosla kapalina!!!");

}
void cerpadlo() {
  for (int i = 200; i <= 255; i += 1) {
    // nastavení PWM na pin s hodnotou i generovanou ve smyčce
    analogWrite(cerpadlo1, i);
    // krátká pauza mezi kroky
    delay(10);
  }

    // obdobná for smyčka, která jde opačně,
  // tedy od hodnoty 255 až po 200
  for (int i = 255; i != 200; i -= 1) {
    // nastavení PWM na pin s hodnotou i generovanou ve smyčce
  analogWrite(cerpadlo1, i);
    // krátká pauza mezi kroky
    delay(10);}
}
void diplay() {
  //LCD
  lcd.clear();
  lcd.setCursor(2, 0); // Set the cursor on the third column and first row.
  lcd.print("teplota:"); // Print the string "Hello World!"
  lcd.setCursor(2, 1); //Set the cursor on the third column and the second row (counting starts at 0!).
  lcd.print(tempCelsius);
  //lcd.cursor();
  //delay(500);
  //lcd.noCursor();
  //lcd.print("Custom Character");
}
void svetlo5rozs() {
  digitalWrite(led1, HIGH);
}
void svetlo6rozs() {
  digitalWrite(led2, HIGH);
}
void svetlo7rozs() {
  digitalWrite(led3, HIGH);
}
void cerpadlohlavnizap() {
  digitalWrite(cerhlav, HIGH);
}
void vlastnilcdznak() {
  //naprogramování vlastních arduino lcd znaků
  byte Heart[] = {
    B00000,
    B01010,
    B11111,
    B11111,
    B01110,
    B00100,
    B00000,
    B00000
  };

  byte Bell[] = {
   B00100,
   B01110,
   B01110,
   B01110,
   B11111,
   B00000,
   B00100,
   B00000
  };

  byte Alien[] = {
   B11111,
   B10101,
   B11111,
   B11111,
   B01110,
   B01010,
   B11011,
   B00000
  };

  byte Check[] = {
    B00000,
    B00001,
    B00011,
    B10110,
    B11100,
    B01000,
    B00000,
    B00000
  };

  byte Speaker[] = {
   B00001,
   B00011,
   B01111,
   B01111,
   B01111,
   B00011,
   B00001,
   B00000
  };

  byte Sound[] = {
    B00001,
    B00011,
    B00101,
    B01001,
    B01001,
    B01011,
    B11011,
    B11000
  };

  byte Skull[] = {
    B00000,
    B01110,
    B10101,
    B11011,
    B01110,
    B01110,
    B00000,
    B00000
  };

  byte Lock[] = {
    B01110,
    B10001,
    B10001,
    B11111,
    B11011,
    B11011,
    B11111,
    B00000
  };
  lcd.createChar(0, Heart);
  lcd.createChar(1, Bell);
  lcd.createChar(2, Alien);
  lcd.createChar(3, Check);
  lcd.createChar(4, Speaker);
  lcd.createChar(5, Sound);
  lcd.createChar(6, Skull);
  lcd.createChar(7, Lock);
}
void menuloop() {
  if (!digitalRead(downButton)){
    menu++;
    updateMenu();
    delay(100);
    while (!digitalRead(downButton));
  }
  if (!digitalRead(upButton)){
    menu--;
    updateMenu();
    delay(100);
    while(!digitalRead(upButton));
  }
  if (!digitalRead(selectButton)){
    executeAction();
    updateMenu();
    delay(100);
    while (!digitalRead(selectButton));
  }
}
void updateMenu() {
  switch (menu) {
    case 0:
      menu = 1;
      break;
    case 1:
      lcd.clear();
      lcd.print(">Kalibrace");
      lcd.setCursor(0, 1);
      lcd.print(" MenuItem2");
      break;
    case 2:
      lcd.clear();
      lcd.print(" Kalibrace");
      lcd.setCursor(0, 1);
      lcd.print(">MenuItem2");
      break;
    case 3:
      lcd.clear();
      lcd.print(">MenuItem3");
      lcd.setCursor(0, 1);
      lcd.print(" MenuItem4");
      break;
    case 4:
      lcd.clear();
      lcd.print(" MenuItem3");
      lcd.setCursor(0, 1);
      lcd.print(">MenuItem4");
      break;
    case 5:
      menu = 4;
      break;
  }
}
void executeAction() {
  switch (menu) {
    case 1:
      action1();
      break;
    case 2:
      action2();
      break;
    case 3:
      action3();
      break;
    case 4:
      action4();
      break;
  }
}
void action1() {
  lcd.clear();
  ph_kalibrace();
  lcd.print(">Executing #1");
  delay(1500);
}
void action2() {
  lcd.clear();
  lcd.print(">Executing #2");
  delay(1500);
}
void action3() {
  lcd.clear();
  lcd.print(">Executing #3");
  delay(1500);
}
void action4() {
  lcd.clear();
  lcd.print(">Executing #4");
  delay(1500);
}