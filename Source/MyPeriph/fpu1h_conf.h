// Конфигурация оборудования USART1 контроллера STM32F051
// Выводы RB6-RB8
// Используется CCP2 таймера 3 для генерации временных промежутков, таймер должен быть настроен до запуска
// модуля, с предделителем 15 от частоты 8МГц
// Частота тактирования - 8МГц
// Использует DMA на прием и на передачу

#ifndef FPU1H_CONFIG_H
#define FPU1H_CONFIG_H

#define FPU1H_RXPIN GPIOB,7       // Нога приемника
#define FPU1H_TXPIN GPIOB,6       // Нога передатчика
#define FPU1H_DEPIN GPIOB,8       // Нога переключения прием/передача
#define FPU1H_AFNUM 0             // Номер альтернативной функции, соответствующей UART
#define FPU1H_USART USART1        // Порт, к котором подключена линия
#define FPU1H_FREQ  16000000      // Частота работы UART и таймера генерации временных промежутков
#define FPU1H_TRDMA DMA1_Channel2 // DMA передачи UART
#define FPU1H_TRDMASTATE DMA1,2   // Флаги DMA передачи UART (DMA и номер потока)
#define FPU1H_RXDMA DMA1_Channel3 // DMA приема UART
#define FPU1H_UARTINTFUNC USART1_IRQHandler // Обработчик прерываний USART
#define FPU1H_USART_IRQn USART1_IRQn // Разрешение прерывания в NVIC
#define FPU1H_MBSIZE 128          // Размер промежуточного буфера чтения

// Таймер, используемый генератором временных промежутков
// Обязательно должен иметь делитель на 16 чтобы расчет временных промежутков совпадал с BaudRate
#define FPU1H_TIMER TIM3
#define FPU1H_TIMEROC TIM3->CCR2
#define FPU1H_CC_FLAG TIM_CC2IE

#endif
