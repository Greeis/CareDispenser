/*
 * Caixa de remedios semi-inteligente - Care Dispenser
 * Novembro/2018
 * 
 * Acessórios e componentes:
 *  celular com comunicacao bluetooth
 *  processador Arduino UNO
 *  RTC - relogio em tempo real DS3231
 *  bluetooth HC 06
 *  I2C para display
 *  display 16 x 2
 *  servo motores SG 90
 *  fonte 5 Volts
 *  resistores, potenciometro,LED's, botoes NA, campainha e cabos de ligacao
 *  caixa de acondicionamento com tampas moveis
  
 * Pinagem do Arduino:
 * 0 - reservado Rx do monitor
 * 1 - reservado Tx do monitor
 * 2 - LED1 (aviso de alarme porta 1) - mr
 * 3  - LED2 (aviso de alarme porta 2) - az
 * 4  - SV1 (servo porta 1) - bc
 * 5  - SV2 (servo porta 2) - pt
 * 6  - botao1 (abertura manual e desliga alarme) - rx
 * 7  - botao2 (abertura manual e desliga alarme) - bc
 * 8  - sensor1 de coleta da porta1 - vm
 * 9  - sensor2 de coleta da porta2 - vd
 * 10 - TX do bluetooth - am
 * 11 - RX do bluetooth - az
 * 12 - buzzer do som de alarme
 * 13 - 
 * A0 - 
 * A1 - 
 * A2 - 
 * A3 - 
 * A4 - SDA do RTC e display - rx
 * A5 - SLC do RTC e display - az
*/

//################# DEFINICOES E INICIALIZACOES GERAIS ########################## 

//---------------------------------------------------------------------- Bluetooth
#include <SoftwareSerial.h>       //carrega a biblioteca SoftwareSerial
SoftwareSerial bluetooth(11,10);  //Pinos da serial  (RX, TX respectivamente)

//----------------------------------------------------- RTC - Relogio em Tempo Real
#include <DS3231.h>       //carrega a biblioteca do RTC
DS3231  rtc(SDA, SCL);    //pinos RTC (hardware A4 e A5; verificar inversao)

//----------------------------------------------------------------------LCD Display
#include <LiquidCrystal_I2C.h>    //hardware, pinos A4 e A5; verificar inversao
LiquidCrystal_I2C lcd(0x27,16,2); //"27" ou "3F" (pesquisar)

//-------------------------------------------------------------------Servo motores
#include <Servo.h>   //carrega biblioteca  para o servo 
Servo sv1;           //servo sg90 - pino 4, para porta1
Servo sv2;           //servo sg90 - pino 5, porta2

//angulos de abertura dos servos
byte aberto1=20;    
byte fechado1=70;   
byte aberto2=55;    
byte fechado2=20; 

//---------------------------------------------------------------Variaveis Globais
String ALARM1        = "10:00";                                //alarme da porta 1
String ALARM2        = "22:00";                                //atarme da porta 2
String RECARGA       = "08:00";                               //horario da recarga

int unsigned espera1 = 0;
int unsigned espera2 = 0;
int unsigned esperaR = 0;

String FLAG = "ffn1nn";                                 //sinalizacoes e condicoes
/*  0 - porta1         a/f (aberta/fechada)
    1 - porta2         a/f (aberta/fechada)
    2 - recarga        s/n (em recarga ou nao)
    3 - proximo alarme 1/2 
    4 - alarme1        s/n (ativo ou nao)
    5 - alarme2        s/n (ativo ou nao)
*/








