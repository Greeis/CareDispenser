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
            sv1.write(fechado1);
            digitalWrite(2,LOW);
            Serial.println("retirou 1");
            delay(60000);
        }
    }
    
    if(tEspera1 == 0)
    { 
        sv1.write(fechado1);                                         //fecha porta1
        digitalWrite(2,LOW);                                         //desliga LED1
        noTone(12);                                                //desliga sirene
    }//------------------