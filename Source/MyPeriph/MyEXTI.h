#ifndef MYEXTI_H
#define MYEXTI_H

#include "mainconfig.h"

// Доступные типы прерываний
#define EI_TRIGGER_FALLING       1
#define EI_TRIGGER_RISING        2
#define EI_TRIGGER_BOTH          3

void EXTIDisable(uint8_t num);                       // Отключает линию
void EXTIEnableInterrupt(uint8_t num,uint8_t type);  // Настраивает линию как прерывание
void EXTIClearPending(uint8_t num);                  // Сбросить флаг прерывания
uint8_t EXTICheckPending(uint8_t num);               // Проверить флаг прерывания
void EXTIPortConnect(GPIO_TypeDef* GPIOx,uint16_t Pin); // Конфигурирует SYSCFG подключая вывод к прерываниям

#endif
