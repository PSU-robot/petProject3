#include "MyEXTI.h"

void EXTIDisable(uint8_t num) {
  if (num>22) return;
  EXTI->IMR&=~(1<<num);
  EXTI->EMR&=~(1<<num);
}

void EXTIEnableInterrupt(uint8_t num,uint8_t type) {
  uint32_t bitt;
  if (num>22) return;
  bitt=(1<<num);
  EXTI->IMR&=~bitt;
  EXTI->EMR&=~bitt;
  if (type&EI_TRIGGER_RISING)  EXTI->RTSR|=bitt; else EXTI->RTSR&=~bitt; 
  if (type&EI_TRIGGER_FALLING) EXTI->FTSR|=bitt; else EXTI->FTSR&=~bitt; 
  EXTIClearPending(num);
  EXTI->IMR|=bitt;
}

// Сбросить флаг прерывания
void EXTIClearPending(uint8_t num) {
  uint32_t cbit=1;
  cbit<<=num;
  EXTI->PR=cbit;
}

// Проверить флаг прерывания
uint8_t EXTICheckPending(uint8_t num) {
  uint32_t cbit=1;
  cbit<<=num;
  return (EXTI->PR&cbit)?1:0;
}

// Конфигурирует SYSCFG подключая вывод к прерываниям
void EXTIPortConnect(GPIO_TypeDef* GPIOx,uint16_t Pin) {
  uint32_t portcode;
  if (Pin>15) return;
  switch ((uint32_t)GPIOx) {
    case ((uint32_t)GPIOA):portcode=0;break;
    case ((uint32_t)GPIOB):portcode=1;break;
    case ((uint32_t)GPIOC):portcode=2;break;
    case ((uint32_t)GPIOD):portcode=3;break;
//    case ((uint32_t)GPIOE):portcode=4;break;
    case ((uint32_t)GPIOF):portcode=5;break;
    default:return;  // Нет такого порта
  }
  portcode<<=((Pin&3)<<2);
  SYSCFG->EXTICR[Pin>>2]&=~(0x0000000F<<((Pin&3)<<2));
  SYSCFG->EXTICR[Pin>>2]|=portcode;
}
