void fingerprint() {
    if(FP == 0 && lcdSidT == 51) { //put your finger
        Data1 = "";
        for (int finger = 1; finger < 128; finger++) {
            Template(finger);
        }
        //Serial.println(Data1);
        id = emptynum(Data1);
        //Serial.print("empty:");
        //Serial.println(id);

        getFingerprintEnroll();
        
    }

    if(irT > 0 && irT < 50) {
        irT += 2;
        //Serial.println(irT);
        
        if(getFingerprintIDez() > 0) { //성공
            /*
            Serial.print("B1&");
            Serial.print(Fid);
            Serial.print("-");
            */
            temCon=1;
            irT = 0;

        }
    }
    else if(irT >= 50) { //시간초과
        irT = 0;
        timeout = 1;
    }

}

//지문을 등록. 저장할 지문의 ID를 지정한 후 두번의 지문을 인식하는데, 두 지문이 같을 경우 해당하는 번호에 등록
uint8_t getFingerprintEnroll() {
  // 첫번째 지문 인식
  int p = -1;
  //Serial.print("Waiting for valid finger to enroll as #"); Serial.println(id);
  while (p != FINGERPRINT_OK) {
    p = finger.getImage();
    switch (p) {
      case FINGERPRINT_OK:
        //Serial.println("Image taken");
        break;
      case FINGERPRINT_NOFINGER:
        //Serial.println(".");
        break;
      case FINGERPRINT_PACKETRECIEVEERR:
        //Serial.println("Communication error");
        break;
      case FINGERPRINT_IMAGEFAIL:
        //Serial.println("Imaging error");
        break;
      default:
        //Serial.println("Unknown error");
        break;
    }
  }

  // OK success!

  p = finger.image2Tz(1);
  switch (p) {
    case FINGERPRINT_OK:
      //Serial.println("Image converted");
      break;
    case FINGERPRINT_IMAGEMESS:
      //Serial.println("Image too messy");
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      //Serial.println("Communication error");
      return p;
    case FINGERPRINT_FEATUREFAIL:
      //Serial.println("Could not find fingerprint features");
      return p;
    case FINGERPRINT_INVALIDIMAGE:
      //Serial.println("Could not find fingerprint features");
      return p;
    default:
      //Serial.println("Unknown error");
      return p;
  }

  //Serial.println("Remove finger");
  delay(2000);
  // 두번째 인식
  p = 0;
  while (p != FINGERPRINT_NOFINGER) {
    p = finger.getImage();
  }
  //Serial.print("ID "); Serial.println(id);
  p = -1;
  //Serial.println("Place same finger again");
  lcd.setCursor(0,1);
  lcd.print("  finger again  ");
  
  while (p != FINGERPRINT_OK) {
    p = finger.getImage();
    switch (p) {
    case FINGERPRINT_OK:
      //Serial.println("Image taken");
      break;
    case FINGERPRINT_NOFINGER:
      //Serial.print(".");
      break;
    case FINGERPRINT_PACKETRECIEVEERR:
      //Serial.println("Communication error");
      break;
    case FINGERPRINT_IMAGEFAIL:
      //Serial.println("Imaging error");
      break;
    default:
      //Serial.println("Unknown error");
      break;
    }
  }

  // OK success!

  p = finger.image2Tz(2);
  switch (p) {
    case FINGERPRINT_OK:
      //Serial.println("Image converted");
      break;
    case FINGERPRINT_IMAGEMESS:
      //Serial.println("Image too messy");
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      //Serial.println("Communication error");
      return p;
    case FINGERPRINT_FEATUREFAIL:
      //Serial.println("Could not find fingerprint features");
      return p;
    case FINGERPRINT_INVALIDIMAGE:
      //Serial.println("Could not find fingerprint features");
      return p;
    default:
      //Serial.println("Unknown error");
      return p;
  }

  // OK converted!
 //Serial.print("Creating model for #");  Serial.println(id);

  p = finger.createModel();
  if (p == FINGERPRINT_OK) {
    //Serial.println("Prints matched!");
    Serial.print("A2&");
    Serial.print(id);
    Serial.print("-");
    FP = 10;
  }
  else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    //Serial.println("Communication error");
    return p;
  }
  else if (p == FINGERPRINT_ENROLLMISMATCH) {
    //Serial.println("Fingerprints did not match");
    lcdsidFF=1;
    FP=2;
    return p;
    
  }
  else {
    //Serial.println("Unknown error");
    return p;
  }

  //Serial.print("ID "); 
  
  
  
  p = finger.storeModel(id);
  if (p == FINGERPRINT_OK) {
    //Serial.println("Stored!");
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    //Serial.println("Communication error");
    return p;
  } else if (p == FINGERPRINT_BADLOCATION) {
    //Serial.println("Could not store in that location");
    return p;
  } else if (p == FINGERPRINT_FLASHERR) {
    //Serial.println("Error writing to flash");
    return p;
  } else {
    //Serial.println("Unknown error");
    return p;
  }
    
  return true;
}

