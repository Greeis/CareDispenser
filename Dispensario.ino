/*
 * Caixa de remedios semi-inteligente
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

//---------------------------------------------------------------------- bluetooth
#include <SoftwareSerial.h>       //carrega a biblioteca SoftwareSerial
SoftwareSerial denteAzul(11,10);  //Pinos da serial  (RX, TX respectivamente)
                                      //-verificar inversao dos cabos na pratica
//char acelera = 'x'; //estado de aceleracao do relogio real

//----------------------------------------------------- RTC - Relogio em Tempo Real
#include <DS3231.h>       //carrega a biblioteca do RTC
DS3231  rtc(SDA, SCL);    //pinos RTC (hardware A4 e A5; verificar inversao)
String HORA;              //coleta horario do RTC
//auxiliares em varias subrotinas interdependentes
  String HR;
  String MR;
  byte hr;
  byte mr;
  String AA ="";

//----------------------------------------------------------------------lcd display
#include <Wire.h>                 //carrega bibliotecas do lcd display
#include <LiquidCrystal_I2C.h>    //hardware, pinos A4 e A5; verificar inversao
LiquidCrystal_I2C lcd(0x27,16,2); //"27" ou "3F" (pesquisar)

//------------------------------------------------------------------ servo motores
#include <Servo.h>   //carrega biblioteca  para o servo 
Servo sv1;           //servo sg90 - pino 5, para porta1
Servo sv2;           //servo sg90 - pino 6, porta2
byte aberto1=68;     //angulos de abertura dos servos (EXPERIMENTAL)
byte fechado1=20;   //
byte aberto2=42;    //
byte fechado2=98;  //

//--------------------------------------------- botoes, sensores, alarmes e portas
byte porta1aberta = 0;        //porta1 aberta
byte porta2aberta = 0;        //porta2 aberta
byte tEspera1     = 0;        //tempo de espera de 30 minutos da porta1
byte tEspera2     = 0;        //tempo de espera de 30 minutos da porta2
String ALARM1     = "10:00";  //alarme da porta 1
String ALARM2     = "22:00";  //atarme da porta 2
String RECARGA    = "08:00";  //habilita recarga. Alterar com insercao de ALARM1
byte proxAlarm    = 1;        //sinalizador do proximo alarme (1 ou 2)

//------------------------------------------------------------------------- outros










void setup() {  //********************************************************** SETUP
  
Serial.begin (115200);  //RETIRAR ESTAS LINHAS NO CODIGO PRONTO ##########################################
Serial.println("...iniciado o 55");
//---------------------------------------------------------------------- bluetooth
  denteAzul.begin(9600);  //Inicia bluetooth serial nas portas 10 e 11
//----------------------------------------------------- RTC - Relogio em Tempo Real
  rtc.begin();         // Initializa RTC
  rtc.setTime(7, 00, 0);//###################################### SOMENTE PARA TESTES
//--------------------------------------------------------------------- lcd display
  lcd.init();           //inicializa LCD
  lcd.backlight();      //luz do fundo
  lcd.clear();
//------------------------------------------------------------------- servo motores
  sv1.attach(4);              //servo sv1  - pino 5
  sv1.write(fechado1);        //fecha a porta 1
  sv2.attach(5);              //servo sv2 - pino 6
  sv2.write(fechado2);        //fecha a porta 2
//---------------------------------------------- botoes, sensores, alarmes e portas
  pinMode(2,OUTPUT);          //LED porta1
  pinMode(3,OUTPUT);          //LED porta2
                              //5 e 6 reservado aos servos
  pinMode(6,INPUT);           //botao1 da porta1
  pinMode(7,INPUT);           //botao2 da porta2
  pinMode(8,INPUT);           //sensor1
  pinMode(9,INPUT);          //sensor2
  pinMode(12,OUTPUT);         //buzzer do som de alarme
  digitalWrite(12,LOW);       //desliga som do alarme
  pinMode(13, OUTPUT);        //apaga o LED do Arduino
  digitalWrite(13,LOW);

//======================================================== ACELERANDO RELOGIO
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
  /*lcd.print("Acelera relogio?");
  lcd.setCursor(0,1);
  lcd.print("1->sim    2->nao");

  while (acelera == 'x')
  {
    if (digitalRead(7)){acelera = 's';}
    if (digitalRead(8)){acelera = 'n';}
  }  
  delay(1000);  
 */
