#-*- coding:utf-8 -*-

from gtts import gTTS
import os
# from playsound import playsound
# playsound 모듈이 제대로 동작하지 않았음. 이유는 아직 찾지 못함
'''
pip install gTTS

1. sudo apt-get update 
2. sudo apt-get upgrade 
3. sudo apt-get dist-upgrade
4. sudo apt-get install portaudio19-dev 
5. sudo pip install pyaudio

playsound 모듈 대신 외부 패키지를 사용해 mp3를 재생
mpg123 패키지 설치 필요
sudo apt-get install mpg123

참고 사이트
https://fast-it.tistory.com/entry/%ED%8C%8C%EC%9D%B4%EC%8D%AC%EC%9C%BC%EB%A1%9C-%EC%9D%8C%EC%84%B1%EC%9D%B8%EC%8B%9D-%EB%B4%87-%EB%A7%8C%EB%93%A4%EA%B8%B01

USB 스피커 세팅하기: https://diy-project.tistory.com/88
'''


def speak(input_text):
    # 음성 파일 만들기
    print(input_text)
    tts = gTTS(text=input_text, lang='ko')
    tts.save('./voice.mp3')

    # 음성 파일을 재생
    # playsound.playsound('./voice.mp3')
    os.system("mpg123 " + "./voice.mp3")


# speak("안녕!")
