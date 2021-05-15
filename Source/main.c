#include "mainconfig.h"
#include "mygpio.h"
#include "myiwdg.h"
#include "ssec.h"
#include "mysettings.h"


int16_t push[100];
int16_t pnum=0;		


// Работаем на 16МГц от внутреннего RC генератора. Точность обещают -3.8..+5.5% во всем диапазоне температур
// ВНИМАНИЕ!!! Нельзя обнулять RCC->CR
void SysInit(void) {
  RCC->CR |= 0x00000001;     // Включаем HSI
  while (!(RCC->CR&0x02));  // Ожидаем HSI (на всякий случай)
// Обнуляем не нужное
  RCC->CR &= 0x0000FFF8;
  RCC->CFGR=0;  // Переходим на HSI и все предделители - на нули
// Запускаем PLL - работать будем на 16 МГц
  RCC->CFGR2=1;  // В том числе предделитель перед PLL
  RCC->CFGR3=0;
  RCC->CFGR|=0x00080000;  // Делим на 2, умножаем на 4, источник - HSI
  RCC->CR |= 0x01000000;  // Запускаем PLL
  while (!RCC->CR&0x02000000);        // Ожидаем включения PLL. Если не включился - сторожевой таймер нас рассудит
  FLASH->ACR = 0x00000010;  // Enable Prefetch Buffer and set Flash Latency
  RCC->CFGR|=0x00000002;    // Выбираем PLL как источник тактирования
  while (!((RCC->CFGR&0x0C)==(0x02<<2)));  // Ожидаем подтверждения
// 
  RCC->CIR = 0x00000000;    // Запрещаем все прерывания
}

void ConfigHW() {
// Сначала подключаем к шине все блоки, какие нам могут понадобиться
  RCC->AHBENR|=RCC_AHBENR_GPIOAEN|RCC_AHBENR_GPIOBEN|RCC_AHBENR_GPIOCEN|RCC_AHBENR_GPIODEN|RCC_AHBENR_GPIOFEN;
  RCC->APB1ENR|=RCC_APB1ENR_TIM3EN;     // Таймер генератора временных промежутков 1мс и одновременно - таймаута UART
  RCC->APB2ENR|=RCC_APB2ENR_SYSCFGCOMPEN;// Системный регистр (пока тут наверное не нужен)
// Теперь настраиваем выводы  
// Выводы
// 
// Выходы
  GSetPinToOutput(RELAYOPEN,0,0);
  GSetPinToOutput(RELAYCLOSE,0,0);  
  GSetPinToOutput(MAINLED,1,0);
  GSetPinToOutput(SOUND,0,0);
// Входы
  GSetPinToInput(KEYOPEN,GPIO_PULLUP);
  GSetPinToInput(KEYCLOSE,GPIO_PULLUP);
  GSetPinToInput(PROGSW,GPIO_PULLUP);
  GSetPinToInput(BUTTON,0);
// Не используемые выводы
  GSetPinToInput(GPIOA,0,GPIO_PULLDOWN);
  GSetPinToInput(GPIOA,1,GPIO_PULLDOWN);
  GSetPinToInput(GPIOA,2,GPIO_PULLDOWN);
  GSetPinToInput(GPIOA,3,GPIO_PULLDOWN);
  GSetPinToInput(GPIOA,4,GPIO_PULLDOWN);
  GSetPinToInput(GPIOA,5,GPIO_PULLDOWN);
  GSetPinToInput(GPIOA,11,GPIO_PULLDOWN);
  GSetPinToInput(GPIOA,12,GPIO_PULLDOWN);
  GSetPinToInput(GPIOB,5,GPIO_PULLDOWN);
  GSetPinToInput(GPIOB,7,GPIO_PULLDOWN);
// Таймер генерации SSEC
  InitSSEC();
  __enable_irq();
}

////////////////////////////////////////////////////////////
// Основная программа
// Эта функция никогда не заканчивается, потому что выходить некуда - мы в контроллере
int main() {
  uint32_t ledtime=0;
  uint32_t ledst=0;
	
	
////////////////////////////////////////////////////
// Инициализация аппаратной части контроллера  
  SysInit();   // Запускаем контроллер на нужной нам частоте (сейчас - 16МГц)
#ifndef MYDEBUG
  InitIWDG(4);
#endif
  ConfigHW();  // Конфигурируем аппаратную часть контроллера (выводы)
// Считываем конфигурацию из памяти контроллера. На самом деле происходит расчет контрольных сумм и сравнение
// их с записанными для двух копий настроечных данных. Если какая-то из копий повреждена - запускается процедура восстановления.
  ReinitFromHardDSt(0);
// Процедура восстановления - 
// Если был сбой памяти - ждём записи в память
// Через таймаут - поскольку в момент включения питания возможны помехи, и будет повторное повреждение копии данных
  while (hssavetime!=0) {
    CLRWDT();
    CheckSaveHardDSt();
  }
// Такая сложная схема хранения позволяет гарантировать, что если даже питание контроллера будет выключено в момент записи данных
// при следующем включении автоматически восстановится или новая (если записана первая копия, а повреждена - вторая), либо старая 
// (если запись прервалась при записи первой копии) копия данных.
//
// Конфигурируем связь с ПК - выводы порта настраиваются внутри данной процедуры
// Параметры настройки порта всегда одинаковы - 115200 бод, 8N1 - то есть 115200 бит в секунду передается,
// 8 бит на одно слово данных, без контроля четности, 1 стоповый бит.
// Начинается основной цикл - из него программа никогда не выходит
  for (;;) {  
// Проверяем - если надо, записываем конфигурационные данные в память     
    CheckSaveHardDSt();
// Основной светодиод - мигает при программировании
//
		
   /* if (GGetPin(PROGSW)) {
      GSetPin(MAINLED);
    } else {
      GResetPin(MAINLED);
    }
    if (GGetPin(KEYCLOSE)) {
      GResetPin(RELAYCLOSE);
    } else {
      GSetPin(RELAYCLOSE);
    }
    if (GGetPin(KEYOPEN)) {
      GResetPin(RELAYOPEN);
    } else {
      GSetPin(RELAYOPEN);
    }    
    if (!GGetPin(BUTTON)) {
      Sound(1);
    } else {
      Sound(0);
    } */
		//считывание нажатий и пауз
if (GGetPin(BUTTON)){
	if (tdlt(getssec())>100){
		if (pnum!=100){
			pnum++;
			push[pnum]=tdlt(getssec());
		}
		else{
			pnum=0;
		}
	}
}
else{
	if (tdlt(getssec())<2000){
			if (pnum!=100){
				pnum++;
				push[pnum]=tdlt(getssec());
		}
	else{
		pnum=0;
	}
	}
}
}
}