//=============================================== recarga ao iniciar o sistema
  /*denteAzul.println("CARREGUE AS PORTAS COM MEDICAMENTOS");
  denteAzul.println("e aperte o sensor 2 para fechar.");
  lcd.print("Possui doses?   ");
  lcd.setCursor(0,1);
  lcd.print("Porta 2->ENCERRA");
  //Serial.println("CARREGUE AS PORTAS COM MEDICAMENTOS");
  //Serial.println("LED's 1 e 2 acesos, sirene ligada e servos levantados");
  digitalWrite(3,HIGH);                             //acende o LED da porta1
  digitalWrite(4,HIGH);                             //acende o LED da porta2
  sv1.write(aberto1);                                        //abre a porta1
  sv2.write(aberto2);                                        //abre a porta2
  while(!digitalRead(10)){  }   //......................aguardando o sensor2
  sv1.write(fechado1);                                      //fecha a porta1
  sv2.write(fechado2);                                      //fecha a porta2
  digitalWrite(3,LOW);
  digitalWrite(4,LOW);
  //Serial.print("LED's apagado, portas carregadas e fechadas = ");
  //Serial.print(porta1aberta);Serial.println(porta2aberta);  
  delay(600);                              //tempo de resposta para soltar botao
  */
  lcd.clear();
  lcd.print("Alarme 1: ");lcd.print(ALARM1);
  lcd.setCursor(0,1);
  lcd.print("Alarme 2: ");lcd.print(ALARM2);
  delay(2000);lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Recarga: ");lcd.print(RECARGA);
  delay(2000);lcd.clear();
  
}  //**************************************************************** FIM DO SETUP










//===================================== ROTINAS DAS ABERTURAS DAS PORTAS HABILITADAS

