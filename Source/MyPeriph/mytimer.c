// Работа с таймерами
// Версия 0.1
//   -Добавлена принудительная генерация переполнения при инициализации для принятия прескалера таймера.
#include "mytimer.h"

void StartTimer(TIM_TypeDef*TIMx) { TIMx->CR1|=1; }

void StopTimer(TIM_TypeDef*TIMx) { TIMx->CR1&=0xFFFFFFFE; }

void InitTimer(TIM_TypeDef*TIMx,uint32_t psc,uint32_t cr1,uint32_t cr2,uint32_t dier) {
  TIMx->CR1=0;
  TIMx->CR2=cr2;
  TIMx->DIER=dier;
  TIMx->PSC=psc;
  TIMx->ARR=0xFFFFFFFF;
  TIMx->CR1=cr1;
  TIMx->EGR=0x0001;  // Добавлено для принудительного сброса таймера и принятия значения прескалера
}

// Инициализация канала сравнения
void InitOC(TIM_TypeDef*TIMx,uint8_t occhnum,uint32_t flags) {
  uint32_t temp;
  switch (occhnum) {
    case (1):
      temp=TIMx->CCMR1&0xFF00;
      temp|=flags;
      TIMx->CCMR1=temp;
      break;
    case (2):
      temp=TIMx->CCMR1&0x00FF;
      temp|=(flags<<8);
      TIMx->CCMR1=temp;
      break;
    case (3):
      temp=TIMx->CCMR2&0xFF00;
      temp|=flags;
      TIMx->CCMR2=temp;
      break;
    case (4):
      temp=TIMx->CCMR2&0x00FF;
      temp|=(flags<<8);
      TIMx->CCMR2=temp;
      break;
  }
}
