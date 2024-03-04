/* 
 * Project: Midterm Room Controller
 * Desacription: Controll restroom environment 
 * Author: Elsmen Arasgon
 * Date: 4-March-2024
 * For comprehensive documentation and examples, please visit:
 * https://docs.particle.io/firmware/best-practices/firmware-template/
 */

// Include Particle Device OS APIs
#include "Particle.h"
#include "Encoder.h"
#include "Button.h"
#include "neopixel.h"
#include "Colors.h"
#include "Adafruit_GFX.h"
#include "Adafruit_SSD1306.h"
#include "Adafruit_BME280.h"
#include "IoTClassroom_CNM.h"


//Declare Variables
const int OLED_RESET=-1;      //reset value
const int HEXADDRESS = 0X76;  //address for BME280
//BME280 Variable
float tempC;  //variablle to hold temp in Celsius
float pressPA; //variable to hold pressure in pascals
float humidRH; //variable to hold relative humidity
bool status;
float tempF; //variable that will convert tempC to temp in fahrenheit
float convertedPA; //variable that will hold the converted pressure from pascals to inches/Mercury (inHg)
//Neopixel Variable
const int PIXELCOUNT = 20;
const int BRI = 50;
//Wemo stuff
const int wemoHeat = 2;
const int wemoHumid = 4;
//default room conditions
float dTemp = 75;
float dHumid = 25;
//userButton1 conditions
float userTemp1 = 80.0;
float userHumid1 = 20.0;

//Delcare Functions
void defaultSettings(float defaultTemp, float defaultHumidity); 
void userButton1Conditions(float userButton1Temp, float userButton1Humidity);

// Declare Objects
Adafruit_SSD1306 displayOled(OLED_RESET); // this is for I2C device OLED
Adafruit_BME280 myReading; //Defining bme280 object mine is called myReading
Adafruit_NeoPixel pixel(PIXELCOUNT, SPI1, WS2812B);
Button buttonSetting1(D3);


// Let Device OS manage the connection to the Particle Cloud
SYSTEM_MODE(MANUAL);

void setup() {
    Serial.begin(9600);
    waitFor(Serial.isConnected, 10000);
    displayOled.begin(SSD1306_SWITCHCAPVCC, 0X3C);
    displayOled.display();
    delay(1000);
    displayOled.clearDisplay();
    displayOled.setTextSize(1);
    displayOled.setTextColor(WHITE);
    displayOled.setCursor(0,0);
    pixel.begin();
    status = myReading.begin(HEXADDRESS);
    if (status == false) {
        Serial.printf("BME280 at address 0x%02x x failed to start ", HEXADDRESS);
    }
    WiFi.on();  //wemo wifi on
    WiFi.clearCredentials();
    WiFi.setCredentials("IoTNetwork"); //use these credentials for the classroom network
    WiFi.connect();
    while(WiFi.connecting()){
        Serial.printf(".");
    }
    Serial.printf("\n\n");
}


void loop() {
    displayOled.setCursor(0,0);
    tempC = myReading.readTemperature();
    tempF = (tempC*9/5)+32;
    pressPA = myReading.readPressure();
    convertedPA = pressPA*0.00029530;
    humidRH = myReading.readHumidity();
    displayOled.printf("ROOM CONDITIONS\n\n Temperature = %0.3f\nHumidity = %0.3f\n", tempF,humidRH);
    displayOled.display();
    displayOled.clearDisplay();


    if(tempF < dTemp || humidRH > dHumid){
       defaultSettings(dTemp, dHumid);            
    }
}

void defaultSettings(float defaultTemp, float defaultHumidity){
    if(tempF<defaultTemp){
        wemoWrite(wemoHeat, HIGH);   
    }
    else{
        wemoWrite(wemoHeat, LOW);
    }
    if(humidRH > defaultHumidity){
        wemoWrite(wemoHumid, HIGH);
    }
    else{
        wemoWrite(wemoHumid, LOW);
    }
}
    
//void userButton1Conditions(float userButton1Temp, float userButton1Humidity)
