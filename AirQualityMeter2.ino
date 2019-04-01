
/************************************************************************************************
 * Air Quality Meter
 * 01/09/2019
 * by Jeff Chap
 * chappy1978@yahoo.com
 * This is a software replacement for a generic air quality meter found on Aliexpress.
 * https://www.aliexpress.com/item/2-8-Car-PM2-5-Detector-Tester-Meter-Air-Quality-Monitor-Home-Gas-Thermometer-0616/32887138552.html?spm=a2g0s.9042311.0.0.31674c4dv3YpkW
 * The software is weak to say the least but the possiblites for this device is good.
 * 
**************************************************************************************************/
#include "PMS.h"
#define ENGLISH 0
#define FRENCH 1
#define LANGUAGE  ENGLISH
//#define LANGUAGE FRENCH

#if LANGUAGE == ENGLISH 
  #define EXCELLENT                     "Excellent"
  #define GOOD                          "Good"
  #define ACCEPTABLE                    "Acceptable"
  #define MODERATE                      "Moderate"
  #define HEAVY                         "Heavy"
  #define SEVERE                        "Severe"
  #define HAZARDOUS                     "Hazardous"
#endif

#if LANGAUGE == FRENCH
  #define EXCELLENT                     "Excellent"
  #define GOOD                          "Bon"
  #define ACCEPTABLE                    "Moyen"
  #define MODERATE                      "Mediocre"
  #define HEAVY                         "Mauvais"
  #define SEVERE                        "Tres mauvais"
  #define HAZARDOUS                     "Dangereux"
#endif

#define GREEN                          ILI9341_GREEN 
#define YELLOW                         ILI9341_YELLOW
#define ORANGE                         ILI9341_ORANGE
#define RED                            ILI9341_RED
#define PURPLE                         ILI9341_PURPLE
#define MAROON                         ILI9341_MAROON

PMS pms(Serial);
PMS::DATA data;

#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>

#define DHTPIN           PB8         // Pin which is connected to the DHT sensor.

// Uncomment the type of sensor in use:
#define DHTTYPE           DHT11     // DHT 11 
//#define DHTTYPE           DHT22     // DHT 22 (AM2302)
//#define DHTTYPE           DHT21     // DHT 21 (AM2301)

// See guide for details on sensor wiring and usage:
//   https://learn.adafruit.com/dht/overview

DHT_Unified dht(DHTPIN, DHTTYPE);

#include "SPI.h"
#include "Adafruit_GFX.h"
#include "Adafruit_ILI9341.h"
#include <Fonts/FreeSans9pt7b.h>
#include <Fonts/FreeSans12pt7b.h>    
#include <Fonts/FreeSansBold18pt7b.h> 
   
// LCD Definitions for the Air Meter connections to the ST32
#define TFT_DC PB1  
#define TFT_CS 0
#define TFT_CLK PA5
#define TFT_MOSI PA7
#define TFT_RST PB0
#define TFT_MISO 0

Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_MOSI, TFT_CLK, TFT_RST, TFT_MISO);

//Data structure for all information output
struct structData{
   // Sensor AQI data
  int         concentrationPM1_0  = 0;
  int         concentrationPM2_5 = 0;
  int         concentrationPM10_0  = 0;
  int         particle0_3     = 0;
  int         particle0_5     = 0;
  int         particle1_0     = 0;
  int         particle2_5     = 0;
  int         particle5_0     = 0;
  int         particle10_0    = 0;
  int         AQI = 0;
  String      AQIString = "";
  int         AQIColor = 0;
  float       temp_f = 0.0;
  float       temp_c = 0.0;
  float       humdity_rel = 0.0;
};

struct structData enviro;

//Button Pinout
int button0 = PB13;
volatile int page = LOW;

unsigned long previousMillis = 0;        
unsigned long interval = 5000;    

//Interrupt for button press.
void interruptFunction() {
  page = !page;
  }

void setup() {

  int rotation=3;

  dht.begin();
  Serial.begin(9600);
  tft.begin();
  pms.activeMode();

  pinMode(button0,INPUT_PULLDOWN);
  attachInterrupt(button0, interruptFunction, LOW);

  tft.setRotation(rotation);
}

