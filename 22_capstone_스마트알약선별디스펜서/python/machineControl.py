#-*- coding:utf-8 -*-

import serial
import time
'''
pip install pyserial

참고 사이트
https://coding-kindergarten.tistory.com/179
https://kevinponce.com/blog/python/send-gcode-through-serial-to-a-3d-printer-using-python/
'''

def calculate_sleeptime(X = 0, Y = 0, Z = 0, FEEDRATE = 1500):
    VELOCITY = FEEDRATE / 60
    DISTANCE = (X**2 + Y**2 + Z**2)**(1/2)
    sec = DISTANCE / VELOCITY
    return sec

MOVING_CMD = "G0"
HOME_CMD = "G28 X0 Z0"
RELEASE_CMD = "M84"
RELATIVE_CMD = "G91"
ABSOLUTE_CMD = "G90"


class machine_class():
    # 클래스 초기화
    def __init__(self, serial_settings, machine_settings):
        # 시리얼 선언
        self.py_serial = serial.Serial(
            port=serial_settings["PORT"],
            baudrate=serial_settings["BAUDRATE"],
        )  # MUST BE CHECKED!!!

        # 기계 설정 상수 입력
        self.COULUM = machine_settings["COULUM"]
        self.ROW = machine_settings["ROW"]
        self.X_DISTANCE = machine_settings["X_DISTANCE"]
        self.Z_DISTANCE = machine_settings["Z_DISTANCE"]
        self.SPEED = machine_settings["SPEED"]

        # 기구 상태 변수 입력
        self.count = 0

    # 명령어 보내기 함수
    def send_command(self, command):
        self.py_serial.write(str.encode(command + "\r\n"))
        print(f"sent: {command}", end=", ")
        time.sleep(1)

        while True:
            line = self.py_serial.readline()
            print(line)
            # ok 사인을 받으면
            if line == b'ok\n':
                break

    # 다음 칸 이동
    def move_next(self):
        # 카운터 증가
        self.count += 1

        # 최대 칸 수를 넘어간 경우
        if (self.count % (self.COULUM * self.ROW)) == 1 and self.count != 1:
            print("over count error!")
            self.count = 0
            self.move_home()
            # 에러코드 출력
            return -1

        # 한 줄을 넘어간 경우
        elif self.count % self.ROW == 1 and self.count != 1:
            # 다음 열로 넘어가고 첫 행으로 돌아가기
            command = f"{MOVING_CMD} X-{self.X_DISTANCE * (self.ROW - 1)} Z{self.Z_DISTANCE} F{self.SPEED * 2}"
            sec = calculate_sleeptime(X = (self.X_DISTANCE * (self.ROW - 1)), Z = self.Z_DISTANCE)

        # 처음인 경우
        elif self.count == 1:
            # 첫 자리로
            command = f"{MOVING_CMD} X{0} Z{0} F{self.SPEED}"
            sec = calculate_sleeptime(X = 10)

        else:
            # 다음 행으로 이동
            command = f"{MOVING_CMD} X{self.X_DISTANCE}"
            sec = calculate_sleeptime(X = (self.X_DISTANCE * (self.ROW - 1)))

        # 명령어 전송
        print(f"count: {self.count}")
        self.send_command(command)
        time.sleep(sec)

    def move_home(self):
        # 상태 변수 초기값
        self.count = 0

        # 기구부 원위치로
        command = HOME_CMD
        self.send_command(command)
        print(f"Homing...")
        
        self.move_next()

        # # moving to initial coordinate
        # command = f"{MOVING_CMD} X{self.X_DISTANCE // 2} Z{self.Z_DISTANCE // 2} F{self.SPEED}"
        # self.send_command(command)
        # print(f"Homing done!")
        

    def release(self):
        # 기구부 모터 끄기
        self.send_command(RELEASE_CMD)
        print(f"Motor released!")

    def relative(self):
        # 기구부 상대좌표 모드
        self.send_command(RELATIVE_CMD)
        print(f"relative mode!")

    def absolute(self):
        # 기구부 절대좌표 모드
        self.send_command(ABSOLUTE_CMD)
        print(f"relative mode!")
        
    def drop(self):
        # 기구부 물건 밀기        
        command = "G0 Y120 F3000"
        self.send_command(command)
        print(f"dropped!")
        time.sleep(4)
        command = "G0 Y-120 F3000"
        self.send_command(command)
        print(f"getting back!")

    def print_info(self):
        # 기구부 설정 상수 터미널에 출력
        print("================================")
        print("<<machine constants>>")
        print(f"COULUM: {self.COULUM}")
        print(f"ROW: {self.ROW}")
        print(f"X_DISTANCE: {self.X_DISTANCE}")
        print(f"Z_DISTANCE: {self.Z_DISTANCE}")
        print("================================")

    def print_state(self):
        # 현재 행, 열 위치 계산
        col = self.count // self.ROW
        row = self.count % self.ROW

        # 현재 기구의 카운트, 행, 열 터미널에 출력
        print("================================")
        print(f"<<machine state>>")
        print(f"count: {self.count}")
        print(f"col: {col}")
        print(f"row: {row}")
        print("================================")


# 참고 3D프린터 주요 G-code
# http://www.nogoora.com/entry/3D%ED%94%84%EB%A6%B0%ED%84%B0%EC%97%90%EC%84%9C-%EC%82%AC%EC%9A%A9%ED%95%98%EB%8A%94-%EC%A3%BC%EC%9A%94-G-%EC%BD%94%EB%93%9C-%EB%AA%85%EB%A0%B9%EC%96%B4-%EC%9D%B4%ED%95%B4

# test code

# serial_settings = {
#     # 포트 확인 필수!
#     "PORT": "/dev/ttyUSB0",
#     "BAUDRATE": 115200,
# }

# machine_settings = {
#     "COULUM": 3,
#     "ROW": 3,
#     "X_DISTANCE": 70,
#     "Z_DISTANCE": 75,
#     "SPEED": 6000,
# }

# machine = machine_class(serial_settings, machine_settings)
# machine.print_info()
# machine.print_state()
# time.sleep(1)
# machine.send_command("M203 Z15")

# machine.move_home()
# machine.send_command("G92 X0 Y0 Z0")
# machine.drop()
# machine.release()
# machine.relative()
# time.sleep(2)

# for i in range(10):
#     machine.move_next()
#     time.sleep(2)

# machine.release()
