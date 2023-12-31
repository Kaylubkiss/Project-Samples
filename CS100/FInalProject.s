; FINAL PROJECT
; Due Date: 11/8/21
; Student Name:	Caleb Kissinger
; Section: B

;NOTES FOR THE GRADER OF THE PROJECT:

;-DEMONSTRATED TO PROFESSOR DUBA AT 11:42 AM PST

;-FULFILLED ALL THE REQUIREMENTS OF A CONTROLLER AND TWO BUTTONS

;-HAVE IMPLEMENTED THE SPEAKER TO BE CONTROLLED BY THE CONTROLLER

;-BAUD RATE IS SET TO 57600. I'm not sure how high it could've gone. 921600 is on teraterm,
;but maybe it can be greater than that.



;-WAS NOT DEMONSTRATED, BUT LEDS WOULD WORK IF PROPERLY WIRED UP TO.


;NOTES TO SHOW FOR PEOPLE WANTING TO TEST CODE:

;-PB0-3 ARE USED FOR THE LEDS TO LIGHT-UP
;-PD2-3 ARE USED FOR THE BUTTONS
;-PE0-3 ARE USED FOR THE CONTROLLER AXIS
;-PB7 IS USED FOR THE POWER OF THE SPEAKER


; Title: "Project_Main"
; © 2021 DigiPen, All Rights Reserved.
	
	GET FinalProj_Data.s             		; Get/include the data file
	GLOBAL __main               		; Global main function
	AREA FinalProj_Main, CODE, READONLY		; Area of code that is read only
	ALIGN 2                     		; Align the data boundary to a multiple of 2
	ENTRY                       		; Entry into the code segment


 ;======Bit Methods-Nothing-new-here===================;
;---------------------------------  
; _SETBITS: Sets bits in memory (passing "1001" in R1 will set bits at position 0 and 3 in R0)
_SETBITS ; Turn on bits at address R0 specified by 1's in R1 
    PUSH {R4-R11, LR}
	LDR R4, [R0]
	ORR R4, R1
	STR R4, [R0]
    POP {R4-R11, LR}
	BX LR

  ;--------------------------------- 
; _CLEARBITS: Clears bits in memory (passing "1001" in R1 will clear bits at position 0 and 3 in R0)
_CLEARBITS ; Turn off bits at address R0 specified by 1's in R1
    PUSH {R4-R11, LR}
	LDR R4, [R0]
	MVN R3, R1
	AND R4, R3
	STR R4, [R0]
    POP {R4-R11, LR}
  BX LR	
;----------------------------------
_DELAY ; Loop R0 times
    PUSH {R4-R11, LR}
	MOV R10, R0
LOOPDELAY
	SUBS R10, R10, #1
	CMP R10, #0 
	BNE LOOPDELAY
    POP {R4-R11, LR}
  BX LR
  
delay 
	MOV R10, #COUNT1
L
	SUBS R10, R10, #1
	CMP R10, #0 
	BNE L
	BX LR
;====================SETUP=======================;
;---------------------------------  
; Subroutine: ports_activation 
; Description: Initializes output Ports so they are set up for use. If we don't
;   do this, the pin won't work. 

pwm_setup 
	
  PUSH { R4-R11, LR } ; stack preserved registers and link register


   ;STUDENT CODE STARTS HERE 
   
   SETBITS SYSCTL_RCC, 2_11000000000000000000
   CLEARBITS SYSCTL_RCC, 2_110000000000000000
   
   WRITEBITS PWM0CTL, 0X00
   WRITEBITS PWM0GENB, 0x80C
   WRITEBITS PWM0CMPB, 0x50

   WRITEBITS PWM0LOAD, MIDDLE_F_DIV
   WRITEBITS PWM0CMPB, 0x80
   SETBITS PWM0CTL, 1
   SETBITS PWMENABLE, 2_10
   
   ;STUDENT CODE ENDS HERE
   

	; Return back to the calling subroutine.
    POP { R4-R11, LR }
    BX LR	

