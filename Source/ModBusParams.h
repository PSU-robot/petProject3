#ifndef MODBUSPARAMS_H
#define MODBUSPARAMS_H

#include "mainconfig.h"

typedef struct {
  uint16_t adc[10];  // Не разобранные значения аналоговых входов
  uint16_t sig1,sig2;// Сигналы с первого магниторезистора (A)
  uint16_t cels;     // Температура процессора
  uint16_t upit;     // Напряжение питания процессора
  uint8_t ledstate;
} RAMInfo;

extern RAMInfo info;

#endif
