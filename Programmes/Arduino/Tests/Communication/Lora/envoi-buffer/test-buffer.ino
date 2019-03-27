unsigned int a=10;
float temp=24.2;
byte buf[20];

#define ADRR 0
#define TEMP 1
    
void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);

    buf[ADRR]=byte(a);
    buf[TEMP]=byte(temp*10);
    Serial.print("Taille du TAB: ");
    Serial.println(sizeof(buf)); 
    Serial.print("Data: "); 
    Serial.println(byte(buf[0]));
    Serial.println(buf[1]);
    delay(100);
}

void loop() {
  // put your main code here, to run repeatedly:
  


}
