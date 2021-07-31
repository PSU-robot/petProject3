#include "myflash.h"

void FLUnlock(void) {
  if(FLASH->CR& FLCR_LOCK) {
    FLASH->KEYR=0x45670123;
    FLASH->KEYR=0xCDEF89AB;  
  }
}

void FLLock(void) {
  FLASH->CR|=FLCR_LOCK;
}

void FLWait(void) {
  while (FLASH->SR&FLSR_BSY);
}

void FLErasePage(uint32_t addr) {
  FLWait();
  FLASH->CR &= ~(FLCR_PER|FLCR_OPTER|FLCR_OPTPG|FLCR_OPTWRE|FLCR_OBL_LAUNCH|FLCR_MER); // Очищаем биты операций
  FLASH->CR |= FLCR_PER;
  FLASH->AR=addr;
  FLASH->CR |= FLCR_STRT;
  FLWait();
  FLASH->CR &= ~FLCR_PER;
}
  
void FLProgramWord(uint32_t addr,uint32_t data) {
  FLWait();
  FLASH->CR &= ~(FLCR_PER|FLCR_OPTER|FLCR_OPTPG|FLCR_OPTWRE|FLCR_OBL_LAUNCH|FLCR_MER); // Очищаем биты операций
  FLASH->CR |= FLCR_PG;
  *(__IO uint16_t*)addr = (uint16_t)data;
  FLWait();
  *(__IO uint16_t*)(addr+2) = (uint16_t)(data>>16);
  FLWait();
  FLASH->CR &= ~FLASH_CR_PG;
}

////////////////////////////////////// Работа с байтами состояния

void FLUnlock_OB(void) {
  if (!(FLASH->CR& FLASH_CR_OPTWRE)) {
    FLASH->OPTKEYR=0x45670123;
    FLASH->OPTKEYR=0xCDEF89AB;  
  }
}

void FLErase_OB(void) {
  FLWait();
//  
  FLASH->CR |= FLASH_CR_OPTER;
  FLASH->CR |= FLASH_CR_STRT;
  FLWait();
//
  FLASH->CR &= ~FLASH_CR_OPTER;
// Enable the Option Bytes Programming operation
  FLASH->CR |= FLASH_CR_OPTPG;
// Restore the last read protection Option Byte value
//  OB->RDP = (uint16_t)rdptmp; 
  FLWait();
  FLASH->CR &= ~FLASH_CR_OPTPG;
}

void FLOB_Config(uint8_t ReadLevel,uint8_t OB_USER) {
  FLWait();
  FLASH->CR |= FLASH_CR_OPTER;
  FLASH->CR |= FLASH_CR_STRT;
  FLWait();
// If the erase operation is completed, disable the OPTER Bit
  FLASH->CR &= ~FLASH_CR_OPTER;
// Enable the Option Bytes Programming operation
  FLASH->CR |= FLASH_CR_OPTPG;
  switch (ReadLevel) {
    case (0):
    default:
      OB->RDP  = 0xAA;
      break;
    case (1):
      OB->RDP  = 0xBB;
      break;
    case (2):
//      OB->RDP  = 0xCC;    // ЭТО НЕОБРАТИМО!!!
      break;
  }
  OB->USER = OB_USER|0xF8;
// Wait for last operation to be completed
  FLWait();
  FLASH->CR &= ~FLASH_CR_OPTPG;
}

void FLProgramOB(void) {
  FLASH->CR |= FLASH_CR_OBL_LAUNCH; 
}

void FLLock_OB(void) {
  FLASH->CR &= ~FLASH_CR_OPTWRE;
}
