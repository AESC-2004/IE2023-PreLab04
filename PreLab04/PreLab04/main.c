/*
 * PreLab04.c
 *
 * Created: 29/03/2025 10:16:44 p. m.
 * Author: ang50
 */
/****************************************************************************************************/
// Encabezado (Libraries)
#define F_CPU 16000000
#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdint.h>

//Variables y constantes
//TIM0
#define T0VALUE			178
//Multiplexación
#define DISP0			4
#define DISP1			5
#define LEDS			6
uint8_t MUX_SEQUENCE	= 0b00000001;
//Botones de conteo
#define COUNTUP			2
#define COUNTDWN		1
uint8_t COUNTUP_LAST;
uint8_t COUNTDWN_LAST;
//Valores HEX para Displays
const uint8_t DISP7SEG[16] = {
	0x5F, 0x06, 0x9B, 0x8F, 0xC6, 0xCD, 0xDD, 0x07, 0xDF, 0xC7,
	0xD7, 0xDC, 0x59, 0x9E, 0xD9, 0xD1
};
//Variables de conteo
uint8_t LEDS_VAL		= 0xFF;
uint8_t COUNT;
uint8_t DISP0_VAL;
uint8_t DISP1_VAL;



/****************************************************************************************************/
// Function prototypes
void SETUP();
void initTMR0();
void initPCINT();
void initADC();

/****************************************************************************************************/
// Main Function
int main(void)
{
	SETUP();
	
	while (1)
	{
		DISP0_VAL = DISP7SEG[COUNT];
		DISP1_VAL = DISP7SEG[COUNT];
	}
}

/****************************************************************************************************/
// NON-Interrupt subroutines
void SETUP()
{
	// Desactivamos interrupciones
	cli();
	// Desactivamos UART1
	UCSR1B			= 0x00;  
	// PORTD: Salida de displays y LEDs							|	PORTD: XXXX XXXX
	DDRD			= 0xFF;
	PORTD			= 0x00;
	// PORTB: Transistores para multiplexación y PINCHANGE		|	PORTB: 0XXX 0110
	DDRB	= (1 << DISP0) | (1 << DISP1) | (1 << LEDS);
	PORTB	= (1 << COUNTUP) | (1 << COUNTDWN);
	// Iniciamos interrupciones
	initTMR0();
	initPCINT();
	// Valor inicial del contador cero
	COUNT			= 0;
	// Valores iniciales de COUNTUP y COUNTDWN
	COUNTUP_LAST	= 0;
	COUNTDWN_LAST	= 0;
	// Valores iniciales de DISP0,1_VAL
	DISP0_VAL		= 0;
	DISP1_VAL		= 0;
	// Rehabilitamos interrupciones
	sei();
}

void initTMR0()
{
	TCCR0B	= (1 << CS02) | (0 << CS01) | (1 << CS00);
	TIMSK0	= (1 << TOIE0);
	TCNT0	= T0VALUE;
}

void initPCINT()
{
	PCICR	= (1 << PCIE0);
	PCMSK0	= (1 << COUNTUP) | (1 << COUNTDWN);
}

/****************************************************************************************************/
// Interrupt routines
ISR(TIMER0_OVF_vect)
{
	// Recargamos TCNT0
	TCNT0	= T0VALUE;
	// Verificamos qué debe sacar PORTD según la secuencia de multiplexación
	if (MUX_SEQUENCE == 0b00000001)
	{
		// Si DISP0 estaba encendido, ahora queremos mostrar DISP1
		// Apagamos DISP0
		PORTB			&= ~(1 << DISP0);
		// Cargamos DISP1_VAL en PORTD
		PORTD			= DISP1_VAL;
		// Encendemos DISP1
		PORTB			|= (1 << DISP1);
		//Y actualizamos el valor de MUX_SEQUENCE
		MUX_SEQUENCE	= 0b00000010;
	} else if (MUX_SEQUENCE == 0b00000010)
	{
		// Si DISP1 estaba encendido, ahora queremos mostrar LEDS
		// Apagamos DISP1
		PORTB			&= ~(1 << DISP1);
		// Cargamos COUNT en PORTD
		PORTD			= LEDS_VAL;
		// Encendemos LEDS
		PORTB			|= (1 << LEDS);
		//Y actualizamos el valor de MUX_SEQUENCE
		MUX_SEQUENCE	= 0b00000100;
	} else if (MUX_SEQUENCE == 0b00000100)
	{
		// Si LEDS estaba encendido, ahora queremos mostrar DISP0
		// Apagamos LEDS
		PORTB			&= ~(1 << LEDS);
		// Cargamos DISP0_VAL en PORTD
		PORTD			= DISP0_VAL;
		// Encendemos DISP0
		PORTB			|= (1 << DISP0);
		//Y actualizamos el valor de MUX_SEQUENCE
		MUX_SEQUENCE	= 0b00000001;		
	}
}

ISR(PCINT0_vect)
{
	// Revisamos qué botón fue presionado
	// 0 LÓGICO = PRESSED
	// 1 LÓGICO = NOT PRESSED
	if ((PINB & ((1 << COUNTUP))) == (0 << COUNTUP)) 
	{
		//Si COUNTUP fue presionado...
		//Revisamos su estado anterior. Si antes estaba presionado, revisamos COUNTDWN, y...
		//si antes COUNTUP no estaba presionado, actualizamos su estado y ejecutamos COUNT++
		if  (COUNTUP_LAST == 1)
		{	
			
		} else 
		{
			COUNTUP_LAST = 1;
			COUNT++;
			if (COUNT > 15) COUNT = 0;
		}
	} else if ((PINB & ((1 << COUNTUP))) == (1 << COUNTUP))
	{
		COUNTUP_LAST = 0;
	}
	if ((PINB & ((1 << COUNTDWN))) == (0 << COUNTDWN))
	{
		//Si COUNTDWN fue presionado...
		//Revisamos su estado anterior. Si antes estaba presionado, nos salimos de la rutina, y...
		//si antes COUNTDWN no estaba presionado, actualizamos su estado y ejecutamos COUNT--
		if  (COUNTDWN_LAST == 1)
		{
			
		} else
		{
			COUNTDWN_LAST = 1;
			COUNT--;
			if (COUNT == 0xFF) COUNT = 15;
		}
	} else if ((PINB & ((1 << COUNTDWN))) == (1 << COUNTDWN))
	{
		COUNTDWN_LAST = 0;
	}
}





