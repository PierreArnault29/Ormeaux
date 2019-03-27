/*********************

Example code for the Adafruit RGB Character LCD Shield and Library

This code displays text on the shield, and also reads the buttons on the keypad.
When a button is pressed, the backlight changes color.

**********************/

// include the library code:
#include <OneWire.h>
#include <Wire.h>
#include <Adafruit_RGBLCDShield.h>
#include <utility/Adafruit_MCP23017.h>
#include <EEPROM.h>


// The shield uses the I2C SCL and SDA pins. On classic Arduinos
// this is Analog 4 and 5 so you can't use those for analogRead() anymore
// However, you can connect other I2C sensors to the I2C bus and share
// the I2C bus.
Adafruit_RGBLCDShield lcd = Adafruit_RGBLCDShield();

// These #defines make it easy to set the backlight color
#define REVEILLE 0x1
#define VEILLE 0x2


// ---------- Initialisation des variables ---------------------

// Variables propres au YF-S201 (Debitmetre)
volatile int NbTopsFan; //measuring the rising edges of the signal
int Calc;                               
int hallsensor = 23;    //The pin location of the sensor
// Variables propres au DS18B20 (Sonde)
const int DS18B20_PIN=22;
const int DS18B20_ID=0x28;
// Déclaration de l'objet ds
OneWire ds(DS18B20_PIN); // on pin DS18B20_PIN

// Variables générales
float DS18B20_temperature=24;
const int SERIAL_PORT=9600;
int time,compteur;
int menu=0;
int bouton;
int idx=1;
int fenetre;
int t=0;
//globales variables addresse
  unsigned int a;
  unsigned int ok=0;
  int b;
  unsigned int x=2;
  unsigned int y=0;
  int addr = 0; // save données dans la ROM
  
void setup() {
  // Debugging output
  pinMode(hallsensor, INPUT); //initializes digital pin 23 as an input
  Serial.begin(SERIAL_PORT);
  Serial.println("Initialisation du programme");
  // set up the LCD's number of columns and rows: 
  lcd.begin(16, 2);

  // Print a message to the LCD. We track how long it takes since
  // this library has been optimized a bit and we're proud of it :)
  time = millis();
  time = millis() - time;
  Serial.print("Took "); Serial.print(time); Serial.println(" ms");
  a= EEPROM.read(addr);

  attachInterrupt(0, rpm, RISING); //and the interrupt is attached
}

