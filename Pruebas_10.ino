//------INI BIBLIOTECAS------------
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <WiFiUdp.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <dht.h>
#include <time.h>
#include "RTClib.h"
#include <Adafruit_ADS1015.h>
#include <DNSServer.h>
#include <WiFiManager.h>   

//----------------- VARIABLES GLOBALES --------------------------


unsigned int localPort = 2390;      // local port to listen for UDP packets
const char* ntpServerName = "time.nist.gov";
const int NTP_PACKET_SIZE = 48; //El formato de tiempo NTP esta en los 48 primeros bytes del mensaje 
byte packetBuffer[ NTP_PACKET_SIZE]; //buffer para retener paquetes que entran y salen

int POS;
bool evento_pos;
bool evento_pul;
int PUL;
int lpul;
bool lena;

struct
{
  uint32_t total;
  uint32_t ok;
  uint32_t crc_error;
  uint32_t time_out;
  uint32_t unknown;
} stat = { 0,0,0,0,0 };

char daysOfTheWeek[7][12] = {"Domingo", "Lunes", "Martes", "Miercoles", "Jueves", "Viernes", "Sabado"};

float humedad;
float temperatura;

//----------------- CONSTANTES GLOBALES --------------------------
const unsigned long MAXUL = 4294967295UL;   


//----------------- DEFINE --------------------------
// El sensor DHT22 está conectado al pin 2 del Wemos
#define DHT22_PIN 2
// El reset de la pantalla Oled esta en el pin 0
#define OLED_RESET 0 
#if defined(ARDUINO_ARCH_SAMD)
// for Zero, output on USB Serial console, remove line below if using programming port to program the Zero!
   #define Serial SerialUSB
#endif

#define ENA D3
#define ENB D6
#define SW D5
//----------------- DEFINE --------------------------

//--------------------INSTACIAS--------------------------------------
//IPAddress timeServer(129, 6, 15, 28); // time.nist.gov NTP server
IPAddress timeServerIP; // time.nist.gov NTP server address

// Una Instacia UDP nos permite enviar y recibir paquetes sobre UDP
WiFiUDP udp;

// Instacia para leer la temperatura y la humedad
dht DHT;

// Puerto 80 para comunicar con el Wemos
ESP8266WebServer server (80);

Adafruit_ADS1115 ads; 

// Instancia de la pantalla
Adafruit_SSD1306 display(OLED_RESET);

RTC_DS1307 rtc;
//--------------------INSTACIAS--------------------------------------

void drawGraph() {
  String out = "";
  char temp[100];
  out += "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\" width=\"400\" height=\"150\">\n";
  out += "<rect width=\"400\" height=\"150\" fill=\"rgb(250, 230, 210)\" stroke-width=\"1\" stroke=\"rgb(0, 0, 0)\" />\n";
  out += "<g stroke=\"black\">\n";
  int y = rand() % 130;
  for (int x = 10; x < 390; x+= 10) {
    int y2 = rand() % 130;
    sprintf(temp, "<line x1=\"%d\" y1=\"%d\" x2=\"%d\" y2=\"%d\" stroke-width=\"1\" />\n", x, 140 - y, x + 10, 140 - y2);
    out += temp;
    y = y2;
  }
  out += "</g>\n</svg>\n";

  server.send ( 200, "image/svg+xml", out);
}

void handleRoot() {
  char temp[500];
  int sec = millis() / 1000;
  int min = sec / 60;
  int hr = min / 60;

  int temperatur = int(temperatura);
  int humeda = int(humedad);
  int decimal_temp = int(temperatura*10)-(int(temperatura)*10);
  int decimal_hume = int(humedad*10)-(int(humedad)*10);

  snprintf ( temp, 500,

"<html>\
  <head>\
    <meta http-equiv='refresh' content='10'/>\
    <title>ESP8266 Demo</title>\
    <style>\
      body { background-color: #cccccc; font-family: Arial, Helvetica, Sans-Serif; Color: #000088; }\
    </style>\
  </head>\
  <body>\
    <h1>Temperatura y humedad</h1>\
    <p>Tiempo en marcha: %02d:%02d:%02d</p>\
    <p>Temperatura: %02d.%01d&ordm;C</p>\
    <p>Humedad relativa: %02d.%01d%c</p>\
    <img src=\"/test.svg\" />\ 
  </body>\
</html>",

    hr, min % 60, sec % 60, temperatur, decimal_temp, humeda, decimal_hume,37
  );
  server.send ( 200, "text/html", temp );
}

