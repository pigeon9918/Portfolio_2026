void ser()
{ 
    String serbufbuf = "";
    String b = "";
    String p;
    int index;
    int sid;
    int hh;
    int mm;
    char mychar;

    /* 문자열로 저장*/
    while(Serial.available()) {
        mychar = Serial.read();
        if(mychar == '-'){
          
            index = serbuf.indexOf('&');
            p = serbuf.substring(index-2,index); //A1
            b = serbuf.substring(index+1,index+2); //T
            sidre = serbuf.substring(index+3,index+11); //00000000
            timere = serbuf.substring(index+12,index+17); //12:12

//            Serial.print("serbuf: ");Serial.println(serbuf);
//            Serial.print("index: ");Serial.println(index);
//            Serial.print("p: ");Serial.println(p);
//            Serial.print("b: ");Serial.println(b);
//            Serial.print("sidre: ");Serial.println(sidre);
//            Serial.print("timere: ");Serial.println(timere);
//            Serial.println("--------");
            
            
            
            if(p == "A1") { //Sid
                //A1&T-
                if (b == "T") {
                    lcdSidT = 1;
                }
                else if (b == "F") {
                  if(sidre == "00000000") { // fid없음
                    lcdSidF = 1;
                  }
                  else if(sidre == "11111111") { // 지문이 이미 등록됨
                    lcdfidFF = 1;
                  }
                }
            }
            else if(p == "A2") { //Fid
                //A2&T-
                //Serial.println(b);
                if (b == "T") {
                    lcdSidT = 51;
                    FP = 1;
                }
                else if (b == "F") {
                    lcdSidF = 1;
                }
            }

            else if(p == "B1") { //Fid
                //B1&T,20202020,12:12-
               //Serial.print(b);
                if (b == "T") {
                    MsTimer2::stop();
                    lcdFinT = 1;
                }
                else if (b == "F") {
                    lcdFinW = 1;
                }
            }
            else if(p == "B2") { //sid, date
                //B2&00000000,HH:MM-
                //Serial.println(b);
                lcdFinT = 1;
                //lcd.print(" %8d,%2d:%2d  ", sid, hh, mm);
                
            }

            else if(p == "C1"){
                //Serial.println("1");
                finger.emptyDatabase();
                //Serial.println("Now database is empty :)");
            }

            serbuf = p = b =  ""; //버퍼 비우기
            break; //while 탈출
        }
        else {
            serbuf   += mychar;
        }
    }
}
