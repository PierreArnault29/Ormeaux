unsigned int a=10;
unsigned int f=1;
unsigned int crc=0;
float temp=28.2;
byte buf[6];
float niveau=24.9;

#define ADRR 0
#define FCT 1
#define TEMP 2//3  //  b1 et b2 // b1 = partie entière, b2 =partie décimale
#define NIVEAU 4//5  // b2 et b3 idem
#define CRC 6   
void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
    buf[FCT]=byte(f);
    buf[ADRR]=byte(a);
    buf[CRC]=byte(crc);
  //  int temp2=temp*10;

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
    else */
       if (niveau>25.5){
      buf[NIVEAU]=255;
      niveau=niveau*10;
      buf[NIVEAU+1]=byte(niveau-buf[NIVEAU]);
      
    }
    else  
    buf[NIVEAU]=byte(niveau*10); 
    
    Serial.print("Taille du TAB: ");
    Serial.println(sizeof(buf)); 
    
    Serial.println("PROTOCOLE"); 
     Serial.print("Fonction: Ox");
    Serial.println(buf[FCT]);
    Serial.print("Adresse: Ox");
    Serial.println(buf[ADRR]);
    Serial.print("Temp: "); 
    Serial.print("[");
    Serial.print(buf[TEMP]);
    Serial.print("]");
    Serial.print("[");
    Serial.print(buf[TEMP+1]);
    Serial.println("]");
    
    Serial.print("Niveau: "); 
    Serial.print("[");    
    Serial.print(buf[NIVEAU]);
    Serial.print("]");
    Serial.print("[");    
    Serial.print(buf[NIVEAU+1]);
    Serial.println("]"); 
    Serial.print("CRC= ");
    Serial.println(buf[CRC]);    
    delay(100);
}

void loop() {
  // put your main code here, to run repeatedly:
  


}
