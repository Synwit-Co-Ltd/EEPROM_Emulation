#include "eeprom_emulation.h"


static bool EE_initialized = false;

static int  EE_activePage    = -1;
static int  EE_receivingPage = -1;

static bool EE_IfAllErased(uint32_t page);
static bool EE_TransferPage(uint32_t ee_addr, uint32_t value);


/******************************************************************************************************************************************
* ��������: EE_Init()
* ����˵��:	EEPROM ��ʼ��
* ��    ��: ��
* ��    ��: bool
* ע������: ��
******************************************************************************************************************************************/
bool EE_Init(void)
{
	if(EE_initialized)
		return true;
	
	for(int i = 0; i < EE_PAGE_COUNT; i++)
	{
		switch(EE_getPageStatus(i))
		{
		case eePageStatusActive:
			if(EE_activePage == -1)
			{
				EE_activePage = i;
			}
			else
			{
				/* More than one active page. Invalid system state. */
				return false;
			}
			break;
		
		case eePageStatusReceiving:
			if(EE_receivingPage == -1)
			{
				EE_receivingPage = i;
			}
			else
			{
				/* More than one receiving page. Invalid system state. */
				return false;
			}
			break;
		
		case eePageStatusErased:
			/* Validate if the page is really erased, and erase it if not. */
			if(!EE_IfAllErased(i))
			{
				EE_FlashErase(i);
			}
			break;
		
		default:
			/* Undefined page status, erase page. */
			EE_FlashErase(i);
			break;
		}
	}
	
	if((EE_activePage == -1) && (EE_receivingPage == -1))
	{
		/* No active page, No receiving page */
		return false;
	}
	else if((EE_activePage != -1) && (EE_receivingPage == -1))
	{
		/* Only active page */
	}
	else if((EE_activePage == -1) && (EE_receivingPage != -1))
	{
		/* Only receiving page, set it as active */
		EE_setPageStatus(EE_receivingPage, eePageStatusActive);
		EE_activePage    = EE_receivingPage;
		EE_receivingPage = -1;
	}
	else
	{
		/* One active page, One receiving page */
		/* Transfer variables from active to receiving page. */
		EE_TransferPage(0xFFFFFFFF, 0);
	}
	
	EE_initialized = true;
	
	return true;
}


/******************************************************************************************************************************************
* ��������: EE_Format()
* ����˵��:	����ȫ��ҳ������ page 0 ����Ϊ active page.
* ��    ��: ��
* ��    ��: ��
* ע������: ��
******************************************************************************************************************************************/
void EE_Format(void)
{
	for(int i = 0; i < EE_PAGE_COUNT; i++)
	{
		EE_FlashErase(i);
	}
	
	EE_activePage    =  0;
	EE_receivingPage = -1;
	
	EE_FlashWrite(EE_activePage, 0, 0xFFFFFFFF, (eePageStatusActive << 24) | 0x000001);
	
	EE_initialized = true;
}


/******************************************************************************************************************************************
* ��������: EE_Read()
* ����˵��:	Read the latest value for specified eeprom address.
* ��    ��: uint32_t ee_addr
*			uint32_t *value
* ��    ��: bool
* ע������: ��
******************************************************************************************************************************************/
bool EE_Read(uint32_t ee_addr, uint32_t *value)
{
	if(!EE_initialized)
		return false;
	
	for(int i = EE_ITEM_COUNT - 1; i > 0; i--)
	{
		uint32_t ee_addr_rd;
		
		EE_FlashRead(EE_activePage, i, &ee_addr_rd, value);
		
		if(ee_addr_rd == ee_addr)
		{
			return true;
		}
	}
	
	return false;
}


/******************************************************************************************************************************************
* ��������: EE_Write()
* ����˵��:	update value for specified eeprom address.
* ��    ��: uint32_t ee_addr
*			uint32_t value
* ��    ��: bool
* ע������: ��
******************************************************************************************************************************************/
bool EE_Write(uint32_t ee_addr, uint32_t value)
{
	if(!EE_initialized)
		return false;
	
	uint32_t ee_addr_rd;
	uint32_t value_rd;
	
	if(EE_Read(ee_addr, &value_rd))
	{
		if(value_rd == value)
		{
			return true;
		}
	}
	
	/* Start at the second item. The fist one is reserved for status and erase count. */
	int i;
	for(i = 1; i < EE_ITEM_COUNT; i++)
	{
		EE_FlashRead(EE_activePage, i, &ee_addr_rd, &value_rd);
		
		if(ee_addr_rd == 0xFFFFFFFF)	// empty
		{
			EE_FlashWrite(EE_activePage, i, ee_addr, value);
			
			break;
		}
	}
	
	if(i == EE_ITEM_COUNT)	// active page full
	{
		return EE_TransferPage(ee_addr, value);
	}
	
	return true;
}


/******************************************************************************************************************************************
* ��������: EE_IfAllErased()
* ����˵��:	��� Flash ָ��ҳ�Ƿ�ȫ�ǡ�1��
* ��    ��: uint32_t page
* ��    ��: bool
* ע������: ��
******************************************************************************************************************************************/
static bool EE_IfAllErased(uint32_t page)
{
	uint32_t ee_addr;
	uint32_t value;
	
	for(int i = 0; i < EE_ITEM_COUNT; i++)
	{
		EE_FlashRead(page, i, &ee_addr, & value);
		
		if((ee_addr != 0xFFFFFFFF) || (value != 0xFFFFFFFF))
			return false;
	}
	
	return true;
}


