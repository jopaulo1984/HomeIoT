#include "agua.h"

void setLed(bool value)  {
  digitalWrite(LED, value);  
}

bool getLed()  {
  return digitalRead(LED);
}

int getEEPROMInt16(int addr) {
  return (EEPROM.read(addr + 1) << 8) | EEPROM.read(addr);
}

void saveEEPROMInt16(int addr, int value) {
  EEPROM.write(addr    , value &  0xff);
  EEPROM.write(addr + 1, value >> 8   ); 
}

//Cistern class
void Cistern::init() {
  pinMode(CIS_HI, INPUT);
  pinMode(CIS_LO, INPUT);
}

uint8_t Cistern::getLevel() {
  uint8_t acc = 0;
  if(digitalRead(CIS_LO)>0) {
    acc++;
  }
  if(digitalRead(CIS_HI)>0) {
    acc++;
  }
  return acc; //digitalRead(CIS_HI) + digitalRead(CIS_LO);
}

bool Cistern::isHi() {
  return getLevel() == 2;
}

bool Cistern::isLo() {
  return getLevel() == 0;
}

//Pump class
void Pump::init() {
  setOn(false);
  pinMode(PMP_OUT, OUTPUT);
  pinMode(FLW_IN, INPUT);
  volumeL     = getEEPROMInt16(0);
  volumeM3    = getEEPROMInt16(2);
  TLOCK       = getEEPROMInt16(4);
  TVCOUNT     = getEEPROMInt16(6);
  volumeCount = getEEPROMInt16(8);
  digitalWrite(PMP_OUT, EEPROM.read(10) > 0);
}

void Pump::doClock100ms() {

  /*
  Flow Range: 100L/H-/1800H-L/H
  =====================
  Flow(L/H)  Frezq.(Hz)
  =========  ==========
  120        16
  240        32.5
  360        49.3
  480        65.5
  600        82
  720        90.2
  */

  // 1 pulso / 0.1 s = 10Hz = 1 * 10
  float fr = count_pulse * 10;
  count_pulse = 0;
  
  // calculo da vazao em L/H
  if (fr == 0) {
    vazao = 0;
  } else {
    vazao = ((600 - 120) / (82 - 16)) * fr + 3.64;
  }
  
  // transformando para L/min
  vazao = vazao / 60;

  float dv = (vazao / 60) * 0.1; //l = l/s * s

  vacc += dv;
  
  if (vacc >= 1) {
    vacc = 0.0;
    if (volumeL == 999) {
      volumeM3++;
      volumeL = 0;
    }else{
      volumeL++;
      volumeCount++;
    }
    save();
  }
  
  switch (estado) {
    case 0:
      if (isOn() && (vazao < 5.0)) {
        vcount = 0;
        estado = 1;
      }
      break;
    case 1:
      if (++vcount > TVCOUNT) {
        lcount = 0;
        estado = 2;
        setLed(0);
      } else if (vazao >= 5.0) {
        reset();
      }
      break;
    case 2:
      if (++lcount > TLOCK) {
        reset();
      }
      break;
  }
  
}

void Pump::resetVolumeCount() {
  volumeCount = 0;
  saveEEPROMInt16(8, volumeCount);
}

void Pump::resetVolume() {
  volumeCount = 0;
  volumeL = 0;
  volumeM3 = 0;
  //vazPulsos = 0;
  //pulsAnt = 0;
  vazao = 0.0;
  //haPulsoVazao = false;
  count_pulse = 0;
}

void Pump::doUpdate() {

  bool now_pulse = digitalRead(FLW_IN);

  // se não há alteração de pulso, retorna
  if (now_pulse == last_pulse) {
    return;
  }

  // verifica pulso na subida
  if (now_pulse > last_pulse) {
    count_pulse++;
  }

  last_pulse = now_pulse;
  
}

void Pump::reset() {
  estado = 0;
}

void Pump::save() {
  saveVolume();
  saveEEPROMInt16(4, TLOCK   );
  saveEEPROMInt16(6, TVCOUNT );
}

