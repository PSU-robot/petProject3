#ifndef SOURCE_MYSETTINGS_H_
#define SOURCE_MYSETTINGS_H_

#include "mainconfig.h"

extern uint32_t hssavetime;

void ReinitFromHardDSt(uint8_t soft);
uint8_t CheckSaveHardDSt(void);

typedef struct {
  uint8_t mbaddr;     // Адрес в сети modbus (по умолчанию 1)
  uint16_t param1,param2; // Два загадочных параметра
} MySettings;

extern MySettings HardDSt;    // Копия в Flash - с ней работает ПО
extern MySettings DSt;        // Копия в RAM - сюда идет запись и чтение по интерфейсу

// Для отображения статуса в ModBus
extern uint16_t RescueState;

#endif
