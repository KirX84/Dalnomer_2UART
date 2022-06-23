/***************************************************************************************/
#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdio.h>
#include <string.h>
#include<util/delay.h>
//#include "uart13.h"
/***************************************************************************************/
#define AVR_USART_9600_VALUE                                     (103)
/***************************************************************************************/
//typedef enum {FALSE = 0, TRUE = !FALSE} bool;
//int i = 0;
unsigned char //Установить разрешение:
              Lazer_1mm[8] = "\xFA\x04\x0C\x01\xF5",		  //Разрешение 1мм
              Lazer_01mm[8] = "\xFA\x04\x0C\x02\xF4",		  //Разрешение 0.1мм
			  //Установить временной интервал:
			  Lazer_1c[8]=  "\xFA\x04\x05\x01\xFC",			  //1 с
              Lazer_0c[8] = "\xFA\x04\x05\x00\xFD",			  //0 с
			  //Установить частоту:
              Lazer_1g[8] = "\xFA\x04\x0A\x00\xF8",			  //Установить частоту 1Гц
			  Lazer_5g[8] = "\xFA\x04\x0A\x05\xF3",			  //Установить частоту 5Гц
			  Lazer_10g[8] = "\xFA\x04\x0A\x0A\xEE",		  //Установить частоту 10Гц
			  Lazer_20g[8] = "\xFA\x04\x0A\x14\xE4",		  //Установить частоту 20Гц
			  //Установить режим измерения
			  Lazer_one_zam[7] "\x80\x06\x02\x78"			  //Один замер
              Lazer_nepr_zam[7] = "\x80\x06\x03\x77",		  //Непрерывное измерение

			  Lazer_shtd[7] = "\x80\x04\x02\x7A",             //Отключение устройства
			  Lazer_adress[8] = "\xFA\x04\x01\x80\x81",       //Установить адрес
			  //Установите начало измерения при включении питания:
			  Lazer_start_off[8] = "\xFA\x04\x0D\x00\xF5",	 //Выключить
			  Lazer_start_on[8] = "\xFA\x04\x0D\x01\xF4",	 //Включить
			  //Установить диапазон:
			  Lazer_5m[8] = "\xFA\x04\x09\x05\xF4",		     //5 м
			  Lazer_10m[8] = "\xFA\x04\x09\x0a\xEF",		 //10 м
			  Lazer_30m[8] = "\xFA\x04\x09\x1E\xDB",		 //30 м
			  Lazer_50m[8] = "\xFA\x04\x09\x32\xC7",		 //50 м
			  Lazer_80m[8] = "\xFA\x04\x09\x50\xA9",		 //80 м

			  Lazer_dal_min[9] = "\xFA\x04\x06\x2D\x01\xCE",   //Изменение расстояния-1
			  Lazer_dal_pl[9] = "\xFA\x04\x06\x2B\x\x01\xD0",  //Изменение расстояния+1
			  //Установить начальную точку
			  Lazer_top[8] = "\xFA\x04\x08\x01\xF9",        //верх
			  Lazer_back[8] = "\xFA\x04\x08\x00\xFA",       //низ
			  
			  Единое измерение (трансляция)	$FA$06$06$FA
			  Чтение кэша:			$80$06$07$73	80 06 82 30 30 33 2E 35 36 39 35 5E
			  Контрольный лазер:
			  $80$06$05$01$74	Открытый	80 06 85 01 F4
			  $80$06$05$00$75	Закрыть		80 06 85 01 F4

			  Otvet_Lazer[12],
			  Otvet_Lazer_TXT[9],
			  i, incomingByte, idx, j, Error_j;
