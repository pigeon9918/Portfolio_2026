#-*- coding:utf-8 -*-

import gspread
from oauth2client.service_account import ServiceAccountCredentials
'''
pip install gspread
pip install --upgrade oauth2client

참고 사이트
https://hleecaster.com/python-google-drive-spreadsheet-api/
'''

scope = [
    'https://spreadsheets.google.com/feeds',
    'https://www.googleapis.com/auth/drive',
]


class search_DB():
    # 클래스 초기화
    def __init__(self, sheet_settings):
        credentials = ServiceAccountCredentials.from_json_keyfile_name(
            sheet_settings["json_file_name"], scope)
        gc = gspread.authorize(credentials)
        # 스프레스시트 문서 가져오기
        doc = gc.open_by_url(sheet_settings["spreadsheet_url"])
        self.worksheet = doc.worksheet(sheet_settings["sheet_name"])
        # 인덱스 불러오기
        self.sIndex_list = self.worksheet.row_values(1)
        # 이름과 바코드 불러와 리스트(캐싱 레이어) 생성
        self.name_list = self.worksheet.col_values(1)[1:]
        self.barcode_list = self.worksheet.col_values(2)[1:]
        print("Data initializing done!")

    # 이름 문자열을 입력받아 DB에 있는지 찾기
    def byname(self, name):
        print(f"searching by name: {name}")
        # 캐싱 레이어 내에서 이름을 찾기
        if name in self.name_list:
            index = self.name_list.index(name) + 2
            print(f"found! index: {index}")
            # 인덱스 번호 리턴
            return index
        else:
            print("no exist!")
            # 에러코드 리턴
            return -1

    # 바코드 문자열을 입력받아 DB에 있나 찾기
    def bycode(self, barcode):
        print(f"searching by code: {barcode}")
        # 캐싱 레이어 내에서 바코드를 찾기
        if barcode in self.barcode_list:
            index = self.barcode_list.index(barcode) + 2
            print(f"found! index: {index}")
            # 인덱스 번호 리턴
            return index
        else:
            print("no exist!")
            # 에러코드 리턴
            return -1

    # 인덱스 번호로 약의 정보를 전부(행을 전부) 불러오기
    def load_row(self, index):
        print(f"searching by index: {index}")
        row = self.worksheet.row_values(index)
        print(row)
        return row

    # 리스트(캐싱 레이어)를 새로고침
    def reload(self):
        self.barcode_list = self.worksheet.col_values(2)[1:]
        self.name_list = self.worksheet.col_values(1)[1:]

    # 캐싱레이어 리스트를 출력
    def print_var(self):
        print(self.name_list)
        print(self.barcode_list)

    # 필드 리스트 표시
    def print_sIndex(self):
        print(self.sIndex_list)


# # test code

# sheet_settings = {
#     "json_file_name": "capstonedatabase-text_key.json",
#     "spreadsheet_url": "https://docs.google.com/spreadsheets/d/10jZliiNwL_4U8Af05rQVH82Cc1nNyF-R9YcEwQZk-nM/edit?usp=sharing",
#     "sheet_name": "sheet1",
# }
# data_search = search_DB(sheet_settings)

# n = data_search.byname("타이레놀")
# data_search.print_sIndex()
# data_search.load_row(n)
