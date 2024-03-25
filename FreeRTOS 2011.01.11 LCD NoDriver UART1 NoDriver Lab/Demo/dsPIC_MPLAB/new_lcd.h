/* SFR that seems to be missing from the standard header files. */
//#define PMAEN				*( ( unsigned short * ) 0x60c )

/*-----------------------------------------------------------*/
#define On		1
#define Off		0
#define Toggle	2

void delayus(int us);
void delayms(int ms);
void LCD_DATA_OR(int val);
void LCD_DATA_AND(int val);
void clear(void);
void send_char2LCD(char ax);
void LCD_line(int linie);
void LCD_init(void);
void LCD_printf(char *text);
void LCD_Goto(int linie, int col);
void LCD_On_Off(int On_Off);
void LCD_LED(int On_Off);
