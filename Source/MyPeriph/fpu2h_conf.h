// Конфигурация оборудования USART2 контроллера STM32F051
// Выводы RA2-RA4
// Используется CCP2 таймера 3 для генерации временных промежутков, таймер должен быть настроен до запуска
// модуля, с предделителем 15 от частоты 8МГц
// Частота тактирования - 8МГц
// Использует DMA на прием и на передачу

#ifndef FPU2H_CONFIG_H
#define FPU2H_CONFIG_H

#define FPU2H_RXPIN GPIOA,3       // Нога приемника
#define FPU2H_TXPIN GPIOA,2       // Нога передатчика
#define FPU2H_DEPIN GPIOA,1       // Нога переключения прием/передача
#define FPU2H_AFNUM 1             // Номер альтернативной функции, соответствующей UART
#define FPU2H_USART USART2        // Порт, к котором подключена линия
#define FPU2H_FREQ  16000000      // Частота работы UART и таймера генерации временных промежутков
#define FPU2H_TRDMA DMA1_Channel4 // DMA передачи UART
#define FPU2H_TRDMASTATE DMA1,4   // Флаги DMA передачи UART (DMA и номер потока)
#define FPU2H_RXDMA DMA1_Channel5 // DMA приема UART
#define FPU2H_UARTINTFUNC USART2_IRQHandler // Обработчик прерываний USART
#define FPU2H_USART_IRQn USART2_IRQn // Разрешение прерывания в NVIC
#define FPU2H_MBSIZE 128          // Размер промежуточного буфера чтения

// Таймер, используемый генератором временных промежутков
// Обязательно должен иметь делитель на 16 чтобы расчет временных промежутков совпадал с BaudRate
#define FPU2H_TIMER TIM3
#define FPU2H_TIMEROC TIM3->CCR2
#define FPU2H_CC_FLAG TIM_CC2IE

#endif