_led_pins_activation 
    PUSH { R4-R11, LR } ; stack preserved registers and link register
	
      SETBITS RCGCGPIO, 0x1B;****Step 3A**** ; A(1), B(2), C(4), D(8), E(10), F(20), we're only turning on ports A+B+E
	  SETBITS GPIODDEN, 0XFF
      SETBITS GPIOBDEN, 0xFF ;****Step 3B****	;Configure used pins of Port B as digital ;7th bit for PWM out_pins
        
    POP { R4-R11, LR } ; Restore the link register and R4-R11 in case we changed them here
	BX LR;Return back to the calling subroutine.

output_pins_config 
	
  PUSH { R4-R11, LR } ; stack preserved registers and link register

   ;STUDENT CODE STARTS HERE 
   SETBITS RCGCPWM, 1 ;set bit 0 of RCGCPWM
   SETBITS RCGCGPIO, 2_10
   LDR r4, =RCGCGPIO
   LDR r5, [r4]
   CMP r5, #2_10
   BLNE delay 
   
   SETBITS GPIOBDEN, 1<<7
   SETBITS GPIOBDIR, 1<<7
   SETBITS GPIOBAFSEL, 1<<7
   SETBITS GPIOBPCTL, 1<<30
  
   ;STUDENT CODE ENDS HERE 
   
    ; Return back to the calling subroutine.
    POP { R4-R11, LR }
    BX LR
;---------------------------------  
; Subroutine: led_initialization
; Description: Initializes our LEDs so they are set up for output. If we don't
;   do this, we will not be able to correctly turn them on or off later.

_led_initialization
    PUSH { R4-R11, LR } ; stack preserved registers and link register
      SETBITS GPIOBDIR, 0xFF ;****Step 3C**** ; MAKE SURE ALL PORTB PINS (PB0-PB7) ARE SET UP FOR OUTPUT ;7th bit for the PWM stuff
      CLEARBITS GPIOBDATA_RW, 0XFF;****Step 3D****; MAKE SURE ALL PORTB PINS (PB0-PB7) START WITH 0 ON THE OUTPUT	
    POP { R4-R11, LR }; restore the preserved registers and link register
	BX LR; Return back to the calling subroutine.



;====================ADC Subroutines=======================;
;---------------------------------  
; Loops until ADC value is ready to be read
; Inputs: R0 = address of value to read
; Outputs: None
_wait_adc
	PUSH {LR, R4-R11}
_WAIT
; STUDENT CODE HERE
	LDR R4, [R0]
	TST R4, #1
	BEQ _WAIT
; END STUDENT CODE
	POP {LR, R4-R11}
	BX LR

;----Checking the blue or green button if its on----
_CHECKBLUE
	PUSH {LR, R4-R11}
	;LDR R4, =GPIODDATA_RW
	LDR R8, [R0]
	TST R8, #2_010
	BLNE _BUTTON_BLUE_OFF
	BLEQ _BUTTON_BLUE_ON
	POP{LR, R4-R11}
	BX LR

_CHECKGREEN
	PUSH {LR, R4-R11}
	;LDR R0, =GPIODDATA_RW
	LDR R8, [R0]
	ANDS R8, #2_010
	BLNE _BUTTON_GREEN_OFF
	BLEQ _BUTTON_GREEN_ON
	POP{LR, R4-R11}
	BX LR

;---turning the green button on or off
_BUTTON_GREEN_ON
	PUSH {LR, R4-R11}
	TRANSMIT8BITS 'G'
	TRANSMIT8BITS 13
	POP {LR, R4-R11}
	BX LR
	
_BUTTON_GREEN_OFF
	PUSH {LR, R4-R11}
	TRANSMIT8BITS 'g'
	TRANSMIT8BITS 13
	POP {LR, R4-R11}
	BX LR
	
;---turning the blue button on or off	
_BUTTON_BLUE_OFF
	PUSH {LR, R4-R11}
	TRANSMIT8BITS 'b'
	TRANSMIT8BITS 13
	POP {LR, R4-R11}
	BX LR
	
