void light(){
    if( !(lcdSidT || lcdSidF || lcdFinT || lcdFinF || lcdsidFF || lcdfidFF || lcdFinW) ){
        //lcdSidT, lcdSidF, lcdFinT, lcdFinF, lcdsidFF, lcdfidFF, lcdFinW
        if(digitalRead(infrared) == 0){
            irT++;
        }
    }
}

void ledFlash(){ // 파랭이
    if ( lcdFinF == 0 && lcdFinT == 0 ){
        digitalWrite(latch, LOW);
        shiftOut(data, clk, MSBFIRST,random(512));
        digitalWrite(latch, HIGH);
    }
}
