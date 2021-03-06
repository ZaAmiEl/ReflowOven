;  LCD routines adapted for the DE1 SoC configured as a CV_8052 processor
cseg

; When using a 33.333333MHz crystal clock
; one cycle takes 1.0/33.333333MHz = 30 ns

;---------------------------------;
; Wait 40 microseconds            ;
;---------------------------------;
Wait40uSec:
	push AR0
	mov R0, #190
L0: 
	nop
	nop
	nop
	nop
	djnz R0, L0 ; 1+1+1+1+3 cycles->7*30ns*190=40us
	pop AR0
    ret

Display_BCD1:
	mov dptr, #T_7seg
	; Display the channel in HEX5
	mov a, b
	anl a, #0x0f
	movc a, @a+dptr
	mov HEX5, a
	
	; Display [R3,R2] in HEX3, HEX2, HEX1, HEX0
	mov a, R3
	swap a
	anl a, #0x0f
	movc a, @a+dptr
	mov HEX3, a
	
	mov a, R3
	anl a, #0x0f
	movc a, @a+dptr
	mov HEX2, a
	
	mov a, R2
	swap a
	anl a, #0x0f
	movc a, @a+dptr
	mov HEX1, a
	
	mov a, R2
	anl a, #0x0f
	movc a, @a+dptr
	mov HEX0, a
	
	ret 
hex2bcd16:
	clr a
	mov R4, a ; Initialize BCD to 00-00-00 
	mov R3, a
	mov R2, a
	mov R5, #16  ; Loop counter.
    
hex2bcd16_L1:
	; Shift binary left	
	mov a, R1
	mov c, acc.7 ; This way [R1,R0] remains unchanged!
	mov a, R0
	rlc a
	mov R0, a
	mov a, R1
	rlc a
	mov R1, a
    
	; Perform bcd + bcd + carry using BCD arithmetic
	mov a, R2
	addc a, R2
	da a
	mov R2, a
	mov a, R3
	addc a, R3
	da a
	mov R3, a
	mov a, R4
	addc a, R4
	da a
	mov R4, a

	djnz R5, hex2bcd16_L1

	ret

Initialize_Serial_Port:
	; Configure serial port and baud rate
	clr TR1 ; Disable timer 1
	anl TMOD, #0x0f ; Mask the bits for timer 1
	orl TMOD, #0x20 ; Set timer 1 in 8-bit auto reload mode
    orl PCON, #80H ; Set SMOD to 1
	mov TH1, #low(TIMER_1_RELOAD)
	mov TL1, #low(TIMER_1_RELOAD) 
	setb TR1 ; Enable timer 1
	mov SCON, #52H
	ret


	
Initialize_LEDs:
    ; Turn off LEDs
	mov	LEDRA,#0x00
	mov	LEDRB,#0x00
	ret
	
Initialize_ADC:
	; Initialize SPI pins connected to LTC2308
	clr	LTC2308_MOSI
	clr	LTC2308_SCLK
	setb LTC2308_ENN
	ret

LTC2308_Toggle_Pins:
    mov LTC2308_MOSI, c
    setb LTC2308_SCLK
    mov c, LTC2308_MISO
    clr LTC2308_SCLK
    ret

LTC2308_RW:
    clr a 
	clr	LTC2308_ENN ; Enable ADC

    ; Send 'S/D', get bit 11
    setb c ; S/D=1 for single ended conversion
    lcall LTC2308_Toggle_Pins
    mov acc.3, c
    ; Send channel bit 0, get bit 10
    mov c, b.2 ; O/S odd channel select
    lcall LTC2308_Toggle_Pins
    mov acc.2, c 
    ; Send channel bit 1, get bit 9
    mov c, b.0 ; S1
    lcall LTC2308_Toggle_Pins
    mov acc.1, c
    ; Send channel bit 2, get bit 8
    mov c, b.1 ; S0
    lcall LTC2308_Toggle_Pins
    mov acc.0, c
    mov R1, a
    
    ; Now receive the lest significant eight bits
    clr a 
    ; Send 'UNI', get bit 7
    setb c ; UNI=1 for unipolar output mode
    lcall LTC2308_Toggle_Pins
    mov acc.7, c
    ; Send 'SLP', get bit 6
    clr c ; SLP=0 for NAP mode
    lcall LTC2308_Toggle_Pins
    mov acc.6, c
    ; Send '0', get bit 5
    clr c
    lcall LTC2308_Toggle_Pins
    mov acc.5, c
    ; Send '0', get bit 4
    clr c
    lcall LTC2308_Toggle_Pins
    mov acc.4, c
    ; Send '0', get bit 3
    clr c
    lcall LTC2308_Toggle_Pins
    mov acc.3, c
    ; Send '0', get bit 2
    clr c
    lcall LTC2308_Toggle_Pins
    mov acc.2, c
    ; Send '0', get bit 1
    clr c
    lcall LTC2308_Toggle_Pins
    mov acc.1, c
    ; Send '0', get bit 0
    clr c
    lcall LTC2308_Toggle_Pins
    mov acc.0, c
    mov R0, a

	setb LTC2308_ENN ; Disable ADC

	ret	