_BUTTON_BLUE_ON
	PUSH {LR, R4-R11}
	TRANSMIT8BITS 'B'
	TRANSMIT8BITS 13
	POP {LR, R4-R11}
	BX LR

;adding and branching to transmit
_TRANSLATE_AND_BRANCH
	PUSH {R4-R11, LR}
	CMP R0, #9
	ADDLE R0, #0X30
	ADDGT R0, #0X37
	BL _TRANSMIT

	POP{R4-R11, LR}
	BX LR
	
;---------------------------------  
; Reads the last 3 nibbles of the register.
; Inputs: R0 = address of value to read
; Outputs: Masked result of ADC in R0
_read_adc
	PUSH {LR, R4-R11}
; STUDENT CODE HERE
	LDR R4, [R0]
	LDR R5, =0xFFF
	AND R4, R5
	MOV R0, R4
; END STUDENT CODE
	POP {LR, R4-R11}
	BX LR


; Subroutine: adc_initialization
;SO OUR CONTROLLER CAN BE READ, WE INITIALIZE THE PINS AND THE SAMPLE COUNT SO THAT ALL THE AXIS OF THE CONTROLLER CAN BE SHOWN.
_adc_initialization
    PUSH { R4-R11, LR } ; stack preserved registers and link register
	
;---ADC Module Initialization (PP 817): this is copy-pasted-edited-- see lecture------------
	SETBITS RCGCADC, 0x01 ;****Step 4A****;1.Enable the ADC clock - RCGCADC (PP 352).
;2.We did this in Step 3A -Enable RCGCGPIO register FOR PORT E(see page 340).
	CLEARBITS GPIOEDEN, 2_01111;****Step 4C****;4.Config AINx AS analog input-clear corresponding DEN bit in(GPIOEDEN) (PP682).
	SETBITS GPIOEAMSEL, 2_01111;5. WRITE TO GPIOEAMSEL (687) ANALOG INPUTS TO BE ANALOG.
;6. SAMPLE SEQUENCER PRIORITY BEYOND SCOPE OF COURSE.

;CONFIGURE Sample Sequencer 0------------------------------------------------

	CLEARBITS ADCACTSS, 0x1 ;****Step 4D****;1. disable SAMPLE SEQUENCER-clear ASENn bit in ADCACTSS.
	CLEARBITS ADCEMUX, 0XF;2. SET SS0 TRIGGER IN ADCEMUX TO USE 'PROCESSOR' TRIGGERING.
;3. NOT using a PWM generator as the trigger source.
	WRITEBITS ADCSSMUX0, 0x00001032;****Step 4E****;SET BITS FOR EACH input source in the ADCSSMUXn register.
	WRITEBITS ADCSSCTL0, 0x00006000;****Step 4F****;5. SET ADCSSCTL0 SO THAT 2ND IN SEQUENCE ENDS SEQUENCE AND STARTS INTERRUPT.
	;WRITEBITS ADCSSCTL0,0X000000014
;6. SKIP - NOT USING INTERRUPT6. If interrupts are to be used, set the corresponding MASK bit in the ADCIM register.
	;SETBITS ADCIM, 2_100
	WRITEBITS ADCPC, 0X3 ; set samples per second to 125,000. 

	SETBITS ADCACTSS, 0x1;****Step 4G****;7. Enable sample sequencer0 - setting the  ASEN0 bit in the ADCACTSS register.

			 
    POP { R4-R11, LR }
    BX LR; Return back to the calling subroutine.

