	AREA	CODE, READONLY, CODE
	ENTRY
	EXPORT	countRedRelAsmOr

	; int countRedRelAsmOr(uint8_t* origin, int size)
	;	r0 = baseAddress
	;	r1 = maxCount
	; return: int (r0)
	;	r2 = cnt
	;	r3 = buf

countRedRelAsmOr
	MOV		r2, #0
L1_cnt
	LDRB	r3, [r0], #1
	CMP	 	r3, #128
	ADDGE	r2, r2, #1
	SUBS	r1, r1, #1
	BGT		L1_cnt
	MOV		r0, r2
	BX lr
	
END