/***************************************************************************************/
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/eeprom.h>
#include <stdio.h>
#include <string.h>
/***************************************************************************************/
#define AVR_USART_9600_VALUE                                     (103)
/***************************************************************************************/
unsigned char //////////////////////////////////////////////////////////////////////////
//Команды дальномера
//Установить разрешение:
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
Lazer_one_zam[7] = "\x80\x06\x02\x78",		  //Один замер               ответ 80 06 82 30 30 30 2E 30 30 30 30 6D  €.‚000.0000m
Lazer_nepr_zam[7] = "\x80\x06\x03\x77",		  //Непрерывное измерение    ответ 80 06 83 30 30 30 2E 30 30 30 30 67  €.ƒ000.0000g

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
Lazer_dal_pl[9] = "\xFA\x04\x06\x2B\x01\xD0",  //Изменение расстояния+1
//Установить начальную точку
Lazer_top[8] = "\xFA\x04\x08\x01\xF9",        //верх
Lazer_back[8] = "\xFA\x04\x08\x00\xFA",       //низ

Lazer_one_zam_broad[7] = "\xFA\x06\x06\xFA",  //Однократное измерение (широковещательное)
Lazer_cash[7] = "\x80\x06\x07\x73",	//Чтение кэша
//Контрольный лазер:
Lazer_close[8] = "\x80\x06\x05\x01\x74",	//Открытый
Lazer_open[8] = "\x80\x06\x05\x00\x75",	//Закрыть
//////////////////////////////////////////////////////////////////////////
//Внешние комады при №00
Zapros_one_zam_orig[9] = "\x73\x30\x30\x67\x0D\x0A", //Запрос на один замер (s00g)
Zapros_nepr_zam_orig[9] = "\x73\x30\x30\x68\x0D\x0A", //Запрос на непрерывное измерение (s00h)
Zapros_number_orig[13] = "\x73\x30\x30\x69\x64\x2B\x30\x30\x0D\x0A", //Запрос на изменение номера на 00 (s00id+00)
Zapros_save_orig[9] = "\x73\x30\x30\x73\x0D\x0A", //Запрос на сохранение параметров (s00s)
//Ответы
Otvet_Lazer_TXT_orig[18] = "\x67\x30\x30\x67\x2B\x30\x30\x30\x30\x30\x30\x30\x30\x0D\x0A", //goog+00000000
//Буферы, для использования их в программе
Zapros_one_zam[6], //Запрос на один замер (s00g)
Zapros_nepr_zam[6], //Запрос на непрерывное измерение (s00h)
Zapros_number[10], //Запрос на изменение номера на 00 (s00id+00)
Zapros_save[6], //Запрос на сохранение параметров (s00s)
Otvet_Lazer_TXT[15],
//////////////////////////////////////////////////////////////////////////

Otvet_Lazer[12],
Test[9] = "\x30\x30\x30\x30\x0D\x0A",
num_dal = 1,                                       //номер дальномера
//  op_mode,
buffer[12],
i, incomingByte, idx, j,
Error_j;
char c;
bool readyToExchange, readyToExchangeUSB, readyToExchangeRec, readyToExchangeRecUSB;
//flag[5] = {1, 1, 1, 1, 1};
int one_dgt = 0;                                                                    //флаг проверки массива  /  добавить элемент для каждой новой команды
unsigned char numOfDataToSend;
unsigned char numOfDataToSendUSB;
unsigned char numOfDataToReceive;
unsigned char numOfDataToReceiveUSB;
unsigned char *sendDataPtr;
unsigned char *sendDataPtrUSB;
unsigned char *receivedDataPtr;
unsigned char *receivedDataPtrUSB;
unsigned char numOfDataSended;
unsigned char numOfDataSendedUSB;
unsigned char numOfDataReceived;
unsigned char numOfDataReceivedUSB;
unsigned char *ptr;
/***************************************************************************************/
void UART_SendData(uint8_t *pSendData, uint8_t nNumOfDataToSend)
{
	sendDataPtr = pSendData;
	numOfDataToSend = nNumOfDataToSend;
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
	UCSR1B |= (1 << RXCIE1)|(1 << RXEN1);
}
/***************************************************************************************/
void UART_ReceiveDataUSB(uint8_t* pReceivedData, uint8_t nNumOfDataToReceive)
{
	receivedDataPtrUSB = pReceivedData;
	numOfDataToReceiveUSB = nNumOfDataToReceive;
	numOfDataReceivedUSB = 0;
	readyToExchangeRecUSB = false;
	UCSR0B |= (1 << RXCIE0)|(1 << RXEN0);
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
	UBRR1 = AVR_USART_9600_VALUE;
	UBRR0 = AVR_USART_9600_VALUE;

	UART_SendData(Lazer_0c,5);
	while(!readyToExchange);
	UART_SendData(Lazer_01mm,5);
	while(!readyToExchange);
	UART_SendData(Lazer_1g,5);
	while(!readyToExchange);
	    while(1)
    {
 	   // UART_ReceiveData(buffer,5);
  	   // while(!readyToExchangeRec);
		UART_SendData(Lazer_one_zam, 5);
		while(!readyToExchange);
		UART_ReceiveData(buffer,12);
		while(!readyToExchangeRec);
		UART_SendDataUSB(buffer, 12);
		while(!readyToExchangeUSB);
    }
}
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

	if ((Otvet_Lazer[0]==0x80) && (Otvet_Lazer[1]==0x06) && ((Otvet_Lazer[2]==0x82)||(Otvet_Lazer[2]==0x83)) && (Otvet_Lazer[3]==0x30) && (Otvet_Lazer[6]==0x2E)) 
	{
		int CS = Otvet_Lazer[0]+Otvet_Lazer[1]+Otvet_Lazer[2]+Otvet_Lazer[3];
		Error_j = 0;
		int t = 4;
		for (int i=0; i < 6; i++) 
		{
			j = Otvet_Lazer[t];
			t++;
			CS=CS+j;
			if ((j > 47) && (j < 59)) Otvet_Lazer_TXT[i+5] = j;
			else if (j==0x2E) i--;
			else Error_j=1;
		}
		if ((Error_j == 0) && (256 - CS % 256 == Otvet_Lazer[11])) 
		{
			readyToExchangeRec = 1;
			UCSR1B &= ~((1 << RXCIE1) | (1 << RXEN1));
		}
	}
}
/***************************************************************************************/
ISR(USART0_RX_vect)
{
	buffer[0] = buffer[1];
    buffer[1] = buffer[2];
    buffer[2] = buffer[3];
	buffer[3] = buffer[4];
    buffer[4] = buffer[5];
    buffer[5] = buffer[6];
    buffer[6] = buffer[7];
    buffer[7] = buffer[8];
    buffer[8] = buffer[9];
    buffer[9] = UDR0;
 	numOfDataReceivedUSB++;
 	if (numOfDataReceivedUSB == numOfDataToReceiveUSB)
     {
         UCSR0B &= ~(1 << RXCIE0);
         readyToExchangeUSB = 1;
     }

}
/***************************************************************************************/