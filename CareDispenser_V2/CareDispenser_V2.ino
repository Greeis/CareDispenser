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

byte porta1aberta = 0;        //porta1 aberta
byte porta2aberta = 0;        //porta2 aberta

long tEspera1 = 60000;        //tempo de espera de 30 minutos da porta1
long tempoInicial = 0;



byte tEspera2 = 0;        //tempo de espera de 30 minutos da porta2
byte tEsperaR = 0;
int unsigned hr;
int unsigned mr;
String AA ="";

byte proxAlarm    = 1;        //sinalizador do proximo alarme (1 ou 2)

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
//----------------------------------------------------------------------- Servo motores
  sv1.attach(4);              //servo sv1  - pino 4
  sv1.write(fechado1);        //fecha a porta 1
  sv2.attach(5);              //servo sv2 - pino 5
  sv2.write(fechado2);        //fecha a porta 2
//------------------------------------------------------------------------ LCD display
  lcd.init();           //inicializa LCD
  lcd.backlight();      //luz do fundo
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


//################################### FIM SETUP ###################################### 







































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
    PegaMenu();
  } //================================================== FIM da entrada via bluetooth

                //################# ALARMES ##################// 

//----------------------------------------------Quando o horario do alarme é atingido
  long tempoAtual = millis();
  if(HORA == ALARM1 && FLAG[4] == 'n')//--------------------------- Sinaliza alarme 1
  {
    FLAG[4] = 's';                                     //habilita abertura da porta 1
    digitalWrite(2,HIGH);                                               //acende LED1
    tone(12,600);                                                       //liga sirene
    Serial.println("Alarme 1: Chegou a hora de tomar o remedio!");
    lcd.clear();
    lcd.print("Chegou a hora de");
    lcd.setCursor(0,1);
    lcd.print("tomar o remedio!");
    delay (1000);
    lcd.clear();
  }
  
  if(HORA != ALARM1 && FLAG[4] == 's')   //--------- Cancela alarme 1 apos UM minuto
  {
    FLAG[4] = 'n';                                    //cancela habilitacao da porta
    digitalWrite(2,LOW);                                                 //apaga LED1
    noTone(12);                                                      //desliga sirene
    Serial.println("Medicacao NAO coletada");                       //comunica evento
    FLAG[3] = '2';
  }

  if(HORA == ALARM2 && FLAG[5] == 'n')//-------------------------- Sinaliza alarme 2
  {
    FLAG[5] = 's';                                    //habilita abertura da porta 2
    digitalWrite(3,HIGH);                                              //acende LED2
    tone(12,400);                                                      //liga sirene
    Serial.println("Alarme 2: Chegou a hora de tomar o remedio!");
    lcd.clear();
    lcd.print("Chegou a hora de");
    lcd.setCursor(0,1);
    lcd.print("tomar o remedio!");
    delay (1000);
    lcd.clear();
  }

  if(HORA != ALARM2 && FLAG[5] == 's')  //---------------------- cancela alarme 2 apos UM minuto
  {
    FLAG[5] = 'n';                                    //cancela habilitacao da porta
    digitalWrite(3,LOW);                                                 //apaga LED2
    noTone(12);                                                      //desliga sirene
    Serial.println("Medicacao NAO coletada");                       //comunica evento
    FLAG[3] = '1';
  }

  if(HORA == RECARGA && FLAG[2] == 'n')//-------------------------- Sinaliza Recarga
  {
    FLAG[2] = 's';                                    //habilita abertura da porta 2
    sv1.write(aberto1);                                              //abre a porta1
    delay(100);
    sv2.write(aberto2);                                              //abre a porta2
    digitalWrite(2,HIGH);                                              //acende LED1
    digitalWrite(3,HIGH);                                              //acende LED2
    tone(12,600);                                                      //liga sirene
    Serial.println("Chegou a hora da recarga");
    lcd.clear();
    lcd.print("Chegou a hora da");
    lcd.setCursor(0,1);
    lcd.print("recarga diaria! ");
    delay (1000);
    lcd.clear();
    lcd.print("Recarga diaria  ");
    lcd.setCursor(0,1);
    lcd.print("Caixa 2  -> Fim ");
    delay (3000);    
   }
  
  if(HORA != RECARGA && FLAG[2] == 's') //----------------------------- Cancela recarga
  {
    FLAG[2] = 'n';                                    //cancela sinalizacao de recarga
    noTone(12);                                                   //desliga sirene
    sv1.write(fechado1);                                            //fecha porta1
    delay(100); 
    sv2.write(fechado2);                                            //fecha porta2
    digitalWrite(2,LOW);                                              //apaga LED1
    digitalWrite(3,LOW);                                              //apaga LED2
    Serial.println("Não recarregou!");   
  }
  //================================================= SE os horarios estao habilitados:
  
  if(FLAG[4] == 's')//------- ------------------------------------ Atividades do Botão1 
  {
    if(digitalRead(6))                                             //SE botao1 acionado
    {
      sv1.write(aberto1);                                               //abre a porta1
    }
    if(digitalRead(8))                                             //SE sensor1 ativado
    {
      digitalWrite(2,LOW);                                                 //apaga LED1
      noTone(12);                                                      //desliga sirene
      Serial.println("Medicacao coletada");                           //comunica evento
      delay(3000);                                          
      sv1.write(fechado1);                                               //fecha porta1
      FLAG[3] = '2';
    }
  }

  if(FLAG[5] == 's') //------------------------------------------- Atividades do Botão2
  {
    if(digitalRead(7))                                             //SE botao2 acionado
    {
      sv2.write(aberto2);                                               //abre a porta2
    }
    if(digitalRead(9))                                             //SE sensor2 ativado
    {
      digitalWrite(3,LOW);                                                 //apaga LED2
      noTone(12);                                                      //desliga sirene
      Serial.println("Medicacao coletada");                           //comunica evento
      delay(3000);                                           
      sv2.write(fechado2);                                               //fecha porta2
      FLAG[3] = '1';
    }
  }

  if(FLAG[2] == 's') //------------------------------------------ Atividades da Recarga
  {
    if(digitalRead(9))                                             //SE botao2 acionado
    {
      noTone(12);                                                   //desliga sirene
      delay(3000); 
      sv1.write(fechado1);                                            //fecha porta1
      delay(100);
      sv2.write(fechado2);                                            //fecha porta2
      digitalWrite(2,LOW);                                              //apaga LED1
      digitalWrite(3,LOW);                                              //apaga LED2
      Serial.print("Recarregou!!");          
    }
  }
  
}//#################################### FIM LOOP ###################################### 










































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
  { AA = AA+(char)bluetooth.read();}                           //soma caracteres em "AA"
  Serial.println("   Relogio: " + AA);
  byte i=AA.length();  //conta os caracteres
  if (i==5)                                                //aceita apenas 5 caracteres
  {
    //Separa as horas e minutos
    String HR="";HR=HR+AA.charAt(0)+AA.charAt(1);            //os primeiros dois digitos
    String MR="";MR=MR+AA.charAt(3)+AA.charAt(4);              //os dois ultimos digitos
    
    byte hr=HR.toInt();
    byte mr=MR.toInt();
    
    if (hr<24 && mr<60){
      rtc.setTime(hr, mr, 0);                          //altera o horario do relogio RTC  
      AA = HR + ":" + MR;                                          //AA no formato HR:MR
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
  Serial.println("   Alarme 1: " + h1);
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

void CalcularTempoEspera()
{ 
  String MR="";                            //minutos
  MR=MR+AA.charAt(3)+AA.charAt(4);  //caracter 3 e 4 (MM)
  mr=MR.toInt();
  if(tEspera1 != 0)     
    { //contagem do tempo ..........................................................
        tEspera1 = mr + 30;                              //incia contagem 30 minutos
        if(tEspera1 > 59)
        {
            tEspera1 = tEspera1 - 59;
            if(tEspera1 == 0)
            {
                tEspera1 = 1;
            }
        }
    }
}

void PegaMenu(){
  String menu = "";
  byte i;
  while(bluetooth.available() > 0)
  { menu = menu+(char)bluetooth.read(); }              //soma caracteres em "AA"
  
  Serial.print("Menu: " + menu);
  i = menu.length();
  if(i != 1)         //2 ou mais caracteres mostram o menu de opcoes
  {
    bluetooth.println("AJUSTES:");
    bluetooth.println("   0 = Relogio");
    bluetooth.println("   1 = Alarme 1");
    bluetooth.println("   2 = Recarga");
  }
  if (menu == "0"){acertaRelogio();}  //acerta relogio
  if (menu == "1"){acertaAlarme1();}  //acerta alarme 1
  if (menu == "2"){acertaRecarga();}  //acerta horario de recarga da caixa 
}