void loop() {

 unsigned long currentMillis = millis();
    
    if(page == LOW){
      graphicDisplay();       //Front page display, when button pressed it will swicth between Page one and two.
      while(page == LOW){
        currentMillis = millis();
         if(currentMillis - previousMillis > interval){
        previousMillis = currentMillis; 
        tempData();
        pmsData();
        updateAQIDisplay();
        updateEnviroDisplay();
        }
      }
    }
    else{
      graphicDisplay1(); //Page two display
      while(page == HIGH){
        currentMillis = millis();
        if(currentMillis - previousMillis > interval){
        previousMillis = currentMillis; 
        pmsData();
        updateParticleDisplay();
      }
    }
  }
}

void updateParticleDisplay() {
    // Displays the Data for the second page.
    tft.setFont();
    tft.setTextSize(1);
    tft.setFont(&FreeSans9pt7b);
    tft.setTextColor(ILI9341_YELLOW);
    tft.fillRect(60,70,50,30, ILI9341_BLACK);
    tft.setCursor(70, 90);
    tft.print(enviro.concentrationPM1_0);
    tft.fillRect(60,140,50,30, ILI9341_BLACK);
    tft.setCursor(70, 160);
    tft.print(enviro.concentrationPM2_5);
    tft.fillRect(60,200,50,30, ILI9341_BLACK);
    tft.setCursor(70, 220);
    tft.print(enviro.concentrationPM10_0);
    tft.fillRect(250,50,50,180, ILI9341_BLACK);
    tft.setCursor(255, 70);
    tft.print(enviro.particle0_3);
    tft.setCursor(255, 100);
    tft.print(enviro.particle0_5);
    tft.setCursor(255, 130);
    tft.print(enviro.particle1_0);
    tft.setCursor(255, 160);
    tft.print(enviro.particle2_5);
    tft.setCursor(255, 190);
    tft.print(enviro.particle5_0);
    tft.setCursor(255, 220);
    tft.print(enviro.particle10_0);
  
}
void graphicDisplay1() {
  //Draws the graphic for second page.
  tft.fillScreen(ILI9341_BLACK);
  tft.setTextSize(1);
  tft.setFont(&FreeSans9pt7b);
  tft.setTextColor(ILI9341_BLUE, ILI9341_BLACK); 
  tft.drawRect(0, 0, 320, 240, ILI9341_WHITE);
  tft.drawLine(10, 0, 10, 240, ILI9341_WHITE);
  tft.drawLine(130, 0, 130, 240, ILI9341_WHITE);
  tft.drawLine(310, 0, 310, 240, ILI9341_WHITE);
  tft.drawLine(10, 40, 310, 40, ILI9341_WHITE);

  tft.setCursor(50, 26);
  tft.print("USA");
  tft.setCursor(150, 26);
  tft.print("Particulate Count");
  tft.setCursor(15, 60);
  tft.print("PM2.5 ug/m3");
  tft.setCursor(15, 120);
  tft.print("PM1.0 ug/m3");
  tft.setCursor(15, 185);
  tft.print("PM10  ug/m3");
  tft.setCursor(140, 70);
  tft.print(">0.3um");
  tft.setCursor(140, 100);
  tft.print(">0.5um");
  tft.setCursor(140, 130);
  tft.print(">1.0um");
  tft.setCursor(140, 160);
  tft.print(">2.5um");
  tft.setCursor(140, 190);
  tft.print(">5.0um");
  tft.setCursor(140, 220);
  tft.print(">10 um");
}
void updateEnviroDisplay() {
    //Displays the data for the start page.
    tft.setTextSize(2);
    tft.setFont(&FreeSans12pt7b);
    tft.setTextColor(YELLOW);

    //Particulate data output
    tft.fillRect(10,130,90,42, ILI9341_BLACK);
    tft.setCursor(10, 170);
    tft.print(enviro.concentrationPM1_0);
    tft.fillRect(110,130,90,42, ILI9341_BLACK);
    tft.setCursor(115, 170);
    tft.print(enviro.concentrationPM2_5);
    tft.fillRect(225,130,90,42, ILI9341_BLACK);
    tft.setCursor(225, 170);
    tft.print(enviro.concentrationPM10_0);

    tft.setTextSize(1);   //Temp and Humdity data output
    tft.fillRect(150,35,50,55, ILI9341_BLACK);
    tft.setCursor(150, 80);
    tft.print(enviro.temp_f, 1);
    tft.fillRect(240,55,40,28, ILI9341_BLACK);
    tft.setCursor(240, 80);
    tft.print(enviro.humdity_rel, 1);

    //AQI Data output
    tft.fillRect(10,210,120,28, ILI9341_BLACK);
    tft.setCursor(20, 227);
    tft.setFont(&FreeSans9pt7b);
    tft.setTextColor(enviro.AQIColor);
    tft.print(enviro.AQIString);
    tft.setTextSize(2);
    
    tft.fillRect(15,35,120,55, ILI9341_BLACK);
    tft.setTextColor(enviro.AQIColor, ILI9341_BLACK);
    tft.setCursor(20, 85);
    tft.setFont(&FreeSansBold18pt7b);
    tft.print(enviro.AQI);
  
}