//인식한 지문의 번호를 반환. 문제 상황의 경우 각 상황에 맞는 음수를 반환함.
int getFingerprintIDez() {
  uint8_t p = finger.getImage();
  if (p != FINGERPRINT_OK)
    return -1;

  p = finger.image2Tz();
  if (p != FINGERPRINT_OK)    
    return -2;  

    p = finger.fingerSearch();
    if (p == FINGERPRINT_OK) {
        //Serial.println("Found a print match!");
        // found a match!
        //Serial.print("Found ID #"); Serial.print(finger.fingerID);
        //Serial.print(" with confidence of "); Serial.println(finger.confidence);
        //lcdFinT = 1;
        Fid = finger.fingerID;
        return Fid;
    }
    else if (p == FINGERPRINT_PACKETRECIEVEERR) {
        //Serial.println("Communication error");
        return -3;
    }
    else if (p == FINGERPRINT_NOTFOUND) {
        //Serial.println("Did not find a match");
        irT = 0;
        lcdFinF = 1;
       // MsTimer2::stop();
        Serial.print("B1&");
        Serial.print("0");
        Serial.print(",");
        Serial.print(temData);
        Serial.print("-");
        return -4;
    }
    else {
        //Serial.println("Unknown error");
        return -5;
    }
  
}

//Data1에서 가장 앞 번호의 0을 찾아서 그 번호를 반환함.
int emptynum(String str) {
    int i = 0;
    while (str[i] == '1') {
        i += 1;
    }
    uint8_t emptynum = i + 1;
    return(emptynum);
}

//지문센서의 내부의 1-127의 번호를 확인한 후, 있는 곳은 1, 없는 곳은 0 으로 Data1에 채움.
uint8_t Template(uint16_t id) {

    //Serial.println("------------------------------------");
    //Serial.print("Attempting to load #"); Serial.println(id);
    uint8_t p = finger.loadModel(id);
    switch (p) {
        case FINGERPRINT_OK:
            //Serial.print("Template "); Serial.print(id); Serial.println(" loaded");
            Data1 += "1";
            break;
        case FINGERPRINT_PACKETRECIEVEERR:
            //Serial.println("Communication error");
            Data1 += "0";
            return p;
        default:
            //Serial.print("Unknown error "); Serial.println(p);
            Data1 += "0";
            return p;
    }
}


//그냥 귀찮아서 만든 지문센서의 초기 세팅.
void fingerinit() {
  while (!Serial);  // For Yun/Leo/Micro/Zero/...
    delay(100);

  // set the data rate for the sensor serial port
  finger.begin(57600);
  delay(5);
  
  if (finger.verifyPassword()) {
    //Serial.println("Found fingerprint sensor!");
  }
  else {
    //Serial.println("Did not find fingerprint sensor :(");
    while (1) { delay(1); }
  }

  finger.getParameters();

  finger.getTemplateCount();

  if (finger.templateCount == 0) {
    //Serial.print("Sensor doesn't contain any fingerprint data. Please run the 'enroll' example.");
  }
  else {
    /*Serial.println("Waiting for valid finger...");
    Serial.print("Sensor contains "); Serial.print(finger.templateCount); Serial.println(" templates");*/
  } //저장된 지문 수 
}

    // if(digitalRead(infrared) == 0) {
    //     Serial.println("Ready to find ID");
    //     int t2 = millis();

    //     while( (t2 - millis()) <= 5000 ) {
    //         delay(1);
    //         if(getFingerprintIDez() > 0) {
    //             break;
    //         }
    //     }
    // }
