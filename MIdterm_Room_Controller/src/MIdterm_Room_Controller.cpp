/* 
 * Project: Midterm Room Controller
 * Desacription: Controll restroom environment 
 * Author: Elsmen Aragon
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
#include "IoTtimer.h"

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
const int PIXELCOUNT = 8;
const int BRI = 36;
//Wemo stuff
const int wemoHeat = 2;
const int wemoHumid = 4;
//default room conditions
float dTemp = 70.0;
float dHumid = 35.0;
//userButton1 conditions
float userTemp1 = 75.0;
float userHumid1 = 30.0;
int programState;
//userButton2 conditions
float userTemp2 = 80.0;
float userHumid2 = 25.0;
//Encoder variables
const int PINA = D9;
const int PINB = D8;
const int ENCBUTTONPIN = D16;
int encoderPosition;
int pixelPosition;
int positionMapped;
bool setReadTemp;

//HUE variables
const int BULB = 5;
int color;
int hueBrightness = 100;

//Delcare Functions
void defaultSettings(float defaultTemp, float defaultHumidity); 
void userButton1Conditions(float userTempB1, float userHumidB1);
void userButton2Conditions(float userTempB2, float userHumidB2);
void userChangeTemp(float tempRead);
void pixelFill(int start, int end, int color);

// Declare Objects
Adafruit_SSD1306 displayOled(OLED_RESET); // this is for I2C device OLED
Adafruit_BME280 myReading; //Defining bme280 object mine is called myReading
Adafruit_NeoPixel pixel(PIXELCOUNT, SPI1, WS2812B);
Button buttonSetting1(D3);
Button buttonSetting2(D4);
Button encoderSettingB3(ENCBUTTONPIN);
IoTTimer timerB1, timerB2, timerB3;
Encoder elsEncoder(PINB, PINA);

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
    pixel.setBrightness(BRI);
    pixel.show();
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
    displayOled.printf("   ROOM CONDITIONS\n\nTemp(F) = %0.2f\n R H(%) = %0.2f\n\n", tempF,humidRH);
    displayOled.display();
    displayOled.clearDisplay();
    
    if(buttonSetting1.isClicked()){
        programState = 1;
        timerB1.startTimer(120000);
    }
    if(buttonSetting2.isClicked()){
        programState = 2;
        timerB2.startTimer(300000);
    }
    if(encoderSettingB3.isClicked()){
        programState = 3;
        setReadTemp = true;
        timerB3.startTimer(60000);
    }
    Serial.printf("programState = %i\n", programState);
    switch (programState){
        case 0: 
            defaultSettings(dTemp, dHumid);
            break;
        case 1: 
            userButton1Conditions(userTemp1, userHumid1);
            break;
        case 2: 
            userButton2Conditions(userTemp2, userHumid2);
            break;
        case 3:
            encoderPosition = elsEncoder.read();
            //positionMapped = map(encoderPosition, 0,4,0,1); /Trying to map turns of encoder to move one position
            userChangeTemp(tempF); //do i need pixel position returned???
            break;
        default: 
            programState = 0;
            break;
    }     
}
//DefaultSettings function
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
    displayOled.printf(" -- D E F A U L T -- ");
    
}
void userButton1Conditions(float userTempB1, float userHumidB1){
    //timer ready then do set programState back to 0
    if(tempF < userTemp1){

        wemoWrite(wemoHeat, HIGH);
    }
    else{
        wemoWrite(wemoHeat, LOW);
    }
    if(humidRH > userHumid1){
        wemoWrite(wemoHumid, HIGH);
    }
    else{
        wemoWrite(wemoHumid, LOW);
    }
    displayOled.printf("--- U S E R  (1) ---   ");
    if(timerB1.isTimerReady()){
        programState = 0;
    }
}
void userButton2Conditions(float userTempB2, float userHumidB2){
  if(tempF < userTemp2){
        wemoWrite(wemoHeat, HIGH);
    }
    else{
        wemoWrite(wemoHeat, LOW);
    }
    if(humidRH > userHumid2){
        wemoWrite(wemoHumid, HIGH);
    }
    else{
        wemoWrite(wemoHumid, LOW);
    }
    displayOled.printf("--- U S E R  (2) ---   ");
    if(timerB2.isTimerReady()){
        programState = 0;
    }
}
void userChangeTemp(float tempRead){
    if(setReadTemp){
       elsEncoder.write(tempRead);
       setReadTemp = false;
    }    
    if(encoderPosition > 90){
        encoderPosition = 90;
        elsEncoder.write(encoderPosition);
    }
    if(encoderPosition < 40){
        encoderPosition = 40;
        elsEncoder.write(encoderPosition);
    }
    displayOled.printf("MANUAL OVERIDE\n");
    displayOled.printf("Temp: %i\n", encoderPosition);
    if(encoderPosition > tempRead){
        wemoWrite(wemoHeat, HIGH);
    }
    else{
        wemoWrite(wemoHeat, LOW);
    }
    if((int)tempRead == encoderPosition){
        setHue(BULB,true, HueRainbow[3],hueBrightness, 255);  
    }
    else{
        setHue(BULB,false, HueRainbow[3],hueBrightness, 255);
    }
    //Here is my pixel fill
    int numPixels = map(encoderPosition, 40, 90, 0, PIXELCOUNT);
    // Light up pixels up to numPixels
    for (int i = 0; i < PIXELCOUNT; i++) {
        if (i < numPixels) {
            // Set color for lit pixels
            pixel.setPixelColor(i, pixel.Color(255, 0, 0));
        } 
        else {
            // Turn off pixels beyond numPixels
            pixel.setPixelColor(i, pixel.Color(0, 0, 0));
        }
    }
    pixel.show();

    if(timerB3.isTimerReady()){
        programState = 0;
    }
    
}
    