/******************************************************************************************************************************************
* ��������: EE_TransferPage()
* ����˵��:	Transfers the most recently written value of each variable, from the active to a new receiving page.
* ��    ��: uint32_t ee_addr
*			uint32_t value
* ��    ��: bool
* ע������: ��
******************************************************************************************************************************************/
static bool EE_TransferPage(uint32_t ee_addr, uint32_t value)
{
	if(EE_receivingPage == -1)
	{
		EE_receivingPage = EE_activePage + 1;
		if(EE_receivingPage >= EE_PAGE_COUNT)
			EE_receivingPage = 0;
		
		if(!EE_IfAllErased(EE_receivingPage))
		{
			EE_FlashErase(EE_receivingPage);
		}
	}
	
	EE_setPageStatus(EE_receivingPage, eePageStatusReceiving);
	
	SW_LOG_INFO("EE_TransferPage from page %d to page %d\n", EE_activePage, EE_receivingPage);
	
	/* ����֮ǰ���Ƚ� active ҳд��ʧ�ܵ����ݣ�д�� receiving ҳ��һ��λ�� */
	if(ee_addr != 0xFFFFFFFF)
	{
		EE_FlashWrite(EE_receivingPage, 1, ee_addr, value);
		
		SW_LOG_INFO("    ee_addr = %d, value = %d\n", ee_addr, value);
	}
	
	for(int i = EE_ITEM_COUNT - 1; i > 0; i--)
	{
		uint32_t ee_addr_rd_act;	// read from active page
		uint32_t value_rd_act;
		uint32_t ee_addr_rd_rcv;	// read from receiving page
		uint32_t value_rd_rcv;
		
		EE_FlashRead(EE_activePage, i, &ee_addr_rd_act, &value_rd_act);
		
		if(ee_addr_rd_act == 0xFFFFFFFF)
		{
			continue;
		}
		else
		{
			for(int j = 1; j < EE_ITEM_COUNT; j++)
			{
				EE_FlashRead(EE_receivingPage, j, &ee_addr_rd_rcv, &value_rd_rcv);
				
				if(ee_addr_rd_rcv == ee_addr_rd_act)	// already transferred
				{
					break;
				}
				else if(ee_addr_rd_rcv == 0xFFFFFFFF)	// empty word found. all transferred variables are checked.
				{
					SW_LOG_INFO("    ee_addr = %d, value = %d\n", ee_addr_rd_act, value_rd_act);
					
					EE_FlashWrite(EE_receivingPage, j, ee_addr_rd_act, value_rd_act);
					break;
				}
			}
		}
	}

	/* Update erase count.
	   If a new page cycle is started, increment the erase count.
	*/
	uint32_t eraseCount = EE_getEraseCount();
	if(EE_receivingPage == 0)
	{
		eraseCount++;
		
		SW_LOG_INFO("eraseCount = %d\n", eraseCount);
	}
	
	/* д�� erase count ʱ������ page status �򲻱� */
	EE_FlashWrite(EE_receivingPage, 0, 0xFFFFFFFF, (eraseCount & 0xFFFFFF) | 0xFF000000);
	
	/* Erase the old active page. */
	EE_FlashErase(EE_activePage);

	/* Set the receiving page to be the new active page. */
	EE_setPageStatus(EE_receivingPage, eePageStatusActive);

	EE_activePage    = EE_receivingPage;
	EE_receivingPage = -1;
	
	return true;
}


/******************************************************************************************************************************************
* ��������: EE_getEraseCount()
* ����˵��:	��ѯ Flash ��������
* ��    ��: ��
* ��    ��: uint32_t
* ע������: ��
******************************************************************************************************************************************/
uint32_t EE_getEraseCount(void)
{
	uint32_t ee_addr;
	uint32_t value;
	
	EE_FlashRead(EE_activePage, 0, &ee_addr, &value);
	
	uint32_t eraseCount = value & 0xFFFFFF;
	
	return eraseCount;
}


/******************************************************************************************************************************************
* ��������: EE_getPageStatus()
* ����˵��:	��ѯָ��ҳ��ҳ״̬
* ��    ��: uint32_t page
* ��    ��: EE_PageStatus_t
* ע������: ��
******************************************************************************************************************************************/
EE_PageStatus_t EE_getPageStatus(uint32_t page)
{
	uint32_t ee_addr;
	uint32_t value;
	
	EE_FlashRead(page, 0, &ee_addr, &value);
	
	return (value >> 24);
}


/******************************************************************************************************************************************
* ��������: EE_setPageStatus()
* ����˵��:	����ָ��ҳ��ҳ״̬
* ��    ��: uint32_t page
*			EE_PageStatus_t status
* ��    ��: ��
* ע������: ��
******************************************************************************************************************************************/
void EE_setPageStatus(uint32_t page, EE_PageStatus_t status)
{
	/* д�� page status ʱ������ erase count �򲻱� */
	uint32_t value = (status << 24) | 0xFFFFFF;
	
	EE_FlashWrite(page, 0, 0xFFFFFFFF, value);
}
