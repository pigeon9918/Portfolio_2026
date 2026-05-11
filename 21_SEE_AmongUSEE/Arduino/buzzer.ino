void buzzer() {
    if (bzCon > 0 && bzCon < 100) {
        digitalWrite(buzzerPin, HIGH);
        //Serial.println(bzCon);
        bzCon++;
    }
    else {
        bzCon = 0;
        digitalWrite(buzzerPin, LOW);
    }
}
