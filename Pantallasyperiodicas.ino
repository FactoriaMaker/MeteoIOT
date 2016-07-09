//------ BIBLIOTECAS ------------
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

//----------------- VARIABLES GLOBALES --------------------------
// -------- Encoder --------
int POS;
int PUL;

bool evento_pos;
bool evento_pul;

bool lpul;
bool lena;

// -------- Pantallas -------
int pantalla;
int lpantalla;
int opcion;
int lopcion;

// ----------------------- Menus y submenus ------------------------------
// La pantalla es de 64x48 pixeles
// El tamaño a utilizar de letra es el más pequeño (5x7 pixeles)
// El máximo de caracteres representables en horizontal son:
// (5+1) = 6 ---> 64/6 = 10,6666 pero si le restamos el triángulo de 3 pixeles horizontales
// y el espacio en blanco...
// 64 - 3 - 1 = 60/6 = 10 caracteres en horizontal
// En vertical, 48/(7+1) = 6 líneas de texto.
// Creamos una matriz de 6x6 con cadenas de 10 caracteres por ejemplo
// de esta forma podemos tener acceso a 25 pantallas a traves de 5 menus con 5 submenus.
// La estructura quedaría de la siguiente manera:
//
//  MENU PRINCIPAL          SUBMENU           PANTALLA
//  (pantalla=0)          (pantalla=1)      (pantalla=6)
// ______________         ______________    ____________
// | PRINCIPAL  |         | VOLVER      |   |   Pan_06  | 
// |>Menu 1   --|----->   |>Pan_07    --|----->         | 
// | Menu 2     |         | Pan_08      |   |           | 
// | Menu 3     |         | Pan_09      |   |           |
// | Menu 4     |         | Pan_10      |   |           |
// | Menu 5     |         | Pan_11      |   |           |
// --------------         --------------    -------------

char menu[6][6][10] = {"PRINCIPAL","Menu 1","Menu 2","Menu 3","Menu 4","Menu 5",
                       "VOLVER","Pan_06","Pan_07","Pan_08","Pan_09","Pan_10",
                       "VOLVER","Pan_11","Pan_12","Pan_13","Pan_14","Pan_15",
                       "VOLVER","Pan_16","Pan_17","Pan_18","Pan_19","Pan_20",
                       "VOLVER","Pan_21","Pan_22","Pan_23","Pan_24","Pan_25",
                       "VOLVER","Pan_26","Pan_27","Pan_28","Pan_29","Pan_30"};


//----------------- CONSTANTES GLOBALES --------------------------
const unsigned long MAXUL = 4294967295UL; 

//----------------- DEFINE --------------------------
// El reset de la pantalla Oled esta en el pin 0
#define OLED_RESET 0 

#define ENA D3
#define ENB D6
#define SW D5

//-------------------- INSTACIAS --------------------------------------
// Instancia de la pantalla
Adafruit_SSD1306 display(OLED_RESET);


//-------------------- SETUP --------------------------------------
void setup ( void ) {

// Entradas digitales para el encoder rotativo
  pinMode(ENA, INPUT); // ENA
  pinMode(ENB, INPUT); // ENB
  pinMode(SW, INPUT); // Pulsador
  
  Serial.begin ( 115200 );

  Serial.println ( "" );
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);  // inicializa la pantalla I2C con la dirección 0x3C
  display.display();
  display.clearDisplay();  // Borra la pantalla

  // Al iniciar, muestra el mensaje
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  display.setTextSize(2);
  display.print("ESP");

  display.setCursor(0, 20);
  display.setTextSize(1);
  display.println("Prueba");
  display.println("Esp8266");
  display.display();
  
  delay(2000);
  display.clearDisplay();
  display.setTextSize(1);
}