;INITIALIZES THE PINS FOR SERIALIZATION, SO THAT TERATERM CAN SHOW EVERYTHING BEING CHANGED
SERIAL_INITIALIZATION
  PUSH { R4-R11, LR }  	; push preseverd registers and link register onto the stack to save them
	SETBITS GPIOADEN, 0x3; Enable PA0 and PA1 as digital ports (not analog) - already done, but demonstrating how
	SETBITS RCGCUART, 0x1 ; 3A- enable the UART module using RCGCUART (pp 344)
	SETBITS GPIOAAFSEL, 0X3		; 3C- Set GPIO Alternate function select GPIOAFSEL (671/1344)
									; DEFAULTS TO 2-mA DRIVE, THIS IS OK; Configure GPIO current level/Slew rate for mode (673/681)							
	SETBITS GPIOAPCTL, 0x11;DEFAULTS TO PA0/PA1 BEING UART, OK;Configure PMCn fields in GPIOPCTL register to assign UART to appropriate pins (688/1351)

SERAL_CONFIGURATION
; EXAMPLE SPECIFIC TO 9600 BAUD/8BIT/1 STOP/NO PARITY/FIFO OFF/NO INTERRUPTS
	CLEARBITS UART0CTL, 0X1		;5A- DISABLE UART WHILE OPERATING-- CLEAR UARTEN BIT (O) IN UARTCTL	
	; NOTE** PLL IS SET TO 3, SO WE'RE WORKING WITH 48MHz.  *
	; SET BAUD-RATE-DIVISOR FOR BRD=48,000,000/(CLKDiv-16 or 8)(9600)=III.FFFFF  
	WRITEBITS UART0IBRD, 52	 	;set to 57600 BAUD
	WRITEBITS UART0FBRD, 5	;5C- Set UART0FBRD = INT(0.FFFFF*64+0.5) - FROM 0 TO 64 for fraction
	WRITEBITS UART0LCRH, 2_11<<5		;5D- Select serial com. parameters in UARTLCRH (8 BITS, the rest should be default)
	WRITEBITS UART0CTL, 0X0  				;5E- Configure UART Clock source in UARTCTL (DEFAULT=0=SYSTEM CLOCK+DIVISOR)
	WRITEBITS UART0CTL, 0X301	;5F- Enable UART0 for receive, Enable UART0 for Transmit, Enable UART0 total
	
	POP { R4-R11, LR } ; Pop back the preserved registers and link register to what they were when we started SERIAL_INITIALIZATION
	BX LR ; Return back to the calling subroutine.

;--playing the c, e, g, or f note. Changes only when the stick is moved, since there is
;no way to clearbits present in this code
_PLAY_MIDDLE_C
	PUSH {R4-R11, LR}

	WRITEBITS PWM0LOAD, MIDDLE_C_DIV
	WRITEBITS PWM0CMPB, 0x80 ;playing with higher volume, because why not
	POP{R4-R11, LR}
	BX LR
	
	
_PLAY_MIDDLE_E
	PUSH {R4-R11, LR}
	WRITEBITS PWM0LOAD, MIDDLE_E_DIV

	POP{R4-R11, LR}
	BX LR
	
_PLAY_MIDDLE_G
	PUSH {R4-R11, LR}
	WRITEBITS PWM0LOAD, MIDDLE_G_DIV
	POP{R4-R11, LR}
	BX LR


_PLAY_MIDDLE_F
	PUSH {R4-R11, LR}
	WRITEBITS PWM0LOAD, MIDDLE_F_DIV
	POP{R4-R11, LR}
	BX LR


;-------------------------------------------
; Subtroutine: _TRANSMIT
; checks for the output fifo to be clear, then sends lowest 8 bits of R0
_TRANSMIT
  PUSH { R4-R11, LR } 
  ; stack preserved registers and link register
_WAIT_FOR_CLEAR_OUTPUT_FIFO
	LDR R4, =UART0FR	;6A1- Load the address of the UART0 Flag register
	LDR R5, [R4]		;6A2- Get the contents of the UART0 Flag register into a register we're not using
	TST R5, #1<<5 ; 6B- Check the Transmit FIFO0 Full bit (TXFF) on that register with a TST (single bit ANDS)
	BNE  _WAIT_FOR_CLEAR_OUTPUT_FIFO; 6B1- If the Transmit FIFO0 IS full, go back to _WAIT_FOR_CLEAR_OUTPUT_FIFO
	AND R0, #0xFF; 6C- Mask out all but the lowest 8 bits for sending from R0
 	LDR R6, =UART0DR; 6D1- Place the data in R0 into the UART0Data Register (UART0DR)
	STR R0, [R6]; 6D2- (two lines)
	
	POP { R4-R11, LR } ; Pop back the preserved registers and link register
	BX LR ; Return back to the calling subroutine.


