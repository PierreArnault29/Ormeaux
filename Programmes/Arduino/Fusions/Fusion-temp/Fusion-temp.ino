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
#include <LoRa.h>
#include <SPI.h>
#include <RH_RF95.h>

#define RFM95_CS 40     //chip select
#define RFM95_RST 41    // pins Lora
#define RFM95_INT 19    // interrupt

#define RF95_FREQ 915.0

// Singleton instance of the radio driver
RH_RF95 rf95(RFM95_CS, RFM95_INT);

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
byte sensorInterrupt = 5;  // 0 = digital pin 2
byte sensorPin       = 18; 

float calibrationFactor = 4.5;

volatile byte pulseCount;  


// The hall-effect flow sensor outputs approximately 4.5 pulses per second per
// litre/minute of flow.
float flowRate;
unsigned int flowMilliLitres;
unsigned long totalMilliLitres;

unsigned long oldTime;

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
  byte buf[6]; //tab
  float niveau=0.0;
  float temp=0.0;
  unsigned int f=1;
  unsigned int a;
  unsigned int crc=0;
  unsigned int ok=0;
  int b;
  unsigned int x=2;
  unsigned int y=0;
  int addr = 0; // save données dans la ROM
//PROTOCOLE PERSO LORA
#define ADRR 0
#define FCT 1
#define TEMP 2//3  //  b1 et b2 // b1 = partie entière, b2 =partie décimale
#define NIVEAU 4//5  // b2 et b3 idem
#define CRC 6 

const int csPin = 40;          // LoRa radio chip select
const int resetPin = 41;       // LoRa radio reset
const int irqPin = 20; 
 int counter=0;
void setup() {
  // Debugging output
 // pinMode(hallsensor, INPUT); //initializes digital pin 23 as an input
 // pinMode(RFM95_RST, OUTPUT);
 // digitalWrite(RFM95_RST, HIGH);
  Serial.begin(SERIAL_PORT);
  pinMode(sensorPin, INPUT);
  digitalWrite(sensorPin, HIGH);
  pulseCount        = 0;
  flowRate          = 0.0;
  flowMilliLitres   = 0;
  totalMilliLitres  = 0;
  oldTime           = 0;
  attachInterrupt(sensorInterrupt, pulseCounter, FALLING);
  
 // Serial.println("Arduino LoRa TX Test!");
    // manual reset
 // digitalWrite(RFM95_RST, LOW);
 // delay(10);
 // digitalWrite(RFM95_RST, HIGH);
 // delay(10);
 //   while (!rf95.init()) {
 //   Serial.println("LoRa radio init failed"); //initialisation si erreur
 //   while (1);
 // }
 /** Serial.println("LoRa radio init OK!"); //OKK

  // Defaults after init are 434.0MHz, modulation GFSK_Rb250Fd250, +13dbM
  if (!rf95.setFrequency(RF95_FREQ)) {
    Serial.println("setFrequency failed"); //frequence erreur
    while (1);
  }
  Serial.print("Set Freq to: "); Serial.println(RF95_FREQ);
  
  // Defaults after init are 434.0MHz, 13dBm, Bw = 125 kHz, Cr = 4/5, Sf = 128chips/symbol, CRC on

  // The default transmitter power is 13dBm, using PA_BOOST.
  // If you are using RFM95/96/97/98 modules which uses the PA_BOOST transmitter pin, then 
  // you can set transmitter powers from 5 to 23 dBm:
  rf95.setTxPower(23, false);

int16_t packetnum = 0;  // packet counter, we increment per xmission
**/
  Serial.println("Initialisation du programme");
  // set up the LCD's number of columns and rows: 
  lcd.begin(16, 2);

  // Print a message to the LCD. We track how long it takes since
  // this library has been optimized a bit and we're proud of it :)
  time = millis();
  time = millis() - time;
  Serial.print("Took "); Serial.print(time); Serial.println(" ms");
  a= EEPROM.read(addr);

}
int16_t packetnum = 0;  // packet counter, we increment per xmission
uint8_t i=0;