void handleNotFound() {
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += ( server.method() == HTTP_GET ) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";

  for ( uint8_t i = 0; i < server.args(); i++ ) {
    message += " " + server.argName ( i ) + ": " + server.arg ( i ) + "\n";
  }

  server.send ( 404, "text/plain", message );
}



void setup ( void ) {

  #ifndef ESP8266
    while (!Serial); // for Leonardo/Micro/Zero
  #endif

  WiFiManager wifiManager;

  wifiManager.autoConnect("AutoConnectAP");
 
  ads.begin();
  ads.setGain(GAIN_ONE);

// Encoder rotativo
  pinMode(ENA, INPUT); // ENA
  pinMode(ENB, INPUT); // ENB
  pinMode(SW, INPUT); // Pulsador
  
  Serial.begin ( 115200 );

  if (! rtc.begin()) {
    Serial.println("No se encuentra el RTC");
    while (1);
  }

  if (! rtc.isrunning()) {
    Serial.println("¡RTC no esta funcionando!");
    // following line sets the RTC to the date & time this sketch was compiled
 //   rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    // This line sets the RTC with an explicit date & time, for example to set
    // January 21, 2014 at 3am you would call:
    // rtc.adjust(DateTime(2014, 1, 21, 3, 0, 0));
  }
  
 // WiFi.begin ( ssid, password );
  Serial.println ( "" );
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);  // inicializa la pantalla I2C con la dirección 0x3C
  display.display();
  display.clearDisplay();  // Borra la pantalla

    // Al iniciar, muestra el mensaje
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  display.setTextSize(2);
  display.print("WIFI");

  display.setCursor(0, 20);
  display.setTextSize(1);
  display.println("Prueba");
  display.println("Wifi");
  display.display();

// Eliminada sin internet
// Wait for connection
//  while ( WiFi.status() != WL_CONNECTED ) {
//    delay ( 500 );
//    Serial.print ( "." );
//  }

  Serial.println ( "" );
  Serial.print ( "Connected to " );
//  Serial.println ( ssid );
  Serial.print ( "IP address: " );
  Serial.println ( WiFi.localIP() );

  if ( MDNS.begin ( "esp8266" ) ) {
    Serial.println ( "MDNS responder started" );
  }

  Serial.println("Starting UDP");
  udp.begin(localPort);
  Serial.print("Local port: ");
  Serial.println(udp.localPort());

  server.on ( "/", handleRoot );
  server.on ( "/test.svg", drawGraph );
  server.on ( "/inline", []() {
    server.send ( 200, "text/plain", "this works as well" );
  } );
  server.onNotFound ( handleNotFound );
  server.begin();
  Serial.println ( "HTTP server started" );
  
  delay(2000);
  display.clearDisplay();
  display.setTextSize(1);
}

void encoder() {
  bool n = digitalRead(ENA);
  PUL = !digitalRead(SW);
  evento_pos = LOW;
  evento_pul = LOW;
  if ((lena == LOW) && (n == HIGH)) {
    if (digitalRead(D6) == LOW) {
      POS--;
    } else {
      POS++;
    }
    evento_pos = HIGH;  
  } 
  lena = n;  
  if (PUL != lpul) {
    lpul = PUL;
    evento_pul = HIGH;
  }
}

void loop ( void ) {
  server.handleClient();
  encoder();
  pantallas();
  periodicas();
}

//----------- VARIABLES GLOBALES --------------//
int pantalla;
int lpantalla;
int opcion;
int lopcion;
char menu[6][6][10] = {"PRINCIPAL","Wifi","Analogica","Rtc","Dht22","Ntp",
                       "VOLVER","IP","SSID","Senal"," "," ",
                       "VOLVER","Valor A0","Valor A1","Valor A2","Valor A3"," ",
                       "VOLVER","Actual","Nuevo"," "," "," ",
                       "VOLVER","Valores"," "," "," "," ",
                       "VOLVER","Actual"," "," "," "," "};


