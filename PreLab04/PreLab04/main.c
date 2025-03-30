/*
 * PreLab04.c
 *
 * Created: 29/03/2025 10:16:44 p. m.
 * Author: ang50
 */
/*********************************************************************************************************************************************/
// Encabezado (Libraries)
#define F_CPU 1000000
#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdint.h>

// Variables y constantes
// TIM0
#define T0VALUE			178
// Multiplexaci�n
#define DISP0			5
#define DISP1			4
#define LEDS			6
uint8_t MUX_SEQUENCE	= 0b00000001;
// Botones de conteo
#define COUNTUP			2
#define COUNTDWN		1
uint8_t COUNTUP_LAST	= 0;
uint8_t COUNTDWN_LAST	= 0;
// Valores HEX para Displays
const uint8_t DISP7SEG[16] = {
	0x5F, 0x06, 0x9B, 0x8F, 0xC6, 0xCD, 0xDD, 0x07, 0xDF, 0xC7,
	0xD7, 0xDC, 0x59, 0x9E, 0xD9, 0xD1
};
// ADC
uint16_t ADC_VALUE		= 0;
// Variables de conteo
uint8_t LEDS_VAL		= 0xFF;
uint8_t COUNT			= 0;
uint8_t DISP0_VAL		= 0;
uint8_t DISP1_VAL		= 0;



/*********************************************************************************************************************************************/
// Function prototypes
void SETUP();
void initTMR0();
void initPCINT();
void initADC();

/*********************************************************************************************************************************************/
// Main Function
int main(void)
{
	SETUP();
	
	while (1)
	{
		// Acomodamos la lectura del ADC a unidades y d�cimas
		// Paso 1: Convertir lectura a milivoltios
		uint16_t MILLIVOLTS		= (ADC_VALUE * 5000UL)/1024;	// Usamos un "Unisgned Long" para evitar Overflow
		DISP0_VAL = DISP7SEG[(MILLIVOLTS % 1000)/100];			// Calculamos d�cimas y las sacamos al DISP0
		DISP1_VAL = DISP7SEG[(MILLIVOLTS)/1000];				// Calculamos unidades y las sacamos al DISP1
	}
}

/*********************************************************************************************************************************************/
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
	// PORTB: Transistores para multiplexaci�n y PINCHANGE		|	PORTB: 0XXX 0110
	DDRB	= (1 << DISP0) | (1 << DISP1) | (1 << LEDS);
	PORTB	= (1 << COUNTUP) | (1 << COUNTDWN);
	// Ajustamos el Prescaler global para F_CPU = 1MHz
	CLKPR			= (1 << CLKPCE);
	CLKPR			= (0 << CLKPCE) | (0 << CLKPS3) | (1 << CLKPS2) | (0 << CLKPS1) | (0 << CLKPS0);
	// Algunos ajustes de interrupciones
	initTMR0();
	initPCINT();
	initADC();
	// Rehabilitamos interrupciones
	sei();
}

void initTMR0()
{
	TCCR0B	= (0 << CS02) | (1 << CS01) | (1 << CS00);
	TIMSK0	= (1 << TOIE0);
	TCNT0	= T0VALUE;
}

void initPCINT()
{
	PCICR	= (1 << PCIE0);
	PCMSK0	= (1 << COUNTUP) | (1 << COUNTDWN);
}

void initADC()
{
	// Activamos solamente ADC6, REF como VCC y SIN justificaci�n izquierda.
	ADMUX	= (0 << REFS1) | (1 << REFS0) | (0 << ADLAR) | (0 << MUX4) | (0 << MUX3) | (1 << MUX2) | (1 << MUX1) | (0 << MUX0);
	// Definimos Auto Trigger Source como Free Running mode
	ADCSRB	= (0 << ADTS3) | (0 << ADTS2) | (0 << ADTS1) | (0 << ADTS0);
	// Ajustamos el ADC Interrupt Enable, el ADC Auto Trigger, y el prescaler del ADC a 8 
	ADCSRA	= (1 << ADEN) | (1 << ADSC) | (1 << ADATE) | (0 << ADIF) | (1 << ADIE) | (0 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);
}

/*********************************************************************************************************************************************/
// Interrupt routines
ISR(TIMER0_OVF_vect)
{
	// Recargamos TCNT0
	TCNT0	= T0VALUE;
	// Verificamos qu� debe sacar PORTD seg�n la secuencia de multiplexaci�n
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
		// Y actualizamos el valor de MUX_SEQUENCE
		MUX_SEQUENCE	= 0b00000001;		
	}
}

ISR(PCINT0_vect)
{
	// Revisamos qu� bot�n fue presionado
	// 0 L�GICO = PRESSED
	// 1 L�GICO = NOT PRESSED
	if ((PINB & ((1 << COUNTUP))) == (0 << COUNTUP)) 
	{
		// Si COUNTUP fue presionado...
		// Revisamos su estado anterior. Si antes estaba presionado, revisamos COUNTDWN, y...
		// si antes COUNTUP no estaba presionado, actualizamos su estado y ejecutamos COUNT++
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
		// Si COUNTDWN fue presionado...
		// Revisamos su estado anterior. Si antes estaba presionado, nos salimos de la rutina, y...
		// si antes COUNTDWN no estaba presionado, actualizamos su estado y ejecutamos COUNT--
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

ISR(ADC_vect)
{
	// Como el ADC se encuentra en Free Running Mode... solo guardamos el valor de lectura
	ADC_VALUE = ADC;
}





