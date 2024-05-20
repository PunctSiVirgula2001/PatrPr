   
/* Standard includes. */
#include <stdio.h>

/* Scheduler includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "croutine.h"

/* Demo application includes. */
#include "BlockQ.h"
#include "crflash.h"
#include "blocktim.h"
#include "integer.h"
#include "comtest2.h"
#include "partest.h"
// #include "lcd.h"
#include "timertest.h"

// Includes proprii
#include "new_lcd.h"
#include "new_serial.h"
#include "libq.h"
#include "pwm.h"
#include "ds18s20.h"
#include "util.h"
#include "adcDrv1.h"
#include "defines.h"

_FOSCSEL(FNOSC_FRC);
_FOSC(FCKSM_CSECMD &OSCIOFNC_OFF); 
_FWDT(FWDTEN_OFF);
static void prvSetupHardware(void);



/* Menu states enum : automatic/manual*/
typedef enum states_app
{
	automatic = 1,
	manual = 2,
	temperatura = 3,
	interogare_mod = 4,
	none = 5,
	on = 6,
	off = 7
} state_app;

// Default state is none
state_app mod_lucru_curent = none;

typedef enum {false,true} bool;

void updateLCDLine(int line, const char *text);
const char *getStateString(state_app state);
const char *getTemperatureString();
const char *getRB3StatusString();
void constructAndUpdateLine(state_app state, char *line1, char *line2);


/* The queue used to send messages to the LCD task. */
xQueueHandle xUART1_Queue;

/* The queue used to notify when the LCD should be changed. */
xQueueHandle LCD_update_queue;


float temp = 25.0f;
int voltage = 0;
int pwm_servo = PWM_SERVO_MIN;
void TaskPwmTemp(void *params)
{

	for (;;)
	{
		if (mod_lucru_curent == automatic)
		{
			temp = ds1820_read();
			pwm_servo = (int)map_with_clamp(temp, 20.0f, 30.0f, PWM_SERVO_MIN, PWM_SERVO_MAX);
			// print to serial the temperature
			//vSerialPutString(mainCOM_TEST_BAUD_RATE, "Temperature: ", 20);
			//vSerialPutString(mainCOM_TEST_BAUD_RATE, getTemperatureString(), 20);
			setDutyCycle(pwm_servo);
		}
		else if (mod_lucru_curent == manual)
		{
			voltage = getADCVal();
			pwm_servo = (int)map_with_clamp(voltage, 0, 4095, PWM_SERVO_MIN, PWM_SERVO_MAX);
			// print to serial the voltage
			vSerialPutString(mainCOM_TEST_BAUD_RATE, "Voltage potentiometer: ", 20);
			vSerialPutString(mainCOM_TEST_BAUD_RATE, getRB3StatusString(), 20);
			vSerialPutString(mainCOM_TEST_BAUD_RATE, "\r\n", 20);
			setDutyCycle(pwm_servo);
			
		}
		else if (mod_lucru_curent == temperatura)
		{
			// print to serial the temperature
			vSerialPutString(mainCOM_TEST_BAUD_RATE, "Temperature: ", 20);
			vSerialPutString(mainCOM_TEST_BAUD_RATE, getTemperatureString(), 20);
			vSerialPutString(mainCOM_TEST_BAUD_RATE, "\r\n", 20);
			mod_lucru_curent = none;
		}
		
		vTaskDelay(250);
	}
}

// Interrupt for button press on SW1
volatile unsigned char app_on = 0U;
volatile portTickType last_time = 0U;
volatile portTickType current_time = 0U;
unsigned char last_state_app = -1;
void __attribute__((interrupt, no_auto_psv)) _INT0Interrupt(void)
{
	current_time = xTaskGetTickCount();
	if (current_time - last_time > DEBOUNCE_MS)
	{
		app_on ^= 1U;
		last_time = current_time;
	}
	_INT0IF = 0; // Resetam flagul corespunzator intreruperii
				 // INT0 pentru a nu se reapela rutina de intrerupere
}

void init_PORTB_AND_INT()
{
	//TRISB = 0x0000;
	_TRISB7 = 1;  // RB7(Buton sw1 pe placuta) este setat ca intrare intrerupere
	 _TRISB1 = 0; // RB0 este setat ca iesire
	// _TRISB14 = 0;
	// _TRISB13 = 0;
	// _TRISB12 = 0;

	// _RB12 = 1U;
	// _RB13 = 1U;
	// _RB14 = 1U;
	_RB1 = 1U;

	_INT0IF = 0; // Resetem flagul coresp. intreruperii INT0
	_INT0IE = 1; // Se permite lucrul cu întreruperea INT0
	_INT0EP = 1; // Se stabileşte pe ce front se generează INT0
}

