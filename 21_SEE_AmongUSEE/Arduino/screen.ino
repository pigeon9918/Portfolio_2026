void lcdClear() { //lcd화면 초기화 후 NUMBER 
    lcd.clear();
    lcdLoc = 0;
    lcd.print("NUMBER:");   //lcd에 ()나타내
    lcdLoc = 7;
}

void screen() {
   // Serial.print(lcdSidT);
    lcd.setCursor(lcdLoc, 0);

    // 커서 깜빡이기
    if (lcdBlink++ == 0) {
        lcd.noCursor();
    }
    else {
        lcdBlink = 0;
        lcd.cursor();
    }

    
    // 지문인식 성공
    if(lcdFinT > 0 && lcdFinT < 100) {
        lcdFinT++;
        //lcdClear();
        digitalWrite(latch, LOW);
        shiftOut(data, clk, MSBFIRST,0);
        digitalWrite(latch, HIGH);
        digitalWrite(ledG, HIGH);
        lcd.setCursor(7,0);
        lcd.print(sidre);
        lcd.setCursor(0,1);
        lcd.print(timere);
        lcd.setCursor(5,1);
        lcd.print("  ");
        lcd.setCursor(7,1);
        lcd.print(String(temData+6));
        lcd.setCursor(11,1);
        lcd.print("'C");
    }
    else if(lcdFinT == 100) {
        // green off
        
        digitalWrite(latch, LOW);
        shiftOut(data, clk, MSBFIRST,0);
        digitalWrite(latch, HIGH);
        digitalWrite(ledG, LOW);
        MsTimer2::start();
        lcdFinT = 0;
        lcdClear();
    }


    //기다려!!
    else if(lcdFinW>0 && lcdFinW<100 && timeout==0)
    {
        lcdFinW++;
        lcd.setCursor(0,1);
        lcd.print("     wait!!!     "); 
    }
    else if(lcdFinW==100)
    {
        lcdFinW=0;
        lcdClear();
    }


    // 지문인식 실패
    else if(lcdFinF > 0 && lcdFinF < 100 ) {
        //lcdFidW==0일때 추가..?
        digitalWrite(latch, LOW);
        shiftOut(data, clk, MSBFIRST,0);
        digitalWrite(latch, HIGH);
        digitalWrite(ledR,HIGH);
        lcdFinF++;
        //lcdClear();
        lcd.setCursor(0, 1);
        lcd.print("    imposter    ");
    }
    else if(lcdFinF == 100) {
        digitalWrite(ledR,LOW);
        MsTimer2::start();
        lcdFinF = 0;
        lcdClear();
    }

   
    //시간초과,
    else if(timeout >= 1 && timeout < 50 && lcdFinT == 0) {
        timeout++;
        //lcdClear();
        lcd.setCursor(0, 1);
        lcd.print("    timeout!    ");
    }
    else if(timeout == 50) {
        timeout = 0;
        lcd.setCursor(0,1);
        lcd.print("                ");
    }


    // 등록 : lcdID인증 실패
    else if(lcdSidF > 0 && lcdSidF < 100) {
        lcdSidF++;
        lcd.setCursor(3, 1);
        lcd.print("try again!");
    }
    else if(lcdSidF == 100) {
//        lcd.setCursor(0,1);
//        lcd.print("                ");
        lcdClear();
        lcdSidF = 0;
        lcdID = "";
    }
     if(lcdsidFF>0&& lcdsidFF<100&&FP==2)
    {
        
        lcdsidFF++;
        lcd.setCursor(0,1);
        lcd.print("  finger fail  ");  
        
        //Serial.println("lcd");
        //Serial.println(lcdsidFF);
    }
    else if(lcdsidFF==100)
    {
        FP = 0;
        lcdsidFF=0;
        lcdClear();
        lcdSidT=50;
    }
    else if (lcdfidFF > 0 && lcdfidFF<100){
        lcdfidFF++;
        lcd.setCursor(0,1);
        lcd.print("already register");  
    }
    else if (lcdfidFF==100){
        lcdfidFF=0;
        lcdClear();
    }
    
    // 등록 : lcdID인증 성공 
    else if(lcdSidT > 0 && lcdSidT < 50) {
        lcdSidT++;
        lcd.setCursor(0, 1);
        lcd.print("    Success    ");
    }
    else if(FP == 0 && lcdSidT == 50) {
        lcdSidT++;
        lcd.setCursor(0, 1);
        lcd.print("put your finger!");
  
    }
    else if(FP == 1 && (lcdSidT > 50 && lcdSidT < 100) ) {
        lcdSidT++;
        lcd.setCursor(0, 1);
        lcd.print("finger success!");
    }
    else if(lcdSidT == 100) {
        lcdSidT = 0;
        lcdClear();
        lcdID = "";
        FP = 0;     
    }




}
