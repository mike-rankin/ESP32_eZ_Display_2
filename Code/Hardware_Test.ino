
#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7789.h> // Hardware-specific library for ST7789
#include <Adafruit_LSM6DS33.h>
#include "ClosedCube_HDC1080.h"
#include <SPI.h>
#include <Wire.h>

#define TFT_CS        4
#define TFT_RST        22
#define TFT_DC         21
#define TFT_BACKLIGHT  26

#define BLUE_LED 9
#define RED_LED 10
#define GREEN_LED 5

//Button Pins
const int buttonPin_0 = 35; //Top_Left 
const int buttonPin_1 = 2; //Top_Right
const int buttonPin_2 = 34; //Bottom_Left
const int buttonPin_3 = 12; //Bottom_Right

int buttonState_0 = 0;
int buttonState_1 = 0;
int buttonState_2 = 0;
int buttonState_3 = 0;


Adafruit_ST7789 tft = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_RST);
Adafruit_LSM6DS33 lsm6ds33;

ClosedCube_HDC1080 hdc1080;

float p = 3.1415926;

void setup(void) 
{
  Serial.begin(9600);
  Serial.print(F("Hello! ST77xx TFT Test"));
  Wire.begin(13,14);
  delay(1000);

  hdc1080.begin(0x40);   //Temp/Humidity related

  lsm6ds33.begin_I2C();  //Accel related
  lsm6ds33.setAccelRange(LSM6DS_ACCEL_RANGE_2_G);
  lsm6ds33.setGyroRange(LSM6DS_GYRO_RANGE_250_DPS);
  lsm6ds33.setAccelDataRate(LSM6DS_RATE_12_5_HZ);
  lsm6ds33.setGyroDataRate(LSM6DS_RATE_12_5_HZ);

  lsm6ds33.configInt1(false, false, true); // accelerometer DRDY on INT1
  lsm6ds33.configInt2(false, true, false); // gyro DRDY on INT2

  pinMode(RED_LED, OUTPUT);
  pinMode(GREEN_LED, OUTPUT);
  pinMode(BLUE_LED, OUTPUT);
  pinMode(TFT_BACKLIGHT, OUTPUT);

  pinMode(buttonPin_0, INPUT);
  pinMode(buttonPin_1, INPUT);
  pinMode(buttonPin_2, INPUT);
  pinMode(buttonPin_3, INPUT);

  digitalWrite(RED_LED, HIGH);   //Keep off
  digitalWrite(GREEN_LED, HIGH); //Keep off
  digitalWrite(BLUE_LED, HIGH);  //Keep off
  
  pinMode(TFT_BACKLIGHT, OUTPUT);

  blinkRGB();

  tft.init(240, 240);  // Init ST7789 240x240
  tft.setRotation(2);
  Serial.println(F("Initialized"));
}

void loop() 
{
  sensors_event_t accel;
  sensors_event_t gyro;
  sensors_event_t temp;
  lsm6ds33.getEvent(&accel, &gyro, &temp);

  buttonState_0 = digitalRead(buttonPin_0); //Top_Left
  buttonState_1 = digitalRead(buttonPin_1); //Top_Right
  buttonState_2 = digitalRead(buttonPin_2); //Bottom_Left
  buttonState_3 = digitalRead(buttonPin_3); //Bottom_Right
  
  digitalWrite(TFT_BACKLIGHT, HIGH);  //High keeps backlight on
  tft.setTextWrap(false);
  tft.fillScreen(ST77XX_BLACK);
  tft.setCursor(30, 10);
  tft.setTextColor(ST77XX_RED);
  tft.setTextSize(2);
  tft.print(hdc1080.readTemperature());
  tft.print((char)247);
  tft.print("C  ");
  tft.print(hdc1080.readHumidity()); 
  tft.print("%  ");

  tft.setCursor(0, 40);
  tft.setTextColor(ST77XX_BLUE);
  tft.println("Accel:");
  tft.print("X: ");
  tft.println(accel.acceleration.x);
  tft.print("Y: ");
  tft.println(accel.acceleration.y);
  tft.print("Z: ");
  tft.println(accel.acceleration.z);
  tft.println(" ");
  
  tft.setCursor(0, 130);
  tft.setTextColor(ST77XX_YELLOW);
  tft.println("Gyro:");
  tft.print("X: ");
  tft.println(gyro.gyro.x);
  tft.print("Y: ");
  tft.println(gyro.gyro.y);
  tft.print("Z: ");
  tft.println(gyro.gyro.z);

  if(buttonState_0 == HIGH)     
  {
   tft.setCursor(0,0);
   tft.setTextColor(ST77XX_GREEN);   
   tft.print("X");
  }

  else if(buttonState_1 == HIGH)    
  {
   tft.setCursor(230,0);
   tft.setTextColor(ST77XX_GREEN);   
   tft.print("X");
  }

  else if(buttonState_2 == HIGH)     
  {
   tft.setCursor(0,225);
   tft.setTextColor(ST77XX_GREEN);   
   tft.print("X");
  }

  else if(buttonState_3 == HIGH)    
  {
   tft.setCursor(230,225);
   tft.setTextColor(ST77XX_GREEN);   
   tft.print("X");
  }
  


  delay(100);
}


void blinkRGB(void)
{
  digitalWrite(RED_LED, LOW);   //led on
  delay(150);
  digitalWrite(RED_LED, HIGH);   //led off
  delay(150);
  digitalWrite(GREEN_LED, LOW);
  delay(150);
  digitalWrite(GREEN_LED, HIGH);
  delay(150);
  digitalWrite(BLUE_LED, LOW);
  delay(150);
  digitalWrite(BLUE_LED, HIGH);
  delay(150);
}