void pantallas(){

  if (pantalla == 0) { if (POS < 2) { POS = 2; } if (POS > 6) { POS = 6; } }
  if (pantalla == 1) { if (POS < 1) { POS = 1; } if (POS > 4) { POS = 4; } }
  if (pantalla == 2) { if (POS < 1) { POS = 1; } if (POS > 5) { POS = 5; } }
  if (pantalla == 3) { if (POS < 1) { POS = 1; } if (POS > 3) { POS = 3; } }
  if (pantalla == 4) { if (POS < 1) { POS = 1; } if (POS > 2) { POS = 2; } }
  if (pantalla == 5) { if (POS < 1) { POS = 1; } if (POS > 2) { POS = 2; } }  

  
  if ((pantalla != lpantalla || evento_pos || evento_pul) && pantalla < 6) {
    if (evento_pul && PUL == HIGH && pantalla == 0) {
      pantalla = POS-1;
      evento_pul = LOW;
    }
    if (evento_pul && PUL == HIGH && pantalla > 0 && pantalla < 6 && POS == 1) {
      pantalla = 0;
      evento_pul = LOW;
    }
    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(4, 0);    display.print(menu[pantalla][0]);
    display.setCursor(4, 8);    display.print(menu[pantalla][1]);
    display.setCursor(4, 16);   display.print(menu[pantalla][2]);
    display.setCursor(4, 24);   display.print(menu[pantalla][3]);
    display.setCursor(4, 32);   display.print(menu[pantalla][4]);
    display.setCursor(4, 40);   display.print(menu[pantalla][5]);
    display.fillTriangle(0, 5+((POS-1)*8),
                         0, 1+((POS-1)*8),
                         2, 3+((POS-1)*8), WHITE);
    display.display();
    lpantalla = pantalla;
  }  
  if (pantalla == 1 && evento_pul && PUL == HIGH && POS == 2) {
    pantalla = 6;
    evento_pul = LOW;
    pantalla_ip();
  }

  if (pantalla == 6 && evento_pul && PUL == HIGH) {
    pantalla = 1;
    evento_pul = LOW;
  }

  if (pantalla == 1 && evento_pul && PUL == HIGH && POS == 3) {
    pantalla = 7;
    evento_pul = LOW;
    pantalla_ssid();
  }

  if (pantalla == 7 && evento_pul && PUL == HIGH) {
    pantalla = 1;
    evento_pul = LOW;
  }

  if (pantalla == 1 && evento_pul && PUL == HIGH && POS == 4) {
    pantalla = 8;
    evento_pul = LOW;
    pantalla_senal();
  }

  if (pantalla == 8) {
    pantalla_senal(); 
  }

  if (pantalla == 8 && evento_pul && PUL == HIGH) {
    pantalla = 1;
    evento_pul = LOW;
  }

  if (pantalla == 2 && evento_pul && PUL == HIGH && POS == 2) {
    pantalla = 11;
    evento_pul = LOW;
    pantalla_A0();
  }

  if (pantalla == 11) {
    pantalla_A0(); 
  }

  if (pantalla == 11 && evento_pul && PUL == HIGH) {
    pantalla = 2;
    evento_pul = LOW;
  }

  if (pantalla == 2 && evento_pul && PUL == HIGH && POS == 3) {
    pantalla = 12;
    evento_pul = LOW;
    pantalla_A1();
  }

  if (pantalla == 12) {
    pantalla_A1(); 
  }

  if (pantalla == 12 && evento_pul && PUL == HIGH) {
    pantalla = 2;
    evento_pul = LOW;
  }

  if (pantalla == 2 && evento_pul && PUL == HIGH && POS == 4) {
    pantalla = 13;
    evento_pul = LOW;
    pantalla_A2();
  }

  if (pantalla == 13) {
    pantalla_A2(); 
  }

  if (pantalla == 13 && evento_pul && PUL == HIGH) {
    pantalla = 2;
    evento_pul = LOW;
  }

  if (pantalla == 2 && evento_pul && PUL == HIGH && POS == 5) {
    pantalla = 14;
    evento_pul = LOW;
    pantalla_A3();
  }

  if (pantalla == 14) {
    pantalla_A3(); 
  }

  if (pantalla == 14 && evento_pul && PUL == HIGH) {
    pantalla = 2;
    evento_pul = LOW;
  }

  if (pantalla == 3 && evento_pul && PUL == HIGH && POS == 2) {
    pantalla = 15;
    evento_pul = LOW;
    pantalla_rtc();
  }

  if (pantalla == 15) {
    pantalla_rtc(); 
  }

  if (pantalla == 15 && evento_pul && PUL == HIGH) {
    pantalla = 3;
    evento_pul = LOW;
  }

  if (pantalla == 4 && evento_pul && PUL == HIGH && POS == 2) {
    pantalla = 16;
    evento_pul = LOW;
    pantalla_dht22();
  }

  if (pantalla == 16) {
    pantalla_dht22();
  }

  if (pantalla == 16 && evento_pul && PUL == HIGH) {
    pantalla = 4;
    evento_pul = LOW;
  }


  if (pantalla == 5 && evento_pul && PUL == HIGH && POS == 2) {
    pantalla = 17;
    evento_pul = LOW;
    pantalla_ntp();
  }


  if (pantalla == 17 && evento_pul && PUL == HIGH) {
    pantalla = 5;
    evento_pul = LOW;
  }

}

