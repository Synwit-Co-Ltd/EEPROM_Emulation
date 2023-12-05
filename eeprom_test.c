/*  定义在 EEPROM 中存储的参数的地址，可取值范围 0 -- 0xFFFFFFFE
 *	注意：参数地址定义后不能改变，否则会导致读取到其他参数的值；
 *	若一定要修改，则需要调用 EE_Format() 将所有参数删除
 */
#define EE_ADDR_Speed	0
#define EE_ADDR_Angle	2
#define EE_ADDR_Dummy	5


void test_eeprom(void)
{
	uint32_t i, j;
	uint32_t speed;
	uint32_t angle;
	uint32_t dummy;
	
	if(!EE_Init())
	{
		/* 初始化失败，有两种可能的原因：
		 * （1）第一次上电执行，对于这种情况，执行 EE_Format() 格式化即可
		 * （2）发生了严重的错误，导致 Flash 内容损坏，需要采用额外的手段区分这种情况并处理
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
}
