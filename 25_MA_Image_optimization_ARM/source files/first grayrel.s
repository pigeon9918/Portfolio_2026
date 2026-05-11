	AREA	CODE, READONLY, CODE
	ENTRY
	EXPORT	convertGrayRelAsmOr
	
	;	void convertGrayRelAsmOr(uint16_t* toSave, uint8_t* targetR, uint8_t* targetG, uint8_t* targetB, int size)
    ;   *toSave = 3 * (uint16_t)r + 6 * (uint16_t)g + (uint16_t)b
	;	r0 = toSave
	;	r1 = targetR
	;	r2 = targetG
	;	r3 = targetB
	;	r4 = size
	;	r5 = r
	;	r6 = g
	;	r7 = b
	;	r8 = buf1
    ;   r9 = buf2

convertGrayRelAsmOr
    STMFD	sp!, {r4-r9, lr}
    LDR		r4, [sp, #28]		 ; r4 ← size
	
L1_gray
    LDRB  r5, [r1], #1
    LDRB  r6, [r2], #1
    LDRB  r7, [r3], #1
    MOV   r8, r5
    ADD   r8, r8, r5, LSL #1
    MOV   r9, r6, LSL #1
    ADD   r9, r9, r6, LSL #2
    ADD   r8, r8, r9
    ADD   r8, r8, r7
    STRH  r8, [r0], #2
    SUBS  r4, r4, #1
    BGT   L1_gray
    LDMFD sp!, {r4-r9, pc}
END