;==================LOGIC==========================;
	LTORG ;so that extra lines can be read
	
__main
	
	BL _led_pins_activation 
	BL _led_initialization
	BL _adc_initialization
	BL SERIAL_INITIALIZATION

	
	BL output_pins_config
	bl delay
	BL pwm_setup
 
   
	
	
	
	
_RUNLOOP ; MADE UP OF READ-SEND-RECEIVE SECTIONS
	
	; Clear interrupt flag so that we know we're reading a *new* adc conversion	
	SETBITS ADCISC, 0X01
	; Start processor trigger (tell sequencer 0 to start converting ADCs)
	SETBITS ADCPSSI, 1
	
	LDR R0, =ADCRIS
	BL _wait_adc
	
	LDR R0, =ADCSSFIFO0
	BL _read_adc
	MOV R5, R0  ; x-value-- in R5.
	LDR R0, =ADCSSFIFO0
	BL _read_adc
	MOV R6, R0  ; store y-value
	LDR R0, =ADCSSFIFO0
	BL _read_adc
	MOV R9, R0 ; store another axis
	LDR R0, =ADCSSFIFO0
	BL _read_adc
	MOV R10, R0 ; store another axis
	BL _read_adc
	
	
leds_off
	CLEARBITS GPIOBDATA_RW, 0XFF ; TURN OFF ALL PB OUTPUTS.

	
	;COMPARING THE AXIS VALUES TO TURN ON AND OFF LEDS, AND TRANSMIT A VALUE TO INDICATE THAT THE CONTROLLERS ARE READ
	CMP R6, #THRESH_LOW 
	BLLT _TURN_ON_UP_LED
	BLGT _TURN_OFF_UP_LED
	
	CMP R6, #THRESH_HIGH
	BLLT _TURN_OFF_DOWN_LED
	BLGT _TURN_ON_DOWN_LED
	
	
	CMP R5, #THRESH_HIGH
	BLGT _TURN_ON_RIGHT_LED
	BLLT _TURN_OFF_RIGHT_LED
	
	CMP R5, #THRESH_LOW
	BLGT _TURN_OFF_LEFT_LED

	
	CMP R5, #THRESH_HIGH
	BLGT _TURN_OFF_LEFT_LED

	
	
	
	;these will change (transmit) for the other controller, as well as changing the notes
	CMP R10, #THRESH_HIGH
	CMP R10, #THRESH_LOW
	CMP R9, #THRESH_HIGH
	CMP R9, #THRESH_LOW
	

	
	
	;transmit the byte-operated values read in
	AND R0, R5, #0XF00
	LSR R0, #8
	BL _TRANSLATE_AND_BRANCH
	AND R0, R5, #0X0F0
	LSR R0, #4
	BL _TRANSLATE_AND_BRANCH
	AND R0, R5, #0X00F
	BL _TRANSLATE_AND_BRANCH
	AND R0, R6, #0XF00
	LSR R0, #8
	BL _TRANSLATE_AND_BRANCH
	AND R0, R6, #0X0F0
	LSR R0, #4
	BL _TRANSLATE_AND_BRANCH
	AND R0, R6, #0X00F
	BL _TRANSLATE_AND_BRANCH
	
	;transmit the OTHER controller
	TRANSMIT8BITS 9
	AND R0, R9, #0XF00
	LSR R0, #8
	BL _TRANSLATE_AND_BRANCH
	AND R0, R9, #0X0F0
	LSR R0, #4
	BL _TRANSLATE_AND_BRANCH
	AND R0, R9, #0X00F
	BL _TRANSLATE_AND_BRANCH
	AND R0, R10, #0XF00
	LSR R0, #8
	BL _TRANSLATE_AND_BRANCH
	AND R0, R10, #0X0F0
	LSR R0, #4
	BL _TRANSLATE_AND_BRANCH
	AND R0, R10, #0X00F
	BL _TRANSLATE_AND_BRANCH
	
	;keep moving down the lines by carrage return, followed by newline
	;scrolls down the page
	TRANSMIT8BITS 13
	TRANSMIT8BITS 10
	
	;spacing for blue button, and reading and transmitting blue button
	TRANSMIT8BITS 9
	TRANSMIT8BITS 9 
	TRANSMIT8BITS 9
	LDR R0, =GPIODDATA_RW
	LDR R7, [R0]
	ANDS R7, #2_100
	BLEQ _BUTTON_BLUE_OFF
	BLNE _BUTTON_BLUE_ON
	
	;spacing for green button, and reading and transmitting green button
	TRANSMIT8BITS 9
	TRANSMIT8BITS 9
	TRANSMIT8BITS 9
	TRANSMIT8BITS 9
	TRANSMIT8BITS 9
	LDR R0, =GPIODDATA_RW
	LDR R8, [R0]
	ANDS R8, #2_1000
	BLEQ _BUTTON_GREEN_OFF
	BLNE _BUTTON_GREEN_ON
	
	;just purely changing the notes; probably not concise coding wise, but works
	CMP R10, #THRESH_HIGH
	BLGT _PLAY_MIDDLE_G

	
	
	CMP R10, #THRESH_LOW
	BLLT _PLAY_MIDDLE_F
	
	
	CMP R9, #THRESH_HIGH
	BLGT _PLAY_MIDDLE_C
	
	
	CMP R9, #THRESH_LOW
	BLLT _PLAY_MIDDLE_E
	
	;end of changing notes

	B _RUNLOOP

	
	;****STEP 6C**** IF COMPARE ENDS WITH OTHER RESULTS, BRANCH TO TURN OFF THAT LED
			;****STEP 6D...**** NOW DO THE SAME FOR DOWN, LEFT, AND RIGHT.

