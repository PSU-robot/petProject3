// Для использования DMA
// Выполнить InitDMACh, настройки собрать из определений ниже. Можно сразу указать включеннный DMA,
// либо выполнить StartDMA позднее
#ifndef MYDMA_H
#define MYDMA_H

#include "mainconfig.h"

// Биты регистра DAC->CR
#define DAC_DMAEN   (1<<12)
#define DAC_TRGTIM6 (0)
#define DAC_TRGEN   (1<<2)
#define DAC_BUFEN   (1<<1)
#define DAC_ON      (1<<0)

//// Определения DMA

// Регистр DMAx_Channely
#define DMA_M2M             (1<<14)     // Обмен "память в память"
#define DMA_PL(lev)         ((lev&0x03)<<12)     // Уровень приоритета - от 0 до 3, 3 - самый максимум
#define DMA_MEMSIZE8BIT     (0<<10)     // Восьмибитные слова в памяти
#define DMA_MEMSIZE16BIT    (1<<10)     // Шестнадцатибитные слова в памяти
#define DMA_MEMSIZE32BIT    (2<<10)     // Тридцатидвухбитные слова в памяти
#define DMA_PRHSIZE8BIT     (0<<8)      // Восьмибитные слова в периферии
#define DMA_PRHSIZE16BIT    (1<<8)      // Шестнадцатибитные слова в периферии
#define DMA_PRHSIZE32BIT    (2<<8)      // Тридцатидвухбитные слова в периферии
#define DMA_MINC            (1<<7)      // Увеличивать адрес в памяти
#define DMA_PINC            (1<<6)      // Увеличивать адрес в периферии
#define DMA_CIRC            (1<<5)      // Кольцевой буфер
#define DMA_MEMSRC          (1<<4)      // Передача из памяти в периферию
#define DMA_TEIE            (1<<3)      // Прерывание при ошибке передачи
#define DMA_HCIE            (1<<2)      // Прерывание при половинной передаче
#define DMA_TCIE            (1<<1)      // Прерывание при окончании передачи
#define DMA_EN              (1<<0)      // Разрешение работы DMA

// Настройка канала DMA
// Первый параметр - канал DMA
// Второй параметр - настроечный, собираемый из определений выше
// Третий и четвертый параметры - адреса в периферии и памяти
// Пятый параметр - количество передаваемых байтов
void InitDMACh(DMA_Channel_TypeDef * dmach,uint32_t dmacfg,uint32_t periph,uint32_t memory,uint32_t count);
void StartDMA(DMA_Channel_TypeDef * dmach);
void StopDMA(DMA_Channel_TypeDef * dmach);
// Количество байтов в буфере
uint32_t GetCNDTR(DMA_Channel_TypeDef * dmach);
// Сборсить и узнать флаги прерывания
void ResetDMAIF(DMA_TypeDef * dma,uint8_t channel,uint32_t flag);
uint8_t CheckDMAIF(DMA_TypeDef * dma,uint8_t channel,uint32_t flag);

// Биты флагов прерываний - в общем для всех каналов регистре DMAx->ISR
// В скобочках - номер канала от 1 до x)
#define DMA_TCIF(IFNUM) ((0x0002)<<(4*(IFNUM-1)))
#define DMA_CGIF(IFNUM) ((0x0001)<<(4*(IFNUM-1)))


#endif