SendNumber:
	mov a, #'\r'
	lcall putchar
	mov a, R3
	swap a
	anl a, #0x0f
	orl a, #'0'
	lcall putchar
	mov a, #'.'
	lcall putchar
	mov a, R3
	anl a, #0x0f
	orl a, #'0'
	lcall putchar
	mov a, R2
	swap a
	anl a, #0x0f
	orl a, #'0'
	lcall putchar
	mov a, R2
	anl a, #0x0f
	orl a, #'0'
	lcall putchar
	ret

putchar_1:
	jbc	TI,putchar_L1
	sjmp putchar
putchar_L1:
	mov	SBUF,a
	ret
	
putchar: ;Copy pasted whack
    jnb TI, putchar
    clr TI
    mov SBUF, a
    ret
getchar:
	jbc	RI,getchar_L1
	sjmp getchar
getchar_L1:
	mov	a,SBUF
	ret

;---------------------------------;
; Wait 'R2' milliseconds          ;
;---------------------------------;
Wait_Milli_Seconds mac
	push AR2
	mov R2, %0
	lcall ?Wait_Milli_Seconds
	pop AR2
endmac

?Wait_Milli_Seconds:
	push AR0
	push AR1
L3: mov R1, #50
L2: mov R0, #223
L1: djnz R0, L1 ; 3 cycles->3*30ns*250=20.07us
    djnz R1, L2 ; 20.07us*50=1.004ms
    djnz R2, L3 ; number of millisecons to wait passed in R2
    pop AR1
    pop AR0
    ret
	
;---------------------------------;
; Toggles the 'E' pin in the LCD  ;
;---------------------------------;
ELCD_pulse:
	setb ELCD_E
	lcall Wait40uSec
	clr ELCD_E
    lcall Wait40uSec ; This line is needed in the DE1SoC running an 8051 because it is much faster than the AT89LP51RC2
    ret

;---------------------------------;
; Writes acc to LCD in 4-bit mode ;
;---------------------------------;
ELCD_byte:
	; Write high 4 bits first
	mov c, ACC.7
	mov ELCD_D7, c
	mov c, ACC.6
	mov ELCD_D6, c
	mov c, ACC.5
	mov ELCD_D5, c
	mov c, ACC.4
	mov ELCD_D4, c
    lcall ELCD_pulse
	; Write low 4 bits next
	mov c, ACC.3
	mov ELCD_D7, c
	mov c, ACC.2
	mov ELCD_D6, c
	mov c, ACC.1
	mov ELCD_D5, c
	mov c, ACC.0
	mov ELCD_D4, c
    lcall ELCD_pulse
	ret

;---------------------------------;
; Write data to LCD               ;
;---------------------------------;
WriteData mac
	mov a, %0
	lcall ?WriteData
endmac
	
?WriteData:
	setb ELCD_RS
	ljmp ELCD_byte

;---------------------------------;
; Write command to LCD            ;
;---------------------------------;
WriteCommand mac
	mov a, %0
	lcall ?WriteCommand
endmac

?WriteCommand:
	clr ELCD_RS
	ljmp ELCD_byte

