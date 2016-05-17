

#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>

#include <Adafruit_SSD1306.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>

#include "FACTORIA_MAKER_LOGO.h"

/*
#if (SSD1306_LCDHEIGHT != 64)
#error("Altura incorrecta. Revisa Adafruit_SSD1306.h!");
#endif
*/
// SCL GPIO5
// SDA GPIO4
#define OLED_RESET 0  // GPIO0
Adafruit_SSD1306 display(OLED_RESET);


// Define un objeto para guardar los datos del DHT22
#define DHTTYPE DHT22
#define DHTPIN 2   // D4 GPIO2 Pin que usa la plaquita DHT22 de Wemos mini

DHT_Unified dht(DHTPIN, DHTTYPE);

uint32_t delayMS;


#define HALL_PIN 12 //D6
volatile byte __rpm;
unsigned long __lastmillis = 0;

void setup()   {
 
  Serial.begin(115200);
  
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);  // inicializa la pantalla I2C con la dirección 0x3C

  sensor_t sensor;

  sensor = setupDHT();
  setupRPM();

  
  splashScreen(sensor);
}


void loop() {

  delay(delayMS);
  sensors_event_t event_temperature;  
  sensors_event_t event_humidity;  
  dht.temperature().getEvent(&event_temperature);
  dht.humidity().getEvent(&event_humidity);

  mainScreen(event_temperature, event_humidity);


 
  Serial.println(getRPM());
}


void splashScreen(sensor_t temperature)
{
  display.clearDisplay();
  display.drawBitmap(32,16,FACTORIA_MAKER_LOGO,64,48,1);
  display.display();
  delay(2000);
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(32,16);
  display.println("Factoria");
  display.setCursor(32,26);
  display.println("Maker");
  display.setCursor(32,36);
  display.setTextSize(2);
  display.println(temperature.name);
  display.display();
  delay(2000);
  display.clearDisplay();  
};

void mainScreen(sensors_event_t event_temperature, sensors_event_t event_humidity)
{
  // Muestra los datos del DHT22 por la pantalla
  display.clearDisplay();
  display.setCursor(32, 16);
  display.setTextSize(0);
  display.println("Humedad");
  display.setCursor(32, 25);
  display.setTextSize(2);
  display.print(event_humidity.relative_humidity,1);
  display.println("%");
  display.setCursor(32, 40);
  display.setTextSize(0);
  display.println("Temperatura");
  display.setCursor(32, 50);
  display.setTextSize(2);
  display.print(event_temperature.temperature,1);
  display.write(247); // Muestra el símbolo de grado para la temperatura
  display.display();
};

int getRPM()
{
   if (millis() - __lastmillis > 1000){  /*Uptade every one second, this will be equal to reading frecuency (Hz).*/
     detachInterrupt(HALL_PIN);    //Disable interrupt when calculating
     int rpm = __rpm * 60;  /* Convert frecuency to RPM, note: this works for one interruption per full rotation. For two interrups per full rotation use rpmcount * 30.*/
     __rpm = 0; // Restart the RPM counter
     __lastmillis = millis(); // Uptade lasmillis
     attachInterrupt(HALL_PIN, tacho_ISR, FALLING);
     
     return rpm;
  }
  return 0;
}

void setupRPM()
{
 
  attachInterrupt(HALL_PIN, tacho_ISR, FALLING);
  
}

void tacho_ISR()
{
  __rpm++;
  
}


sensor_t setupDHT()
{
  sensor_t temperature;
  
  dht.temperature().getSensor(&temperature);
  delayMS = temperature.min_delay / 1000;  
  return temperature;
}
