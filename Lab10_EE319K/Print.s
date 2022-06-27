; Print.s
; Student names: change this to your names or look very silly
; Last modification date: change this to the last modification date or look very silly
; Runs on TM4C123
; EE319K lab 7 device driver for any LCD
;
; As part of Lab 7, students need to implement these LCD_OutDec and LCD_OutFix
; This driver assumes two low-level LCD functions
; ST7735_OutChar   outputs a single 8-bit ASCII character
; ST7735_OutString outputs a null-terminated string 

    IMPORT   ST7735_OutChar
    IMPORT   ST7735_OutString
    EXPORT   LCD_OutDec
    EXPORT   LCD_OutFix
	PRESERVE8
    AREA    |.text|, CODE, READONLY, ALIGN=2
    THUMB

count EQU 0
num EQU 4
FP RN 11 	
  

;-----------------------LCD_OutDec-----------------------
; Output a 32-bit number in unsigned decimal format
; Input: R0 (call by value) 32-bit unsigned number
; Output: none
; Invariables: This function must not permanently modify registers R4 to R11
; R0=0,    then output "0"
; R0=3,    then output "3"
; R0=89,   then output "89"
; R0=123,  then output "123"
; R0=9999, then output "9999"
; R0=4294967295, then output "4294967295"
LCD_OutDec
	PUSH {R4-R8, R11, LR}
	SUB SP, #8
	MOV FP , SP
	CMP R0, #0
	BEQ Case1
	MOV R4, #0
	STR R4, [FP, #count]
	STR R0, [FP, #num]
	MOV R5, #10
Loop
	LDR R4, [FP, #num]
	CMP R4, #0
	BEQ Next
	UDIV R6, R4, R5
	MUL R7, R6, R5
	SUB R8, R4, R7
	PUSH{R8}
	STR R6,[FP, #num]
	LDR R6,[FP, #count]
	ADD R6, #1
	STR R6,[FP, #count]
	B Loop
Next
	LDR R4, [FP, #count]
	CMP R4, #0
	BEQ Done
	POP{R0}
	ADD R0, #0x30
	BL ST7735_OutChar
	SUB R4, #1
	STR R4,[FP, #count]
	B Next
Case1
	MOV R0, #0x30
	BL ST7735_OutChar
Done
	ADD SP, #8
	POP {R4-R8, R11, LR}
      BX  LR
;* * * * * * * * End of LCD_OutDec * * * * * * * *

; -----------------------LCD _OutFix----------------------
; Output characters to LCD display in fixed-point format
; unsigned decimal, resolution 0.001, range 0.000 to 9.999
; Inputs:  R0 is an unsigned 32-bit number
; Outputs: none
; E.g., R0=0,    then output "0.000"
;       R0=3,    then output "0.003"
;       R0=89,   then output "0.089"
;       R0=123,  then output "0.123"
;       R0=9999, then output "9.999"
;       R0>9999, then output "*.***"
; Invariables: This function must not permanently modify registers R4 to R11
LCD_OutFix
	PUSH {R4-R8, LR}
	MOV R8, #9999
	CMP R0, R8
	BLS Begin
	MOV R0,#0x2A
	BL ST7735_OutChar
	MOV R0, #0x2E
	BL ST7735_OutChar
	MOV R0, #0x2A
	BL ST7735_OutChar
	MOV R0, #0x2A
	BL ST7735_OutChar
	MOV R0, #0x2A
	BL ST7735_OutChar
	B Done2
	
Begin
	MOV R4, #1000
	UDIV R5, R0, R4
	MOV R6, R0
	MOV R0, R5
	BL LCD_OutDec
	MOV R0, #0x2E
	BL ST7735_OutChar
	MUL R5, R4
	SUB R7, R6, R5
	
	CMP R7, #9
	BHI DDigit
	MOV R0, #0x30
	BL ST7735_OutChar
	MOV R0, #0x30
	BL ST7735_OutChar
	MOV R0, R7
	BL LCD_OutDec
	B Done2
DDigit
	CMP R7, #99
	BHI TDigit
	MOV R0, #0x30
	BL ST7735_OutChar
TDigit
	MOV R0, R7
	BL LCD_OutDec
Done2 
	POP{R4-R8, LR} 
     BX   LR
 
     ALIGN
;* * * * * * * * End of LCD_OutFix * * * * * * * *

     ALIGN                           ; make sure the end of this section is aligned
     END                             ; end of file
