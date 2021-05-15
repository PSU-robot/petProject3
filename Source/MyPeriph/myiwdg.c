#include "myiwdg.h"

void InitIWDG(uint8_t psc) {
  if (psc>=2) psc-=2;
  if (psc>7) psc=7;
	IWDG->KR = 0x5555;
	IWDG->PR = psc;
	IWDG->KR = 0x0000;
}

void StartIWDG(void) {
	IWDG->KR = 0xCCCC;
}