void pantalla_ip() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(4, 0);    display.print("Direccion");  
  display.setCursor(20, 8);   display.print("IP:");
  display.setCursor(0, 24);   display.print(WiFi.localIP());
  display.display();
}

void pantalla_ssid() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(4, 0);    display.print("SSID:");  
  display.setCursor(0, 16);   display.print(WiFi.SSID());
  display.display();
}

void pantalla_senal() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(4, 0);    display.print("Senal:");  
  display.setCursor(4, 16);   display.print("RSSI:");     display.print(WiFi.RSSI());
  display.setTextSize(2);
  display.setCursor(4, 24);   display.print(int(2.3256*WiFi.RSSI()+162.79)); display.print("%");
  display.display();
}

void pantalla_A0() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(4, 0);   display.print("Analogica");  
  display.setCursor(4, 8);   display.print("A0:");   
  display.setTextSize(1);
  display.setCursor(4, 16);   display.print(ads.readADC_SingleEnded(0));
  display.setCursor(4, 32);   display.print(ads.readADC_SingleEnded(0)*0.003799); display.print("%");
  display.display();
}

void pantalla_A1() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(4, 0);   display.print("Analogica");  
  display.setCursor(4, 8);   display.print("A1:");   
  display.setTextSize(1);
  display.setCursor(4, 16);   display.print(ads.readADC_SingleEnded(1));
  display.setCursor(4, 32);   display.print(ads.readADC_SingleEnded(1)*0.003799); display.print("%");
  display.display();
}

void pantalla_A2() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(4, 0);   display.print("Analogica");  
  display.setCursor(4, 8);   display.print("A2:");   
  display.setTextSize(1);
  display.setCursor(4, 16);   display.print(ads.readADC_SingleEnded(2));
  display.setCursor(4, 32);   display.print(ads.readADC_SingleEnded(2)*0.003799); display.print("%");
  display.display();
}

void pantalla_A3() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(4, 0);   display.print("Analogica");  
  display.setCursor(4, 8);   display.print("A3:");   
  display.setTextSize(1);
  display.setCursor(4, 16);   display.print(ads.readADC_SingleEnded(3));
  display.setCursor(4, 32);   display.print(ads.readADC_SingleEnded(3)*0.003799); display.print("%");
  display.display();
}

