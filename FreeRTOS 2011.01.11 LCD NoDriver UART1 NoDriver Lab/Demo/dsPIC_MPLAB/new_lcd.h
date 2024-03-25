/* SFR that seems to be missing from the standard header files. */
//#define PMAEN				*( ( unsigned short * ) 0x60c )

/*-----------------------------------------------------------*/
#define ON		1
#define OFF		0
#define Toggle	2
#define LCD_RS _RB11
#define LCD_RW _RB10
#define LCD_E  _RB9
#define LCD_BL _RB8

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
