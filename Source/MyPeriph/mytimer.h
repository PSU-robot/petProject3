// Для работы с таймером выполнить InitTimer, или:
// Обнулить CR1, таймер отключится. 
// Установить в регистре PSC предделитель (Fsck/PSC+1)
// Установить значение счета в регистре ARR. Данное значение будет последним, после которого таймер сбросится в ноль
// Установить регистры CR2, DIER, последним - CR1 с включением счета

#ifndef MYTIMER_H
#define MYTIMER_H

#include "mainconfig.h"

// Биты регистра TIMx->CR1
#define TIM_ON      (1<<0)

// Биты регистра TIMx->CR2
#define TIM_MMS_RESET   (0x00<<4)   // TRGO подключен к биту UG
#define TIM_MMS_ENABLE  (0x01<<4)   // TRGO подключен к сигналу разрешения работы таймера
#define TIM_MMS_UPDATE  (0x02<<4)   // TRGO подключен к сигналу переполнения таймера

// Биты регистров TIMx->DIER и TIMx->SR
#define TIM_UDE         (1<<8)      // Запрос DMA при переполнении таймера
#define TIM_CC3IE       (1<<3)      // Флаг разрешения прерывания от CC2  (совпадает с битом в SR)
#define TIM_CC2IE       (1<<2)      // Флаг разрешения прерывания от CC2  (совпадает с битом в SR)
#define TIM_CC1IE       (1<<1)      // Флаг разрешения прерывания от CC1  (совпадает с битом в SR)
#define TIM_UIE         (1<<0)      // Запрос прерывания при переполнении таймера

// Биты определения каналов сравнения/захвата TIMx->CCMRx (младшая половина)
#define TIM_CCMR_OCxCE  (1<<7)      
#define TIM_CCMR_OCxMODE0 (0<<4)   // Frozen - выходы отключены  (можно не указывать)
#define TIM_CCMR_OCxMODE1 (1<<4)   // Установка выхода OCREF в 1 при совпадении
#define TIM_CCMR_OCxMODE2 (2<<4)   // Установка выхода OCREF в 0 при совпадении
#define TIM_CCMR_OCxMODE3 (3<<4)   // Переключение OCREF при совпадении
#define TIM_CCMR_OCxMODE4 (4<<4)   // OCREF=0
#define TIM_CCMR_OCxMODE5 (5<<4)   // OCREF=1
#define TIM_CCMR_OCxMODE6 (6<<4)   // PWM Mode1
#define TIM_CCMR_OCxMODE7 (7<<4)   // PWM Mode2
#define TIM_CCMR_OCxPE    (1<<3)   // Теневой регистр перед CCR
#define TIM_CCMR_OCxFE    (1<<2)   // Быстрый выход схемы сравнения
#define TIM_CCMR_CCxS0    (0)      // Канал конфигурируется как выход
#define TIM_CCMR_CCxS1    (1)      // Канал конфигурируется как вход, с TI1
#define TIM_CCMR_CCxS2    (2)      // Канал конфигурируется как вход, с TI2
#define TIM_CCMR_CCxS3    (3)      // Канал конфигурируется как вход, с TRC


// Инициализация таймера
void InitTimer(TIM_TypeDef*TIMx,uint32_t psc,uint32_t cr1,uint32_t cr2,uint32_t dier);
// Инициализация канала сравнения
void InitOC(TIM_TypeDef*TIMx,uint8_t occhnum,uint32_t flags);
// Запуск и остановка таймера
void StartTimer(TIM_TypeDef*TIMx);
void StopTimer(TIM_TypeDef*TIMx);

#endif