void pantalla_rtc() {
  DateTime now = rtc.now();
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(10, 0);   display.print("RTC:");  
  display.setCursor(4, 10);    

  display.print(now.hour(), DEC);
  display.print(':');
  
  if ( (now.minute() % 60) < 10 ) {
    display.print('0');
  }
  display.print(now.minute(), DEC);
  display.print(':');
  if ( (now.second() % 60) < 10 ) {
    display.print('0');
  }
  display.print(now.second(), DEC);

  display.setCursor(4, 20);
  display.print(daysOfTheWeek[now.dayOfTheWeek()]);
   
  display.setCursor(0, 30);
  if ( now.day() < 10 ) {
    display.print('0');
  }
  display.print(now.day(), DEC);
  display.print('/');
  if ( now.month() < 10 ) {
    display.print('0');
  }
  display.print(now.month(), DEC);
  display.print('/');
  display.print(now.year(), DEC);
  
  display.display();
}

void pantalla_dht22() {
  display.clearDisplay();
  display.setCursor(0, 0); display.print("DHT 22:");  
  display.setCursor(0, 10);
  display.print("Hum: ");
  display.print(humedad, 1);
  display.println("%");
  display.print("Tem: ");
  display.print(temperatura, 1);
  display.write(247); // Muestra el símbolo de grado para la temperatura
  display.display();
}

void pantalla_ntp() {
  display.clearDisplay();
  display.setCursor(0, 0); display.print("Ntp:");  
  ntp();
  display.display();
}
//----------- VARIABLES GLOBALES --------------//







//------------ VARIABLES GLOBALES -------------------------
unsigned long LastTime1;
unsigned long LastTime2;
unsigned long LastTime3;
unsigned long LastTime4;
unsigned long LastTime5;
unsigned long LastTime6;
unsigned long LastTime7;
unsigned long LastTime8;

void periodicas() {
  unsigned long T = millis();
  if (LastTime1+100 <= T)   {if (LastTime1 < MAXUL - 100)   {LastTime1 = T;} else {LastTime1 = MAXUL - T;} OB35();}
  if (LastTime2+200 <= T)   {if (LastTime2 < MAXUL - 200)   {LastTime2 = T;} else {LastTime2 = MAXUL - T;} OB36();}
  if (LastTime3+500 <= T)   {if (LastTime3 < MAXUL - 500)   {LastTime3 = T;} else {LastTime3 = MAXUL - T;} OB37();}
  if (LastTime4+1000 <= T)  {if (LastTime4 < MAXUL - 1000)  {LastTime4 = T;} else {LastTime4 = MAXUL - T;} OB38();}
  if (LastTime5+2000 <= T)  {if (LastTime5 < MAXUL - 2000)  {LastTime5 = T;} else {LastTime5 = MAXUL - T;} OB39();}
  if (LastTime6+5000 <= T)  {if (LastTime6 < MAXUL - 5000)  {LastTime6 = T;} else {LastTime6 = MAXUL - T;} OB40();}
  if (LastTime7+10000 <= T) {if (LastTime7 < MAXUL - 10000) {LastTime7 = T;} else {LastTime7 = MAXUL - T;} OB41();}
  if (LastTime8+20000 <= T) {if (LastTime8 < MAXUL - 20000) {LastTime8 = T;} else {LastTime8 = MAXUL - T;} OB42();}
}


//---------LLAMADAS FUNCIONES PERIODICAS --------------------------
void OB35() {
  // Se ejecuta cada 100ms

}

void OB36() {
  // Se ejecuta cada 200ms

}

void OB37() {
  // Se ejecuta cada 500ms

}

void OB38() {
  // Se ejecuta cada 1s
 

}

void OB39() {
  // Se ejecuta cada 2s
  int chk = DHT.read22(DHT22_PIN);
  humedad = DHT.humidity;
  temperatura = DHT.temperature; 
}

void OB40() {
  // Se ejecuta cada 5s

}

void OB41() {
  // Se ejecuta cada 10s

}

void OB42() {
  // Se ejecuta cada 20s

}
//---------LLAMADAS FUNCIONES PERIODICAS --------------------------


