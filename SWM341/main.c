#include "SWM341.h"
#include "eeprom_emulation.h"


/*  ������ EEPROM �д洢�Ĳ����ĵ�ַ����ȡֵ��Χ 0 -- 0xFFFFFFFE
 *	ע�⣺������ַ������ܸı䣬����ᵼ�¶�ȡ������������ֵ��
 *	��һ��Ҫ�޸ģ�����Ҫ���� EE_Format() �����в���ɾ��
 */
#define EE_ADDR_Speed	0
#define EE_ADDR_Angle	2
#define EE_ADDR_Dummy	5


void SerialInit(void);

int main(void)
{
	uint32_t i, j;
	uint32_t speed;
	uint32_t angle;
	uint32_t dummy;
	
 	SystemInit();
	
	SerialInit();
	
	if(!EE_Init())
	{
		/* ��ʼ��ʧ�ܣ������ֿ��ܵ�ԭ��
		 * ��1����һ���ϵ�ִ�У��������������ִ�� EE_Format() ��ʽ������
		 * ��2�����������صĴ��󣬵��� Flash �����𻵣���Ҫ���ö�����ֶ������������������
		 */
		EE_Format();
	}
	
	EE_Write(EE_ADDR_Speed, 1000);
	EE_Write(EE_ADDR_Angle, 2000);
	EE_Write(EE_ADDR_Dummy, 3000);
	
	EE_Write(EE_ADDR_Speed, 1001);
	
	EE_Write(EE_ADDR_Angle, 4000);
	
	if(EE_Read(EE_ADDR_Speed, &speed))
	{
		printf("speed: %d\n\n", speed);
	}
	else
	{
		printf("speed read fail\n\n");
	}
	
	if(EE_Read(EE_ADDR_Angle, &angle))
	{
		printf("angle: %d\n\n", angle);
	}
	else
	{
		printf("angle read fail\n\n");
	}
	
	if(EE_Read(EE_ADDR_Dummy, &dummy))
	{
		printf("dummy: %d\n\n", dummy);
	}
	else
	{
		printf("dummy read fail\n\n");
	}
	
	printf("Trigger EE_TransferPage() calling\n\n");
	
	for(i = 0; i < EE_ITEM_COUNT / 2; i++)
	{
		EE_Write(EE_ADDR_Speed, 1000 + i);
		EE_Write(EE_ADDR_Angle, 2000 + i);
	}
	
	EE_Read(EE_ADDR_Speed, &speed);
	EE_Read(EE_ADDR_Angle, &angle);
	EE_Read(EE_ADDR_Dummy, &dummy);
	
	printf("speed: expect %d, get %d\n\n", 1000 + i - 1, speed);
	printf("angle: expect %d, get %d\n\n", 2000 + i - 1, angle);
	printf("dummy: expect %d, get %d\n\n", 3000,		  dummy);
	
	printf("Trigger eraseCount increase by 1\n\n");
	
	for(j = 0; j < EE_PAGE_COUNT * EE_ITEM_COUNT / 2; j++)
	{
		EE_Write(EE_ADDR_Angle, 2000 + j);
		EE_Write(EE_ADDR_Dummy, 3000 + j);
	}
	
	EE_Read(EE_ADDR_Speed, &speed);
	EE_Read(EE_ADDR_Angle, &angle);
	EE_Read(EE_ADDR_Dummy, &dummy);
	
	printf("speed: expect %d, get %d\n\n", 1000 + i - 1, speed);
	printf("angle: expect %d, get %d\n\n", 2000 + j - 1, angle);
	printf("dummy: expect %d, get %d\n\n", 3000 + j - 1, dummy);
		
 	while(1==1)
 	{
 	}
}


void SerialInit(void)
{
	UART_InitStructure UART_initStruct;
	
	PORT_Init(PORTM, PIN0, PORTM_PIN0_UART0_RX, 1);	//GPIOM.0����ΪUART0��������
	PORT_Init(PORTM, PIN1, PORTM_PIN1_UART0_TX, 0);	//GPIOM.1����ΪUART0�������
 	
 	UART_initStruct.Baudrate = 57600;
	UART_initStruct.DataBits = UART_DATA_8BIT;
	UART_initStruct.Parity = UART_PARITY_NONE;
	UART_initStruct.StopBits = UART_STOP_1BIT;
	UART_initStruct.RXThreshold = 3;
	UART_initStruct.RXThresholdIEn = 0;
	UART_initStruct.TXThreshold = 3;
	UART_initStruct.TXThresholdIEn = 0;
	UART_initStruct.TimeoutTime = 10;
	UART_initStruct.TimeoutIEn = 0;
 	UART_Init(UART0, &UART_initStruct);
	UART_Open(UART0);
}

/****************************************************************************************************************************************** 
* ��������: fputc()
* ����˵��: printf()ʹ�ô˺������ʵ�ʵĴ��ڴ�ӡ����
* ��    ��: int ch		Ҫ��ӡ���ַ�
*			FILE *f		�ļ����
* ��    ��: ��
* ע������: ��
******************************************************************************************************************************************/
int fputc(int ch, FILE *f)
{
	UART_WriteByte(UART0, ch);
	
	while(UART_IsTXBusy(UART0));
 	
	return ch;
}

