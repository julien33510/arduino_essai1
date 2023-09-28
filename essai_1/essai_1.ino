#include <SoftwareSerial_hm.h>

#include <Arduino.h>



#define VCC 13
#define RST 12
#define CLK 11
#define RX 2
#define TX 3
#define PRES 5

//Variables
byte TS, T0, TA1, TB1, TC1, TD1, K;
byte TA2, TB2, TC2, TD2;
byte TA3, TB3, TC3, TD3 ;
byte TCK, T ;
String historicals_caracters;

bool isntCardPresente = digitalRead(PRES); //Variable de détection de la présence de la carte : carte présente = 0 & carte absente = 1



// création d'un objet Software Serial
SoftwareSerial_hm serialCard (RX, TX, false);


void analyse_atr(String ATR) {
  String _atr = ATR;
  _atr.replace(" ","");
  Serial.println("_atr = " + _atr);
  TS = stringToByte(_atr.substring(0, 2));  // [0, 2[
  T0 = stringToByte(_atr.substring(2, 4));  // [2, 4[
  
  //i représente la position des caractères dans l'ATR, commence à 4 juste après le caractère T0, chaque caractère est sur 2i soit 8bits
  // il servira à déterminer la position des caractères facultatif de l'ATR
  int i = 4 ;

  //Analyse T0
    //détermination de K, nb de caratères d'historique de l'ATR
    K = T0 & 0x0F ;

    // Présence de TA1 : sur le bit 4
    if((T0 & 0x10)) {
      TA1 = stringToByte(_atr.substring(i, i+2));
      i += 2; }
    // Peésence de TB1 sur le bit 5
    if((T0 & 0x20)) {
      TB1 = stringToByte(_atr.substring(i, i+2));
      i += 2; }
    // Peésence de TC1 sur le bit 6
    if((T0 & 0x40)) {
      TC1 = stringToByte(_atr.substring(i, i+2));
      i += 2; }
    // Peésence de TD1 sur le bit 7
    if((T0 & 0x80)) {
      TD1 = stringToByte(_atr.substring(i, i+2));
      i += 2; }

  //Caractères Tx2 
    if ((TD1 & 0xF0)) { //présence d'un bit 1 sur l'un des bits 4 à 7 alors présence de caractère Tx2
      // Présence de TA1 : sur le bit 4
      if((TD1 & 0x10)) {
      TA2 = stringToByte(_atr.substring(i, i+2));
      i += 2; }
      // Peésence de TB1 sur le bit 5
      if((TD1 & 0x20)) {
      TB2 = stringToByte(_atr.substring(i, i+2));
      i += 2; }
      // Peésence de TC1 sur le bit 6
      if((TD1 & 0x40)) {
      TC2 = stringToByte(_atr.substring(i, i+2));
      i += 2; }
      // Peésence de TD1 sur le bit 7
      if((TD1 & 0x80)) {
      TD2 = stringToByte(_atr.substring(i, i+2));
      i += 2;}
    }

  //Caractères Tx3
    if ((TD2 & 0xF0)) { //présence d'un bit 1 sur l'un des bits 4 à 7 alors présence de caractère Tx2
      // Présence de TA1 : sur le bit 4
      if((TD2 & 0x10)) {
      TA3 = stringToByte(_atr.substring(i, i+2));
      i += 2; }
      // Peésence de TB1 sur le bit 5
      if((TD2 & 0x20)) {
      TB3 = stringToByte(_atr.substring(i, i+2));
      i += 2; }
      // Peésence de TC1 sur le bit 6
      if((TD2 & 0x40)) {
      TC3 = stringToByte(_atr.substring(i, i+2));
      i += 2; }
      // Peésence de TD1 sur le bit 7
      if((TD2 & 0x80)) {
      TD3 = stringToByte(_atr.substring(i, i+2));
      i += 2;}
    }
  
  //Caractères historiques
  historicals_caracters = _atr.substring(i, i+(K*2));
  i = i+(K*2);
  
  //Caractères TCK
  TCK = stringToByte(_atr.substring(i, i+2));

  // Protocole T
  T = TD1 & 0x0F;



    
  

}

