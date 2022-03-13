/* Criado por: João Paulo
 * Versão: 0.1
 * Abstrai a comunicação entre o mestre e o medidor de energia DTS238-7.
 */

#ifndef DTS2387_H
#define DTS2387_H

#include "HardwareSerial.h"

#define PHASE_A 0
#define PHASE_B 1
#define PHASE_C 2
#define DTS2387_REGS_COUNT 43
#define DTS2387_MAX_BUF 50
#define RS485_RD LOW
#define RS485_WR HIGH
#define MODBUS_FUNC_HOLD_REGS 3
#define MODBUS_FUNC_INPUT_REGS 4
#define NOT_ERROR 0
#define CRC_ERROR 1
#define TIME_OUT 2
#define DTS2387_BAUDRATE_1 9600
#define DTS2387_BAUDRATE_2 4800
#define DTS2387_BAUDRATE_3 2400
#define DTS2387_BAUDRATE_4 1200

class DTS2387
{
private:
    char _buffer[DTS2387_MAX_BUF];
    char _sbuffer[8];
    int _serial;
    char _addr = 1;
    char _cntrl_pin = 25;
    unsigned int _baudrate = 9600;
    unsigned int get_crc(char buf[], int buflen);
    void serial_write(char buf[], int count);
    unsigned int _query(char send_buf[], int send_count, char rcv_buffer[], int rcv_count);
    void serial_clear_read_buffer();
    char serial_read();
    bool serial_available();
    float getTotalRegister(char reg, float factor);
    float getPhaseRegister(char reg, float factor);
public:
    DTS2387(int uart_nr, char controlpin);
    DTS2387(int uart_nr, char controlpin, char addr);
    DTS2387(int uart_nr, char controlpin, char addr, unsigned long baudrate);
    ~DTS2387();
    void begin();
    void setRelayOn(bool value);
    bool relayIsOn();
    unsigned int load();
    unsigned int query(char reg, char count, char buffer[]);
    float queryDWord(char reg, float factor);
    float queryWord(char reg, float factor);
    float getVoltage(unsigned int phase);
    float getCurrent(unsigned int phase);
    float getEnergy();
    float getReverseEnergy();
    float getForwordEnergy();
    float getFrequency();
    float getActivePower();
    float getActivePower(unsigned int phase);
    float getReactivePower();
    float getReactivePower(unsigned int phase);
    float getApparentPower();
    float getApparentPower(unsigned int phase);
    float getPowerFactor();
    float getPowerFactor(unsigned int phase);
};

#endif