void Task_StareApp(void *params)
{
	LCD_init();
	LCD_On_Off(ON);
	LCD_LED(ON);
	clear();	
	while (1)
	{
		if (app_on == 0)
		{
			last_state_app = app_on;
			_RB1 ^= 1U; // aici trb rb1
			vTaskDelay(1000);
		}
		else if (app_on == 1)
		{
			last_state_app = app_on;
			_RB1 = 0U;
		}
		if (last_state_app != app_on)
		{
			
			// send to LCD queue the state of the app
			if (app_on == 1)
			{
				mod_lucru_curent = on;
				xQueueSend(LCD_update_queue, &mod_lucru_curent, portMAX_DELAY);
			}
			if (app_on == 0)
			{
				mod_lucru_curent = off;
				xQueueSend(LCD_update_queue, &mod_lucru_curent, portMAX_DELAY);
			}
		}
		vTaskDelay(150);
	}
}
signed char input_user[1];
void Task_UartInterfaceMenu(void *params) // interogare mod de lucru, selectare mod de lucru, afisare temperatura
{
	bool reset_menu = true;
	while (1)
	{
		// Display work mode menu : automatic/manual only when app is on
		if (app_on == 1)
		{
			xSerialGetChar(mainCOM_TEST_BAUD_RATE, &input_user, 100);

			if (strcmp("r",input_user) == 0)
			{ // when user inputs 'r' reset menu
				reset_menu = true;
				mod_lucru_curent = none;
				//app_on = 0;
				vSerialPutString(mainCOM_TEST_BAUD_RATE, "Reset: No mode active.\n\r", 20);
			}
			else if(atoi(input_user) == interogare_mod)
			{
				// dont save the state in mod_lucru_curent
			//	validInput = true;
				vSerialPutString(mainCOM_TEST_BAUD_RATE, "Current work mode: ", 20);
				vSerialPutString(mainCOM_TEST_BAUD_RATE, getStateString(mod_lucru_curent), 20);
				vSerialPutString(mainCOM_TEST_BAUD_RATE, "\n\r", 20);
				input_user[0] = 24;
			}
		}

		if (reset_menu == true && app_on == 1) // Display menu only once after reset
		{
			vSerialPutString(mainCOM_TEST_BAUD_RATE, "\n\r==Select work mode:==\n\r", 35);
			vSerialPutString(mainCOM_TEST_BAUD_RATE, "Automatic = '1'\n\r", 20);
			vSerialPutString(mainCOM_TEST_BAUD_RATE, "Manual = '2'\n\r", 20);
			vSerialPutString(mainCOM_TEST_BAUD_RATE, "Display temperature = '3'\n\r", 20);
			vSerialPutString(mainCOM_TEST_BAUD_RATE, "Interogare mod = '4'\n\r", 20);
			// put a end line
			vSerialPutString(mainCOM_TEST_BAUD_RATE, "\n\r", 20);
			vTaskDelay(200);
			reset_menu = false;
			// Wait for user input
			bool validInput = false;

			while (!validInput || mod_lucru_curent == temperatura) // wait for valid input  // if temperature is selected, display it
			{
				// wait max time for user input
				xSerialGetChar(mainCOM_TEST_BAUD_RATE, &input_user, 5000);
				if (atoi(input_user) == automatic)
				{
					mod_lucru_curent = automatic;
					validInput = true;
					xQueueSend(LCD_update_queue, &mod_lucru_curent, portMAX_DELAY); // send to LCD queue the selected work mode
					vSerialPutString(mainCOM_TEST_BAUD_RATE, "Automatic mode selected.\n\r", 20);
					input_user[0] = 24;
				}
				else if (atoi(input_user) == manual)
				{
					mod_lucru_curent = manual;
					validInput = true;
					xQueueSend(LCD_update_queue, &mod_lucru_curent, portMAX_DELAY); // send to LCD queue the selected work mode
					vSerialPutString(mainCOM_TEST_BAUD_RATE, "Manual mode selected.\n\r", 20);
					input_user[0] = 24;
				}
				else if (atoi(input_user) == temperatura)
				{
					mod_lucru_curent = temperatura;
					validInput = true;
					xQueueSend(LCD_update_queue, &mod_lucru_curent, portMAX_DELAY); // send to LCD queue the selected work mode
					vSerialPutString(mainCOM_TEST_BAUD_RATE, "Display temperature \n\r", 20);
					input_user[0] = 24;
				}
				vTaskDelay(50);
			}
		}
		vTaskDelay(50);
	}
}

// Task update LCD with current work mode (automatic/manual/temperatura), Voltage from RB3 and last received command from UART
void Task_updateLCD(void *params)
{
	// Default state
	state_app received_state = none; 

	// Constructed lines
	char line1[40] = {0};
	char line2[40] = {0};

	// Wait for the Queue to be created
	while (LCD_update_queue == 0)
	{
		vTaskDelay(100);
	}

	while (1)
	{
		//Receive a state and update accordingly
		if (xQueueReceive(LCD_update_queue, &received_state, 250) == pdTRUE)
		{
			// Construct and update lines based on the received state
			//constructAndUpdateLine(received_state, line1, line2);
		}
		else
		{ 	// when no state is received, update with the last received state and the read values
			// Construct and update lines based on the last received state
			//constructAndUpdateLine(none, line1, line2);
		}
		vTaskDelay(100);
	}
}