byte stringToByte(String stb) {
  String sub;
  char buf[3];
  stb.toCharArray(buf, 3);
  byte b = (byte)strtol(buf,0,16);
  return b;
}

String transmitAPDU_T0(String apdu) {

  String sub;
  char buf[2];

  pinMode(TX, OUTPUT); 
  serialCard.stopListening();
  for(int i = 0; i < apdu.length()+1; i += 2){
    sub = apdu.substring(i, i+1);
    sub.toCharArray(buf, 2);
    byte b = (byte)strtol(buf, 0, 16);
    serialCard.write_8E2(b);

  }
  //serialCard.listen();
  String response1 = read_response();

  return response1;


}




String read_response() {
  String result = "";
  
  pinMode(TX, INPUT_PULLUP);  // Prevent signal collision.
  serialCard.listen();
  unsigned long first = millis();
  while(millis() - first < 100){  // If there's no incoming data for 100ms. Break.
    if(serialCard.available()){
      first = millis();
      byte c = serialCard.read();
      if(c < 0x10) result += '0';
      result += String(c, HEX) + ' ';
    }
  }
  
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
  

  card_activate ();

  //récup de l'ATR
  String ATR = read_response();
  Serial.println("ATR = " + ATR);

  //Analyse de l'ATR
  analyse_atr(ATR);
  Serial.print("TS = ");
  Serial.println(TS, HEX);
  Serial.print("T0 = ");
  Serial.println(T0, HEX);
  Serial.print("Nb de caractères d'historique K : ");
  Serial.println(K, DEC);
  Serial.println(K);
  Serial.print("TA1 = ");
  Serial.println(TA1, HEX);
  Serial.println(TA1, BIN);
  Serial.print("TB1 = ");
  Serial.println(TB1, HEX);
  Serial.print("TC1 = ");
  Serial.println(TC1, HEX);
  Serial.print("TD1 = ");
  Serial.println(TD1, HEX);
  if ((TA2 & 0xFF)) {Serial.print("TA2 = ");Serial.println(TA2, HEX);};
  if ((TB2 & 0xFF)) {Serial.print("TB2 = ");Serial.println(TB2, HEX);};
  if ((TC2 & 0xFF)) {Serial.print("TC2 = ");Serial.println(TC2, HEX);};
  if ((TD2 & 0xFF)) {Serial.print("TD2 = ");Serial.println(TD2, HEX);};
  if ((TA3 & 0xFF)) {Serial.print("TA3 = ");Serial.println(TA3, HEX);};
  if ((TB3 & 0xFF)) {Serial.print("TB3 = ");Serial.println(TB3, HEX);};
  if ((TC3 & 0xFF)) {Serial.print("TC3 = ");Serial.println(TC3, HEX);};
  if ((TD3 & 0xFF)) {Serial.print("TD3 = ");Serial.println(TD3, HEX);};
  Serial.print("Caractères historiques = ");
  Serial.println(historicals_caracters);
  Serial.print("TCK = ");
  Serial.println(TCK, HEX);
  Serial.print("Protocole T = ");
  Serial.println(T, DEC);

  //Envoi première commande
  //String First_Com = "00A4040C020520";
  String First_Com = "00A40000023F00";
  String First_response = transmitAPDU_T0(First_Com);
  Serial.println(First_response);

  Serial.println(F_CPU);
  uint16_t bit_delay = (F_CPU / 10753) / 4;
  Serial.println(bit_delay);
  uint16_t _tx_delay = bit_delay - 15/4 ;
   Serial.println(_tx_delay);


  while (!isntCardPresente) {
    isntCardPresente = digitalRead(PRES);
  }

}
