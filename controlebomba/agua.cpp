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
  // vazao(fr) [L/H] = ((600 - 120) / (82 - 16)) * (fr - 16) + 120 = 7.27 * fr + 3.68
  // vazao(fr) [L/m] = vazao(fr) / 60 = (7.27 * fr) / 60 + (3.68 / 60) = 0.12 * fr + 0.06
  if (fr == 0) {
    vazao = 0;
  } else {
    vazao = 0.121 * fr + 0.0611;
  }

  float dv = vazao * 0.00167; //(vazao / 60) * 0.1; //l = l/s * s

  vacc += dv;
  
  if (vacc >= 1) {
    vacc = 0;
    incVolume();
  }
  
  switch (estado) {
    case 0:
      if (isOn() && (vazao < 5)) {
        vcount = 0;
        estado = 1;
      }
      break;
    case 1:      
      if (++vcount > TVCOUNT) {
        lcount = 0;
        estado = 2;
        setOn(0);
        setLed(0);
      } else if (vazao >= 5) {
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

void Pump::incVolume() {
  volumeL++;
  volumeCount++;
  if (volumeL >= 1000) {
    volumeM3++;
    volumeL = 0;
  }
  save();
}

void Pump::resetVolumeCount() {
  volumeCount = 0;
  saveEEPROMInt16(8, volumeCount);
}

void Pump::resetVolume() {
  volumeCount = 0;
  volumeL = 0;
  volumeM3 = 0;
  vazao = 0.0;
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

void AbastAgua::doUpdate1ms() {
  pump.doUpdate();
}

void AbastAgua::doUpdate100ms() {
  
  bool result;

  pump.doClock100ms();

  if (pump.locked() && caixa.isHi()) {
    pump.reset();
    setLed(0);
    return;
  }
  
  if (!manual) {
    result = pump.isOn() | caixa.isLo() | isBtnOnPress();
    result &= !caixa.isHi();
  } else {
    result = pump.isOn() | isBtnOnPress();
  }
  
  result &= !pump.locked();

  pump.setOn(result);  

  if (pump.getState()!= LOCKED) {
    setLed(pump.isOn());
  }
  
}

void AbastAgua::doUpdate1s() {
  if (pump.locked()) {
    setLed(!getLed());
  }
}

bool AbastAgua::isBtnOnPress() {
  return digitalRead(BTN_ON) > 0;
}

/*String AbastAgua::getJSON() {
  return "{""\"state""\":" + String(pump.getState(), DEC) + 
  ",""\"level""\":" + String(caixa.getLevel(), DEC) + 
  ",""\"timelocked""\":" + String(pump.getTimeLocked(), DEC) +
  ",""\"volumecount""\":" + String(pump.getVolumeCount(), DEC) + 
  ",""\"volumel""\":" + String(pump.getVolumeL(), DEC) + 
  ",""\"volumem3""\":" + String(pump.getVolumeM3(), DEC) + 
  ",""\"flowrate""\":" + String(pump.getFlowRate(), 1) +
  ",""\"pump""\":"+ (pump.isOn() ? "1" : "0") +
  ",""\"tlock""\":" + String(pump.getTLock(), DEC) +
  ",""\"tpriming""\":" + String(pump.getTPriming(), DEC) +
  ",""\"manual""\":" + (isModeManual() ? "1" : "0") + "}";
}*/

void AbastAgua::printJSONStatus(bool ln = true) {
  Serial.print("{\"state\":" + String(pump.getState(), DEC) + 
  ",\"pump\":" + (pump.isOn() ? "1" : "0") + 
  ",\"manual\":" + (isModeManual() ? "1" : "0") +"}");
  if (ln) {
    Serial.println("");
  }
}

void AbastAgua::printJSONMeasures(bool ln = true) {
  Serial.print("{");
  Serial.print("\"cxlevel\":"); Serial.print(String(caixa.getLevel(), DEC));
  Serial.print(",\"vcount\":"); Serial.print(String(pump.getVolumeCount(), DEC));
  Serial.print(",\"flowr\":"); Serial.print(String(pump.getFlowRate(), 1));
  Serial.print("}");
  if (ln) {
    Serial.println("");
  }
}

void AbastAgua::printJSONInfos() {
  Serial.print("{\"counts\":{");
  Serial.print("\"volumel\":" + String(pump.getVolumeL(), DEC));
  Serial.print(",\"volumem3\":" + String(pump.getVolumeM3(), DEC));
  Serial.print(",\"tlock\":" + String(pump.getTLock(), DEC));
  Serial.print(",\"tpriming\":" + String(pump.getTPriming(), DEC));
  Serial.print("},\"measures\":");
  printJSONMeasures(false);
  Serial.print(",\"status\":");
  printJSONStatus(false);
  Serial.println("}");
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
