#include "mygpio.h"

#define SETBITS(A) GPIOx->BSRR=A
#define RESETBITS(A) GPIOx->BSRR=(((uint32_t)A)<<16)

//////////////////////////
// Процедуры конфигурирования выводов
void GSetPinToInput(GPIO_TypeDef* GPIOx,uint16_t Pin,uint32_t Flags) {
  uint32_t sb2;
  uint32_t sb;
  uint32_t temp;
  sb=(1<<Pin);
  sb2=(3<<(Pin*2));
//  
  GPIOx->MODER=(GPIOx->MODER&~sb2);
  if (Flags&GPIO_OD) 
    GPIOx->OTYPER|=sb;
  else 
    GPIOx->OTYPER&=~sb;
// Скорость
  temp=GPIOx->OSPEEDR&~sb2;
  if (Flags&GPIO_SPEED_HI) 
    temp|=sb2;
  else if (Flags&GPIO_SPEED_MED) 
    temp|=(0x55555555&sb2);
  GPIOx->OSPEEDR=temp;
// Подтяжка
  temp=GPIOx->PUPDR&~sb2;
  if (Flags&GPIO_PULLUP) 
    temp|=(0x55555555&sb2);
  else if (Flags&GPIO_PULLDOWN) 
    temp|=(0xAAAAAAAA&sb2);
  GPIOx->PUPDR=temp;
}


void GSetPinToOutput(GPIO_TypeDef* GPIOx,uint16_t Pin,uint8_t StartState,uint32_t Flags) {
  uint32_t sb2;
  uint32_t sb;
  uint32_t temp;
  sb=(1<<Pin);
  sb2=(3<<(Pin*2));
//  
  GPIOx->MODER=(GPIOx->MODER&~sb2)|(0x55555555&sb2);
  if (Flags&GPIO_OD) 
    GPIOx->OTYPER|=sb;
  else 
    GPIOx->OTYPER&=~sb;
// Скорость
  temp=GPIOx->OSPEEDR&~sb2;
  if (Flags&GPIO_SPEED_HI) 
    temp|=sb2;
  else if (Flags&GPIO_SPEED_MED) 
    temp|=(0x55555555&sb2);
  GPIOx->OSPEEDR=temp;
// Подтяжка
  temp=GPIOx->PUPDR&~sb2;
  if (Flags&GPIO_PULLUP) 
    temp|=(0x55555555&sb2);
  else if (Flags&GPIO_PULLDOWN) 
    temp|=(0xAAAAAAAA&sb2);
  GPIOx->PUPDR=temp;
// Начальное состояние
  if (StartState)
    SETBITS(sb);
  else
	RESETBITS(sb);
}

void GSetPinToAnalog(GPIO_TypeDef* GPIOx,uint16_t Pin) {
  uint32_t sb2;
//  uint32_t sb;
//  uint32_t temp;
//  sb=(1<<Pin);
  sb2=(3<<(Pin*2));
//  
  GPIOx->MODER|=sb2;
// Подтяжка
  GPIOx->PUPDR&=~sb2;
}

void GSetPinToAF(GPIO_TypeDef* GPIOx,uint16_t Pin,uint8_t AFNum,uint32_t Flags) {
  uint32_t sb2;
  uint32_t sb;
  uint32_t temp;
  sb=(1<<Pin);
  sb2=(3<<(Pin*2));
//  
  GPIOx->MODER=(GPIOx->MODER&~sb2)|(0xAAAAAAAA&sb2);
  if (Flags&GPIO_OD) 
    GPIOx->OTYPER|=sb;
  else 
    GPIOx->OTYPER&=~sb;
// Скорость
  temp=GPIOx->OSPEEDR&~sb2;
  if (Flags&GPIO_SPEED_HI) 
    temp|=sb2;
  else if (Flags&GPIO_SPEED_MED) 
    temp|=(0x55555555&sb2);
  GPIOx->OSPEEDR=temp;
// Подтяжка
  temp=GPIOx->PUPDR&~sb2;
  if (Flags&GPIO_PULLUP) 
    temp|=(0x55555555&sb2);
  else if (Flags&GPIO_PULLDOWN) 
    temp|=(0xAAAAAAAA&sb2);
  GPIOx->PUPDR=temp;
// Номер альтернативной функции
  if (Pin>7) {
    temp=GPIOx->AFR[1];
    temp&=~(0x000F<<((Pin-8)*4));
    temp|=(AFNum<<((Pin-8)*4));
    GPIOx->AFR[1]=temp;
  } else {
    temp=GPIOx->AFR[0];
    temp&=~(0x000F<<(Pin*4));
    temp|=(AFNum<<(Pin*4));
    GPIOx->AFR[0]=temp;
  }
}

// Процедуры прямой работы с выводами, выводы нумеруются числом
void GSetPin(GPIO_TypeDef* GPIOx,uint16_t Pin) { SETBITS(1<<Pin); }
void GResetPin(GPIO_TypeDef* GPIOx,uint16_t Pin) { RESETBITS(1<<Pin); }
void GOutToPin(GPIO_TypeDef* GPIOx,uint16_t Pin,uint8_t State) {
  if (State) {
    SETBITS(1<<Pin);
  } else {
    RESETBITS(1<<Pin);
  }
}
uint8_t GGetPin(GPIO_TypeDef* GPIOx,uint16_t Pin) { return  (GPIOx->IDR&(1<<Pin))?1:0; }

// Процедуры прямой работы с выводами, выводы нумеруются маской
void GSetPins(GPIO_TypeDef* GPIOx,uint16_t Pins) { SETBITS(Pins); }
void GResetPins(GPIO_TypeDef* GPIOx,uint16_t Pins) { RESETBITS(Pins); }
void GOutToPins(GPIO_TypeDef* GPIOx,uint16_t Pins,uint8_t State) {
  if (State) {
    SETBITS(Pins);
  } else {
    RESETBITS(Pins);
  }
}