uint8_t i=0;
void loop() {
  debit();
  // print the number of seconds since reset:

    compteur=(millis()/1000);
while (compteur/60==idx){
    DS18B20_temperature = getTemperatureDS18b20();  // Actualisation toutes les 60s de l'acquisition de la temp
    idx++;
  break;
}
while(bouton==1){
  t++;
  if(t==20){
   // ok=0;
}break;
t=0;
}

//debit();
switch(menu)
{
case 0:
  initialisation();
  break; 
//-------------MENU 1 Régler add Lora
case 1:
  lcd.setCursor(0, 0);
  lcd.print("Votre Add Lora: ");
  lcd.setCursor(0, 1);
  lcd.print("Ox");
  lcd.print(a);

  break;
 //-------------MENU 2 TEMP + DEBIT  
case 2: 
 
  lcd.setCursor(0, 0);
  lcd.print("Temp : ");
  lcd.print(DS18B20_temperature);
  lcd.print(" C");
  //debit();
  lcd.setCursor(0, 1);
  lcd.print("Debit: ");
  lcd.print(Calc, DEC); //Prints the number calculated above
  lcd.print (" L/min"); //Prints "L/hour" and returns a  new line
  
  break;

case 3:
  x=2;
  lcd.setCursor(0, 0);
  lcd.print("Entrer Add Lora: ");
  lcd.setCursor(0, 1);
  lcd.print("Ox");
  adresse();
  break;
case 4:
  lcd.setCursor(0, 0);
  lcd.print(" Reset en cours ");
  lcd.setCursor(0, 1);
  lcd.print("Patientez...");
  clearrom();
  lcd.clear();
  menu=3;
  break;
}

//-----------SWITCH MENUS---------------
  uint8_t buttons = lcd.readButtons();
  bouton = lcd.readButtons();
//RESET bouton

switch(bouton){
  case 1: // SELECT
      menu=4;

 /**      if(bouton==1&&t>20){
       ok==0;
       t=0;
       }
 /**     else{
      lcd.clear();
      menu++;
      if (menu>3){
        menu=0;
        }
 **/    
      
    break;
    
  case 2: // RIGHT
    //  lcd.setCursor(0, 1);
   // lcd.print("Right ");
    /** if (menu==0){
      x++;
    }**/
    menu++;
       if (menu>2){
        menu=1;
      }
    lcd.clear();
    break;
  case 4: // DOWN
    //lcd.setCursor(0, 1);
   // lcd.print("Down ");
     if (menu==3&&x==2&&ok==0){
    a--;
  }
    break;
  case 8:// UP
  //  lcd.setCursor(0, 1);
  //  lcd.print("UP   ");
  if (menu==3&&x==2&&ok==0){
    a++;
  }
    break;
  case 16: // LEFT
   // lcd.setCursor(0, 1);
    //lcd.print("Left");
   /**  if (menu==0){
      x--;
    } **/
    menu--;
    if (menu<1){
        menu=2;
      }

    lcd.clear();
    break;  
}


}
/* --------------- Acquisition de la température ----------------------------------- */
float getTemperatureDS18b20(){
byte i;
byte data[12];
byte addr[8];
float temp =0.0;

//Il n'y a qu'un seul capteur, donc on charge l'unique adresse.
ds.search(addr);

// Cette fonction sert à surveiller si la transmission s'est bien passée
if (OneWire::crc8( addr, 7) != addr[7]) {
Serial.println("getTemperatureDS18b20 : <!> CRC non valide! <!>");
return false;
}

// On vérifie que l'élément trouvé est bien un DS18B20
if (addr[0] != DS18B20_ID) {
Serial.println("L'équipement trouvé n'est pas un DS18B20");
return false;
}

// Demander au capteur de mémoriser la température et lui laisser 850ms pour le faire (voir datasheet)
ds.reset();
ds.select(addr);
ds.write(0x44);
delay(850);
// Demander au capteur de nous envoyer la température mémorisé
ds.reset();
ds.select(addr);
ds.write(0xBE);

// Le MOT reçu du capteur fait 9 octets, on les charge donc un par un dans le tableau data[]
for ( i = 0; i < 9; i++) {
data[i] = ds.read();
}
// Puis on converti la température (*0.0625 car la température est stockée sur 12 bits)
temp = ( (data[1] << 8) + data[0] )*0.0625;

return temp;
}

void adresse(){
  lcd.setCursor(x, 1);
  if(!a==0){
    if(a==100){
      a=0;
    }
    else if(a>99){
      a=99;
    }
    rom();
    lcd.print(a);
    lcd.print(" (1-99)");
  }
  else
  lcd.print("__ (1-99)");
  if (a<10&&!a==0){
  lcd.setCursor(x+1, 1);
  lcd.print(" ");
  }
  
}
void initialisation(){ 
  lcd.setCursor(1, 0);
  lcd.print("Initialisation");
  lcd.setCursor(6, 1);
  lcd.print("10%");
  delay(1000);
  lcd.setCursor(6, 1);
  lcd.print("20%");
  delay(1000);
    lcd.setCursor(6, 1);
    lcd.print("30%");
  delay(1000);
    lcd.setCursor(6, 1);
    lcd.print("40%");
  delay(1000);
    lcd.setCursor(6, 1);
    lcd.print("50%");
  delay(1000);
    lcd.setCursor(6, 1);
    lcd.print("100%");
  delay(1000);
  lcd.clear();
  menu=1;
/**  lcd.print(bouton);
  lcd.print("->");
  lcd.print(t);
  **/
}
void debit(){
  NbTopsFan = 0;   //Set NbTops to 0 ready for calculations
  sei();      //Enables interrupts
  delay (1000);   //Wait 1 second
        //Disable interrupts
  Calc = (NbTopsFan/ 5.5); //(Pulse frequency) / 5.5Q, = flow rate 
}
void rom(){
  
  b=byte(a);
  EEPROM.update(addr, b);
  delay(10);
  
}
void clearrom(){
    for (int i = 0 ; i < EEPROM.length() ; i++) {
     EEPROM.write(i, 0);
   }
}
void rpm ()     //This is the function that the interupt calls 
{ 
  NbTopsFan++;  //This function measures the rising and falling edge of the 
 
//hall effect sensors signal;
}
