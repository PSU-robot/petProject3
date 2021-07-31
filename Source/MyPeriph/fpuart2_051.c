#include "fpuart2.h"
#include "mygpio.h"
#include "mydma.h"
#include "mytimer.h"
#include "myuart.h"
#include "string.h"

uint8_t FPU2H_errflg=0;  // Флаг наличия ошибки приема
// Расчетное число тактов таймера, равное 1 байту
uint32_t FPU2H_onebyte=0;

uint8_t FPU2H_buf[FPU2H_MBSIZE];  // Временный буфер приема на случай слишком длительного запрета прерываний
uint8_t FPU2H_bufpos=0;
uint8_t getto=0;

// Состояние передатчика
// 0 - передача не ведется, прием кончился
// 1 - пауза перед передачей
// 2 - передача
// 3 - завершающая пауза
// 4 - идет прием
uint8_t FPU2H_trst=0;
uint32_t FPU2H_trpnt;
uint16_t FPU2H_trlen;

uint32_t InitFPU2(uint32_t speed,uint8_t parity) {
  uint8_t dvdr;
// Отключаем USART на всякий случай
  FPU2H_USART->CR1=0;
// Настраиваем выводы
  GSetPinToAF(FPU2H_RXPIN,FPU2H_AFNUM,GPIO_PULLUP);
  GSetPinToAF(FPU2H_TXPIN,FPU2H_AFNUM,GPIO_PULLUP|GPIO_PUSHPULL);
  #ifdef FPU2H_DEPIN
    GSetPinToOutput(FPU2H_DEPIN,0,GPIO_PUSHPULL);
  #endif
// Настраиваем UART
  InitUSART(FPU2H_USART,speed,parity,FPU2H_FREQ);
// Разрешение прерываний от приема и от ошибки
  FPU2H_USART->CR1|=UART_PEIE;
  FPU2H_USART->CR3|=UART_EIE;
// Разрешаем прерывания от USART
  NVIC_EnableIRQ(FPU2H_USART_IRQn);
// Рассчитываем интервал таймера, равный одному байту
  dvdr=10;
  if (parity&0x80) dvdr++;
  if (parity&0x03) dvdr++;
  speed=((16*speed)/(dvdr)+1);
  FPU2H_onebyte=FPU2H_FREQ/speed;   // Длительность одного байта
// Конфигурируем DMA приема
  InitDMACh(FPU2H_RXDMA,
            DMA_MEMSIZE8BIT|DMA_PRHSIZE8BIT|DMA_MINC|DMA_CIRC|DMA_EN,
            (uint32_t)(&FPU2H_USART->RDR),(uint32_t)FPU2H_buf,FPU2H_MBSIZE);
  FPU2H_errflg=0;
  FPU2H_bufpos=0;
  FPU2H_trst=0;
  return FPU2H_onebyte;
}

// Установка паузы схемой сравнения таймера
void FPU2H_settoc(uint16_t newstate,uint32_t pause) {
  FPU2H_trst=newstate;
  FPU2H_TIMEROC=FPU2H_TIMER->CNT+pause;
  FPU2H_TIMER->SR&=~FPU2H_CC_FLAG;
  FPU2H_TIMER->DIER|=FPU2H_CC_FLAG;
}

void FPU2H_UARTINTFUNC(void) {
  if ((FPU2H_USART->CR1&UART_TCIE)&&(FPU2H_USART->ISR&UART_IR_TC)) {
// Передача завершена
    USART_DisableIRQ(FPU2H_USART,UART_IR_TC,UART_TCIE);
    FPU2H_settoc(3,FPU2H_onebyte>>1);
  }
  if (FPU2H_USART->ISR&(UART_IR_PE|UART_IR_ORE|UART_IR_FE|UART_IR_NF)) {
// Ошибка приема
    FPU2H_USART->ICR=(UART_IR_PE|UART_IR_ORE|UART_IR_FE|UART_IR_NF|UART_IR_RXNE); // БЫЛО: FPU2H_USART->ICR=(UART_IR_PE|UART_IR_ORE|UART_IR_RXNE);
    FPU2H_errflg=1;
  }
}

