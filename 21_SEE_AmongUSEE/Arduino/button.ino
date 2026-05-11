void button() {
    // lcd에 숫자 입력받기
    if(irT == 0){
    if (digitalRead(BT1) == LOW) {// bt1 : 숫자가 1만큼 증가
        if (btPress++ == 0) {
          //Serial.println("1");
              if (lcdNum == 9)
                lcdNum = -1;   
            bzCon= 95;
            lcd.print(String(++lcdNum));
        }
    }
    else if (digitalRead(BT2) == LOW) {// bt2 : 커서 다음칸으로 넘아가기
        if (btPress++ == 0 && lcdNum!= -1) {
          //Serial.println("2");
            bzCon = 95;
            lcdID += String(lcdNum); 
            lcdNum = -1;
            lcdLoc = (lcdLoc == 14) ?  14 : lcdLoc + 1;
        }
    }

    else if (digitalRead(BT3) == LOW) {// bt3 : 숫자지우면서 이전칸으로 넘어가기
        if (btPress++ == 0) {
          //Serial.println("3");
            bzCon = 95;
            lcdID = lcdID.substring(0, lcdLoc - 8);
            lcdNum = -1;
            lcd.print(" ");
            lcdLoc = (lcdLoc == 7) ? 7 : lcdLoc - 1;
            //Serial.print(lcdID);
        }
    }

    else if (digitalRead(BT4) == LOW) {// bt4 : 확인
        if (btPress++ == 0) {
          //Serial.println("4");
            if (lcdLoc == 14)
            {
                lcdID += String(lcdNum);
                
                Serial.print("A1&");
                Serial.print(lcdID.substring(0,8));
                Serial.print("-");
                
            }
        }
    }
    else {
        btPress = 0;
    }
    }
}