void loop()  //#################################################³³############# LOOP
{

//=================================================== atualizando relogio no display
  mostraRelogio();  //atualiza display com horario, alarme
//============================================================ verificando bluetooth
  AA = "";
  if (denteAzul.available() > 0)      //comunica com bluetooth apenas SE tem entrada
  {
    char menu;
    while(denteAzul.available() > 0)
    {
      menu = denteAzul.read(); //caracter em 'menu'
      AA=AA+menu;  //--- junta os digitados em "AA"
    }
// temos AA com a string total dos caracteres digitados (futuras possibilidades)
// temos o ultimo (ou unico) caracter da entrada em 'menu'
    denteAzul.println("AJUSTES:");
    denteAzul.println("   0 = relogio");
    denteAzul.println("   1 = alarme 1");
    denteAzul.println("   2 = recarga");
    denteAzul.println("   3 = mostra os horarios"); 
    if (AA == "0"){acertaRelogio();}  //acerta relogio
    if (AA == "1"){acertaAlarme1();}  //acerta alarme 1
    if (AA == "2"){acertaRecarga();}  //acerta horario de recarga da caixa
    if (AA == "3"){mostraHorarios();}  //mostra os horarios programados
  } //recebeu entrada via bluetooth
//========================================================== ROTINAS DOS ALARMES 

  //  recarga dos medicamentos ***************************************************
  //----------------------------------verifica horario da recarga e abre as portas
  if (HORA == RECARGA  && porta1aberta == 0 && porta2aberta == 0) 
  {
    lcd.clear();
    lcd.print("Possui doses?   ");
    lcd.setCursor(0,1);
    lcd.print("Porta 2->ENCERRA");
    //denteAzul.println("CARREGUE AS PORTAS COM MEDICAMENTOS");
    //denteAzul.println("e aperte o botao 1 para fechar.");
    digitalWrite(2,HIGH);                             //acende o LED da porta1
    digitalWrite(4,HIGH);                             //acende o LED da porta2
    tone(12,300);                                               //ativa sirene
    sv1.write(aberto1);                                        //abre a porta1
    delay(500);
    sv2.write(aberto2);                                        //abre a porta2
    porta1aberta = 1;                                 //sinaliza porta1 aberta
    porta2aberta = 1;                                 //sinaliza porta2 aberta
    Serial.println("CARREGUE AS PORTAS COM MEDICAMENTOS");
    Serial.println("LED's 1 e 2 acesos, sirene ligada"); 
    delay(2000);
  }

  //----------------------------------------------- SE as portas estao abertas
  //............................................ fecha as portas com o botao 1
  if (porta1aberta == 1 && porta2aberta == 1 && digitalRead(9))                  
  { 
        sv1.write(fechado1);                                  //fecha a porta1
        delay(500);
        sv2.write(fechado2);                                  //fecha a porta2
        porta1aberta = 0;                            //sinaliza porta1 fechada
        porta2aberta = 0;                            //sinaliza porta2 fechada
        digitalWrite(2,LOW);                           //apaga o LED da porta1
        digitalWrite(3,LOW);                           //apaga o LED da porta2
        noTone(12);                                           //desliga sirene
        delay(600);                      //tempo de resposta para soltar botao
        //denteAzul.println("Portas carregadas e fechadas.");
        Serial.print("LED's apagado, portas carregadas e fechadas = ");
        Serial.print(porta1aberta);Serial.println(porta2aberta);
  }
  //.................................. fecha as portas se não houver recargas
  if(porta1aberta == 1 && porta2aberta == 1 && HORA == ALARM1)
  { 
    sv1.write(fechado1);                                        //fecha a porta1
    delay(500);
    sv2.write(fechado2);                                        //fecha a porta2
    porta1aberta = 0;
    porta2aberta = 0;
    denteAzul.println("Portas fechadas SEM carregar.");
    Serial.println("Portas fechadas sem recarga.....");
  }
  //*********************************************  FIM da recarga dos medicamentos 
  //Alarmes************************************************************************

  if (HORA == ALARM1 && tEspera1 == 0)  //*************************verifica alarme1
  {
    digitalWrite(2,HIGH);                                  //acende o LED da porta1
    tone(12,700);                                             //ativa som do alarme
    tEspera1 = 1;                                 //habilita contagem de 30 minutos
    //denteAzul.println("1 -> Remedio das " + ALARM1);            //comunica evento
    proxAlarm = 2;                                         //ativa o proximo alarme
    
    hr = hr + 12;
    if(hr > 24){hr = hr - 24;}
    intTOstring();                  //vai com hr&mr, volta com string HR&MR e mr&hr
    ALARM2 = HR + ":" + MR; 
    
    Serial.println("porta1 habilitada por 30 minutos");    
    Serial.println("1 -> Remedio das   " + ALARM1);  
    Serial.println("LED1 aceso, alarme tocando");
   }  //************************************************* FIM do verifica alarme1
  
  //*****************************verifica alarme2  
  if (HORA == ALARM2 && tEspera2 == 0)  //************************verifica alarme1
  { digitalWrite(3,HIGH);                                  //acende o LED da porta1
    tone(12,700);                                             //ativa som do alarme
    tEspera2 = 1;                                 //habilita contagem de 30 minutos
    //denteAzul.println("2 -> Remedio das " + ALARM2);              //comunica evento
    proxAlarm = 1;                                         //ativa o proximo alarme 
    
    hr = hr + 12;
    if(hr > 24){hr = hr - 24;}
    intTOstring();                  //vai com hr&mr, volta com string HR&MR e mr&hr
    ALARM2 = HR + ":" + MR; 
    
    Serial.println("porta1 habilitada por 30 minutos");    
    Serial.println("2 -> Remedio das   " + ALARM2);  
    Serial.println("LED2 aceso, alarme tocando");
  }
  //******************************************************** FIM do verifica alarme2

  //==================================================== FIM DAS ROTINAS DOS ALARMES

  if (digitalRead(6) && digitalRead(7)){
    mostraHorarios();
  }

//===================================== ROTINAS DAS ABERTURAS DAS PORTAS HABILITADAS

//------------------------------------------------- atividades  SE porta1 habilitada
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
    //..............................................................................

        if (digitalRead(6)) //------------SE botao1 acionado no intervalo habilitado
        {
            sv1.write(aberto1);                                      //abre a porta1
            porta1aberta = 1;                               //sinaliza porta1 aberta
            Serial.println("abriu a porta1");
            delay(600);                        //tempo de resposta para soltar botao
        }  //----------FIM da verificacao SE botao1 acionado no intervalo habilitado
        if(porta1aberta == 1 && digitalRead(8))        //SE porta1 aberta e coletado
        {  
            sensor1();
            noTone(12);                                             //desliga sirene
            Serial.println("retirou 1");
        }
    }
    if(tEspera1 == 0)
    { 
        sv1.write(fechado1);                                         //fecha porta1
        digitalWrite(2,LOW);                                         //desliga LED1
        noTone(12);                                                //desliga sirene
    }//----------------------- ------------FIM das atividades  SE porta1 habilitada
    


