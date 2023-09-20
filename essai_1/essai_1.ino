#include <Arduino.h>
#include <SoftwareSerial.h>


#define VCC 13
#define RST 12
#define CLK 11
#define RX 2
#define TX 3
#define PRES 5

// création d'un objet Software Serial
SoftwareSerial serialCard (RX, TX, false);


//Variable de détection de la présence de la carte
bool isntCardPresente = digitalRead(PRES); // carte présente = 0 & carte absente = 1


String transmitAPDU_T0(String apdu) {

  String sub;
  char buf[3];

  serialCard.stopListening();

  for(int i = 0; i < apdu.length()+1; i += 2){
    sub = apdu.substring(i, i+2);
    sub.toCharArray(buf, 3);
    byte b = (byte)strtol(buf, 0, 16);
    serialCard.write(b);
  }
  serialCard.listen();
  String response1 = read_response();

  return response1;


}




String read_response() {
  String result = "";
  
  pinMode(TX, INPUT_PULLUP);  // Prevent signal collision.
  unsigned long first = millis();
  while(millis() - first < 100){  // If there's no incoming data for 100ms. Break.
    if(serialCard.available()){
      first = millis();
      byte c = serialCard.read();
      if(c < 0x10) result += '0';
      result += String(c, HEX) + ' ';
    }
  }
  
  //pinMode(TX, OUTPUT);
  if(result.length() < 2) {
    Serial.println("No responses.");
  }
    
  return result;

}




void card_activate () {
  digitalWrite(VCC, HIGH);
  pinMode(CLK, OUTPUT);
  pinMode(RX, INPUT_PULLUP);
  pinMode(TX, INPUT_PULLUP);
  delay(1);
  digitalWrite(RST, HIGH);
  serialCard.stopListening();
  serialCard.listen();
  while(!serialCard.isListening());
  Serial.println("Card Activate");
}

void card_desactivate() {
  digitalWrite(VCC, LOW);
  digitalWrite(RST, LOW);
  pinMode(CLK, INPUT);
  serialCard.stopListening();
  

}






void setup() {
  // Clock 4MHz
  //pinMode(TX, OUTPUT);
  //pinMode(CLK, OUTPUT);
  TCCR2A = _BV(COM2A0) | _BV(COM2B1) | _BV(WGM21) | _BV(WGM20);
  TCCR2B = _BV(WGM22) | _BV(CS20);
  OCR2A = 1;

  // Déclaration des Pin
  pinMode(PRES, INPUT_PULLUP);
  pinMode(VCC, OUTPUT);
  pinMode(RST, OUTPUT);
  digitalWrite(VCC, LOW);
  digitalWrite(RST, LOW);

  //Ouverture liaisons séries 
  Serial.begin(9600);
  while (!Serial);
  serialCard.begin(10753);
  while (!serialCard);

  Serial.println("Prêt");

  
}

void loop() {
  // put your main code here, to run repeatedly:
  isntCardPresente = digitalRead(PRES); // carte présente = 0 & carte absente = 1
  
  while (isntCardPresente) {  // tant que carte absente, on n'alimente pas les contacteurs sur le lecteur : card_desactivate();
    isntCardPresente = digitalRead(PRES);
    //Serial.println("en attente");
    card_desactivate();
    
  };
  
  // on sort de la boucle donc une carte est présente
  //Serial.println("Carte présente");
  //if (serialCard.overflow()) {
  //      Serial.println("portOne overflow!");
  //  }
  //delay(1000);
  //card_desactivate();
  //delay(1000);
  card_activate ();


  String ATR = read_response();
  Serial.println("ATR = " + ATR);

  String _atr = ATR;
  _atr.replace(" ","");
  Serial.println("_atr = " + _atr);
  String ts = _atr.substring(0, 2);
  String t0 = _atr.substring(2, 4);
  String ta1 = _atr.substring(4, 6);
  Serial.println("ts = " + ts);
  Serial.println("t0 = " + t0);
  Serial.println("ta1 = " + ta1);

  // convertion en binaire
  byte buffer[t0.length()+1];
  t0.toCharArray(buffer, t0.length()+1);
  byte _t0 = (byte)strtol(buffer, 0, 16);
  Serial.println(_t0, BIN);

  //TA1 exist ?
  if((_t0 & 0x80)) {
    Serial.println("Présence de TA1");
  }

  /*
  serialCard.begin(125000);
  String GO_TO_DF = "00 A4 04 0C 06 FF 54 41 43";
  String EF_ICC = "00 02";
  String mess_apdu = GO_TO_DF + " " + EF_ICC ;
  //mess_apdu = mess_apdu.replace(" ","");
  Serial.println(mess_apdu);
  String result = "";
  result = transmitAPDU_T0(mess_apdu);
  */

  while (!isntCardPresente) {
    isntCardPresente = digitalRead(PRES);
  }

}