void ntp() {
  WiFi.hostByName(ntpServerName, timeServerIP); 
  sendNTPpacket(timeServerIP); // send an NTP packet to a time server
  delay(1000);

    int cb = udp.parsePacket();
 
    Serial.print("packet received, length=");
    Serial.println(cb);
    // We've received a packet, read the data from it
    udp.read(packetBuffer, NTP_PACKET_SIZE); // read the packet into the buffer

    //the timestamp starts at byte 40 of the received packet and is four bytes,
    // or two words, long. First, esxtract the two words:

    unsigned long highWord = word(packetBuffer[40], packetBuffer[41]);
    unsigned long lowWord = word(packetBuffer[42], packetBuffer[43]);
    // combine the four bytes (two words) into a long integer
    // this is NTP time (seconds since Jan 1 1900):
    unsigned long secsSince1900 = highWord << 16 | lowWord;
    Serial.print("Seconds since Jan 1 1900 = " );
    Serial.println(secsSince1900);

    // now convert NTP time into everyday time:
    Serial.print("Unix time = ");
    // Unix time starts on Jan 1 1970. In seconds, that's 2208988800:
    const unsigned long seventyYears = 2208988800UL;
    // subtract seventy years:
    unsigned long epoch = secsSince1900 - seventyYears;
    // print Unix time:
    Serial.println(epoch);

    DateTime reali = epoch;

    rtc.adjust(epoch + 7201L);

    // print the hour, minute and second:
    Serial.print("The UTC time is ");       // UTC is the time at Greenwich Meridian (GMT)
    Serial.print(((epoch  % 86400L) / 3600)+2); // print the hour (86400 equals secs per day)
    Serial.print(':');
    if ( ((epoch % 3600) / 60) < 10 ) {
      // In the first 10 minutes of each hour, we'll want a leading '0'
      Serial.print('0');
    }
    Serial.print((epoch  % 3600) / 60); // print the minute (3600 equals secs per minute)
    Serial.print(':');
    if ( (epoch % 60) < 10 ) {
      // In the first 10 seconds of each minute, we'll want a leading '0'
      Serial.print('0');
    }
    Serial.println(epoch % 60); // print the second

//    print the hour, minute and second:
    display.println("");       // UTC is the time at Greenwich Meridian (GMT)

    display.print((epoch  % 86400L) / 3600 + 2); // print the hour (86400 equals secs per day)
    display.print(':');
    if ( ((epoch % 3600) / 60) < 10 ) {
      // In the first 10 minutes of each hour, we'll want a leading '0'
      display.print('0');
    }
    display.print((epoch  % 3600) / 60); // print the minute (3600 equals secs per minute)
    display.print(':');
    if ( (epoch % 60) < 10 ) {
      // In the first 10 seconds of each minute, we'll want a leading '0'
      display.print('0');
    }
    display.println(epoch % 60); // print the second

    Serial.print(reali.year(), DEC);
    Serial.print('/');
    Serial.print(reali.month(), DEC);
    Serial.print('/');
    Serial.print(reali.day(), DEC);
    Serial.print(" (");
    Serial.print(daysOfTheWeek[reali.dayOfTheWeek()]);
    Serial.print(") ");
    Serial.print(reali.hour(), DEC);
    Serial.print(':');
    Serial.print(reali.minute(), DEC);
    Serial.print(':');
    Serial.print(reali.second(), DEC);
    Serial.println();
    
//  display.display();
}




unsigned long sendNTPpacket(IPAddress& address)
{
  Serial.println("sending NTP packet...");
  // set all bytes in the buffer to 0
  memset(packetBuffer, 0, NTP_PACKET_SIZE);
  // Initialize values needed to form NTP request
  // (see URL above for details on the packets)
  packetBuffer[0] = 0b11100011;   // LI, Version, Mode
  packetBuffer[1] = 0;     // Stratum, or type of clock
  packetBuffer[2] = 6;     // Polling Interval
  packetBuffer[3] = 0xEC;  // Peer Clock Precision
  // 8 bytes of zero for Root Delay & Root Dispersion
  packetBuffer[12]  = 49;
  packetBuffer[13]  = 0x4E;
  packetBuffer[14]  = 49;
  packetBuffer[15]  = 52;

  // all NTP fields have been given values, now
  // you can send a packet requesting a timestamp:
  udp.beginPacket(address, 123); //NTP requests are to port 123
  udp.write(packetBuffer, NTP_PACKET_SIZE);
  udp.endPacket();
}



