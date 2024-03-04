
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
// #include "serial.h"

/* Demo task priorities. */
#define mainBLOCK_Q_PRIORITY (tskIDLE_PRIORITY + 2)
#define mainCHECK_TASK_PRIORITY (tskIDLE_PRIORITY + 3)
#define mainCOM_TEST_PRIORITY (2)

/* The check task may require a bit more stack as it calls sprintf(). */
#define mainCHECK_TAKS_STACK_SIZE (configMINIMAL_STACK_SIZE * 2)

/* The execution period of the check task. */
#define mainCHECK_TASK_PERIOD ((portTickType)3000 / portTICK_RATE_MS)

/* The number of flash co-routines to create. */
#define mainNUM_FLASH_COROUTINES (5)

/* Baud rate used by the comtest tasks. */
// #define mainCOM_TEST_BAUD_RATE				( 19200 )
#define mainCOM_TEST_BAUD_RATE (9600)

// Definire lungime coada UART1
#define comBUFFER_LEN (10)

/* We should find that each character can be queued for Tx immediately and we
don't have to block to send. */
#define comNO_BLOCK ((portTickType)0)

/* The Rx task will block on the Rx queue for a long period. */
#define comRX_BLOCK_TIME ((portTickType)0xffff)

/* The LED used by the comtest tasks.  mainCOM_TEST_LED + 1 is also used.
See the comtest.c file for more information. */
#define mainCOM_TEST_LED (6)

/* The frequency at which the "fast interrupt test" interrupt will occur. */
#define mainTEST_INTERRUPT_FREQUENCY (20000)

/* The number of processor clocks we expect to occur between each "fast
interrupt test" interrupt. */
#define mainEXPECTED_CLOCKS_BETWEEN_INTERRUPTS (configCPU_CLOCK_HZ / mainTEST_INTERRUPT_FREQUENCY)

/* The number of nano seconds between each processor clock. */
#define mainNS_PER_CLOCK ((unsigned short)((1.0 / (double)configCPU_CLOCK_HZ) * 1000000000.0))

/* Dimension the buffer used to hold the value of the maximum jitter time when
it is converted to a string. */
#define mainMAX_STRING_LENGTH (20)

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
xTaskHandle task_idle, task_app;
volatile unsigned int app_on = 0U;
void __attribute__((interrupt, no_auto_psv)) _INT0Interrupt(void)
{
	app_on ^= 1U;
	_INT0IF = 0; // Resetam flagul corespunzator intreruperii
				 // INT0 pentru a nu se reapela rutina de intrerupere
}

void Task_StareApp(void *params)
{
	while (1)
	{
		if (app_on == 0U)
		{
			_RB15 ^= 1U;
			vTaskDelay(1000);
		}
		else
		{
		}
	}
}
int main(void)
{
	prvSetupHardware();

	TRISB = 0x0000;
	_TRISB7 = 1;  // RB7 este setat ca intrare intrerupere
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

	// xTaskCreate(Task_StareApp, (signed portCHAR *)"T_app", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 4, &task_app);
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