;The branches for threshold reaches to turn on LEDs; not used for demonstration, BUT SHOULD WORK
_TURN_ON_UP_LED	
	PUSH { R4-R11, LR }
	SETBITS GPIOBDATA_RW, LED_UP	 
	POP { R4-R11, LR }
	BX LR
_TURN_ON_LEFT_LED
	PUSH { R4-R11, LR }
	SETBITS GPIOBDATA_RW, LED_LEFT
	POP { R4-R11, LR }
	BX LR
_TURN_ON_RIGHT_LED
	PUSH { R4-R11, LR }
	SETBITS GPIOBDATA_RW, LED_RIGHT
	POP { R4-R11, LR }
	BX LR
_TURN_ON_DOWN_LED
	PUSH { R4-R11, LR }
	SETBITS GPIOBDATA_RW, LED_DOWN
	POP { R4-R11, LR }
	BX LR
_TURN_OFF_UP_LED
	PUSH { R4-R11, LR }
	CLEARBITS GPIOBDATA_RW, LED_UP
	POP { R4-R11, LR }
	BX LR
_TURN_OFF_LEFT_LED
	PUSH { R4-R11, LR }
	CLEARBITS GPIOBDATA_RW, LED_LEFT 
	POP { R4-R11, LR }
	BX LR
_TURN_OFF_RIGHT_LED
	PUSH { R4-R11, LR }
	CLEARBITS GPIOBDATA_RW, LED_RIGHT
	POP { R4-R11, LR }
	BX LR
_TURN_OFF_DOWN_LED
	PUSH { R4-R11, LR }
	CLEARBITS GPIOBDATA_RW, LED_DOWN
	POP { R4-R11, LR }
	BX LR
; END STUDENT CODE
	

	END
		
		
		
		