// Функция, вызываемая внутри процедуры обработки прерываний таймера
// Анализ бита схемы захвата производится внутри функции
void FPU2H_TimerInterrupt(void) {
  if (FPU2H_TIMER->SR&FPU2H_TIMER->DIER&FPU2H_CC_FLAG) {
// Таймаут
    switch (FPU2H_trst) {
      case (1):
        ResetDMAIF(FPU2H_TRDMASTATE,0x0F);
        InitDMACh(FPU2H_TRDMA,
                  DMA_MEMSIZE8BIT|DMA_PRHSIZE8BIT|DMA_MINC|DMA_MEMSRC|DMA_EN,
                  (uint32_t)(&FPU2H_USART->TDR),FPU2H_trpnt,FPU2H_trlen);
        FPU2H_TIMER->DIER&=~FPU2H_CC_FLAG;
        FPU2H_trst=2;
        USART_EnableIRQ(FPU2H_USART,UART_IR_TC,UART_TCIE);
        break;
      case (3):
        FPU2H_TIMER->DIER&=~FPU2H_CC_FLAG;
        FPU2H_trst=0;
        #ifdef FPU2H_DEPIN
          GResetPin(FPU2H_DEPIN);
        #endif
// Включаем приемную часть UART
        FPU2H_USART->CR1|=UART_RE;
        break;
      case (4):  // Таймаут приема
        getto=1;
        FPU2H_trst=0;
        break;
      default:
        FPU2H_TIMER->SR&=~FPU2H_CC_FLAG;
    }      
  }
}

// Возвращает наличие таймаута приема
uint8_t gettoFPU2(void) {
  return getto;
}

// Возвращает количество байт в буфере приема
uint8_t numbyteFPU2(void) {
  uint8_t pp2;
  pp2=(FPU2H_MBSIZE-GetCNDTR(FPU2H_RXDMA));
  if (FPU2H_bufpos!=pp2) {
    if (FPU2H_bufpos>pp2) {
      return (FPU2H_MBSIZE+pp2)-FPU2H_bufpos;
    } else return pp2-FPU2H_bufpos;
  } else return 0;    
}

// Возвращает 0 если байта нету
uint8_t getbyteFPU2(void) {
  if (FPU2H_bufpos!=(FPU2H_MBSIZE-GetCNDTR(FPU2H_RXDMA))) {
    uint8_t bt;
    bt=FPU2H_buf[FPU2H_bufpos++];
    if (FPU2H_bufpos>=FPU2H_MBSIZE) FPU2H_bufpos=0;
    return bt;    
  } else return 0;  
}

// Возвращает 0 если ошибок нет
uint8_t geterrFPU2(void) {
  uint8_t err;
  __disable_irq();
  err=FPU2H_errflg;
  FPU2H_errflg=0;
  __enable_irq();
  return (err!=0)?1:0;
};    

// Инициирует передачу пакета
// buff - указатель на буфер, в котором сформирован пакет
// len - длина пакета или 0, при нулевом значении длина пакета определяется по нулю в последнем байте
void sendpkgFPU2(uint8_t * buf,uint16_t length) {
// Отключаем приемную часть UART
  FPU2H_USART->CR1&=~UART_RE;
// Начинаем передачу, копируем буфер внутрь
  if (!length) length=strlen((char*)buf);    
  FPU2H_trpnt=(uint32_t)buf;
  FPU2H_trlen=length;
  FPU2H_trst=1;
  myEnterCritical();
  #ifdef FPU2H_DEPIN
    GSetPin(FPU2H_DEPIN);
  #endif
  FPU2H_settoc(1,((FPU2H_onebyte*3)>>1));
  myExitCritical();
}

// Возвращает 0 если передача не производится
uint8_t getsendstateFPU2(void) {
  return FPU2H_trst;
};

uint32_t get1ByteTimeFPU2(void) {
  return FPU2H_onebyte;
}
