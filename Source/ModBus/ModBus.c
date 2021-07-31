#include "ModBus.h"
#include "fpuart1.h"
#include "mytimer.h"

// Данные
uint8_t  mb_adr; 	// Адрес ModBus
uint8_t  lstMode; // Режим прослушивания 1-включен; 0-выключен

// Диагностика
uint16_t dReg;	// Диагностический регистр
uint16_t CNT0B;	// Счетчик всех обнаруженных пакетов
uint16_t CNT0C;	// Счетчик обнаруженных пакетов с ошибочным CRC
uint16_t CNT0D;	// Счетчик ответных пакетов с исключениями
uint16_t CNT0E;	// Счетчик пакетов, адресуемых нам
uint16_t CNT0F;	// Счетчик пакетов, адресуемых нам на которые не было никаких ответов
uint16_t CNT10;	// Счетчик пакетов, адресуемых нам с исключениями NAK (05)
uint16_t CNT11;	// Счетчик пакетов, адресуемых нам с исключениями Slave Device Busy (06)
uint16_t CNT12;	// Счетчик переполнений

// Параметры принятого пакета
uint8_t rdpkg[257]; // Принятый пакет данных
uint8_t ecd;        // Наличие ошибок (0 - без ошибок)
uint16_t rdpkgpos=0;// Позиция приема
uint32_t gbtimeout; // Межбайтный таймаут

// Параметры передаваемого пакета
uint8_t trpkg[257]; // Передаваемые данные
uint16_t trpkgpos;  // Позиция передачи

// Прототипы внутренних функций
void MB_ResetCnt(void);
uint16_t MB_Parse(void);
// Функции интерфейса устройства
// Возвращают значение по адресу регистра
uint16_t GetHoldingRegisters(uint16_t);
uint16_t GetInputRegisters(uint16_t);
// Возвращает 0 если значение установлено
uint16_t SetHoldingRegisters(uint16_t,uint16_t);


// Настраиваем UART и таймер переключения прием/передача
void InitModBus(uint16_t mbaddr,uint16_t mbconn) {
  uint8_t speed;
  uint8_t odd;
  odd=mbconn&3;
  speed=(mbconn>>3)&0x1F;
  if (speed>4) {
    speed=0;
    odd=0;
  } else {
    if (mbconn&0x04) odd|=0x80;
  }
  switch (speed) {
    case (0):gbtimeout=InitFPU1(9600,odd);break;
    case (1):gbtimeout=InitFPU1(19200,odd);break;
    case (2):gbtimeout=InitFPU1(38400,odd);break;
    case (3):gbtimeout=InitFPU1(57600,odd);break;
    case (4):gbtimeout=InitFPU1(115200,odd);break;
  }
  gbtimeout*=3;
  mb_adr=mbaddr;
  if ((mb_adr<1)||(mb_adr>247)) mb_adr=200;  // Адрес по умолчанию
  lstMode=0;
  MB_ResetCnt();
  
}

uint8_t hsv=0;

// Основная процедура работы с ModBus
// Получает все байты, которые успели придти с её предыдущего запуска, и следит за таймаутом
// Если таймаут больше времени передачи 1.5 байта - пакет принят полностью, вызывается процедура 
// расшифровки и формаирования ответа, после чего ответ передается в последовательный порт
// Процедуру надо вызывать в основном цикле, очень желательно - не реже чем 1 раз в течении времени приема байта
uint32_t ModBusProc(void) {
  hsv=0;
  while (numbyteFPU1()) {
    if (rdpkgpos<256) rdpkg[rdpkgpos++]=getbyteFPU1(); 
      else getbyteFPU1();
    TIM3->CCR3=TIM3->CNT+gbtimeout;
    TIM3->SR&=~TIM_CC3IE;
  }
  if ((TIM3->SR&TIM_CC3IE)&&(rdpkgpos)) {
    TIM3->SR&=~TIM_CC3IE;
    if (geterrFPU1()) ecd=1;
// Запускаем обработку пакета
    MB_Parse();    
    if (trpkgpos) {
      sendpkgFPU1(trpkg,trpkgpos);
    }
    rdpkgpos=0;
    ecd=0;
  }
  return hsv;
}

// Информация об устройстве
const char SlaveID[] = "PGU STARTBOARD";
const uint8_t szSlaveID = 21;

/////////////////////////
// Сброс счетчиков ModBus
void MB_ResetCnt(void) {
  CNT0B=CNT0C=CNT0D=CNT0E=CNT0F=CNT10=CNT11=CNT12=0;
  dReg=0;
}