//##################################### SETUP ######################################## 
void setup(){
  Serial.begin (115200);       //RETIRAR ESTAS LINHAS NO CODIGO PRONTO ##########################################
  Serial.println("Iniciado...");
//-------------------------------------------------------------------------- Bluetooth
  bluetooth.begin(9600);       //Inicia bluetooth serial nas portas 10 e 11
//----------------------------------------------------- ---RTC - Relogio em Tempo Real
  rtc.begin();                 // Initializa RTC
  rtc.setTime(7, 00, 0);       //SOMENTE PARA TESTES
//------------------------------------------------------------------------ LCD display
  lcd.init();           //inicializa LCD
  lcd.backlight();      //luz do fundo
  lcd.clear();
  lcd.clear();
  lcd.print("APENAS PARA FINS");
  lcd.setCursor(0,1);
  lcd.print("DE DEMONSTRACAO ");
  delay(3000);
  lcd.clear();
  lcd.print(" CARE");
  lcd.setCursor(0,1);
  lcd.print("      DISPENSER ");
  delay(3000);
  lcd.clear();
  
//----------------------------------------------------------------------- Servo motores
  sv1.attach(4);              //servo sv1  - pino 4
  sv1.write(fechado1);        //fecha a porta 1
  sv2.attach(5);              //servo sv2 - pino 5
  sv2.write(fechado2);        //fecha a porta 2
//------------------------------------------------- Botoes, Sensores, Alarmes e Portas
  pinMode(2,OUTPUT);          //LED porta1
  pinMode(3,OUTPUT);          //LED porta2
                              //4 e 5 reservado aos servos
  pinMode(6,INPUT);           //botao1 da porta1
  pinMode(7,INPUT);           //botao2 da porta2
  pinMode(8,INPUT);           //sensor1
  pinMode(9,INPUT);           //sensor2
                              //10 e 11 reservado aos TX, RX
  pinMode(12,OUTPUT);         //buzzer do som de alarme
  digitalWrite(12,LOW);       //desliga som do alarme
  pinMode(13, OUTPUT);        //apaga o LED do Arduino
  digitalWrite(13,LOW);
}







//##################################### LOOP ######################################### 
void loop ()
{
//------------------------------------------------------ Mostrando horarios no Display  
  String HORA = "";
  HORA = (rtc.getTimeStr());    //pega horario do RTC em HH:MM:SS    
  HORA.remove(5);               //formata em HH:MM
  lcd.setCursor(0,0);           //mostra horario e recarga no display
  lcd.print(HORA);
  lcd.print("  R-> ");
  lcd.print(RECARGA);
  lcd.setCursor(0,1);          //mostra horario do alarme
  
  if (FLAG[3] == '1' && ALARM1 != "00:00")  //alarme 1 eh o proximo
  { lcd.print("Alarme ");lcd.print(FLAG[3]);lcd.print("-> ");
    lcd.print(ALARM1);
  }
  if (FLAG[3] == '2' && ALARM2 != "00:00")  //alarme 2 eh o proximo  
  { lcd.print("Alarme ");lcd.print(FLAG[3]);lcd.print("-> ");
    lcd.print(ALARM2);
  }
//------------------------------------------------------------------------- Bluetooth
  if (bluetooth.available() > 0){  //==================== SE tem entrada do Bluetooth
    String AA = "";
    byte i;
    while(bluetooth.available() > 0)
      { AA = AA+(char)bluetooth.read(); }              //soma caracteres em "AA"
    Serial.print("Menu: " + AA);
    i = AA.length();
    if(i != 1)         //2 ou mais caracteres mostram o menu de opcoes
    {         
      bluetooth.println("AJUSTES:");
      bluetooth.println("   0 = Relogio");
      bluetooth.println("   1 = Alarme 1");
      bluetooth.println("   2 = Recarga");
    }
    if (AA == "0"){acertaRelogio();}  //acerta relogio
    if (AA == "1"){acertaAlarme1();}  //acerta alarme 1
    if (AA == "2"){acertaRecarga();}  //acerta horario de recarga da caixa
  } //================================================== FIM da entrada via bluetooth

}








//################################## SUB ROTINAS ##################################### 
//-------------------------------------------------------------------- Acertar Relogio

