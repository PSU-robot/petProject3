#ifndef MYDAC_H
#define MYDAC_H

#include "mainconfig.h"

////// Биты регистра DAC_CR
#define DAC1CH2_UDRIE    (1<<29)       // Прерывание при ошибке получения данных DAC от DMA 
#define DAC1CH2_DMAEN    (1<<28)       // Разрешение DMA для канала 2 DAC
#define DAC1CH2_TRGSOFT  (0x7<<19)     // Программный триггер
#define DAC1CH2_TEN      (1<<18)       // Разрешение триггера ЦАП
#define DAC1CH2_BOFF     (1<<17)       // Отключение выходного буфера
#define DAC1CH2_EN       (1<<16)       // Включение ЦАП
#define DAC1CH1_UDRIE    (1<<13)       // Прерывание при ошибке получения данных DAC от DMA 
#define DAC1CH1_DMAEN    (1<<12)       // Разрешение DMA для канала 2 DAC
#define DAC1CH1_TRGSOFT  (0x7<<3)     // Программный триггер
#define DAC1CH1_TEN      (1<<2)       // Разрешение триггера ЦАП
#define DAC1CH1_BOFF     (1<<1)       // Отключение выходного буфера
#define DAC1CH1_EN       (1<<0)       // Включение ЦАП

void InitDAC(uint32_t cfg);
void SetDAC1(uint32_t zn);    //  Установить 16-битное значение
void SetDAC2(uint32_t zn);    //  Установить 16-битное значение

#endif
