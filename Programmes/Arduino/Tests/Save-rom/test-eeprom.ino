#include <EEPROM.h>

void setup() {
  Serial.begin(9600);
  Serial.print(EEPROM.length());

  int valeur_lue_1;
  EEPROM.get(0, valeur_lue_1);
  Serial.print("Valeur 1 = ");
  Serial.println(valeur_lue_1);

  float valeur_lue_2;
  EEPROM.get(2, valeur_lue_2);
  Serial.print("Valeur 2 = ");
  Serial.println(valeur_lue_2);

}

void loop() {
} 