bool readyToExchange, readyToExchangeUSB, readyToExchangeRec;
unsigned char numOfDataToSend;
unsigned char numOfDataToSendUSB;
unsigned char numOfDataToReceive;
unsigned char *sendDataPtr;
unsigned char *sendDataPtrUSB;
unsigned char *receivedDataPtr;
unsigned char *receivedDataPtrUSB;
unsigned char numOfDataSended;
unsigned char numOfDataSendedUSB;
unsigned char numOfDataReceived;
/***************************************************************************************/
void UART_SendData(uint8_t *pSendData, uint8_t nNumOfDataToSend)
{
    sendDataPtr = pSendData;
    numOfDataToSend = nNumOfDataToSend;
	//uart_send(pSendData);
    numOfDataSended = 0;
    readyToExchange = false;
    UCSR1B |= (1 << UDRIE1);
}
/***************************************************************************************/
void UART_SendDataUSB(uint8_t *pSendData, uint8_t nNumOfDataToSend)
{
	sendDataPtrUSB = pSendData;
	numOfDataToSendUSB = nNumOfDataToSend;
	numOfDataSendedUSB = 0;
	readyToExchangeUSB = false;
	UCSR0B |= (1 << UDRIE0);
}
/***************************************************************************************/
void UART_ReceiveData(uint8_t* pReceivedData, uint8_t nNumOfDataToReceive)
{
    receivedDataPtr = pReceivedData;
    numOfDataToReceive = nNumOfDataToReceive;
    numOfDataReceived = 0;
    readyToExchangeRec = false;
    UCSR1B |= (1 << RXCIE1);
}
/***************************************************************************************/
int main(void)
{
    sei();

	UBRR0H = 0;
	UBRR0L = 103; //скорость 9600
	//UCSR0B = (1<<RXEN0)|(1<<TXEN0)|(1<<RXCIE0)|(1<<UDRIE0); // разрешаем прием, передачу и прерывания по окончанию приема и опустошению буфера
	UCSR0B = (1<<RXEN0)|(1<<TXEN0); // разрешаем прием, передачу
	UCSR0C = (1<<UCSZ01)|(1<<UCSZ00); //8 бит, 1 стоп бит
	UBRR1H = 0;
	UBRR1L = 103; //скорость 9600
	//UCSR1B = (1<<RXEN1)|(1<<TXEN1)|(1<<RXCIE1)|(1<<UDRIE1); // разрешаем прием, передачу и прерывания по окончанию приема и опустошению буфера
	UCSR1B = (1<<RXEN1)|(1<<TXEN1); // разрешаем прием, передачу
	UCSR1C = (1<<UCSZ11)|(1<<UCSZ10); //8 бит, 1 стоп бит

    /*UBRR1 = AVR_USART_9600_VALUE;
	UCSR1C = (1 << UCSZ11) | (1 << UCSZ10); 
	UBRR0 = AVR_USART_9600_VALUE;
	UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);
	//uart_init();
	//UCSR1B |= (1 << RXEN1) | (1 << RXCIE1);
	//_delay_ms(10);*/
	UART_SendData(Lazer_0c,5);
	while(!readyToExchange);
	//_delay_ms(10);
	UART_SendData(Lazer_1g,5);
	while(!readyToExchange);
	//_delay_ms(10);
	UART_SendData(Lazer_nepr_zam,4);
	while(!readyToExchange);
	//_delay_ms(10);
	/*memcpy(testBuffer, 
	"\x67\x30\x73\x76\x2B\x30\x30",
	 sizeof testBuffer);*/
	//testBuffer = {0x67, 0x30, 0x73, 0x76, 0x2B, 0x30, 0x30};
    while(1)
    {
        UART_ReceiveData(Otvet_Lazer, 12);
		while(!readyToExchangeRec);
		//_delay_ms(150);
		UART_SendDataUSB(Otvet_Lazer_TXT, 8);
		while(!readyToExchangeUSB);
		//_delay_ms(150);
    }
}
/***************************************************************************************/
ISR(USART1_UDRE_vect)
{
    UDR1 = *sendDataPtr;
    sendDataPtr++;
    numOfDataSended++;
    
    if (numOfDataSended == numOfDataToSend)
    {
        UCSR1B &= ~(1 << UDRIE1);
        readyToExchange = 1;
    }
}
/***************************************************************************************/
ISR(USART0_UDRE_vect)
{
	UDR0 = *sendDataPtrUSB;
	sendDataPtrUSB++;
	numOfDataSendedUSB++;
	
	if (numOfDataSendedUSB == numOfDataToSendUSB)
	{
		UCSR0B &= ~(1 << UDRIE0);
		readyToExchangeUSB = 1;
	}
}
/***************************************************************************************/
ISR(USART1_RX_vect)
{
	/*receivedDataPtr = UDR1;
	receivedDataPtr++;
	numOfDataReceived++;
	
	/*if (numOfDataReceived == numOfDataToReceive)
	{
		UCSR1B &= ~((1 << RXCIE1) | (1 << RXEN1));
		readyToExchangeRec = 1;
	}*/


    Otvet_Lazer[0] = Otvet_Lazer[1];
    Otvet_Lazer[1] = Otvet_Lazer[2];
    Otvet_Lazer[2] = Otvet_Lazer[3];
    Otvet_Lazer[3] = Otvet_Lazer[4];
    Otvet_Lazer[4] = Otvet_Lazer[5];
    Otvet_Lazer[5] = Otvet_Lazer[6];
    Otvet_Lazer[6] = Otvet_Lazer[7];
    Otvet_Lazer[7] = Otvet_Lazer[8];
    Otvet_Lazer[8] = Otvet_Lazer[9];
    Otvet_Lazer[9] = Otvet_Lazer[10];
    Otvet_Lazer[10] = Otvet_Lazer[11];
    Otvet_Lazer[11] = UDR1;

	//UART_SendDataUSB(Otvet_Lazer, 12);
    //*receivedDataPtr = UDR1;

	if ((Otvet_Lazer[0]==0x80) && (Otvet_Lazer[1]==0x06) && (Otvet_Lazer[2]==0x83) && (Otvet_Lazer[3]==0x30) && (Otvet_Lazer[6]==0x2E)) 
	{
		int CS = Otvet_Lazer[0]+Otvet_Lazer[1]+Otvet_Lazer[2]+Otvet_Lazer[3];
		Error_j = 0;
		for (int i=0; i <= 6; i++) 
		{
			j = Otvet_Lazer[i+4];
			CS=CS+j;
			if (((j > 47) && (j < 59)) || (j==0x2E)) Otvet_Lazer_TXT[i] = j;
			else Error_j=1;
		}
		if ((Error_j == 0) && (256 - CS % 256 == Otvet_Lazer[11])) 
		{
			Otvet_Lazer_TXT[7] = 0x0D;              //Ставим символ переноса в конце строки
			Otvet_Lazer_TXT[2] = 0x2C;              //Ставим запятую вместо точки в число на выдачу
			//UART_SendDataUSB(Otvet_Lazer_TXT, 8);
			//while(!readyToExchangeUSB);
			//_delay_ms(150);
			readyToExchangeRec = 1;
		}
	}
   // receivedDataPtr++; 
   // numOfDataReceived++;
    
    /*if (numOfDataReceived == numOfDataToReceive)
    {
        UCSR1B &= ~((1 << RXCIE1) | (1 << RXEN1));
        readyToExchangeUSB = 1;
    }*/
}
/***************************************************************************************/