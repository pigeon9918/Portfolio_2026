	AREA	CODE, READONLY, CODE
	ENTRY
	EXPORT	convertReverseRelAsmOr

    ;   void convertReverseRelAsmOr(uint8_t* origin, uint8_t* toSave, int size) {
    ;   
	;	r0 = baseAddress
	;	r1 = targetAddress
	;	r2 = maxCount
	;	r3 = converted
	;
	;	r4 = buf

convertReverseRelAsmOr
	STMFD sp!, {r4, lr}
CRA_L1
	; r4 = r
	LDRB	r4, [r0], #1
	MVN     r3, r4
	STRB	r3, [r1], #1
	
	SUBS	r2, r2, #1
	BGT		CRA_L1
	;BX lr;
    LDMFD sp!, {r4, pc}
	
END