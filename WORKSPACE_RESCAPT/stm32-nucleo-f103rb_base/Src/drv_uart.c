/*
 * drv_uart.c
 *
 *  Created on: 31 mars 2016
 *      Author: kerhoas
 */

#include "drv_uart.h"
#include "math.h"
#include "util.h"

uint8_t rec_buf[2];

int put_char(int);

//=================================================================
//	UART 2 INIT (STLINK UART)
//=================================================================

void uart2_Init()
{
	  Uart2Handle.Instance        = USART2;
	  Uart2Handle.Init.BaudRate   = UART_BAUDRATE;
	  Uart2Handle.Init.WordLength = UART_WORDLENGTH_8B;
	  Uart2Handle.Init.StopBits   = UART_STOPBITS_1;
	  Uart2Handle.Init.Parity     = UART_PARITY_NONE;
	  Uart2Handle.Init.HwFlowCtl  = UART_HWCONTROL_NONE;
	  Uart2Handle.Init.Mode       = UART_MODE_TX_RX;
	  Uart2Handle.Init.OverSampling = UART_OVERSAMPLING_16;

	  HAL_UART_Init(&Uart2Handle);

	  HAL_UART_Receive_IT(&Uart2Handle, (uint8_t *)rec_buf, 1);
}

//=================================================================
//	UART WRITE
//=================================================================

void uart_Write(char* toprint, int size)
{
	HAL_UART_Transmit(&Uart2Handle, (uint8_t *)toprint, size, 0xFFFF);
}

//=================================================================
//	PUCHAR PROTOTYPE (USED BY PRINTF FUNCTIONS)
//=================================================================
int put_char(int ch)
{
	HAL_UART_Transmit(&Uart2Handle, (uint8_t *)&ch, 1, 0xFFFF);
	return 0;
}

//=================================================================
//	UART RECEIVE CALLBACK5
//=================================================================
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *UartHandle)
{
	 HAL_UART_Receive_IT(&Uart2Handle, (uint8_t *)rec_buf, 1);
}

//================================================================
//				PUT STRING
//================================================================
void put_string(char* s)
{
	while(*s != '\0')
	{
		put_char(*s);
		s++;
	}
}



//================================================================
//				TERM_PRINTF
//================================================================
void term_printf(const char* fmt, ...)
{
	va_list        ap;
	char          *p;
	char           ch;
	unsigned long  ul;
	unsigned long  size;
	unsigned int   sp;
	char           s[34];

	va_start(ap, fmt);
	while (*fmt != '\0') {
		if (*fmt =='%') {
			size=0; sp=1;
			if (*++fmt=='0') {fmt++; sp=0;}	// parse %04d --> sp=0
			ch=*fmt;
			if ((ch>'0') && (ch<='9')) {	// parse %4d --> size=4
				char tmp[10];
				int i=0;
				while ((ch>='0') && (ch<='9')) {
					tmp[i++]=ch;
					ch=*++fmt;
				}
				tmp[i]='\0';
				size=str2num(tmp,10);
			}
			switch (ch) {
				case '%':
					put_char('%');
					break;
				case 'c':
					ch = va_arg(ap, int);
					put_char(ch);
					break;
				case 's':
					p = va_arg(ap, char *);
					put_string(p);
					break;
				case 'd':
					ul = va_arg(ap, long);
					if ((long)ul < 0) {
						put_char('-');
						ul = -(long)ul;
						//size--;
					}
					num2str(s, ul, 10, size, sp);
					put_string(s);
					break;
				case 'u':
					ul = va_arg(ap, unsigned int);
					num2str(s, ul, 10, size, sp);
					put_string(s);
					break;
				case 'o':
					ul = va_arg(ap, unsigned int);
					num2str(s, ul, 8, size, sp);
					put_string(s);
					break;
				case 'p':
					put_char('0');
					put_char('x');
					ul = va_arg(ap, unsigned int);
					num2str(s, ul, 16, size, sp);
					put_string(s);
					break;
				case 'x':
					ul = va_arg(ap, unsigned int);
					num2str(s, ul, 16, size, sp);
					put_string(s);
					break;
				case 'f':
					ul = va_arg(ap, long);
					int sign = ( ul & 0x80000000 ) >> 31;
					int m = (ul & 0x000FFFFF) ; // should be 0x007FFFFF
					float mf = (float)m ;
					mf = mf / pow(2.0,20.0);
					mf = mf + 1.0;
					int e = ( ul & 0x78000000 ) >> 23 ; // should be int e = ( ul & 0x7F800000 ) >> 23;
					e = e | (( ul & 0x000F00000 ) >> 20);
					e = e - 127;
					float f = mf*pow(2.0,e);
					if(sign==1){ put_char('-'); }
					float2str(s, f, 5);
					put_string(s);
					break;

				default:
					put_char(*fmt);
			}
		} else put_char(*fmt);
		fmt++;
	}
	va_end(ap);
}
//================================================================

