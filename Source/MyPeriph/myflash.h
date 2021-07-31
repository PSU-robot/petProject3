#ifndef MYFLASH_H
#define MYFLASH_H

#include "mainconfig.h"

// Биты регистра FLASH->CR
#define FLCR_OBL_LAUNCH 0x00002000   // 
#define FLCR_EOPIE      0x00001000
#define FLCR_ERRIE      0x00000200
#define FLCR_OPTWRE     0x00000100
#define FLCR_LOCK       0x00000080
#define FLCR_STRT       0x00000040
#define FLCR_OPTER      0x00000020
#define FLCR_OPTPG      0x00000010
#define FLCR_MER        0x00000004
#define FLCR_PER        0x00000002
#define FLCR_PG         0x00000001

// Биты регистра FLASH->SR
#define FLSR_EOP        0x00000020
#define FLSR_WPERR      0x00000010
#define FLSR_PGERR      0x00000004
#define FLSR_BSY        0x00000001

void FLUnlock(void); // Разблокирует флеш
void FLLock(void);   // Блокирует флеш
void FLWait(void);   // Ожидает окончания операции
void FLErasePage(uint32_t addr); // Стирает страницу
void FLProgramWord(uint32_t addr,uint32_t data); // Программирует 32 бита

// Биты регистра OB->USER
#define OB_IWDG_HW        0x0000    // Аппаратный неотключаемый WDT
#define OB_IWDG_SW        0x0001
#define OB_STOP_RESET     0x0000    // Генерировать сброс при засыпании
#define OB_STOP_NORST     0x0002
#define OB_STDBY_RESET    0x0004    // Генерировать сброс при входе в режим остановки
#define OB_STDBY_NORST    0x0000

void FLUnlock_OB(void);
void FLErase_OB(void);
// Установка конфигурации
// ReadLevel - 0-без защиты, 1-защита, 2-суперзащита без возможности восстановить
// OB_USER - собирается из определний выше. Остальные биты устанавливаются в "1" - по умолчанию
void FLOB_Config(uint8_t ReadLevel,uint8_t OB_USER);
void FLProgramOB(void);
void FLLock_OB(void);

#endif