//------------------------------------------------ atividades  SE porta2 habilitada
    if(tEspera2 != 0)     
    { //contagem do tempo .........................................................
        tEspera2 = mr + 30;                             //incia contagem 30 minutos
        if(tEspera1 > 59)
        {
            tEspera2 = tEspera2 - 59;
            if(tEspera2 == 0)
            {
                tEspera2 = 1;
            }
        }
  //..............................................................................
        if (digitalRead(7)) //----------SE botao2 acionado no intervalo habilitado
        {
            sv1.write(aberto1);                                    //abre a porta2
            porta1aberta = 1;                             //sinaliza porta2 aberta
            Serial.println("abriu a porta2");
            delay(600);                      //tempo de resposta para soltar botao
        }  //------- FIM da verificacao SE botao2 acionado no intervalo habilitado
    
        if(porta2aberta == 1 && digitalRead(9))     //SE porta2 aberta e coletado
        {  
            sensor2();
            noTone(12);                                           //desliga sirene
            Serial.println("retirou 2");
        }
    }
    if(tEspera2 == 0)
    { 
        sv2.write(fechado2);                                       //fecha porta2
        digitalWrite(3,LOW);                                       //desliga LED2
        noTone(12);                                              //desliga sirene
    }//--------------------------------- FIM das atividades  SE porta2 habilitada

  //=========================FIM DAS ROTINAS DAS ABERTURAS DAS PORTAS HABILITADAS

}  //################################################################ FIM DO LOOP



































































//#################################################################### SUBROTINAS

void mostraRelogio() 
{ //--------------------------------------------------------------- mostra relogio
  //-----------------------------------------leitura do Relogio em Tempo Real (RTC)
    HORA   = (rtc.getTimeStr());                    //atualiza hora  HR:MR:SS
   //-------------------------------------------------------- transformar em HR:MR
   /* HR="";HR=HR+HORA.charAt(0)+HORA.charAt(1);  //dois digitos
    MR="";MR=MR+HORA.charAt(3)+HORA.charAt(4);
    HORA = HR + ":" + MR;
//......................................................... acelerando o relogio
    if (acelera == 's') //SE foi aceita a aceleracao do relogio
    {
        AA=HORA;                //
        stringTOint();                 //volta com AA em HR:MR,  HR&MR e hr&mr (valores)
        mr++;
        if(mr>=60)
        {
            mr=0;
            hr++;
            if(hr>=24){hr=0;}
        }
        rtc.setTime(hr, mr, 0); //coloca minuto acelerado no RTC
        intTOstring();         //volta com string HR&MR e mr&hr
        HORA = HR + ":" + MR;  //HORA no formato HR:MR  (recoloca horario acelerado)
        delay(50);             //fator de aceleracao
    }*/
//...........................................................................
  
//....................................  para usar formato "HH:MM"
    HR="";HR=HR+HORA.charAt(0)+HORA.charAt(1);                     //dois digitos
    MR="";MR=MR+HORA.charAt(3)+HORA.charAt(4);
    HORA = HR + ":" + MR;                                 //HORA no formato HR:MR
    lcd.setCursor(0,0);
    lcd.print("Horario: ");lcd.print(HORA);
    lcd.setCursor(0,1);
    lcd.print("Alarme : ");
    if (proxAlarm == 1 && ALARM1 != "00:00")             //alarme 1 eh o proximo
    {
        lcd.print(ALARM1);lcd.print("  ");
    }
    if (proxAlarm == 2 && ALARM2 != "00:00")             //alarme 2 eh o proximo  
    {
        lcd.print(ALARM2);lcd.print("  ");
    }
} //------- esta voltando com HORA, HR&MR, mr&hr


void mostraHorarios()
{ //------------------------------------------------------- mostra os horarios
  denteAzul.println("......................");
  denteAzul.println("Horarios dos alarmes: ");
  denteAzul.print(ALARM1);
  denteAzul.print("  e  ");
  denteAzul.println(ALARM2);
  denteAzul.print("Horario da recarga : ");
  denteAzul.println(RECARGA);
  lcd.clear();
  lcd.print("Alarme 1: ");lcd.print(ALARM1);
  lcd.setCursor(0,1);
  lcd.print("Alarme 2: ");lcd.print(ALARM2);
  delay(3000);lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Recarga: ");lcd.print(RECARGA);
  delay(3000);lcd.clear();
}