;---------------------------------;
; Configure LCD in 4-bit mode     ;
;---------------------------------;
ELCD_4BIT:
	clr ELCD_E   ; Resting state of LCD's enable pin is zero
	clr ELCD_RW  ; We are only writing to the LCD in this program
	
	; After power on, let the LCD start up before initializing
	Wait_Milli_Seconds(#40)
	
	; First make sure the LCD is in 8-bit mode and then change to 4-bit mode
	WriteCommand(#0x33)
	WriteCommand(#0x33)
	WriteCommand(#0x32) ; change to 4-bit mode

	; Configure the LCD
	WriteCommand(#0x28)
	WriteCommand(#0x0c)
	WriteCommand(#0x01) ;  Clear screen command (takes some time)

    ;Wait for the clear screen command to finish.
	Wait_Milli_Seconds(#2)
    ret

;---------------------------------;
; Send a constant string to LCD   ;
;---------------------------------;
Send_Constant_String mac
	push dph
	push dpl
	push acc
	mov dptr, %0
	lcall ?Send_Constant_String
	pop acc
	pop dpl
	pop dph
endmac

?Send_Constant_String:
    clr a
    movc a, @a+dptr
    jz ?Send_Constant_String_Done
    lcall ?WriteData
    inc dptr
    sjmp ?Send_Constant_String
?Send_Constant_String_Done:
    ret	

;---------------------------------;
; Set LCD cursor at row, column   ;
;---------------------------------;
Set_Cursor mac
	push acc
	mov a, #%1
	dec a
	lcall ?Set_Cursor_%0 ; Select column and row
	pop acc
endmac

?Set_Cursor_2:
	orl a, #01000000B
?Set_Cursor_1:
	orl a, #10000000B
	ljmp ?WriteCommand ; Select column and row

;---------------------------------;
; Display a BCD number in the LCD ;
;---------------------------------;
Display_BCD mac
	push ar0
	mov r0, %0
	lcall ?Display_BCD
	pop ar0
endmac

?Display_BCD:
	push acc
	; Write most significant digit
	mov a, r0
	swap a
	anl a, #0fh
	orl a, #30h
	lcall ?WriteData
	; write least significant digit
	mov a, r0
	anl a, #0fh
	orl a, #30h
	lcall ?WriteData
	pop acc
	ret

;------------------------------------;
; Display a char in the LCD          ;
;------------------------------------;
Display_char mac
	push acc
	mov a, %0
	lcall ?WriteData
	pop acc
endmac

Read_ADC_Channel MAC
	mov b, #%0
	lcall _Read_ADC_Channel
ENDMAC

_Read_ADC_Channel:
	clr CE_ADC
	mov R0, #00000001B ; Start bit:1
	lcall DO_SPI_G
	mov a, b
	swap a
	anl a, #0F0H
	setb acc.7 ; Single mode (bit 7).
	mov R0, a
	lcall DO_SPI_G
	mov a, R1 ; R1 contains bits 8 and 9
	anl a, #00000011B ; We need only the two least significant bits
;	da a
	mov vResult+1, a ; Save result high.
	mov R0, #55H ; It doesn't matter what we transmit...
	lcall DO_SPI_G
	mov vResult, R1 ; R1 contains bits 0 to 7. Save result low.
	setb CE_ADC
	ret	


;---------------------------------;
; Send a BCD number to PuTTY      ;
;---------------------------------;

Send_BCD mac
	push ar0
	mov r0, %0
	lcall ?Send_BCD
	pop ar0
	endmac
?Send_BCD:
	push acc
	; Write most significant digit
	mov a, r0
	swap a
	anl a, #0fh
	orl a, #30h
	lcall putchar
	; write least significant digit
	mov a, r0
	anl a, #0fh
	orl a, #30h
	lcall putchar
	pop acc
	ret

INIT_SPI: ;Copy pasted whack
	setb MY_MISO ; Make MISO an input pin
	clr MY_SCLK ; For mode (0,0) SCLK is zero
	ret
DO_SPI_G:;Copy pasted whack
	push acc
	mov R1, #0 ; Received byte stored in R1
	mov R2, #8 ; Loop counter (8-bits)
DO_SPI_G_LOOP:;Copy pasted whack
	mov a, R0 ; Byte to write is in R0
	rlc a ; Carry flag has bit to write
	mov R0, a
	mov MY_MOSI, c
	setb MY_SCLK ; Transmit
	mov c, MY_MISO ; Read received bit
	mov a, R1 ; Save received bit in R1
	rlc a
	mov R1, a
	clr MY_SCLK
	djnz R2, DO_SPI_G_LOOP
	pop acc
	ret

SendString: ;Copy pasted whack
    clr A
    movc A, @A+DPTR
    jz SendStringDone
    lcall putchar
    inc DPTR
    sjmp SendString
SendStringDone: ;Copy pasted whack
    ret

	