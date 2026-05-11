void temperature() {
    if(temCon == 1) {
        temBuf += mlx.readAmbientTempC();
        temCount++;
        
        if(temCount == 10) {
            temData = temBuf/10;
            temBuf = 0;
            temCount = 0;
            temCon = 0;
            //lcdFinT = 1;
            //bzCon = 70;
            Serial.print("B1&");
            Serial.print(Fid);
            Serial.print(",");
            Serial.print(temData + 6);
            Serial.print("-");
        }
    }
}
