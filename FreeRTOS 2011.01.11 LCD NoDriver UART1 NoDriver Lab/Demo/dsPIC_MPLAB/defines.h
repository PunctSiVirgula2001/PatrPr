#define DEBOUNCE_MS 400U
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
#define comBUFFER_LEN (100)

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
#define mainMAX_STRING_LENGTH (100)