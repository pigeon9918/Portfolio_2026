# 마이크로 프로세서 응용 7조 코드 제출파일
각 파일의 분류

## startup codes
예제에서 제공된 startup code

### *.o files
    serial.o
    startup.o

## rellocation 이전 실행 파일
rellocation 이전의 실행

### *.c files
    main.c
    
    find_IDAT.c
    isSame.c
    convertGray.c
   
### *.h files 
    find_IDAT.h
    isSame.h
    convertGray.h
    ㄴ convertGray()
    ㄴ countRed()
    ㄴ convertReverse()

### *.s files 
    convertGray.s
    ㄴ convertGrayAsm()

    countRed.s
    ㄴ countRedAsm()
    
    convertReverse.s
    ㄴ convertReverseAsm()
    
## rellocation 이후 실행 파일

rellocation 및 이후의 실행

### *.c files
    main_reallo.c

    find_IDAT.c
    isSame.c
    convertGray.c
   
### *.h files 
    find_IDAT.h
    isSame.h
    reAllocation.h
    ㄴ convertGrayRel()
    ㄴ countRedRel()
    ㄴ convertReverseRel()

### *.s files 
최적화 완료

    countRedRel.s
    ㄴ countRedRelAsm()

    convertReverseRel.s
    ㄴ convertReverseRelAsm()

    convertGrayRelRel.s
    ㄴ convertGrayRelAsm()

최적화 이전

    first countrel.s
    ㄴ countRedRelAsmOr()

    first reverserel.s
    ㄴ convertReverseRelAsmOr()

    first grayrel.s
    ㄴ convertGrayRelAsmOr()