void graphicDisplay() {
  //Draws graphics for startup page. 
  tft.fillScreen(ILI9341_BLACK);
  tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK); 
  tft.drawRect(0, 0, 320, 240, ILI9341_WHITE);
  tft.drawLine(0, 25, 320, 25, ILI9341_WHITE);
  tft.drawRect(1, 26, 140, 100, ILI9341_WHITE);
  tft.drawLine(0, 205, 320, 205, ILI9341_WHITE); //Air Quality Scale
  tft.drawLine(0, 125, 320, 125, ILI9341_WHITE);
  tft.drawLine(106, 125, 106, 205, ILI9341_WHITE); //PM Sensor Output
  tft.drawLine(212, 125, 212, 205, ILI9341_WHITE); 
  tft.drawLine(318, 125, 318, 205, ILI9341_WHITE);
  tft.drawLine(230, 25, 230, 125, ILI9341_WHITE);

  tft.fillRect(150,210,25,25, GREEN); //AQI Color Scale Index
  tft.fillRect(175,210,25,25, YELLOW);
  tft.fillRect(200,210,25,25, ORANGE);
  tft.fillRect(225,210,25,25, RED);
  tft.fillRect(250,210,25,25, PURPLE);
  tft.fillRect(275,210,25,25, MAROON);
  tft.drawRect(150, 210, 150, 25, ILI9341_WHITE);

  tft.setFont();
  tft.setTextSize(2);
  tft.setTextColor(ILI9341_BLUE);
  tft.setCursor(205, 55);
  tft.print("o");

  tft.setTextSize(1);
  tft.setCursor(65, 15);
  tft.setFont(&FreeSans12pt7b);
  tft.setTextColor(ILI9341_BLUE);
  tft.print("Air Quality Meter");
  tft.setCursor(5, 120);
  tft.print("AQI@2.5um");
  tft.setCursor(145, 120);
  tft.print("Temp F");
  tft.setCursor(235, 120);
  tft.print("RH");
  tft.setCursor(290, 75);
  tft.print("%");
  tft.setFont(&FreeSans9pt7b);
  tft.setCursor(3, 200);
  tft.print("PM 1.0");
  tft.setCursor(110, 200);
  tft.print("PM 2.5");
  tft.setCursor(215, 200);
  tft.print("PM 10");

}

unsigned long tempData() {
  //Retieves Temp and Humidty data and save to Data Strucure.
  unsigned long start = micros();

  // Get temperature event data.
  sensors_event_t event;  
  dht.temperature().getEvent(&event);
  if (isnan(event.temperature)) {
  }
  else {
    enviro.temp_c = event.temperature;
    enviro.temp_f = (enviro.temp_c * 9.0)/5.0 + 32.0;   
  }
  // Get humidity event data.
  dht.humidity().getEvent(&event);
  if (isnan(event.relative_humidity)) {
  }
  else {
    enviro.humdity_rel = event.relative_humidity;
  }
  
  return micros() - start;
}

void pmsData()
{
  //pms.wakeUp();
  //delay(5000);

  // Get data from PMS5003 Air Quality Sensor
  pms.requestRead();

  if (pms.readUntil(data))
  {
    // Loading data from sensor into the data structure.
    
    enviro.concentrationPM1_0 = data.PM_AE_UG_1_0;
    enviro.concentrationPM2_5 = data.PM_AE_UG_2_5;
    enviro.concentrationPM10_0 = data.PM_AE_UG_10_0;
    
    enviro.particle0_3 = data.Particle_0_3;
    enviro.particle0_5 = data.Particle_0_5;
    enviro.particle1_0 = data.Particle_1_0;
    enviro.particle2_5 = data.Particle_2_5;
    enviro.particle5_0 = data.Particle_5_0;
    enviro.particle10_0 = data.Particle_10_0;

    // Calculating the AQI @ 2.5um
    enviro.AQI = getAQI(0, enviro.concentrationPM2_5);
    
  }
  else
  {
    tft.setFont();
    tft.setCursor(110, 150);
    tft.println("No data");
  }

  //pms.sleep();
}