void acertaRelogio()  
{
  String AA = "";
  bluetooth.println("Horario do relogio em HHMM");
  lcd.clear();
  lcd.print("Relogio (HH:MM) ");
  while(!bluetooth.available() > 0){true;}               //espera entrada via bluetooth 
  while(bluetooth.available() > 0)
  { AA = AA+(char)bluetooth.read();}                          //soma caracteres em "AA"
  Serial.println("   Relogio: " + AA);
  byte i=AA.length();  //conta os caracteres
  if (i==5)                                                 //aceita apenas 5 caracteres
  {
    //Separa as horas e minutos
    String HR="";HR=HR+AA.charAt(0)+AA.charAt(1);            //os primeiros dois digitos
    String MR="";MR=MR+AA.charAt(3)+AA.charAt(4);              //os dois ultimos digitos
    
    byte hr=HR.toInt();
    byte mr=MR.toInt();
    
    if (hr<24 && mr<60){
      rtc.setTime(hr, mr, 0);                           //altera o horario do relogio RTC  
      AA = HR + ":" + MR;                                           //AA no formato HR:MR
      Serial.println(AA);
    }
    else {
      bluetooth.println("Formato invalido (HH:MM)");
      lcd.setCursor(0,1);
      lcd.print("Horario Invalido");
      delay(1000);
    }
  }else{
    bluetooth.println("Formato invalido (HH:MM)");
    lcd.setCursor(0,1);
    lcd.print("Horario Invalido");
    delay(1000);
  }
}

//--------------------------------------------------------------------------- Alarme 1

void acertaAlarme1()  
{
  String h1;
  bluetooth.println("Informe o horário do alarme em HH:MM");
  lcd.clear();
  lcd.print("Alarme 1 (HH:MM)");
  while(!bluetooth.available() > 0){true;}             //espera entrada via bluetooth 
  while(bluetooth.available() > 0)
  { h1 = h1+(char)bluetooth.read();}                        //soma caracteres em "h1"
  Serial.println("   Alarme 1 " + h1);
  byte i=h1.length();  //conta os caracteres
  if (i==5)                                              //aceita apenas 5 caracteres
  {
    //Separa as horas e minutos
    String HR="";HR=HR+h1.charAt(0)+h1.charAt(1);         //os primeiros dois digitos
    String MR="";MR=MR+h1.charAt(3)+h1.charAt(4);           //os dois ultimos digitos
 
    byte hr=HR.toInt();
    byte mr=MR.toInt();
    
    if (hr<24 && mr<60){
      ALARM1 = HR + ":" + MR;                                //ALARM1 no formato HR:MR
      //----------------------------------------------------------- Ajuste do Alarme 2
      hr = hr + 12;                                              //acrescenta 12 horas
      if (hr >= 24) { hr = hr - 24;}
      HR = String(hr);                                           //retorna para string
      i=HR.length();                                      //verifica se passou 0 horas
      if (i<2){HR="0" + HR;}                                   //verifica dois digitos
      ALARM2 = HR + ":" + MR;                                    //determinado alarme2
      Serial.println("Alarme 2: " + ALARM2);     
    }
    else {
      bluetooth.println("Formato invalido (HH:MM)");
      lcd.setCursor(0,1);
      lcd.print("Horario Invalido");
      delay(1000);
    }
  }else{
    bluetooth.println("Formato invalido (HH:MM)");
    lcd.setCursor(0,1);
    lcd.print("Horario Invalido");
    delay(1000);
  }
}

//-------------------------------------------------------------------- Acertar Recarga
void acertaRecarga()  
{
  String r;
  bluetooth.println("Informe o horário de Recarga em HH:MM");
  lcd.clear();
  lcd.print("Recarga  (HH:MM)");
  while(!bluetooth.available() > 0){true;}              //espera entrada via bluetooth 
  while(bluetooth.available() > 0)
  { r = r+(char)bluetooth.read();}                             //soma caracteres em "r"
  Serial.println("  Recarga " + r);
  byte i=r.length();  //conta os caracteres
  Serial.println(i);
  if (i==5)                                                //aceita apenas 5 caracteres
  {
    //Separa as horas e minutos
    String HR="";HR=HR+r.charAt(0)+r.charAt(1);             //os primeiros dois digitos
    String MR="";MR=MR+r.charAt(3)+r.charAt(4);               //os dois ultimos digitos
    
    byte hr=HR.toInt();
    byte mr=MR.toInt();
    
    if (hr<24 && mr<60){
      RECARGA = HR + ":" + MR;                               //RECARGA no formato HR:MR    
    }
    else {
      bluetooth.println("Formato invalido (HH:MM)");
      lcd.setCursor(0,1);
      lcd.print("Horario Invalido");
      delay(1000);
    }
  }else{
    bluetooth.println("Formato invalido (HH:MM)");
    lcd.setCursor(0,1);
    lcd.print("Horario Invalido");
    delay(1000);
  }
}
