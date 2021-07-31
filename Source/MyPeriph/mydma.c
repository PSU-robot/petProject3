#include "mydma.h"

// Настройка канала DMA
// Первый параметр - канал DMA
// Второй параметр - настроечный, собираемый из определений выше
// Третий и четвертый параметры - адреса в периферии и памяти
// Пятый параметр - количество передаваемых байтов
void InitDMACh(DMA_Channel_TypeDef * dmach,uint32_t dmacfg,uint32_t periph,uint32_t memory,uint32_t count) {
  dmach->CCR=0;
  dmach->CPAR=periph;
  dmach->CMAR=memory;
  dmach->CNDTR=count;
  dmach->CCR=dmacfg;
}

void StartDMA(DMA_Channel_TypeDef * dmach) {
  dmach->CCR|=DMA_EN;
}

void StopDMA(DMA_Channel_TypeDef * dmach) {
  dmach->CCR&=~DMA_EN;
}

uint32_t GetCNDTR(DMA_Channel_TypeDef * dmach) {
  return dmach->CNDTR;
}

void ResetDMAIF(DMA_TypeDef * dma,uint8_t channel,uint32_t flag) {
  dma->IFCR=((flag*0x0F)<<(channel*4));
}

uint8_t CheckDMAIF(DMA_TypeDef * dma,uint8_t channel,uint32_t flag) {
  uint32_t flg;
  flg=((flag*0x0F)<<(channel*4));
  return ((flg&dma->ISR)?1:0);
}
