#include "mainconfig.h"
#include "mygpio.h"
#include "myiwdg.h"
#include "ssec.h"
#include "mysettings.h"


int16_t push[100];
int16_t pnum=0;		


// �������� �� 16��� �� ����������� RC ����������. �������� ������� -3.8..+5.5% �� ���� ��������� ����������
// ��������!!! ������ �������� RCC->CR
void SysInit(void) {
  RCC->CR |= 0x00000001;     // �������� HSI
  while (!(RCC->CR&0x02));  // ������� HSI (�� ������ ������)
// �������� �� ������
  RCC->CR &= 0x0000FFF8;
  RCC->CFGR=0;  // ��������� �� HSI � ��� ������������ - �� ����
// ��������� PLL - �������� ����� �� 16 ���
  RCC->CFGR2=1;  // � ��� ����� ������������ ����� PLL
  RCC->CFGR3=0;
  RCC->CFGR|=0x00080000;  // ����� �� 2, �������� �� 4, �������� - HSI
  RCC->CR |= 0x01000000;  // ��������� PLL
  while (!RCC->CR&0x02000000);        // ������� ��������� PLL. ���� �� ��������� - ���������� ������ ��� ��������
  FLASH->ACR = 0x00000010;  // Enable Prefetch Buffer and set Flash Latency
  RCC->CFGR|=0x00000002;    // �������� PLL ��� �������� ������������
  while (!((RCC->CFGR&0x0C)==(0x02<<2)));  // ������� �������������
// 
  RCC->CIR = 0x00000000;    // ��������� ��� ����������
}

void ConfigHW() {
// ������� ���������� � ���� ��� �����, ����� ��� ����� ������������
  RCC->AHBENR|=RCC_AHBENR_GPIOAEN|RCC_AHBENR_GPIOBEN|RCC_AHBENR_GPIOCEN|RCC_AHBENR_GPIODEN|RCC_AHBENR_GPIOFEN;
  RCC->APB1ENR|=RCC_APB1ENR_TIM3EN;     // ������ ���������� ��������� ����������� 1�� � ������������ - �������� UART
  RCC->APB2ENR|=RCC_APB2ENR_SYSCFGCOMPEN;// ��������� ������� (���� ��� �������� �� �����)
// ������ ����������� ������  
// ������
// 
// ������
  GSetPinToOutput(RELAYOPEN,0,0);
  GSetPinToOutput(RELAYCLOSE,0,0);  
  GSetPinToOutput(MAINLED,1,0);
  GSetPinToOutput(SOUND,0,0);
// �����
  GSetPinToInput(KEYOPEN,GPIO_PULLUP);
  GSetPinToInput(KEYCLOSE,GPIO_PULLUP);
  GSetPinToInput(PROGSW,GPIO_PULLUP);
  GSetPinToInput(BUTTON,0);
// �� ������������ ������
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
// ������ ��������� SSEC
  InitSSEC();
  __enable_irq();
}

////////////////////////////////////////////////////////////
// �������� ���������
// ��� ������� ������� �� �������������, ������ ��� �������� ������ - �� � �����������
int main() {
  uint32_t ledtime=0;
  uint32_t ledst=0;
	
	
////////////////////////////////////////////////////
// ������������� ���������� ����� �����������  
  SysInit();   // ��������� ���������� �� ������ ��� ������� (������ - 16���)
#ifndef MYDEBUG
  InitIWDG(4);
#endif
  ConfigHW();  // ������������� ���������� ����� ����������� (������)
// ��������� ������������ �� ������ �����������. �� ����� ���� ���������� ������ ����������� ���� � ���������
// �� � ����������� ��� ���� ����� ����������� ������. ���� �����-�� �� ����� ���������� - ����������� ��������� ��������������.
  ReinitFromHardDSt(0);
// ��������� �������������� - 
// ���� ��� ���� ������ - ��� ������ � ������
// ����� ������� - ��������� � ������ ��������� ������� �������� ������, � ����� ��������� ����������� ����� ������
  while (hssavetime!=0) {
    CLRWDT();
    CheckSaveHardDSt();
  }
// ����� ������� ����� �������� ��������� �������������, ��� ���� ���� ������� ����������� ����� ��������� � ������ ������ ������
// ��� ��������� ��������� ������������� ������������� ��� ����� (���� �������� ������ �����, � ���������� - ������), ���� ������ 
// (���� ������ ���������� ��� ������ ������ �����) ����� ������.
//
// ������������� ����� � �� - ������ ����� ������������� ������ ������ ���������
// ��������� ��������� ����� ������ ��������� - 115200 ���, 8N1 - �� ���� 115200 ��� � ������� ����������,
// 8 ��� �� ���� ����� ������, ��� �������� ��������, 1 �������� ���.
// ���������� �������� ���� - �� ���� ��������� ������� �� �������
  for (;;) {  
// ��������� - ���� ����, ���������� ���������������� ������ � ������     
    CheckSaveHardDSt();
// �������� ��������� - ������ ��� ����������������
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
		//���������� ������� � ����
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