// Получить перевернутый int
uint16_t GetSWB(uint8_t * pos) {
  uint16_t res;
  res=*pos;
  res<<=8;
  res|=(*(pos+1));
  return res;
}

//////////////////////////////////////////
// Расчет контрольной суммы пакетов ModBus
uint16_t CalcCRC(uint8_t * curbuf,int16_t cnt) {
  uint16_t crc;
  uint16_t n,m;
  crc=0xffff;
  for (n=0;n<cnt;n++) {
    crc^=curbuf[n];
    for (m=0;m<8;m++) {
      if (crc&1) {
        crc>>=1;
        crc^=0xA001;
      } else {
        crc>>=1;
      }
    }
  }
  return crc;
}

void AddToTrPkg(uint16_t data) {
  trpkg[trpkgpos++]=(uint8_t)(data>>8); 
	trpkg[trpkgpos++]=(uint8_t)(data);
}

//////////////////////////////////////////////////////////////////////////
// Функция разбора пакета ModBus (u.rsbuf) и формирования ответа (u.trbuf)
// Возвращает количество байт в ответе (u.trcnt)
// Возвращает 0 если пакет разобран и требуется ответ
//            1 если пакет разобран но адрес не наш или ответа не требуется
//           10 если пакет ошибочен
uint16_t MB_Parse(void) {
	uint16_t	crc;			// контрольная сумма
  uint16_t  rcrc;     // Полученная crc
	uint8_t		cmd;			// код команды
	uint16_t	sub_cmd;	// подфункция
	uint16_t	st_adr;		// начальный адрес
	uint16_t	value;		// значение
	uint16_t	quantity;	// количество регистров
	uint8_t		byte_cnt;	// количество байт данных
	uint8_t		ex;				// код исключения
	
	uint8_t		answ;			// тип ответа: 0-как в запросе; 1-вручную; 2-без ответа
	uint16_t 	i, tmp; 

	CNT0B++;
	ex=0;
	answ=1;

// Убираем ответный пакет
	trpkgpos = 0;
  
 // Счетчики ошибок
	if (ecd) {
		CNT12++;
		return 10;
	}
	if (rdpkgpos<4) {
		CNT0C++;
		return 10;
	}
// Проверка контрольной суммы
	crc = CalcCRC(rdpkg ,rdpkgpos-2);
  rcrc=rdpkg[rdpkgpos-1];
  rcrc<<=8;
  rcrc|=rdpkg[rdpkgpos-2];
	if (crc!=rcrc) {
		CNT0C++;
		return 10;
	}
// Проверка адреса (0-ой или mb_adr)
	if ((rdpkg[0]!=mb_adr) && (rdpkg[0]!=0)) return 1;
//
	CNT0E++;
// Режим прослушивания
	if(lstMode)	{
		if(rdpkg[1]!=0x08 || rdpkg[2]!=0x00 || rdpkg[3]!=0x01) {
			CNT0F++;
			return 1;
		}
	}
// Разбираем пакет
	cmd = rdpkg[1];
	trpkgpos = 0;
  trpkg[trpkgpos++]=rdpkg[0];
  trpkg[trpkgpos++]=rdpkg[1];

  switch(cmd)	{
// Функция 0x03 - чтение регистров хранения
// Функция 0x04 - чтение входных регистров
    case 3:
    case 4:
      if(rdpkgpos!=8) {   // Ошибка приема, неверный размер кадра
        CNT0C++;
        trpkgpos=0;
        return 1;
      }
      st_adr = GetSWB(rdpkg+2);
      quantity = GetSWB(rdpkg+4);
      if ((quantity<1)||(quantity>125)) {
        ex=3;
        break;
      }
      if ((st_adr+quantity)<st_adr) {
        ex=2;
        break;
      }
      byte_cnt=0;
      trpkgpos++;
      for (i=st_adr; i<(st_adr + quantity); i++) {
        if(cmd==3) tmp=GetHoldingRegisters(i);
          else tmp=GetInputRegisters(i);
        AddToTrPkg(tmp);
        byte_cnt+=2;
      }
      trpkg[2]=byte_cnt;
      break;
// Функция 0x06 - запись одного регистра
    case 6:				
      if(rdpkgpos!=8) { // Неверный размер кадра
        CNT0C++;
        trpkgpos = 0;
        return 1;
      }
      st_adr = GetSWB(rdpkg+2);
      value = GetSWB(rdpkg+4);
      if (SetHoldingRegisters(st_adr,value)) ex=4;
      answ=0;
      break;
// Функция 0x08 - диагностика SerialLine
    case 8:
      if(rdpkg[2]!=0) {
        ex=0x01;
        break;
      }
      trpkg[trpkgpos++]=rdpkg[2];
      trpkg[trpkgpos++]=rdpkg[3];
      sub_cmd = GetSWB(rdpkg+2);
      if ((sub_cmd==0x00) || (sub_cmd==0x01) || (sub_cmd==0x03)) {
        answ=0;
        switch(sub_cmd) { // Подфункции
          case 0x00: break;  // Возврат полученных данных
					case 0x01:  // Сброс настроек связи
            if (rdpkgpos!=8) {  // Ошибка приема, неверный размер кадра
              CNT0C++;
              trpkgpos = 0;
              return 1;
            }
            if(rdpkg[5]||((rdpkg[4]!=0x00) && (rdpkg[4]!=0xFF))) {
              ex=3;
              break;
            }
            if(rdpkg[4]) MB_ResetCnt();
            lstMode=0;
            break;
          case 0x03:  // Изменить ASCII символ десятичный разделитель
            if(rdpkgpos!=8) { // Ошибка приема, неверный размер кадра
              CNT0C++;
              trpkgpos = 0;
              return 1;
            }
            if(rdpkg[5]) {
              ex=3;
              break;
            }	
            break;					
        }
      } else {  
        if (rdpkgpos!=8) {  // Ошибка приема, неверный размер кадра
          CNT0C++;
          trpkgpos = 0;
          return 1;
        }
        if (!(rdpkg[4]|rdpkg[5])) { // 2 байта нуля в данных
          ex=3;
          break;
        }
        switch(sub_cmd) {  // Подфункции
          case 0x02: // Прочитать регистр диагностики
            trpkg[trpkgpos++]=(uint8_t)(dReg>>8); 
            trpkg[trpkgpos++]=(uint8_t)(dReg); 
            break; 
          case 0x04: // Войти в режим прослушивания
            lstMode=1;
            answ=2;
            break;				
          case 0x0A: // Сброс счетчиков 
            MB_ResetCnt();
            answ=0;
            break;
          case 0x0B:      // CNT0B
            AddToTrPkg(CNT0B);
            break;				
          case 0x0C:      // CNT0C
            AddToTrPkg(CNT0C);
            break;				
          case 0x0D:      // CNT0D
            AddToTrPkg(CNT0D);
            break;				
          case 0x0E:      // CNT0E
            AddToTrPkg(CNT0E);
            break;				
          case 0x0F:      // CNT0F
            AddToTrPkg(CNT0F);
            break;				
          case 0x10:      // CNT10
            AddToTrPkg(CNT10);
            break;				
          case 0x11:      // CNT11
            AddToTrPkg(CNT11);
            break;				
          case 0x12:      // CNT12
            AddToTrPkg(CNT0B);
            break;				
          case 0x14:      // Очистить счетчик переполнений и флаг
            CNT12=0;
            answ=0;
            break;
          default:
            ex=0x01;
            break;
        }
      }
      break;
// Функция 0x10 - записать несколько регистров
    case 0x10:    // Ошибка приема, неверный размер кадра
      if (rdpkgpos<9)	{
        CNT0C++;
        trpkgpos = 0;
        return 1;
      }
      st_adr = GetSWB(rdpkg+2);
      quantity = GetSWB(rdpkg+4);				
      byte_cnt = rdpkg[6];
      if ((quantity<1)||(quantity>123)||(byte_cnt!=(2*quantity))||((2*quantity+9)!=rdpkgpos)) {
        ex=3;
        break;
      }
      if((st_adr+quantity)<st_adr) {
        ex=2;
        break;
      }	
      for(i=st_adr; i<(st_adr+quantity);i++) {
        value=GetSWB(rdpkg+7+(i-st_adr)*2);
				if(SetHoldingRegisters(i,value)) ex=4;
      }
      for(i=2; i<6;i++) trpkg[trpkgpos++]=rdpkg[i];
      break;			
    case 0x11:  // Функция 0x11 - чтение информации об устройстве
      if(rdpkgpos!=4) {  // Ошибка приема, неверный размер кадра
        CNT0C++;
        trpkgpos = 0;
        return 1;
      }
      // SlaveID
      trpkg[trpkgpos++]=szSlaveID+1;
      for(i=0;i<szSlaveID;i++) trpkg[trpkgpos++]=SlaveID[i];
      trpkg[trpkgpos++]=0xFF;  // Устройство работает
      break;
    default:
      ex=0x01;
      break;
  }
// Формирование ответного пакета
  if(answ!=2)	{ // Отвечаем
		if(ex) {    // Обработка исключений
      trpkgpos=0;
      trpkg[trpkgpos++]=rdpkg[0];      
      trpkg[trpkgpos++]=rdpkg[1]|0x80;
      trpkg[trpkgpos++]=ex;
    } else if(answ==0) {  // Отвечаем тем же, что и в запросе
      for(trpkgpos=0;trpkgpos<(rdpkgpos-2);trpkgpos++)
        trpkg[trpkgpos]=rdpkg[trpkgpos];
    }
// Подсчет CRC
    crc = CalcCRC(trpkg,trpkgpos);
    trpkg[trpkgpos++]=(uint8_t)crc;
    trpkg[trpkgpos++]=(uint8_t)(crc>>8);
  }	else CNT0F++;
  return 0;
}

