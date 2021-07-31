#include "mydac.h"

void InitDAC(uint32_t cfg) {
  DAC->CR=cfg;
}

void SetDAC1(uint32_t zn) {
  DAC->DHR12L1=zn;
}

/*void SetDAC2(uint32_t zn) {
  DAC->DHR12L2=zn;  
}*/

