#-*- coding:utf-8 -*-

import time
# 라즈베리파이 GPIO
import RPi.GPIO as GPIO

# 음성 인식 함수
from speechRecognition import speech_to_text
# TTS 함수
from textToSpeech import speak
# 기구부 Gcode 전송 클래스
from machineControl import machine_class
# 데이터 접근 클래스
from DataSearcher import search_DB
# 바코드 인식 함수
from barcodeRecognition import barcode_rec

serial_settings = {
    # 시리얼 포트 확인 필수!
    # dmesg | grep tty
    "PORT": "/dev/ttyUSB0",
    "BAUDRATE": 115200,
}
machine_settings = {
    # 가로 세로 칸 수
    "COULUM": 3,
    "ROW": 3,
    # 가로 세로 간격(mm)
    "X_DISTANCE": 70,
    "Z_DISTANCE": 75,
    # 이동 속도
    "SPEED": 3000,
}
# 기구 제어부 인스턴스 생성
machine = machine_class(serial_settings, machine_settings)
machine.print_info()

sheet_settings = {
    # 주의! 경로는 절대경로로 지정
    "json_file_name": "capstonedatabase-text_key.json",
    "spreadsheet_url": "https://docs.google.com/spreadsheets/d/10jZliiNwL_4U8Af05rQVH82Cc1nNyF-R9YcEwQZk-nM/edit?usp=sharing",
    "sheet_name": "sheet1",
}
# 데이터 검색부 인스턴스 생성
data_search = search_DB(sheet_settings)

# 버튼 관련 GPIO 선언
SW_BTN = 17
GPIO.setmode(GPIO.BCM)
GPIO.setup(SW_BTN, GPIO.IN)

# state 상수
WAITING_STATE_NAME = "waiting"
INITIALIZE_STATE_NAME = "initialize"
CHATBOT_STATE_NAME = "chat"
FIND_STATE_NAME = "find"
TAKEOUT_STATE_NAME = "takeout"

# 전역 변수
state_node = ""
behavior_node = ""
to_find = ""

# 시작
state_node = WAITING_STATE_NAME

# Behavior Tree Model
while True:
    # waiting state
    while state_node == WAITING_STATE_NAME:
        print(WAITING_STATE_NAME + "...")
        if GPIO.input(SW_BTN) == True:
            state_node = INITIALIZE_STATE_NAME
            break
        time.sleep(0.1)

    # initializing whole machine
    while state_node == INITIALIZE_STATE_NAME:
        print(INITIALIZE_STATE_NAME)
        # initailizng
        data_search.reload()

        machine.move_home()
        machine.relative()
        machine.print_state()
        machine.send_command("M203 X500 Z15")
        machine.send_command("G92 X0 Y0 Z0")

        state_node = CHATBOT_STATE_NAME
        behavior_node = ""
        to_find = ""

    # chatbot and speech input
    while state_node == CHATBOT_STATE_NAME:
        print(CHATBOT_STATE_NAME)
        SPEECH_INPUT_NAME = "speech input"
        INPUT_ANALYZE_NAME = "input recognition"
        fail_count = 0
        # setup
        while behavior_node == "":
            speak("안녕하세요. 원하는 약을 검색하라고 말해보세요.")
            speech_input = ""
            behavior_node = SPEECH_INPUT_NAME

        # 입력
        while behavior_node == SPEECH_INPUT_NAME:
            try:
                speech_input = speech_to_text()
                # speech_input = input("음성입식 내용: ")
                behavior_node = INPUT_ANALYZE_NAME
            except:
                fail_count += 1
                
            if fail_count > 3:
                speak("인식 오류 횟수 3회 초과로 종료합니다.")
                state_node = WAITING_STATE_NAME
                behavior_node = ""
                fail_count = 0

        # 내용 판단
        while behavior_node == INPUT_ANALYZE_NAME:
            bool_success = False
            
            # 종료
            if "종료" in speech_input:
                speak("종료합니다.")
                state_node = WAITING_STATE_NAME
                behavior_node = ""
            # 입력된 음성에서 약을 검색
            elif "찾아" in speech_input or "차자" in speech_input or "검색" in speech_input:
                # "(약 이름) 찾아줘"라고 말했다면, 리스트에 있는 약 이름이 speech_input에 있는지 확인
                for name in data_search.name_list:
                    if name in speech_input:
                        to_find = name
                        print(to_find)
                        bool_success = True
                        behavior_node = ""
                        state_node = FIND_STATE_NAME
                        speak(f"{to_find}을 찾습니다.")
                        break
            else:
                speak("이해하지 못했습니다.")
                bool_success = True
                # repeat input state
                behavior_node = SPEECH_INPUT_NAME
                
            if bool_success == False:
                speak("약을 찾지 못했습니다.")
                # repeat input state
                behavior_node = SPEECH_INPUT_NAME
                
            # end

    # finding drug
    while state_node == FIND_STATE_NAME:
        print(FIND_STATE_NAME)
        # setup
        BARCODE_RECOGNITION_NAME = "barcode recognition"
        DRUG_SEARCH_NAME = "data search"
        ERROR = "Error"
        EMPTY = "Empty"
        if behavior_node == "":
            behavior_node = BARCODE_RECOGNITION_NAME

        # 카메라 인식
        if behavior_node == BARCODE_RECOGNITION_NAME:
            code = barcode_rec()
            # code = input("바코드 인식: ")

            # 빈 경우
            if code == EMPTY:
                speak("선반이 비었습니다")
                machine.move_next()
                code = ""
            # 인식 에러
            elif code == ERROR:
                speak("인식에 문제가 있습니다.")
                machine.move_next()
                code = ""
            # 인식 성공
            else:
                behavior_node = DRUG_SEARCH_NAME

        # 약 검색
        if behavior_node == DRUG_SEARCH_NAME:
            index = data_search.bycode(code)
            if index != -1:
                recog = data_search.load_row(index)
                if recog[0] == to_find:
                    speak(f"{to_find}을 찾았습니다")
                    # end
                    state_node = TAKEOUT_STATE_NAME
                    behavior_node = ""
                else:
                    machine.move_next()
                    code = ""
                    behavior_node = BARCODE_RECOGNITION_NAME
            # 찾고자 하는 약이 아닌경우
            else:
                speak("찾는 약이 아니므로 다음 칸으로 넘어갑니다.")
                machine.move_next()
                code = ""
                behavior_node = BARCODE_RECOGNITION_NAME

    # taking out the drug
    while state_node == TAKEOUT_STATE_NAME:
        print(TAKEOUT_STATE_NAME)
        # 기구부 동작
        # machine.drop()

        # 약 정보 안내방송
        index = data_search.byname(to_find)
        if index != -1:
            recog = data_search.load_row(index)
            speak(f"{to_find}에대한 정보입니다.")
            # 약에대한 안내 DB를 구성해야할 듯
            speak(recog[2])
            machine.drop()
            machine.move_home()
            state_node = WAITING_STATE_NAME
            behavior_node = ""

    # End of loop