void intTOstring()
{ //------------------------------------------------- converte valores em string
//chega com hr&mr em valores
  int unsigned i;
  HR=String(hr);
  i=HR.length();
  if (i<2){HR="0" + HR;}
  MR=String(mr);
  i=MR.length();
  if (i<2){MR="0" + MR;}
} //volta com string HR&MR e mr&hr

void stringTOint()
{ //------------------------------------------------- converte string em valores
//chega com AA string HH:MM
  HR="";                            //horas
  HR=HR+AA.charAt(0)+AA.charAt(1);  //caracter 0 e 1 (HH)
  hr=HR.toInt();
  MR="";                            //minutos
  MR=MR+AA.charAt(3)+AA.charAt(4);  //caracter 3 e 4 (MM)
  mr=MR.toInt();

} //---------------------------//volta com AA em HR:MR,  HR&MR e hr&mr (valores)

 

void pegaMenu()
{ //-------------------------------------------------- recebe entrada do celular
  AA = "";
  int unsigned i;
  while(!denteAzul.available() > 0){true;}        //espera entrada via bluetooth  
  while(denteAzul.available() > 0)
  {
    char menu;
    menu = denteAzul.read();                                //caracter em 'menu'
    AA=AA+menu;                                     //junta os digitados em "AA"
  }
  i=AA.length();  //...................................... verifica 4 caracteres
  if (i==4)                                          //aceita apenas 4 caracteres
  {
  //............................................ coloca ":" entre hora e minuto
    HR="";
    HR=HR+AA.charAt(0)+AA.charAt(1);          //os primeiros dois digitos
    MR="";
    MR=MR+AA.charAt(2)+AA.charAt(3);            //os dois ultimos digitos
    AA = HR + ":" + MR;                                   //AA no formato HR:MR
    stringTOint();            //volta com AA em HR:MR,  HR&MR e hr&mr (valores)
  }else{
     //false;
    denteAzul.println("Tem que ser HHMM,");
    denteAzul.println(" 4 digitos numericos");
  }
} //--------------------------- volta com AA em HR:MR,  HR&MR e hr&mr (valores)



void acertaRelogio ()
{ //----------------------------------------------- ajuste do horario do relogio
  //denteAzul.println("Horario do relogio em HHMM");
  pegaMenu();  //volta com AA (HR:MR),  HR&MR e hr&mr (valores)
  rtc.setTime(hr, mr, 0);     //altera o horario do relogio RTC
  HORA = AA;
  denteAzul.println(HORA);    //confirma o horario no bluetooth
} //----------------------------------------- fim do ajuste do horario do relogio


//============================================================ acerto dos alarmes
void acertaAlarme1()
{ //............................................................. hora do alarme 1
  //denteAzul.println("Horario dos Alarmes em HHMM ");
  pegaMenu();                  //volta com AA (HR:MR),  HR&MR e hr&mr (valores)
  ALARM1 = AA;
  //............................................................ calcula alarme 2
  /*hr = hr + 12;
  if(hr > 24){hr = hr - 24;}
  intTOstring(); 
                   //vai com hr&mr, volta com string HR&MR e mr&hr
  ALARM2 = HR + ":" + MR;*/
  mostraHorarios();
  proxAlarm    = 1;
}


void acertaRecarga()
{ //............................................................ hora da recarga
  //denteAzul.print("Horario de recarga em HHMM -> ");
  pegaMenu();             //volta com AA (HR:MR),  HR&MR e hr&mr (valores)
  RECARGA = HR + ":" + MR;
  mostraHorarios();
}




//============================================================= sensores acionados
void sensor1()  //...............................................................
{
    delay(1000);  
    tEspera1 = mr + 2;
    if(tEspera1 > 59)
    {
        tEspera1 = tEspera1 - 59;
        if(tEspera1 == 0)
        {
            tEspera1 = 1;
        }
    }
}
void sensor2()  //...............................................................
{
    delay(1000);  
    tEspera2 = mr + 2;
    if(tEspera2 > 59)
    {
        tEspera2 = tEspera2 - 59;
        if(tEspera2 == 0)
        {
            tEspera2 = 1;
        }
    }
}
