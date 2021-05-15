#include "ModBusParams.h"
#include "ModBus.h"
#include "mysettings.h"

RAMInfo info;

// Таблица параметров. Должна заканчиваться записью 0,0,0
MbStruct mbs[]={
// Информационные параметры - состояние клапана, реле и напряжения
 {   1,&info.sig1       ,MBT_INT16},
 {   2,&info.sig2       ,MBT_INT16},
 {   3,&info.cels       ,MBT_INT16},
 {   4,&info.upit       ,MBT_INT16},
 {   5,&info.ledstate   ,MBT_INT8},
 {  100,&info.adc[0]       ,MBT_INT16},
 {  101,&info.adc[1]       ,MBT_INT16},
 {  102,&info.adc[2]       ,MBT_INT16},
 {  103,&info.adc[3]       ,MBT_INT16},
 {  104,&info.adc[4]       ,MBT_INT16},
 {  105,&info.adc[5]       ,MBT_INT16},
 {  106,&info.adc[6]       ,MBT_INT16},
 {  107,&info.adc[7]       ,MBT_INT16},
 {  108,&info.adc[8]       ,MBT_INT16},
 {  109,&info.adc[9]       ,MBT_INT16},
// Общие настроечные параметры
 {1000,&DSt.mbaddr         ,MBT_AWRITIBLE|MBT_EESAVE|MBT_INT8},
 {1001,&DSt.param1         ,MBT_AWRITIBLE|MBT_EESAVE|MBT_INT16},
 {1002,&DSt.param2         ,MBT_AWRITIBLE|MBT_EESAVE|MBT_INT16},
//
 {0,0,0}
};

// Возвращает текущий уровень доступа
// 0 - никакой (MBT_WRITIBLE)
// 1 - административный (MBT_AWRITIBLE)
// 3 - супервайзерский  (MBT_SWRITIBLE)
uint8_t MBCanWrite(void) {
  return 3;
}
