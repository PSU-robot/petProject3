//////////////////////////////////////////////////////////////
// Версия 0.1
//
#ifndef MYUART_H
#define MYUART_H

#include "mainconfig.h"

// Биты регистра CR1
#define UART_BITS_DEAT 0x03E00000 // Биты параметра DEAT
#define UART_DEAT(A)   (A<<21)    // Установка параметра DEAT
#define UART_BITS_DEDT 0x001F0000 // Биты параметра DEDT
#define UART_DEDT(A)   (A<<16)    // Установка параметра DEDT
#define UART_9BIT  0x00001000     // Передача 9 бит (включать при контроле четности
#define UART_RTOIE 0x04000000     // Разрешение прерывания по таймауту приема CR1
#define UART_CMIE  0x00004000     // Разрешение прерывания по приему байта    CR1
#define UART_PCE   0x00000400     // Разрешение контроля четности
#define UART_ODD   0x00000200     // ODD контроль четности
#define UART_PEIE  0x00000100     // Разрешение прерывания от четности
#define UART_TXEIE 0x00000080     // Разрешение прерывания от опустошения буфера
#define UART_TCIE  0x00000040     // Разрешение прерывания от окончания передачи
#define UART_RXNEIE 0x00000020    // Разрешение прерывания при переполнении или приеме байта
#define UART_TE    0x00000008     // Разрешение передатчика
#define UART_RE    0x00000004     // Разрешение приемника
#define UART_UE    0x00000001     // Разрешение работы USART

// Биты регистра CR2
#define UART_2STOP  (0x02<<12)    // Два стоповых бита
// Биты регистра CR3
#define UART_OVRDIS 0x00001000    // Запрещение анализа переполнения буфера
#define UART_DMAT   0x00000080    // Разрешение DMA при передаче
#define UART_DMAR   0x00000040    // Разрешение DMA при приеме
#define UART_EIE    0x00000001    // Разрешение генерации прерывания при ошибке (флаги ORE, FE, NF)

// Биты регистра CR3
#define UART_ONEBIT 0x00000800    // Фиксация значения по одному измерению
#define UART_DEP    0x00008000    // Полярность сигнала DE
#define UART_DEM    0x00004000    // Разрешение сигнала DE


// Биты регистров ISR и ICR
// Разрешаются прерывания в регистре CR1, некоторые возможно - CR2
//
#define UART_IR_TXE  0x00000080     // Флаг пустого буфера передачи
#define UART_IR_TC   0x00000040     // Флаг и прерывание окончания передачи
#define UART_IR_RXNE 0x00000020     // Буфер приема не пуст
#define UART_IR_ORE  0x00000008     // Переполнение буфера приема
#define UART_IR_NF   0x00000004     // Шум
#define UART_IR_FE   0x00000002     // Ошибка кадра
#define UART_IR_PE   0x00000001     // Ошибка четности

// Конфигурация блока USART - без конфигурации выводов
// speed - скорость в бодах
// parity - четность, 0-N, 1-E, 2-O, +128 - 2 стоповых бита
// USARTFreq - частота USART, зависит от шины, если все предделители =1, то можно использовать SystemCoreClock
void InitUSART(USART_TypeDef * USARTx,uint32_t speed,uint8_t parity,uint32_t USARTFreq);
void InitHardDE(USART_TypeDef * USARTx);  // Включение аппаратного вывода приема/передачи

// Работают только с битами, определенными в CR1
void USART_EnableIRQ(USART_TypeDef * USARTx,uint32_t flg,uint32_t enbit);
void USART_DisableIRQ(USART_TypeDef * USARTx,uint32_t flg,uint32_t enbit);

#endif
