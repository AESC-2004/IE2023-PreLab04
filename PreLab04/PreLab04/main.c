/*
 * PreLab04.c
 *
 * Created: 29/03/2025 10:16:44 p. m.
 * Author: ang50
 */
/********************************************************************************/
// Encabezado (Libraries)

#define F_CPU 16000000
#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdint.h>

//Variables y constantes

//TIM0
#define T0VALUE 178
//Multiplexación
#define DISP0	4
#define DISP1	5
#define LEDS	6
uint8_t MUX_SEQUENCE	= 0b00000001;

const uint8_t COUNT			= 0b10101010;
const uint8_t DISP1_VAL		= 0xD1;
const uint8_t DISP0_VAL		= 0x06;



/********************************************************************************/
// Function prototypes

void SETUP();
void initTMR0();
void initADC();

/********************************************************************************/
// Main Function

int main(void)
{
	SETUP();
	
	while (1)
	{
	}
}

/********************************************************************************/
// NON-Interrupt subroutines

void SETUP()
{
	// Desactivamos interrupciones
	cli();
	// Desactivamos UART1
	UCSR1B = 0x00;  
	// PORTD: Salida de displays y contador			|	PORTD: XXXX XXXX
	DDRD	= 0xFF;
	PORTD	= 0x00;
	// PORTB: Transistores para multiplexación		|	PORTB: 0XXX 0000
	DDRD	= 0b01110000;
	PORTB	= 0x00;
	initTMR0();
	// Rehabilitamos interrupciones
	sei();
}

void initTMR0()
{
	TCCR0B	= (1 << CS02) | (0 << CS01) | (1 << CS00);
	TIMSK0	= (1 << TOIE0);
	TCNT0	= T0VALUE;
}

/********************************************************************************/
// Interrupt routines

ISR(TIMER0_OVF_vect)
{
	// Recargamos TCNT0
	TCNT0	= T0VALUE;
	// Verificamos qué debe sacar PORTD según la secuencia de multiplexación
	if (MUX_SEQUENCE == 0b00000001)
	{
		// Si DISP0 estaba encendido, ahora queremos mostrar DISP1
		// Apagamos PORTB momentáneamente
		PORTB			= 0x00;
		// Cargamos DISP1_VAL en PORTD
		PORTD			= DISP1_VAL;
		// Encendemos DISP1
		PORTB			= (1 << DISP1);
		//Y actualizamos el valor de MUX_SEQUENCE
		MUX_SEQUENCE	= 0b00000010;
	} else if (MUX_SEQUENCE == 0b00000010)
	{
		// Si DISP1 estaba encendido, ahora queremos mostrar LEDS
		// Apagamos PORTB momentáneamente
		PORTB			= 0x00;
		// Cargamos COUNT en PORTD
		PORTD			= COUNT;
		// Encendemos LEDS
		PORTB			= (1 << LEDS);
		//Y actualizamos el valor de MUX_SEQUENCE
		MUX_SEQUENCE	= 0b00000100;
	} else if (MUX_SEQUENCE == 0b00000100)
	{
		// Si LEDS estaba encendido, ahora queremos mostrar DISP0
		// Apagamos PORTB momentáneamente
		PORTB			= 0x00;
		// Cargamos DISP0_VAL en PORTD
		PORTD			= DISP0_VAL;
		// Encendemos DISP0
		PORTB			= (1 << DISP0);
		//Y actualizamos el valor de MUX_SEQUENCE
		MUX_SEQUENCE	= 0b00000001;		
	}
}






