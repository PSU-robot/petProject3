#include "ssec.h"
#include "mytimer.h"
#include "mygpio.h"
#include "fpuart1.h"

uint32_t ssecpulse;   // �������� ��� �������� SSECVOLUME ��������� � �������
uint32_t ssec;        // ������� �����������
uint32_t div2sec;     // �������� �� 2 - ������ ���
uint8_t sound=0;      // ������ ��� �� ������

//////////////////////////////////
// ������� ������ �� ��������
uint32_t getssec(void) {
  uint32_t rez;
  rez=ssec;
  return rez;
};      

uint32_t zgetssec(void) {
  uint32_t rez;
  rez=ssec;
  if (rez==0) rez--;
  return rez;
};      

uint32_t tdlt(uint32_t time) {
  uint32_t t;
  t=ssec;
  return t-time;
}

// ���������� �� ������� ��������� SSEC
// ���������� ������ 500 ��.
void TIM3_IRQHandler(void) {
//
  if ((TIM3->SR & TIM_CC1IE)&&(TIM3->DIER & TIM_CC1IE)) {
    if ((div2sec++)&1) ssec++;
    TIM3->SR = (uint16_t)~TIM_CC1IE;
    TIM3->CCR1=TIM3->CCR1+ssecpulse;
    if (sound) {
      if (div2sec&1) {
        GSetPin(SOUND);
      } else {
        GResetPin(SOUND);
      }
    } else {
      GResetPin(SOUND);
    }
  }
// ������ � UART
//  FPU1H_TimerInterrupt();
}

void InitSSEC(void) {
  SystemCoreClockUpdate();
  ssecpulse=SystemCoreClock/(8*SSECVOLUME)-1;
  InitTimer(TIM3,15,0,0,TIM_CC1IE);
  InitOC(TIM3,1,0);
  TIM3->CCR1=ssecpulse;
//  TIM3->CCR2=ssecpulse;
  TIM3->CNT=0;
  NVIC_EnableIRQ(TIM3_IRQn);
  StartTimer(TIM3);  
}

void Sound(uint8_t snd) {
  sound=snd;
}
