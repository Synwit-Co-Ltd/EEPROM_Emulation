#ifndef __EEPROM_EMULATION_H__
#define __EEPROM_EMULATION_H__

#include "eeprom_emulation_port.h"


#define EE_PAGE_COUNT	((EE_FLASH_END - EE_FLASH_BASE) / EE_PAGE_SIZE)

#define EE_ITEM_COUNT	(EE_PAGE_SIZE / EE_ITEM_SIZE)


typedef enum
{
	eePageStatusErased    = 0xFF,
	eePageStatusReceiving = 0xAA,
	eePageStatusActive    = 0x00,
} EE_PageStatus_t;


bool EE_Init(void);
void EE_Format(void);
bool EE_Read(uint32_t ee_addr, uint32_t *value);
bool EE_Write(uint32_t ee_addr, uint32_t value);

uint32_t EE_getEraseCount(void);
EE_PageStatus_t EE_getPageStatus(uint32_t page);
void EE_setPageStatus(uint32_t page, EE_PageStatus_t status);


#endif
