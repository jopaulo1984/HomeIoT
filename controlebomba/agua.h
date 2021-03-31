

#ifndef AGUA_H
#define AGUA_H

#include <Arduino.h>
#include <EEPROM.h>

#define PMP_OUT 2
#define FLW_IN 3
#define CIS_LO 4
#define CIS_HI 5
#define BTN_ON 8
#define LED 13

//status pump
#define OK 0
#define NOFLOW 1
#define LOCKED 2

void setLed(bool value);
bool getLed();

class Cistern {
public:

  void init();
  bool isHi();
  bool isLo();

  uint8_t getLevel();
  
};

class Pump {
private:
  uint16_t volumeCount  = 0;
  uint16_t volumeL      = 0;
  uint16_t volumeM3     = 0;
  uint16_t vcount       = 0;
  uint16_t lcount       = 0;
  uint16_t TLOCK        = 36000; // x100ms
  uint16_t TVCOUNT      = 600; // x100ms
  uint8_t  estado       = 0;
  float    vazao        = 0;
  float    vacc         = 0; // acumulador do volume
  uint8_t  last_pulse   = 0;
  uint8_t  count_pulse  = 0;
  
public:
  void init();
  void doClock100ms();
  void incVolume();
  void resetVolume();
  void resetVolumeCount();
  void saveVolume();
  void doUpdate();
  void reset();
  void save();
  void setTLock(int t);    // x100ms -> tempo da bomba bloqueada.
  void setTPriming(int t); // x100ms -> tempo de escorva.
  
  bool isOn();
  bool locked();
  float getFlowRate();
  uint8_t getState();
  uint16_t getVolumeCount();
  uint16_t getVolumeL();
  uint16_t getVolumeM3();
  uint16_t getTimeLocked();
  uint16_t getTLock();
  uint16_t getTPriming();

  void setOn(bool value);
  
};

class AbastAgua {
private:  
  bool manual = false;
  
public:
  Pump pump;
  Cistern caixa;
  
  void init();
  void doUpdate1ms();
  void doUpdate100ms();
  void doUpdate1s();
  void reset();
  void printJSONMeasures(bool ln = true);
  void printJSONStatus(bool ln = true);
  void printJSONInfos();
  
  bool isBtnOnPress();
  String getJSON();
  
  void setModeManual(bool value);
  bool isModeManual();
  
};

#endif
