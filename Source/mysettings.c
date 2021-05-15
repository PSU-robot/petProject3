// В проекте нужно освободить последние два сегмента флэш-памяти
// Для версии STM32F051R8T6 полный объем памяти - 10000h
// За два сегмента до этого адреса - 0x00F7FF
// Прервая копия - по адресу 0xF800, вторая - по адресу 0xFC00

#include "mysettings.h"
#include "ssec.h"
#include "myflash.h"
#include "mysettings.h"
#include "ModBus.h"
#include "ModBusParams.h"
#include "myiwdg.h"

#define BASEMEMADDR 0x000F800

MySettings HardDSt __attribute__((used)) __attribute__((at(BASEMEMADDR)))={0};
uint16_t HDStCRC __attribute__((used)) __attribute__((at(BASEMEMADDR+0x0300)))=0;

MySettings HardDStCopy __attribute__((used)) __attribute__((at(BASEMEMADDR+0x0400)))={0};
uint16_t HDStCopyCRC __attribute__((used)) __attribute__((at(BASEMEMADDR+0x0400+0x0300)))=0;

MySettings DSt;

// Копия в обычной части памяти - для восстановления при ошибке
const MySettings HardDStMaster = {
  1,                  // Адрес в сети modbus
  123,456             // Загадочные параметры
};


uint32_t hssavetime=0;
uint16_t RescueState=0;

void ReinitFromHardDSt(uint8_t soft) {
  uint32_t len;
  uint32_t crch,crcc;
// Проверяем копии. Если надо - восстанавливаем
  len=sizeof(MySettings);
  crch=CalcCRC((uint8_t *)(&HardDSt),len);
  crcc=CalcCRC((uint8_t *)(&HardDStCopy),len);
  if (crch==HDStCRC) {
    if ((crcc!=crch)||(HDStCopyCRC!=crch)) {  // Восстанавливаем вторую копию
      RescueState=1;
      hssavetime=zgetssec();
    }
    DSt=HardDSt;
  } else {  // Первая копия повреждена!
    if (crcc==HDStCopyCRC) {
      DSt=HardDStCopy;
      RescueState=2;
      hssavetime=zgetssec();
    } else {// Вторая копия тоже повреждена!!! Полный трындец!
      DSt=HardDStMaster;
      RescueState=3;
      hssavetime=zgetssec();      
    }
  }
}

void SaveCopy(uint32_t adrhdst,uint32_t adrcrc) {
    uint32_t Adr;
    uint32_t * AdrSrc;
    uint32_t num;
    uint32_t crc;
    crc=CalcCRC((uint8_t *)(&DSt),sizeof(MySettings));
    hssavetime=0;
    Adr=adrhdst;
    FLUnlock();
    FLWait();
    FLErasePage(Adr);
    FLWait();
    num=(sizeof(MySettings)+3)/4;
    AdrSrc=(uint32_t*)(&DSt);
    for (;num--;) {
      FLProgramWord(Adr, *(AdrSrc++));
      Adr+=4;
      FLWait();
      CLRWDT();
    }
    FLProgramWord(adrcrc,crc);
    FLLock();  
}

uint8_t CheckSaveHardDSt(void) {
  uint8_t result=0;
  if (hssavetime?(tdlt(hssavetime)>1000):0) {  // Сохраняем через секунду
    SaveCopy((uint32_t)(&HardDSt),(uint32_t)(&HDStCRC));
    SaveCopy((uint32_t)(&HardDStCopy),(uint32_t)(&HDStCopyCRC));
    result=1;
    hssavetime=0;
  }
  return result;
}