// -------------- FUNCION ENCODER --------------------
// Se debe llamar siempre antes de las funciones
// que usen el encoder para poder leer el evento
// y usarlo cuando sea necesario
// --------------------------------------------------
void encoder() {
  bool n = digitalRead(ENA);
  PUL = !digitalRead(SW);
  evento_pos = LOW;
  evento_pul = LOW;
  if ((lena == LOW) && (n == HIGH)) {
    if (digitalRead(ENB) == LOW) {
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
  encoder();
  pantallas();
  periodicas();
}

//----------- VARIABLES GLOBALES --------------//


void pantallas(){


//  MENU PRINCIPAL          SUBMENU           PANTALLA
//  (pantalla=0)          (pantalla=1)      (pantalla=6)
// ______________         ______________    ____________
// | PRINCIPAL  |         | VOLVER      |   |   Pan_06  | 
// |>Menu 1   --|----->   |>Pan_07    --|----->         | 
// | Menu 2     |         | Pan_08      |   |           | 
// | Menu 3     |         | Pan_09      |   |           |
// | Menu 4     |         | Pan_10      |   |           |
// | Menu 5     |         | Pan_11      |   |           |
// --------------         --------------    -------------

// ------- Limitación de las opciones del menu y submenus ---------------
// Si no se usan todos los menus o submenus, se deben acotar para evitar problemas
  if (pantalla == 0) { if (POS < 2) { POS = 2; } if (POS > 6) { POS = 6; } }
  if (pantalla == 1) { if (POS < 1) { POS = 1; } if (POS > 6) { POS = 6; } }
  if (pantalla == 2) { if (POS < 1) { POS = 1; } if (POS > 6) { POS = 6; } }
  if (pantalla == 3) { if (POS < 1) { POS = 1; } if (POS > 6) { POS = 6; } }
  if (pantalla == 4) { if (POS < 1) { POS = 1; } if (POS > 6) { POS = 6; } }
  if (pantalla == 5) { if (POS < 1) { POS = 1; } if (POS > 6) { POS = 6; } }  

// Si ha habido cambio de pantalla o se a girado el encoder o se ha pulsado el encoder y estamos dentro de la zona de los menus y submenus entonces...
  if ((pantalla != lpantalla || evento_pos || evento_pul) && pantalla < 6) {

  // ______________     
  // | PRINCIPAL  | POS = 1      
  // |>Menu 1     | POS = 2 
  // | Menu 2     | POS = 3     
  // | Menu 3     | POS = 4     
  // | Menu 4     | POS = 5       
  // | Menu 5     | POS = 6    
  // --------------   

    // Si se ha pulsado en encoder y estamos en la pantalla de menu principal, saltamos a la pantalla que nos indica POS o el triángulo.
    if (evento_pul && PUL == HIGH && pantalla == 0) {
      pantalla = POS-1;
      evento_pul = LOW;  // Reconocemos el evento
    }

    // Si estámos en los Submenus y pulsamos sobre "VOLVER" volvemos a la pantalla 0 o Menú principal
    if (evento_pul && PUL == HIGH && pantalla > 0 && pantalla < 6 && POS == 1) {
      pantalla = 0;
      evento_pul = LOW; // Reconocemos el evento
    }

    // El siguiente codigo se ejecuta cada vez que hay cambio de pantalla o evento (subir o bajar el selector (triángulo)
    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(4, 0);    display.print(menu[pantalla][0]); // Escribe lo que hay en la matriz (PRINCIPAL)
    display.setCursor(4, 8);    display.print(menu[pantalla][1]); // Escribe lo que hay en la matriz (Menu 1)
    display.setCursor(4, 16);   display.print(menu[pantalla][2]); // Escribe lo que hay en la matriz (Menu 2)
    display.setCursor(4, 24);   display.print(menu[pantalla][3]); // Escribe lo que hay en la matriz (Menu 3)
    display.setCursor(4, 32);   display.print(menu[pantalla][4]); // Escribe lo que hay en la matriz (Menu 4)
    display.setCursor(4, 40);   display.print(menu[pantalla][5]); // Escribe lo que hay en la matriz (Menu 5)
    display.fillTriangle(0, 5+((POS-1)*8),                        
                         0, 1+((POS-1)*8),
                         2, 3+((POS-1)*8), WHITE);  // Dibuja el triángulo dependiendo de la posición del encoder
    display.display();
    lpantalla = pantalla;  // Recordamos la última pantalla
  }  

  // --------- LLAMADA A LAS PANTALLAS -------------------------
  // -----Llamada a la pantalla 6 (la primera que tenemos libre)
  // Ruta -> PRINCIPAL -> Menu 1 -> PAN_06
  // Si estamos en el Submenu 1 y pulsamos el encoder y estamos en la posición 2 (PAN_06) llamamos a la función correspondiente
  // En este caso solo llamamos una vez a la pantalla 6 (cuando entramos).
  if (pantalla == 1 && evento_pul && PUL == HIGH && POS == 2) {
    pantalla = 6;
    evento_pul = LOW;
    pan_06();
  }

  // Para salir de la pantalla a modo de escape y volver a la superior (al submenu) Simplemente hay que pulsar (esto se puede refinar)
  if (pantalla == 6 && evento_pul && PUL == HIGH) {
    pantalla = 1;
    evento_pul = LOW;
  }

  //------------------ Pantalla solo ejecuta una vez al entrar -------------------------------
  // Por ejemplo, para ver datos que no varian IP, SSID, Parametros...
  
  // Entrar en la pantalla 07
  if (pantalla == 1 && evento_pul && PUL == HIGH && POS == 3) { pantalla = 7; evento_pul = LOW; pan_07(); }
  
  // Salir de la pantalla 07
  if (pantalla == 7 && evento_pul && PUL == HIGH) { pantalla = 1; evento_pul = LOW; }
  //------------------------------------------------------------------------------------------

  
  //------------------ Pantalla que se ejecuta siempre -------------------------------
  // Por ejemplo, para ver datos que si varian, analogicas, tiempos, temperaturas, etc...
  
  // Entrar en la pantalla 08
  if (pantalla == 1 && evento_pul && PUL == HIGH && POS == 4) { pantalla = 8; evento_pul = LOW; pan_08(); }

  // Refrescar la pantalla 08 siempre que pueda
  if (pantalla == 8) { pan_08(); }
  
  // Salir de la pantalla 08
  if (pantalla == 8 && evento_pul && PUL == HIGH) { pantalla = 1; evento_pul = LOW; }
  //-----------------------------------------------------------------------------------

  //Pantalla 09
  if (pantalla == 1 && evento_pul && PUL == HIGH && POS == 5) { pantalla = 9; evento_pul = LOW; pan_09(); }
  //if (pantalla == 09) { pan_09(); }
  if (pantalla == 9 && evento_pul && PUL == HIGH) { pantalla = 1; evento_pul = LOW; }

  //Pantalla 10
  if (pantalla == 1 && evento_pul && PUL == HIGH && POS == 6) { pantalla = 10; evento_pul = LOW; pan_10(); }
  //if (pantalla == 10) { pan_10(); }
  if (pantalla == 10 && evento_pul && PUL == HIGH) { pantalla = 1; evento_pul = LOW; }

  //Pantalla 11
  if (pantalla == 2 && evento_pul && PUL == HIGH && POS == 2) { pantalla = 11; evento_pul = LOW; pan_11(); }
  //if (pantalla == 10) { pan_11(); }
  if (pantalla == 11 && evento_pul && PUL == HIGH) { pantalla = 2; evento_pul = LOW; }

  //Pantalla 12
  if (pantalla == 2 && evento_pul && PUL == HIGH && POS == 3) { pantalla = 12; evento_pul = LOW; pan_12(); }
  //if (pantalla == 10) { pan_12(); }
  if (pantalla == 12 && evento_pul && PUL == HIGH) { pantalla = 2; evento_pul = LOW; }

  //Pantalla 13
  if (pantalla == 2 && evento_pul && PUL == HIGH && POS == 4) { pantalla = 13; evento_pul = LOW; pan_13(); }
  //if (pantalla == 10) { pan_13(); }
  if (pantalla == 13 && evento_pul && PUL == HIGH) { pantalla = 2; evento_pul = LOW; }

  //Pantalla 14
  if (pantalla == 2 && evento_pul && PUL == HIGH && POS == 5) { pantalla = 14; evento_pul = LOW; pan_14(); }
  //if (pantalla == 10) { pan_14(); }
  if (pantalla == 14 && evento_pul && PUL == HIGH) { pantalla = 2; evento_pul = LOW; }

  //Pantalla 15
  if (pantalla == 2 && evento_pul && PUL == HIGH && POS == 6) { pantalla = 15; evento_pul = LOW; pan_15(); }
  //if (pantalla == 10) { pan_15(); }
  if (pantalla == 15 && evento_pul && PUL == HIGH) { pantalla = 2; evento_pul = LOW; }

  //Pantalla 16
  if (pantalla == 3 && evento_pul && PUL == HIGH && POS == 2) { pantalla = 16; evento_pul = LOW; pan_16(); }
  //if (pantalla == 10) { pan_16(); }
  if (pantalla == 16 && evento_pul && PUL == HIGH) { pantalla = 3; evento_pul = LOW; }

  //Pantalla 17
  if (pantalla == 3 && evento_pul && PUL == HIGH && POS == 3) { pantalla = 17; evento_pul = LOW; pan_17(); }
  //if (pantalla == 10) { pan_17(); }
  if (pantalla == 17 && evento_pul && PUL == HIGH) { pantalla = 3; evento_pul = LOW; }

  //Pantalla 18
  if (pantalla == 3 && evento_pul && PUL == HIGH && POS == 4) { pantalla = 18; evento_pul = LOW; pan_18(); }
  //if (pantalla == 10) { pan_18(); }
  if (pantalla == 18 && evento_pul && PUL == HIGH) { pantalla = 3; evento_pul = LOW; }

  // ............

  //Pantalla 30
  if (pantalla == 5 && evento_pul && PUL == HIGH && POS == 6) { pantalla = 30; evento_pul = LOW; pan_30(); }
  //if (pantalla == 30) { pan_30(); }
  if (pantalla == 30 && evento_pul && PUL == HIGH) { pantalla = 5; evento_pul = LOW; }


}

void pan_06() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(4, 0);    display.print(menu[1][1]);  

  //Meter aqui el codigo a visualizar

  display.display();
}

void pan_07() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(4, 0);    display.print(menu[1][2]);  

  //Meter aqui el codigo a visualizar

  display.display();
}