void Pump::saveVolume() {
  saveEEPROMInt16(0, volumeL );
  saveEEPROMInt16(2, volumeM3);
  saveEEPROMInt16(8, volumeCount);
}

void Pump::setTLock(int t) {
  saveEEPROMInt16(4, t);
  TLOCK = getEEPROMInt16(4);
}

void Pump::setTPriming(int t) {
  saveEEPROMInt16(6, t);
  TVCOUNT = getEEPROMInt16(6);
}

bool Pump::isOn() {
  return digitalRead(PMP_OUT) > 0;
}

bool Pump::locked() {
  return estado==2;
}

uint8_t Pump::getState() {
  return estado;
}

float Pump::getFlowRate() {
  return vazao;
}

uint16_t Pump::getVolumeCount() {
  return volumeCount;
}

uint16_t Pump::getVolumeL() {
  return volumeL;
}

uint16_t Pump::getVolumeM3() {
  return volumeM3;
}

uint16_t Pump::getTimeLocked() {
  return lcount / 10;
}

uint16_t Pump::getTLock() {
  return TLOCK;
}

uint16_t Pump::getTPriming() {
  return TVCOUNT;
}

void Pump::setOn(bool value) {
  if(isOn()==value){return;}
  digitalWrite(PMP_OUT, value);
  EEPROM.write(10, value);
}

//AbastAgua class
void AbastAgua::init() {
  pinMode(BTN_ON, INPUT);
  caixa.init();
  pump.init();
}

void AbastAgua::doUpdate() {
  
  pump.doUpdate();

  bool result; // = !pump.locked(); // & (pump.isOn() | isBtnOnPress());

  if (!manual) {
    result = pump.isOn() | caixa.isLo() | isBtnOnPress();
    result &= !caixa.isHi();
  } else {
    result = pump.isOn() | isBtnOnPress();
  }
  
  result &= !pump.locked();

  pump.setOn(result);

  if (caixa.isHi() && (pump.getState() > 0)) {
    pump.reset();
  }

  if (pump.getState()!= LOCKED) {
    setLed(pump.isOn());
  }
  
  /*  
  if (!test) {
    bool acc = (pump.isOn() | cistern.isLo() | isBtnOnPress()); //!cistern.isHi();
    acc &= !pump.locked();
    acc &= !cistern.isHi(); //(pump.isOn() | cistern.isLo() | isBtnOnPress());
    pump.setOn(acc);
    if (cistern.isHi() && (pump.getState() > 0)) {
      pump.reset();
    }
  }
  
  if (pump.getState()!=LOCKED) {
    setLed(pump.isOn());
  }
  */
  
}

bool AbastAgua::isBtnOnPress() {
  return digitalRead(BTN_ON) > 0;
}

String AbastAgua::getJSON() {
  return "{""\"state""\":" + String(pump.getState(),DEC) + ",""\"level""\":" + String(caixa.getLevel(),DEC) + 
            ",""\"timelocked""\":" + String(pump.getTimeLocked(),DEC) +
            ",""\"volumecount""\":" + String(pump.getVolumeCount(),DEC) + 
            ",""\"volumel""\":" + String(pump.getVolumeL(),DEC) + 
            ",""\"volumem3""\":" + String(pump.getVolumeM3(),DEC) + 
            ",""\"flowrate""\":"+  String(pump.getFlowRate(), 1) +
            ",""\"pump""\":"+ String(pump.isOn()) +            
            ",""\"tlock""\":" + String(pump.getTLock(),DEC) +
            ",""\"tpriming""\":" + String(pump.getTPriming(),DEC) + 
            ",""\"manual""\":" + (isModeManual() ? "1" : "0") + "}";
}

void AbastAgua::reset() {
  pump.reset();
}

void AbastAgua::setModeManual(bool value) {
  manual = value;
  pump.setOn(false);
  pump.reset();
}

bool AbastAgua::isModeManual() {
  return manual;
}