void loop() {   // DEBUT BOUCLE
    if((millis() - oldTime) > 1000)    // Only process counters once per second
  { 
    // Disable the interrupt while calculating flow rate and sending the value to
    // the host
    detachInterrupt(sensorInterrupt);
        
    // Because this loop may not complete in exactly 1 second intervals we calculate
    // the number of milliseconds that have passed since the last execution and use
    // that to scale the output. We also apply the calibrationFactor to scale the output
    // based on the number of pulses per second per units of measure (litres/minute in
    // this case) coming from the sensor.
    flowRate = ((1000.0 / (millis() - oldTime)) * pulseCount) / calibrationFactor;
    
    // Note the time this processing pass was executed. Note that because we've
    // disabled interrupts the millis() function won't actually be incrementing right
    // at this point, but it will still return the value it was set to just before
    // interrupts went away.
    oldTime = millis();
    
    // Divide the flow rate in litres/minute by 60 to determine how many litres have
    // passed through the sensor in this 1 second interval, then multiply by 1000 to
    // convert to millilitres.
    flowMilliLitres = (flowRate / 60) * 1000;
    
    // Add the millilitres passed in this second to the cumulative total
    totalMilliLitres += flowMilliLitres;
      
    unsigned int frac;
    
    // Print the flow rate for this second in litres / minute
    Serial.print("Flow rate: ");
    Serial.print(int(flowRate));  // Print the integer part of the variable
    Serial.print("L/min");
    Serial.print("\t");       // Print tab space

    // Print the cumulative total of litres flowed since starting
    Serial.print("Output Liquid Quantity: ");        
    Serial.print(totalMilliLitres);
    Serial.println("mL"); 
    Serial.print("\t");       // Print tab space
  Serial.print(totalMilliLitres/1000);
  Serial.print("L");
    

    // Reset the pulse counter so we can start incrementing again
    pulseCount = 0;
    
    // Enable the interrupt again now that we've finished sending output
    attachInterrupt(sensorInterrupt, pulseCounter, FALLING);
  }
  
 // debit();
  // print the number of seconds since reset:

    compteur=(millis()/1000);
while (compteur/60==idx){
    DS18B20_temperature = getTemperatureDS18b20();  // Actualisation toutes les 60s de l'acquisition de la temp
    envoitrame();
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
  lcd.print(flowRate); //Prints the number calculated above
  lcd.print (" L/m "); //Prints "L/hour" and returns a  new line
  
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
  lcd.print("Patientez...   ");
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
    envoitrame();
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
/**void debit(){
  NbTopsFan = 0;   //Set NbTops to 0 ready for calculations
  sei();      //Enables interrupts
  delay (1000);   //Wait 1 second
        //Disable interrupts
  Calc = (NbTopsFan/ 5.5); //(Pulse frequency) / 5.5Q, = flow rate 
} **/
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
void envoitrame(){
    delay(10);
    niveau=flowRate;
    temp=DS18B20_temperature;
    delay(10);
    buf[FCT]=byte(f);
    buf[ADRR]=byte(a);
    buf[CRC]=byte(crc);
  while (!Serial);

  Serial.println("LoRa Sender");
LoRa.setPins(csPin, resetPin, irqPin);
  if (!LoRa.begin(915E6)) {
    Serial.println("Starting LoRa failed!");
    while (1);
  }
  Serial.println(niveau);
      if (temp>25.5){
      buf[TEMP]=255;
      temp=temp*10;
      buf[TEMP+1]=byte(temp-buf[TEMP]);
      
    }
    else
      buf[TEMP]=byte(temp*10);
 /**  if (temp2>255){
      buf[TEMP]=255;
      buf[TEMP+1]=temp2-255;
      
    }
    else **/
      
      if (niveau>25.5){
      buf[NIVEAU]=255;
      niveau=niveau*10;
      buf[NIVEAU+1]=byte(niveau-buf[NIVEAU]);
      
    }
    else  
    buf[NIVEAU]=byte(niveau*10); 


  Serial.print("Sending packet: ");
  Serial.println(counter);

  // send packet
  LoRa.beginPacket();
  LoRa.write(buf,7);
  LoRa.endPacket();
}
 /*   Serial.println("Sending to rf95_server");
    String str1=String(a);
    String str2=String(Calc);
    String str3=String(DS18B20_temperature);
    String str=String(";")+str1+";"+str2+";"+str3;
    
    char buffer[30];
    char *tab = buffer;
    Serial.print("Taille du TAB: ");
    Serial.println(sizeof(tab)); 
    
    Serial.print("Taille: a -> ");
    Serial.println(sizeof(a));
        Serial.print("Taille: Temp -> ");
        Serial.println(sizeof(DS18B20_temperature));
            Serial.print("Taille: Niveau -> ");
            Serial.println(sizeof(Calc));
            
    char radiopacket[20] = "";
   
   str.toCharArray (radiopacket, 20); 
    itoa(packetnum++, radiopacket+13, 10);
    
    Serial.print("Envoi Donnees: "); Serial.println(radiopacket);
    radiopacket[19] = 0;
     Serial.println("Envoi en cours..."); delay(10);
  rf95.send((uint8_t *)radiopacket, 30);

  Serial.println("En attente du transfert..."); delay(10);
  rf95.waitPacketSent();
  // Now wait for a reply
  uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];
  uint8_t len = sizeof(buf);

  Serial.println("En attente de la reponse..."); delay(10);
  if (rf95.waitAvailableTimeout(1000))
  { 
    // Should be a reply message for us now   
    if (rf95.recv(buf, &len))
   {
      Serial.print("Message recu: ");
      Serial.println((char*)buf);
      Serial.print("RSSI: ");
      Serial.println(rf95.lastRssi(), DEC);    
    }
    else
    {
      Serial.println("Message non recu");
    }
  }
  else
  {
    Serial.println("No reply, is there a listener around?");
  }
  delay(1000);
  **/

  


void pulseCounter()
{
  // Increment the pulse counter
  pulseCount++;
}