int main(void)
{
	RCONbits.SWDTEN=0;
	prvSetupHardware();
	
	// queue for updating LCD
	LCD_update_queue = xQueueCreate(10, sizeof(state_app));
	initPwm(); // init PWM
	init_ds1820(); // init DS1820
	initAdc1(); // init ADC1
	initTmr3(); // init TMR3
	init_PORTB_AND_INT(); // init PORTB and INT0
	xTaskCreate(TaskPwmTemp, (signed portCHAR *)"TsC2", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 4, NULL);
	xTaskCreate(Task_StareApp, (signed portCHAR *)"T_app", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 4, NULL);
	xTaskCreate(Task_UartInterfaceMenu, (signed portCHAR *)"T_UIM", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 4, NULL);
	//xTaskCreate(Task_updateLCD, (signed portCHAR *)"T_ULCD", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 4, NULL);

	/* Finally start the scheduler. */
	vTaskStartScheduler();

	return 0;
}
/*-----------------------------------------------------------*/

void initPLL(void)
{
	// Configure PLL prescaler, PLL postscaler, PLL divisor
	PLLFBD = 41; // M = 43 FRC
	// PLLFBD = 30; 		// M = 32 XT
	CLKDIVbits.PLLPOST = 0; // N1 = 2
	CLKDIVbits.PLLPRE = 0;	// N2 = 2

	// Initiate Clock Switch to Internal FRC with PLL (NOSC = 0b001)
	__builtin_write_OSCCONH(0x01); // FRC
	//__builtin_write_OSCCONH(0x03);	// XT
	__builtin_write_OSCCONL(0x01);

	// Wait for Clock switch to occur
	while (OSCCONbits.COSC != 0b001)
		; // FRC
	// while (OSCCONbits.COSC != 0b011);	// XT

	// Wait for PLL to lock
	while (OSCCONbits.LOCK != 1)
	{
	};
}

static void prvSetupHardware(void)
{
	ADPCFG = 0xFFDF; // make ADC pins all digital - adaugat
	//vParTestInitialise();
	initPLL();
	LCD_init();
	// Initializare interfata UART1
	xSerialPortInitMinimal(mainCOM_TEST_BAUD_RATE, comBUFFER_LEN);
}
/*-----------------------------------------------------------*/

void updateLCDLine(int line, const char *text) // update line with text
{
	LCD_line(line);
	LCD_printf(text);
}
const char *getStateString(state_app state) // return string for state
{
	switch (state)
	{
	case on:
		return "On";
	case off:
		return "Off";
	case automatic:
		return "Auto";
	case manual:
		return "Manual";
	case temperatura:
		return "Temp";
	case interogare_mod:
		return "Interogare";
	}
}

const char *getTemperatureString() // return string for temperature
{
	static char temp_str[5];
	snprintf(temp_str, 5, "%.2f", ds1820_read());
	return temp_str;
}

const char *getRB3StatusString() // return string for RB3 status
{
	static char rb3_str[5];
	snprintf(rb3_str, 5, "%d", getADCVal());
	return rb3_str;
}

// Line 1: Mod=state Ultima Comanda= any command
// Line 2: Temperatura=state V_RB3=state
void constructAndUpdateLine(state_app state, char *line1, char *line2)
{
	clear();
	vTaskDelay(50);
	static char *last_command = "N/A";	  // Stores the previous command for display
	static char *current_command = "N/A"; // Stores the current command during this cycle

	// Determine the current command based on state
	if(state != none)
	{
		switch (state)
		{
		case automatic:
			current_command = "Automatic";
			break;
		case manual:
			current_command = "Manual";
			break;
		case temperatura:
			current_command = "Temperature";
			break;
		case on:
			current_command = "On";
			break;
		case off:
			current_command = "Off";
			break;
		}
	}
	// Build the display strings
	snprintf(line1, 40, "Mod=%s Ultima Comanda=%s", current_command, last_command);
	// Get the temperature string
	snprintf(line2, 40, "Temperatura=%s V_RB3=%s", getTemperatureString(), getRB3StatusString());

	// Update the LCD
	updateLCDLine(1, line1);
	updateLCDLine(2, line2);

	// Now update last_command to the current command for use in the next cycle
	if(state != none)
		last_command = current_command;
}



/*-----------------------------------------------------------*/

void vApplicationIdleHook(void)
{
	/* Schedule the co-routines from within the idle task hook. */
	vCoRoutineSchedule();
}
/*-----------------------------------------------------------*/
