#-*- coding:utf-8 -*-

import cv2              # pip install opencv-python
import pyzbar.pyzbar as pyzbar          # pip install pyzbar


def barcode_rec():
    print("recognition start!")
    rec_list = {}
    count = 0
    while True:
        count += 1
        cap = cv2.VideoCapture(0)
        bol, frame = cap.read()

        if bol:
            try:
                for code in pyzbar.decode(frame):
                    barcode = code.data.decode('utf-8')
                    print(f"Success: {barcode}")
                    
                    if barcode in rec_list.keys():
                        rec_list[barcode] += 1
                        print("b")
                    else:
                        rec_list[barcode] = 1
                        print("a")

            except Exception as e:
                print(f"Error: {e}")
                return "Error"

            # cv2.imshow('cam', frame)
            key = cv2.waitKey(1)
            if key == 27:
                print("No")
                return "No"
            
        cap.release()
        cv2.destroyAllWindows()
        
        for key, value in rec_list.items():
            if value >= 3:
                print(key)
                return key
            
        print(f"barcode count: {count}")
        if count >= 5:
            print("Empty!")
            return "Empty"
        

# while True:
#     barcode_rec()