void pan_08() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(4, 0);    display.print(menu[1][3]);  

  //Meter aqui el codigo a visualizar

  display.display();
}

void pan_09() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(4, 0);    display.print(menu[1][4]);  

  //Meter aqui el codigo a visualizar

  display.display();
}

void pan_10() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(4, 0);    display.print(menu[1][5]);  

  //Meter aqui el codigo a visualizar

  display.display();
}

void pan_11() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(4, 0);    display.print(menu[2][1]);  

  //Meter aqui el codigo a visualizar

  display.display();
}

void pan_12() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(4, 0);    display.print(menu[2][2]);  

  //Meter aqui el codigo a visualizar

  display.display();
}

void pan_13() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(4, 0);    display.print(menu[2][3]);  

  //Meter aqui el codigo a visualizar

  display.display();
}

void pan_14() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(4, 0);    display.print(menu[2][4]);  

  //Meter aqui el codigo a visualizar

  display.display();
}

void pan_15() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(4, 0);    display.print(menu[2][5]);  

  //Meter aqui el codigo a visualizar

  display.display();
}

void pan_16() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(4, 0);    display.print(menu[3][1]);  

  //Meter aqui el codigo a visualizar

  display.display();
}

void pan_17() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(4, 0);    display.print(menu[3][2]);  

  //Meter aqui el codigo a visualizar

  display.display();
}

void pan_18() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(4, 0);    display.print(menu[3][3]);  

  //Meter aqui el codigo a visualizar

  display.display();
}

//................


void pan_30() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(4, 0);    display.print(menu[5][5]);  

  //Meter aqui el codigo a visualizar

  display.display();
}





//------------ VARIABLES GLOBALES -------------------------
unsigned long LastTime1, LastTime2, LastTime3, LastTime4, LastTime5, LastTime6, LastTime7, LastTime8;

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





