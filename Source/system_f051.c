/*
  Это моя версия файла, отличается наличием общих для всех проектов функций и
порядком инициализации генератора.
  После SystemInit генератор всегда настроен на 8МГц внутреннего RC генератора
  Включение кварца на разную частоту выполнять уже в main
  В файл включены функции:
    CLRWDT - сброс сторожевого таймера
    ConfigWDT - инициализация (включение и настройка большого предделителя) сторожевого таймера
    ConfigProtect - проверка бита защиты и при отсутствии - включение защиты и Hardware WDT
   
  Функции, доступные в данном файле, описаны в заголовочном файле mainconfig.h
  В этом же файле должна быть ссылка на заголовочный файл #include "stm32f0xx.h"

  */

#include "mainconfig.h"

#define HSI_VALUE ((uint32_t)8000000)
#define HSE_VALUE ((uint32_t)4000000)
                              
uint32_t SystemCoreClock    = 48000000;
const uint8_t AHBPrescTable[16] = {0, 0, 0, 0, 0, 0, 0, 0, 1, 2, 3, 4, 6, 7, 8, 9};

//static void SetSysClock(void);

void SystemInit (void)
{    
  RCC->CR |= (uint32_t)0x00000001;    // Set HSION bit
  RCC->CFGR &= (uint32_t)0xF8FFB80C;  // Reset SW[1:0], HPRE[3:0], PPRE[2:0], ADCPRE and MCOSEL[2:0] bits
  RCC->CR &= (uint32_t)0xFEF6FFFF;    // Reset HSEON, CSSON and PLLON bits
  RCC->CR &= (uint32_t)0xFFFBFFFF;    // Reset HSEBYP bit
  RCC->CFGR &= (uint32_t)0xFFC0FFFF;  // Reset PLLSRC, PLLXTPRE and PLLMUL[3:0] bits
  RCC->CFGR2 &= (uint32_t)0xFFFFFFF0; // Reset PREDIV1[3:0] bits
  RCC->CFGR3 &= (uint32_t)0xFFFFFEAC; // Reset USARTSW[1:0], I2CSW, CECSW and ADCSW bit
  RCC->CR2 &= (uint32_t)0xFFFFFFFE;   // Reset HSI14 bit
//
  RCC->CIR = 0x00000000;              // Disable all interrupts
}

void SystemCoreClockUpdate (void)
{
  uint32_t tmp = 0, pllmull = 0, pllsource = 0, prediv1factor = 0;

  /* Get SYSCLK source -------------------------------------------------------*/
  tmp = RCC->CFGR & RCC_CFGR_SWS;
  
  switch (tmp)
  {
    case 0x00:  /* HSI used as system clock */
      SystemCoreClock = HSI_VALUE;
      break;
    case 0x04:  /* HSE used as system clock */
      SystemCoreClock = HSE_VALUE;
      break;
    case 0x08:  /* PLL used as system clock */
      /* Get PLL clock source and multiplication factor ----------------------*/
      pllmull = RCC->CFGR & RCC_CFGR_PLLMUL;
      pllsource = RCC->CFGR & RCC_CFGR_PLLSRC;
      pllmull = ( pllmull >> 18) + 2;
      
      if (pllsource == 0x00)
      {
        /* HSI oscillator clock divided by 2 selected as PLL clock entry */
        SystemCoreClock = (HSI_VALUE >> 1) * pllmull;
      }
      else
      {
        prediv1factor = (RCC->CFGR2 & 0x0F) + 1;
        /* HSE oscillator clock selected as PREDIV1 clock entry */
        SystemCoreClock = (HSE_VALUE / prediv1factor) * pllmull; 
      }      
      break;
    default: /* HSI used as system clock */
      SystemCoreClock = HSI_VALUE;
      break;
  }
  /* Compute HCLK clock frequency ----------------*/
  /* Get HCLK prescaler */
  tmp = AHBPrescTable[((RCC->CFGR & RCC_CFGR_HPRE) >> 4)];
  /* HCLK clock frequency */
  SystemCoreClock >>= tmp;  
}

/*
static void SetSysClock(void)
{
  __IO uint32_t StartUpCounter = 0, HSEStatus = 0;  
  
#ifdef HSE_PRESENT
// Enable HSE
  RCC->CR |= ((uint32_t)RCC_CR_HSEON);  
  do {
    HSEStatus = RCC->CR & RCC_CR_HSERDY;
    StartUpCounter++;  
  } while((HSEStatus == 0) && (StartUpCounter != HSE_STARTUP_TIMEOUT));
  if ((RCC->CR & RCC_CR_HSERDY) != RESET) HSEStatus=1; else HSEStatus=0;
#endif
//
  if (HSEStatus) {
    FLASH->ACR = FLASH_ACR_PRFTBE; // | FLASH_ACR_LATENCY; 
    RCC->CFGR |= (uint32_t)RCC_CFGR_HPRE_DIV1;  // HCLK = SYSCLK
    RCC->CFGR |= (uint32_t)RCC_CFGR_PPRE_DIV1;  // PCLK = HCLK
// PLL configuration = HSE * 2 = 8 MHz
    RCC->CFGR &= (uint32_t)((uint32_t)~(RCC_CFGR_PLLSRC | RCC_CFGR_PLLXTPRE | RCC_CFGR_PLLMULL));
    RCC->CFGR |= (uint32_t)(RCC_CFGR_PLLSRC_PREDIV1 | RCC_CFGR_PLLXTPRE_PREDIV1 | RCC_CFGR_PLLMULL2);
// Enable PLL 
    RCC->CR |= RCC_CR_PLLON;
// Wait till PLL is ready
    while((RCC->CR & RCC_CR_PLLRDY) == 0) {
    }
// Select PLL as system clock source
    RCC->CFGR &= (uint32_t)((uint32_t)~(RCC_CFGR_SW));
    RCC->CFGR |= (uint32_t)RCC_CFGR_SW_PLL;    
// Wait till PLL is used as system clock source
    while ((RCC->CFGR & (uint32_t)RCC_CFGR_SWS) != (uint32_t)RCC_CFGR_SWS_PLL) {
    }
  } else { 
// РљРІР°СЂС† РЅРµ Р·Р°РїСѓСЃС‚РёР»СЃСЏ - РїСЂРѕСЃС‚Рѕ РѕСЃС‚Р°РІР»СЏРµРј HSI
    FLASH->ACR = FLASH_ACR_PRFTBE; // | FLASH_ACR_LATENCY; 
    RCC->CFGR |= (uint32_t)RCC_CFGR_HPRE_DIV1;  // HCLK = SYSCLK
    RCC->CFGR |= (uint32_t)RCC_CFGR_PPRE_DIV1;  // PCLK = HCLK
  }  
}*/

/*void ConfigProtect(void) {
  ConfigWDT();
  if (FLASH_OB_GetRDP()!=SET) {
    uint32_t a;
    FLASH_Unlock();
    FLASH_OB_Unlock();
    while (FLASH_OB_Erase()!=FLASH_COMPLETE);
// vvv 
    while (FLASH_OB_RDPConfig(OB_RDP_Level_1)!=FLASH_COMPLETE);     
    while (FLASH_OB_UserConfig(OB_IWDG_HW,OB_STOP_NoRST,OB_STDBY_NoRST)!=FLASH_COMPLETE);
// ^^^
    FLASH_OB_Launch();
    FLASH_OB_Lock();
    FLASH_Lock();
  }
}
*/
