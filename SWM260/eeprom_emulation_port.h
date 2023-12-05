#ifndef __EEPROM_EMULATION_PORT_H__
#define __EEPROM_EMULATION_PORT_H__

#include "SWM260.h"


#define EE_FLASH_BASE	0x1E000

#define EE_FLASH_END	0x20000

#define EE_PAGE_SIZE	2048

#define EE_ITEM_SIZE	8		// 4 �ֽ����ݡ�4 �ֽڵ�ַ




static inline void EE_FlashErase(uint32_t page)
{
	FLASH_Erase(EE_FLASH_BASE + EE_PAGE_SIZE * page);
}


static inline void EE_FlashWrite(uint32_t page, uint32_t index, uint32_t ee_addr, uint32_t value)
{
	/* ��д���ݺ�д��ַ����ֹ��ַд������ݻ�û���ü�д���ͻȻ�������� */
	uint32_t buffer[2] = { value, ee_addr };
	
	FLASH_Write(EE_FLASH_BASE + EE_PAGE_SIZE * page + EE_ITEM_SIZE * index, buffer, EE_ITEM_SIZE / 4);
}


static inline void EE_FlashRead(uint32_t page, uint32_t index, uint32_t * ee_addr, uint32_t * value)
{
	uint32_t * pWord = (uint32_t *)(EE_FLASH_BASE + EE_PAGE_SIZE * page + EE_ITEM_SIZE * index);
	
	*value   = pWord[0];
	*ee_addr = pWord[1];
}


#endif
