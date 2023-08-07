#include <Arduino.h>
#include <SoftwareSerial.h>


#define VCC 13
#define RST 12
#define CLK 11
#define RX 2
#define TX 3
#define PRES 5

// création d'un objet Software Serial
SoftwareSerial serialCard (RX, TX);




String read_response() {
  String result = "";
  
  pinMode(TX, INPUT_PULLUP);  // Prevent signal collision.
  unsigned long first = millis();
  while(millis() - first < 500){  // If there's no incoming data for 500ms. Break.
    if(serialCard.available()){
      first = millis();
      byte c = serialCard.read();
      if(c < 0x10) result += '0';
      result += String(c, HEX) + ' ';
      Serial.println(String(c, BIN));
    }
  }
  
  //pinMode(RX, OUTPUT);
  if(result.length() < 2)
    Serial.println("No responses.");
  return result;

}


void card_activate () {
  digitalWrite(VCC, HIGH);
  pinMode(CLK, OUTPUT);
  delay(100);
  digitalWrite(RST, HIGH);


}

void card_desactivate() {
  digitalWrite(VCC, LOW);
  digitalWrite(RST, LOW);
  pinMode(CLK, INPUT);

}






void setup() {
  // Clock 4MHz
  pinMode(3, OUTPUT);
  //pinMode(11, OUTPUT);
  TCCR2A = _BV(COM2A0) | _BV(COM2B1) | _BV(WGM21) | _BV(WGM20);
  TCCR2B = _BV(WGM22) | _BV(CS20);
  OCR2A = 1;

  // Déclaration des Pin
  pinMode(PRES, INPUT_PULLUP);
  pinMode(VCC, OUTPUT);
  pinMode(RST, OUTPUT);

  //Ouverture liaisons séries 
  Serial.begin(9600);
  while (!Serial);
  while(!serialCard);
  serialCard.begin(10753);
  serialCard.listen();
  Serial.println("Prêt");
}

void loop() {
  // put your main code here, to run repeatedly:
  bool isntCardPresente = digitalRead(PRES); // carte présente = 0 & carte absente = 1
  
  while (isntCardPresente) {  // tant que carte absente, on n'alimente pas les contacteurs sur le lecteur : card_desactivate();
    isntCardPresente = digitalRead(PRES);
    //Serial.println("en attente");
    card_desactivate();
    
  };
  
  // on sort de la boucle donc une carte est présente
  Serial.println("Carte présente");
  card_activate ();
  Serial.print(read_response());

  while (!isntCardPresente) {
    isntCardPresente = digitalRead(PRES);
  }

}
