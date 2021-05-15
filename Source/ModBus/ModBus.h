#ifndef SOFTWARE_SOURCE_MODBUS_MODBUS_H_
#define SOFTWARE_SOURCE_MODBUS_MODBUS_H_

#include "mainconfig.h"

// Функции процесса работы с ModBus
void InitModBus(uint16_t mbaddr,uint16_t mbconn);
// Функция обмена по modBus
// Вызывать в основном цикле
// Возвращает не ноль, если надо сохранить HardDSt
uint32_t ModBusProc(void);  

// Описание структуры и таблицы параметров устройства
typedef struct {
  uint16_t reg;  // Номер регистра (с 0)
  void * addr;   // Адрес в памяти
  uint32_t flag; // Флаги
} MbStruct;

// Флаги параметров, доступных по ModBus
// Типы данных
#define MBT_FMASK   0x0F
#define MBT_INT8    0x00  // Расширяется до uint16, записывается и считывается из памяти как uint8
#define MBT_INT16   0x01
#define MBT_INT32   0x02
#define MBT_FLOAT   0x04
// Модификаторы доступности записи
#define MBT_WRITIBLE  0x0800   // Доступно на запись, всегда
#define MBT_AWRITIBLE 0x0400   // Доступно на запись, в административном режиме
#define MBT_SWRITIBLE 0x0200   // Доступно на запись, в режиме супервайзера
#define MBT_WRITEMASK (MBT_SWRITIBLE|MBT_AWRITIBLE|MBT_WRITIBLE)   // Маска флагов на запись
#define MBT_EESAVE   0x4000   // Записывать EEPROM после записи
#define MBT_CHECKSAV 0x2000   // Выполнять функцию MBCanWrite для определения возможности записи

// Таблица параметров. Должна заканчиваться записью 0,0,0
extern MbStruct mbs[];

// Возвращает текущий уровень доступа
// 0 - никакой (MBT_WRITIBLE)
// 1 - административный (MBT_AWRITIBLE)
// 3 - супервайзерский  (MBT_SWRITIBLE)
uint8_t MBCanWrite(void);

// Дополнительная функция - расчет CRC. Используется для проверки памяти данных
uint16_t CalcCRC(uint8_t * curbuf,int16_t cnt);

#endif
