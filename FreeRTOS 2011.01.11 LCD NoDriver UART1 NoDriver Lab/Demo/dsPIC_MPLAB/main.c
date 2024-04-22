
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
// #include "serial.h"
#include "defines.h"

/* Menu states enum : automatic/manual*/
typedef enum states_app
{
	automatic = 1,
	manual = 2,
	temperatura = 3,
	none = 4
}state_app;
state_app mod_lucru_curent = none;

typedef enum { false, true } bool;


// Select Internal FRC at POR
_FOSCSEL(FNOSC_FRC);
// Enable Clock Switching and Configure
_FOSC(FCKSM_CSECMD &OSCIOFNC_OFF); // FRC + PLL
//_FOSC(FCKSM_CSECMD & OSCIOFNC_OFF & POSCMD_XT);		// XT + PLL
_FWDT(FWDTEN_OFF); // Watchdog Timer Enabled/disabled by user software

/*
 * Setup the processor ready for the demo.
 */
static void prvSetupHardware(void);

/* The queue used to send messages to the LCD task. */
static xQueueHandle xUART1_Queue;

void TaskCerinta2(void *params)
{
	float temp = 0;
	for (;;)
	{
		temp = ds1820_read();
		vTaskDelay(250);
	}
}
xTaskHandle task_idle, task_app;
volatile unsigned char app_on = 0U;
volatile portTickType last_time = 0U;
volatile portTickType current_time = 0U;
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
	TRISB = 0x0000;
	_TRISB7 = 1;  // RB7(Buton sw1 pe placuta) este setat ca intrare intrerupere
	_TRISB15 = 0; // RB0 este setat ca iesire
	_TRISB14 = 0;
	_TRISB13 = 0;
	_TRISB12 = 0;

	_RB12 = 1U;
	_RB13 = 1U;
	_RB14 = 1U;
	_RB15 = 1U;

	_INT0IF = 0; // Resetem flagul coresp. intreruperii INT0
	_INT0IE = 1; // Se permite lucrul cu întreruperea INT0
	_INT0EP = 1; // Se stabileşte pe ce front se generează INT0
}
unsigned char stare_rb15 = 0;
unsigned char last_state_app = -1;
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
			_RB15 ^= 1U; // aici trb rb0
			stare_rb15 = _RB15;
			vTaskDelay(1000);
		}
		else if (app_on == 1)
		{
			last_state_app = app_on;
			_RB15 = 0U;
			stare_rb15 = _RB15;
		}
		if (last_state_app != app_on)
		{
			clear();
			vTaskDelay(50);
			if (app_on == 1)
			{

				LCD_line(1);
				LCD_printf("APP_ON");
			}
			if (app_on == 0)
			{

				LCD_line(1);
				LCD_printf("APP_OFF");
			}
		}
	}
}
signed char input_user[1];
void Task_UartInterfaceMenu(void *params) // interogare mod de lucru, selectare mod de lucru, afisare temperatura
{
	
	bool reset_menu = true;
	
	while(1)
	{
		// Display work mode menu : automatic/manual only when app is on
		if (app_on == 1) {
			xSerialGetChar(mainCOM_TEST_BAUD_RATE, &input_user, 100);
			if (input_user[0] == 'r') { // CTRL + C or SPACE or ESC -- reset to menu
				reset_menu = true;
				mod_lucru_curent = none;
				vSerialPutString(mainCOM_TEST_BAUD_RATE, "Reset: No mode active.\n\r", 20);
			}
		}

		if(reset_menu == true && app_on == 1) // Display menu only once
		{
			vSerialPutString(mainCOM_TEST_BAUD_RATE, "Select work mode: automatic(1)/manual(2)\n\r", 35);
			vTaskDelay(200);
			reset_menu = false;
			// Wait for user input
			bool validInput = false;
			while (!validInput) {
				//wait max time for user input
				xSerialGetChar(mainCOM_TEST_BAUD_RATE,&input_user,5000);
				if (atoi(input_user) == automatic) {
					mod_lucru_curent = automatic;
					validInput = true;
					vSerialPutString(mainCOM_TEST_BAUD_RATE, "Automatic mode selected.\n\r", 20);
				} else if (atoi(input_user) == manual) {
					mod_lucru_curent = manual;
					validInput = true;
					vSerialPutString(mainCOM_TEST_BAUD_RATE, "Manual mode selected.\n\r", 20);
				} else if (atoi(input_user) == temperatura) {
					mod_lucru_curent = temperatura;
					validInput = true;
					vSerialPutString(mainCOM_TEST_BAUD_RATE, "Display temperature:.\n\r", 20);
				}
				vTaskDelay(50);
			}
		}
		vTaskDelay(50);
	}
}
// Task selectare mod de lucru

void Task_WorkMode(void *params)
{
	while (1)
	{
		// if (app_on == 1)
		// {
		// 	switch (mod_lucru_curent):
		// 	case automatic:
		// 		// do something
		// 		break;
		// 	case manual:
		// 		// do something
		// 		break;
		// 	case temperatura:
		// 		// do something
		// 		break;
		// }
		vTaskDelay(100);
	}
}


int main(void)
{
	prvSetupHardware();
	initPwm();
	init_ds1820();
	init_PORTB_AND_INT();
	xTaskCreate(TaskCerinta2, (signed portCHAR *)"TsC2", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 2, NULL);
	xTaskCreate(Task_StareApp, (signed portCHAR *)"T_app", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 4, &task_app);
	xTaskCreate(Task_UartInterfaceMenu, (signed portCHAR *)"T_UIM", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 3, NULL);
	xTaskCreate(Task_WorkMode, (signed portCHAR *)"T_WM", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 3, NULL);
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
	ADPCFG = 0xFFFF; // make ADC pins all digital - adaugat
	vParTestInitialise();
	initPLL();
	LCD_init();
	LCD_line(1);
	LCD_printf("Task1:");
	LCD_line(2);
	LCD_printf("Task2:");
	LCD_Goto(1, 13);
	LCD_printf("FreeRTOS");
	LCD_Goto(2, 14);
	LCD_printf("01.2011");
	// Initializare interfata UART1
	xSerialPortInitMinimal(mainCOM_TEST_BAUD_RATE, comBUFFER_LEN);
}
/*-----------------------------------------------------------*/

/*-----------------------------------------------------------*/

void vApplicationIdleHook(void)
{
	/* Schedule the co-routines from within the idle task hook. */
	vCoRoutineSchedule();
}
/*-----------------------------------------------------------*/
