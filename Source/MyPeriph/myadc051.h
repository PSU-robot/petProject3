#ifndef MYADC051_H
#define MYADC051_H
//// Как правильно использовать АЦП
// 1. Разрешить тактирование АЦП - RCC->APB2ENR |= RCC_APB2ENR_ADC1EN;
// 2. Запустить калибровку АЦП - ADC051StartCal. Выждать завершение или сделать паузу
// 3. Настроить АЦП (он одновременно включится) - ADC051Init
// 4. Настроить каналы АЦП - ADC051SelCh
// 5. Настроить DMA и включить его (если используется)
// 6. Запустить АЦП - ADC051Start
// 7. Если хочется начать все заново - ADC_DeInit и возвращаемся к пункту 2

#include "mainconfig.h"

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
#define ADCCFG1_EXTSEL(A) (A<<6)   // Источник запуска
#define ADCCFG1_EXTEN(A) (A<<10)   // Способ запуска


// Биты общего регистра для функции ADC051Common
#define ADCCOM_VBATEN  0x01000000  // Разрешаем измерение батарейки
#define ADCCOM_TSEN    0x00800000  // Разрешаем термодатчик
#define ADCCOM_VREFEN  0x00400000  // Разрешаем опорное напряжение

// Настройка каналов измерения напряжения, температуры, Vref
void ADC051Common(uint32_t flags);

// Сброс АЦП
void ADC051DeInit(void);
void ADC051StartCal(ADC_TypeDef * adc);
// Настройка АЦП
// Предварительно включает АЦП
// Первый параметр - настраиваемый АЦП
// Второй параметр - частота работы
// Третий параметр - значение регистра конфигурации
// Четвертый параметр - разрешенные прерывания (регистр ADC_IER)
void ADC051Init(ADC_TypeDef *,uint32_t samptime,uint32_t cfgr1,uint32_t intst);

// Разрешение каналов, побитовое
#define ADCCH_VBAT 0x00040000   // Батарейка
#define ADCCH_TEMP 0x00010000   // Термодатчик
#define ADCCH_VREF 0x00020000   // Опорное напряжение
void ADC051SelCh(ADC_TypeDef *,uint32_t channels); 

void ADC051Start(ADC_TypeDef *);// Запуск измерения

// Параметры, хранящиеся в памяти
#define VREFINT_CAL (*((uint16_t*)(0x1FFFF7BA)))    // Значение ADC Vref при 30* и 3.3V
#define TS_CAL1     (*((uint16_t*)(0x1FFFF7B8)))    // Значение ADC TS при 30* и 3.3V
#define TS_CAL2     (*((uint16_t*)(0x1FFFF7C2)))    // Значение ADC TS при 110* и 3.3V

// Функции расчета
// Расчеты ведутся с пересчетом к измеренному VREF
// Возвращает значение в микровольтах
uint32_t GetUpwr(uint16_t vref); // Расчет текущего напряжения питания
uint32_t CalcU(uint16_t vref,uint16_t adczn);    // Расчет напряжения с пересчетом к измеренному VREF
// Рассчитывает значение температуры в 0.1 градусах
int32_t CalcT(uint16_t vref,uint32_t tszn); 

#endif