// Поиск параметра в таблице
MbStruct * MBFindParam(uint16_t adr) {
  MbStruct * fs;
  uint16_t len;   // Длина в словах модбас
  uint32_t typ;
  fs=mbs;
  while (fs->addr) {
    if (!((adr<fs->reg)||(adr>(fs->reg+4)))) {
// Имеет смысл рыться дальше
      typ=fs->flag&MBT_FMASK;
      switch (typ) {
        case (MBT_INT8):
        case (MBT_INT16):len=1;break;
        case (MBT_INT32):
        case (MBT_FLOAT):len=2;break;        
      }
      if ((adr>=fs->reg)&&(adr<(fs->reg+len))) {
        return fs;
      }
    }
    fs++;
  }
  return 0;
}

///////////////////////////////////////////////////////////////
// Функции интерфейса устройства

uint16_t GetInputRegisters(uint16_t a) {
  return GetHoldingRegisters(a); 
}

// Возвращают значение по адресу регистра
uint16_t GetHoldingRegisters(uint16_t adr) {
  MbStruct * fps;
  uint32_t type;
  uint8_t * rga;
  uint16_t rez;
  fps=MBFindParam(adr);
  if (fps) {
    type=fps->flag&MBT_FMASK;
    rga=(uint8_t*)(fps->addr);
    switch (type) {
      case (MBT_INT8):return *(rga);
      case (MBT_INT16):
        rez=*(rga+1);
        rez<<=8;
        rez|=*(rga);
        return rez;          
      case (MBT_INT32):
      case (MBT_FLOAT):
        if (adr==fps->reg) {  // Младшая половина
          rez=*(rga+1);
          rez<<=8;
          rez|=*(rga);
        } else {              // Старшая половина
          rez=*(rga+3);
          rez<<=8;
          rez|=*(rga+2);          
        }
        return rez;
    }
  }
  return 0;
}

