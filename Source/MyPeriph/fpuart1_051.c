#include "fpuart1.h"
#include "mygpio.h"
#include "mydma.h"
#include "mytimer.h"
#include "myuart.h"
#include "string.h"

uint8_t FPU1H_errflg=0;  // Флаг наличия ошибки приема
// Расчетное число тактов таймера, равное 1 байту
uint32_t FPU1H_onebyte=0;

uint8_t FPU1H_buf[FPU1H_MBSIZE];  // Временный буфер приема на случай слишком длительного запрета прерываний
uint8_t FPU1H_bufpos=0;
uint8_t getto=0;

// Состояние передатчика
// 0 - передача не ведется, прием кончился
// 1 - пауза перед передачей
// 2 - передача
// 3 - завершающая пауза
// 4 - идет прием
uint8_t FPU1H_trst=0;
uint32_t FPU1H_trpnt;
uint16_t FPU1H_trlen;

uint32_t InitFPU1(uint32_t speed,uint8_t parity) {
  uint8_t dvdr;
// Отключаем USART на всякий случай
  FPU1H_USART->CR1=0;
// Настраиваем выводы
  GSetPinToAF(FPU1H_RXPIN,FPU1H_AFNUM,GPIO_PULLUP);
  GSetPinToAF(FPU1H_TXPIN,FPU1H_AFNUM,GPIO_OD);     // ВНИМАНИЕ!!! Версия с открытым коллектором для 5В схем
  #ifdef FPU1H_DEPIN
    GSetPinToOutput(FPU1H_DEPIN,0,GPIO_OD);         // ВНИМАНИЕ!!! Версия с открытым коллектором для 5В схем
  #endif
// Настраиваем UART
  InitUSART(FPU1H_USART,speed,parity,FPU1H_FREQ);
// Разрешение прерываний от приема и от ошибки
  FPU1H_USART->CR1|=UART_PEIE;
  FPU1H_USART->CR3|=UART_EIE;
// Разрешаем прерывания от USART
  NVIC_EnableIRQ(FPU1H_USART_IRQn);
// Рассчитываем интервал таймера, равный одному байту
  dvdr=10;
  if (parity&0x80) dvdr++;
  if (parity&0x03) dvdr++;
  speed=((16*speed)/(dvdr)+1);
  FPU1H_onebyte=FPU1H_FREQ/speed;   // Длительность одного байта
// Конфигурируем DMA приема
  InitDMACh(FPU1H_RXDMA,
            DMA_MEMSIZE8BIT|DMA_PRHSIZE8BIT|DMA_MINC|DMA_CIRC|DMA_EN,
            (uint32_t)(&FPU1H_USART->RDR),(uint32_t)FPU1H_buf,FPU1H_MBSIZE);
  FPU1H_errflg=0;
  FPU1H_bufpos=0;
  FPU1H_trst=0;
  return FPU1H_onebyte;
}

// Установка паузы схемой сравнения таймера
void FPU1H_settoc(uint16_t newstate,uint32_t pause) {
  FPU1H_trst=newstate;
  FPU1H_TIMEROC=FPU1H_TIMER->CNT+pause;
  FPU1H_TIMER->SR&=~FPU1H_CC_FLAG;
  FPU1H_TIMER->DIER|=FPU1H_CC_FLAG;
}

void FPU1H_UARTINTFUNC(void) {
  if ((FPU1H_USART->CR1&UART_TCIE)&&(FPU1H_USART->ISR&UART_IR_TC)) {
// Передача завершена
    USART_DisableIRQ(FPU1H_USART,UART_IR_TC,UART_TCIE);
    FPU1H_settoc(3,FPU1H_onebyte>>1);
  }
  if (FPU1H_USART->ISR&(UART_IR_PE|UART_IR_ORE|UART_IR_FE|UART_IR_NF)) {
// Ошибка приема
    FPU1H_USART->ICR=(UART_IR_PE|UART_IR_ORE|UART_IR_FE|UART_IR_NF|UART_IR_RXNE); // БЫЛО: FPU1H_USART->ICR=(UART_IR_PE|UART_IR_ORE|UART_IR_RXNE);
    FPU1H_errflg=1;
  }
}

