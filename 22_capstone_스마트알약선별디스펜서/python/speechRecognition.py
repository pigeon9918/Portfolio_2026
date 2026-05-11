#-*- coding:utf-8 -*-

import speech_recognition as sr
'''
pip install SpeechRecognition

flac 다루는 패키지 필요, 없으면 에러
sudo apt-get install flac

참고 사이트
https://fast-it.tistory.com/entry/%ED%8C%8C%EC%9D%B4%EC%8D%AC%EC%9C%BC%EB%A1%9C-%EC%9D%8C%EC%84%B1%EC%9D%B8%EC%8B%9D-%EB%B4%87-%EB%A7%8C%EB%93%A4%EA%B8%B01

USB 마이크 세팅하기: https://diy-project.tistory.com/88
'''

Recognizer = sr.Recognizer()
mic = sr.Microphone()


def speech_to_text():
    print("now speak!")

    with mic as source:
        # 마이크 입력 녹음
        audio = Recognizer.listen(source)
        try:
            # 구글 api를 통하여 입력된 음성을 텍스트로 변환
            data = Recognizer.recognize_google(audio, language="ko")
        except:
            # 인식에 실패한 경우
            print("이해하지 못했습니다.")
            return ("이해하지 못했습니다.")

    # 입력된 내용 터미널에 출력
    print(f"입력된 음성: {data}")
    return data

# test code

# speech_to_text()
