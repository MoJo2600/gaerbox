#include <ClickEncoder.h>
#include <LiquidCrystal.h>
#include <TimerOne.h>
#include <Wire.h>

// defines how many temperature samples should be taken and averaged
#define temp_samples_to_take 4

// set maximum temperature
const float MAX_TEMP = 40.0;

// set hysteresis 0.5 degrees up and down
const float HYSTERESIS = 0.5;

// pin of heater (lamp)
int heater = 9;

// pins for the encoder
int enc1 = 10;
int enc2 = 11;
int enc3 = 12;

// pin of temperature sensor
int temp_sensor = 4;

// variables
int temp_reading;
float temp_is;
float temp_is_tmp;
float temp_target = 20.0;
static char temp_ist_outstr[4];
static char temp_target_outstr[4];
int temp_samples_taken = 0;
unsigned long lastrun;
bool heat = false;

// initialize the library with the numbers of the interface pins
LiquidCrystal lcd(7, 6, 2, 5, 3, 4);

// Rotary encoder
ClickEncoder *encoder;
int16_t last;

void timerIsr() {
  encoder->service();
}

void setup() {
  // Set lamp port to output
  pinMode(heater, OUTPUT);
  
  // setup encoder
  encoder = new ClickEncoder(enc2, enc1, enc2, 4);
  encoder->setAccelerationEnabled(true);

  Timer1.initialize(1000);
  Timer1.attachInterrupt(timerIsr);   
  last = -1;  
  
  // setup temperature measurement
  analogReference(INTERNAL);  
      
  // set up the LCD's number of columns and rows:
  lcd.begin(16, 2);
  // Print a message to the LCD.
  lcd.print("Gaerbox v0.1");
  delay(2000);
}

// updates all states
void update() {  
  temp_target += encoder->getValue() / 10.0;
  if (temp_target > MAX_TEMP) {
    temp_target = MAX_TEMP;
  }

  // take a sample every 500ms and update the value all temp_samples_to_take times
  if (millis() - lastrun > 500) {
    temp_reading = analogRead(temp_sensor);
    temp_is_tmp += temp_reading / 9.31;

    if (temp_samples_taken >= temp_samples_to_take) {
      temp_is = temp_is_tmp / temp_samples_to_take;
      temp_is_tmp = 0.0;
      temp_samples_taken = 0;
    }
    temp_samples_taken++;
    lastrun = millis();
  }
    
  // heater control
  if (temp_is > 0.0) {
    // ignore first measurement, which is null

    if (temp_is >= MAX_TEMP) {
      // security... set maximum temperature
      heat = false;
      digitalWrite(heater, LOW);
    } else {
      // control heating with hysteresis
      if (temp_is <= (temp_target - HYSTERESIS)) {
        heat = true;
        digitalWrite(heater, HIGH);
      } else if (temp_is >= (temp_target + HYSTERESIS)) {
        heat = false;
        digitalWrite(heater, LOW);
      }
    }
  }
}

// displays everything on the lcd display
void printstate() {

  dtostrf(temp_is, 4, 1, temp_ist_outstr);

  lcd.setCursor(0, 0);
  if (heat == true) {
    lcd.print("HEIZE     ");
  } else {
    lcd.print("Temp IST  ");
  }
  lcd.print(temp_ist_outstr); 
  lcd.write(0xDF);
  lcd.print("C");  

  dtostrf(temp_target, 4, 1, temp_target_outstr);

  lcd.setCursor(0, 1);
  lcd.print("Temp SOLL ");
  lcd.print(temp_target_outstr);
  lcd.write(0xDF);
  lcd.print("C");
   
}

// main loop
void loop() {
  update();
  printstate();
}