// Функция, вызываемая внутри процедуры обработки прерываний таймера
// Анализ бита схемы захвата производится внутри функции
void FPU1H_TimerInterrupt(void) {
  if (FPU1H_TIMER->SR&FPU1H_TIMER->DIER&FPU1H_CC_FLAG) {
// Таймаут
    switch (FPU1H_trst) {
      case (1):
        ResetDMAIF(FPU1H_TRDMASTATE,0x0F);
        InitDMACh(FPU1H_TRDMA,
                  DMA_MEMSIZE8BIT|DMA_PRHSIZE8BIT|DMA_MINC|DMA_MEMSRC|DMA_EN,
                  (uint32_t)(&FPU1H_USART->TDR),FPU1H_trpnt,FPU1H_trlen);
        FPU1H_TIMER->DIER&=~FPU1H_CC_FLAG;
        FPU1H_trst=2;
        USART_EnableIRQ(FPU1H_USART,UART_IR_TC,UART_TCIE);
        break;
      case (3):
        FPU1H_TIMER->DIER&=~FPU1H_CC_FLAG;
        FPU1H_trst=0;
        #ifdef FPU1H_DEPIN
          GResetPin(FPU1H_DEPIN);
        #endif
// Включаем приемную часть UART
        FPU1H_USART->CR1|=UART_RE;
        break;
      case (4):  // Таймаут приема
        getto=1;
        FPU1H_trst=0;
        break;
      default:
        FPU1H_TIMER->SR&=~FPU1H_CC_FLAG;
    }      
  }
}

// Возвращает наличие таймаута приема
uint8_t gettoFPU1(void) {
  return getto;
}

// Возвращает количество байт в буфере приема
uint8_t numbyteFPU1(void) {
  uint8_t pp2;
  pp2=(FPU1H_MBSIZE-GetCNDTR(FPU1H_RXDMA));
  if (FPU1H_bufpos!=pp2) {
    if (FPU1H_bufpos>pp2) {
      return (FPU1H_MBSIZE+pp2)-FPU1H_bufpos;
    } else return pp2-FPU1H_bufpos;
  } else return 0;    
}

// Возвращает 0 если байта нету
uint8_t getbyteFPU1(void) {
  if (FPU1H_bufpos!=(FPU1H_MBSIZE-GetCNDTR(FPU1H_RXDMA))) {
    uint8_t bt;
    bt=FPU1H_buf[FPU1H_bufpos++];
    if (FPU1H_bufpos>=FPU1H_MBSIZE) FPU1H_bufpos=0;
    return bt;    
  } else return 0;  
}

// Возвращает 0 если ошибок нет
uint8_t geterrFPU1(void) {
  uint8_t err;
  __disable_irq();
  err=FPU1H_errflg;
  FPU1H_errflg=0;
  __enable_irq();
  return (err!=0)?1:0;
};    

// Инициирует передачу пакета
// buff - указатель на буфер, в котором сформирован пакет
// len - длина пакета или 0, при нулевом значении длина пакета определяется по нулю в последнем байте
void sendpkgFPU1(uint8_t * buf,uint16_t length) {
// Отключаем приемную часть UART
  FPU1H_USART->CR1&=~UART_RE;
// Начинаем передачу, копируем буфер внутрь
  if (!length) length=strlen((char*)buf);    
  FPU1H_trpnt=(uint32_t)buf;
  FPU1H_trlen=length;
  FPU1H_trst=1;
  myEnterCritical();
  #ifdef FPU1H_DEPIN
    GSetPin(FPU1H_DEPIN);
  #endif
  FPU1H_settoc(1,((FPU1H_onebyte*3)>>1));
  myExitCritical();
}

// Возвращает 0 если передача не производится
uint8_t getsendstateFPU1(void) {
  return FPU1H_trst;
};

uint32_t get1ByteTimeFPU1(void) {
  return FPU1H_onebyte;
}