// Возвращает 0 если значение установлено
uint16_t SetHoldingRegisters(uint16_t adr,uint16_t zn) {
  MbStruct * fps;
  uint32_t type;
  uint8_t * rga;
  fps=MBFindParam(adr);
  if (fps) {
    if (!(fps->flag&MBT_WRITEMASK)) return 1;  // Если не пишется - и говорить не о чем
    type=MBCanWrite();
    if (((fps->flag&MBT_AWRITIBLE)&&(!(type&1)))||((fps->flag&MBT_SWRITIBLE)&&(!(type&2)))) return 1;
    if (fps->flag&MBT_EESAVE) hsv=1;     // Сохранять EEPROM
    type=fps->flag&MBT_FMASK;
    rga=(uint8_t*)(fps->addr);
    switch (type) {
      case (MBT_INT8):*(rga)=zn;return 0;
      case (MBT_INT16):
        *(rga)=(uint8_t)zn;
        zn>>=8;
        *(rga+1)=zn;
        return 0;          
      case (MBT_INT32):
      case (MBT_FLOAT):
        if (adr==fps->reg) {  // Младшая половина
          *(rga+0)=(uint8_t)zn;
          zn>>=8;
          *(rga+1)=zn;
        } else {              // Старшая половина
          *(rga+2)=(uint8_t)zn;
          zn>>=8;
          *(rga+3)=zn;
        }
        return 0;
    }    
  }
  return 1;
}
