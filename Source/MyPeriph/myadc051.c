#include "myadc051.h"

// Биты регистров ADC_SR и ADC_IER
#define ADCSR_OVRIF  0x0010     // Переполнение АЦП
#define ADCSR_EOSEQ  0x0008     // Окончание последовательности
#define ADCSR_EOC    0x0004     // Окончание измерения
#define ADCSR_EOS    0x0002     // Окончание зарядка конденсатора
#define ADCSR_ADCRDY 0x0001     // АЦП готов к работе

// Биты регистра ADC_CR
#define ADCCR_ADCAL  0x80000000 // Калибровка (запуск, окончание)
#define ADCCR_STOP   0x00000010 // Принудительная остановка
#define ADCCR_START  0x00000004 // Запуск
#define ADCCR_ADDIS  0x00000002 // Отключение АЦП
#define ADCCR_ADEN   0x00000001 // Включение АЦП

// Биты регистра ADC_CFGR1
#define ADCCFG1_DISCEN 0x00010000  // Непрерывный режим при нуле, одноразовый при 1
#define ADCCFG1_AUTOFF 0x00008000  // Автоматическое отключение
#define ADCCFG1_CONT   0x00002000  // 1 - непрерывное измерение, 0 - одноразовое измерение
#define ADCCFG1_OVRMOD 0x00001000  // 1 - не прерывать измерение при переполнении
#define ADCCFG1_ALIGN  0x00000020  // Выравнивание 1-левое
#define ADCCFG1_RES(N) (N<<3)      // Разрешение в битах 0-12, 1-10, 2-8 3-6
#define ADCCFG1_SCANDR 0x00000004  // Направление сканирования
#define ADCCFG1_DMACFG 0x00000002  // 1-DMA circular, 0 - one shot DMA
#define ADCCFG1_DMAEN  0x00000001  // Разрешение запроса DMA

// Биты общего регистра для функции ADC051Common
#define ADCCOM_VBATEN  0x01000000  // Разрешаем измерение батарейки
#define ADCCOM_TSEN    0x00800000  // Разрешаем термодатчик
#define ADCCOM_VREFEN  0x00400000  // Разрешаем опорное напряжение

// Настройка каналов измерения напряжения, температуры, Vref
void ADC051Common(uint32_t flags) {
  ADC->CCR=flags;
}


void ADC051DeInit(void) {
  RCC->APB2RSTR|=RCC_APB2RSTR_ADC1RST;
  RCC->APB2RSTR&=~RCC_APB2RSTR_ADC1RST;
}

// Запускает калибровку
// Если АЦП работает - останавливает АЦП 
void ADC051StartCal(ADC_TypeDef * adc) {
  if (adc->CR&ADCCR_ADEN) {
    adc->CR|=ADCCR_ADDIS;
    while (adc->CR&ADCCR_ADEN);
  }
  adc->CR|=ADCCR_ADCAL;
}

// Настройка АЦП
// Предварительно отключает АЦП если включен, затем запускает калибровку но не АЦП!
// Первый параметр - настраиваемый АЦП
// Второй параметр - частота работы из даташита, 0..7
// Третий параметр - значение регистра конфигурации
// Четвертый параметр - разрешенные прерывания (регистр ADC_IER)
// Процедура настраивает АЦП и сразу запускает калибровку
void ADC051Init(ADC_TypeDef * adc,uint32_t samptime,uint32_t cfgr1,uint32_t intst) {
// Ожидаем завершения калибровки если идет
  while (adc->CR&ADCCR_ADCAL);
  adc->CR|=ADCCR_ADEN;
// Настройка
  adc->CFGR1=cfgr1;
  adc->CFGR2=0;
  adc->IER=intst;
  adc->SMPR=samptime;
}

void ADC051SelCh(ADC_TypeDef * adc,uint32_t channels) {
  adc->CHSELR=channels;
}

// Запуск измерения
void ADC051Start(ADC_TypeDef * adc) {
  adc->CR|=ADCCR_START;
};

// Параметры, хранящиеся в памяти
#define VREFINT_CAL (*((uint16_t*)(0x1FFFF7BA)))    // Значение ADC Vref при 30* и 3.3V
#define TS_CAL1     (*((uint16_t*)(0x1FFFF7B8)))    // Значение ADC TS при 30* и 3.3V
#define TS_CAL2     (*((uint16_t*)(0x1FFFF7C2)))    // Значение ADC TS при 110* и 3.3V

// Функции расчета
// Расчет текущего напряжения питания в микровольтах по измеренному VREF
uint32_t GetUpwr(uint16_t vref) {
  uint32_t tmp;
  tmp=33000*VREFINT_CAL;
  tmp/=vref;
  return tmp*100;
}


// Расчет напряжения с пересчетом к измеренному VREF
// Возвращает значение в микровольтах
uint32_t CalcU(uint16_t vref,uint16_t adczn) {
  uint32_t upit;
  upit=GetUpwr(vref)/4096;
  return upit*adczn;
};

// Рассчитывает значение температуры в 0.1 градусах
int32_t CalcT(uint16_t vref,uint32_t tszn) {
  int32_t A25,KA;
  int32_t tmp;
// Пересчитываем tszn к 30 градусам и 3.3В по vref
  tszn*=VREFINT_CAL;
  tszn/=vref;
// Получили значение Asens
  KA=((int32_t)TS_CAL1-TS_CAL2)*100;
  KA/=80;
  A25=5*KA+TS_CAL1*100;
  tmp=(A25*10-1000*tszn);
  tmp=tmp/KA+250;
  return tmp;
};

