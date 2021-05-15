//////////////////////////////////////////////////////////////
// Версия 0.1
//
#include "myuart.h"

// USART всегда настраивается на работу с DMA
// Передача запускается разрешением работы DMA
// Прием ведется в кольцевой буфер
// Управление потоком - вывод DE - программный, не подключен к USART
// 

// Конфигурация блока USART - без конфигурации выводов
// speed - скорость в бодах
// parity - четность, 0-N, 1-E, 2-O, +0x80-два стоп-бита
// Частота используется для расчета скорости, зависит от настройки предделителей
// Стоп-бит зависит от четности
void InitUSART(USART_TypeDef * USARTx,uint32_t speed,uint8_t parity,uint32_t USARTFreq) {
  uint32_t apbclock,divider;
  USARTx->CR1=0;
  USARTx->CR3=UART_OVRDIS|UART_ONEBIT|UART_DMAT|UART_DMAR;
  USARTx->CR2=(parity&0x80)?UART_2STOP:0;
  apbclock =USARTFreq;
// Расчет ведется всегда для 16 отсчетов на бит. OVER8 должен быть равен нулю
  divider = (uint32_t)((apbclock*2) / (speed));
  divider++;
  divider>>=1;
//  Записываем результат
  USARTx->BRR = (uint16_t)divider;
  switch (parity&3) {
    default:divider=0;break;
    case (1):divider=UART_9BIT|UART_PCE;break;
    case (2):divider=UART_9BIT|UART_ODD|UART_PCE;break;
  }
  divider=divider|UART_TE|UART_RE|UART_UE;
  USARTx->CR1=divider;
}

void InitHardDE(USART_TypeDef * USARTx) {
  USARTx->CR1&=~UART_UE;  // Отключаем UART
	USARTx->CR3 |= UART_DEM;
//	USARTx->CR3 &= (uint32_t)~((uint32_t)UART_DEP); ???
  USARTx->CR3 &= ~UART_DEP;
  USARTx->CR1 &= ~UART_BITS_DEAT;
  USARTx->CR1 |= UART_DEAT(10);
  USARTx->CR1 &= ~UART_BITS_DEDT;
  USARTx->CR1 |= UART_DEDT(0);
  USARTx->CR1|=UART_UE;  // Включаем UART
}

void USART_EnableIRQ(USART_TypeDef * USARTx,uint32_t flg,uint32_t enbit) {
  USARTx->ICR|=flg;
  USARTx->CR1|=enbit;
}

void USART_DisableIRQ(USART_TypeDef * USARTx,uint32_t flg,uint32_t enbit) {
  USARTx->ICR|=flg;
  USARTx->CR1&=~enbit;
}