/*
 * AQI formula: https://en.wikipedia.org/wiki/Air_Quality_Index#United_States
 * Arduino code https://gist.github.com/nfjinjing/8d63012c18feea3ed04e
 * On line AQI calculator https://www.airnow.gov/index.cfm?action=resources.conc_aqi_calc
 */
float calcAQI(float I_high, float I_low, float C_high, float C_low, float C) {
  return (I_high - I_low) * (C - C_low) / (C_high - C_low) + I_low;
}

int getAQI(int sensor, float density) {
  int d10 = (int)(density * 10);
  if ( sensor == 0 ) {
    if (d10 <= 0) {
      return 0;
    }
    else if(d10 <= 120) {
      return calcAQI(50, 0, 120, 0, d10);
    }
    else if (d10 <= 354) {
      return calcAQI(100, 51, 354, 121, d10);
    }
    else if (d10 <= 554) {
      return calcAQI(150, 101, 554, 355, d10);
    }
    else if (d10 <= 1504) {
      return calcAQI(200, 151, 1504, 555, d10);
    }
    else if (d10 <= 2504) {
      return calcAQI(300, 201, 2504, 1505, d10);
    }
    else if (d10 <= 3504) {
      return calcAQI(400, 301, 3504, 2505, d10);
    }
    else if (d10 <= 5004) {
      return calcAQI(500, 401, 5004, 3505, d10);
    }
    else if (d10 <= 10000) {
      return calcAQI(1000, 501, 10000, 5005, d10);
    }
    else {
      return 1001;
    }
  } else {
    if (d10 <= 0) {
      return 0;
    }
    else if(d10 <= 540) {
      return calcAQI(50, 0, 540, 0, d10);
    }
    else if (d10 <= 1540) {
      return calcAQI(100, 51, 1540, 541, d10);
    }
    else if (d10 <= 2540) {
      return calcAQI(150, 101, 2540, 1541, d10);
    }
    else if (d10 <= 3550) {
      return calcAQI(200, 151, 3550, 2541, d10);
    }
    else if (d10 <= 4250) {
      return calcAQI(300, 201, 4250, 3551, d10);
    }
    else if (d10 <= 5050) {
      return calcAQI(400, 301, 5050, 4251, d10);
    }
    else if (d10 <= 6050) {
      return calcAQI(500, 401, 6050, 5051, d10);
    }
    else {
      return 1001;
    }
  } 
}  

void updateAQIDisplay(){
  /*
   * 1 EXCELLENT                    
   * 2 GOOD                         
   * 3 ACCEPTABLE               
   * 4 MODERATE            
   * 5 HEAVY               
   * 6 SEVERE
   * 7 HAZARDOUS
   */
     if(enviro.AQI == 0) {
        enviro.AQIString = EXCELLENT;
        enviro.AQIColor = GREEN;           
   } else if ( enviro.AQI <= 50 ) {
        enviro.AQIString = GOOD;
        enviro.AQIColor = GREEN;
    } else if ( enviro.AQI > 50 && enviro.AQI <= 100 ) {
        enviro.AQIString = ACCEPTABLE;
        enviro.AQIColor = YELLOW;
    } else if ( enviro.AQI > 100 && enviro.AQI <= 150 ) {
        enviro.AQIString = MODERATE;
        enviro.AQIColor = ORANGE;
    } else if ( enviro.AQI > 150 && enviro.AQI <= 200 ) {
        enviro.AQIString = HEAVY;
        enviro.AQIColor = RED;
    } else if ( enviro.AQI > 200 && enviro.AQI <= 300 ) {  
        enviro.AQIString = SEVERE;
        enviro.AQIColor = PURPLE;
    } else {    
       enviro.AQIString = HAZARDOUS;
       enviro.AQIColor = MAROON;
    }  
}
