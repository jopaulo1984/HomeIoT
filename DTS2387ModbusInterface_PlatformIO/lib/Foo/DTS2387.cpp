#include "DTS2387.h"

String to_hex(char _byte)
{
    String result = String(_byte, HEX);
    if (result.length() == 1)
    {
        result = "0" + result;
    }
    return result;
}

void delay_ms(unsigned long interval)
{
    unsigned long last = millis();
    while (millis() - last < interval);
}

void DTS2387::serial_clear_read_buffer()
{
    while (Serial2.available()) {Serial2.read();};
}

unsigned int DTS2387::get_crc(char buf[], int buflen)
{
    unsigned int crc = 0xFFFF;

    for (int pos = 0; pos < buflen; pos++)
    {
        crc ^= (unsigned int)buf[pos]; // XOR byte into least sig. byte of crc

        for (int i = 8; i != 0; i--)
        { // Loop over each bit
            if ((crc & 0x0001) != 0)
            {              // If the LSB is set
                crc >>= 1; // Shift right and XOR 0xA001
                crc ^= 0xA001;
            }
            else           // Else LSB is not set
                crc >>= 1; // Just shift right
        }
    }
    
    return ((crc >> 8) | (crc << 8)) & 0xFFFF; // invertendo bytes para hi e lo
}

bool DTS2387::serial_available()
{
    return Serial2.available();   
}

void DTS2387::serial_write(char buf[], int count)
{
    Serial2.write(buf, count);
    Serial2.flush();
}

unsigned int DTS2387::_query(char send_buf[], int send_count, char rcv_buffer[], int rcv_count)
{
    unsigned long last;
    int i = 0;

    /*Serial.print("register = "); Serial.print(to_hex(send_buf[3])); Serial.println("; ");

    Serial.print("send_buf =");
    for (i = 0; i < send_count; i++)
    {
        Serial.print(" "); Serial.print(to_hex(send_buf[i]));
    }
    Serial.println("");*/

    //limpando buffer da serial
    serial_clear_read_buffer();

    //define RS-485 para escrita
    digitalWrite(_cntrl_pin, RS485_WR);
    //envia o buffer para o DTS238-7
    Serial2.write(send_buf, send_count);
    //aguarda o envio de todos os bytes
    Serial2.flush();
    //retorna RS-485 para somente leitura
    digitalWrite(_cntrl_pin, RS485_RD);
    
    delay_ms(200);

    //verifica resposta
    int len = 3;
    char read = 0x00;
    i = 0;
    last = millis();
    while (i < len)
    {
        if (Serial2.available())
        {
            read = Serial2.read();
            rcv_buffer[i] = read;
            if (i == 2)
            {
                //tamanho do buffer = addr + func + count + len(data) + hicrc + locrc
                len = 3 + read + 2;
                if (len > rcv_count)
                {
                    len = rcv_count;
                }
            }
            //last = millis();
            i++;
        }
        else if (millis() - last < 200)
        {
            Serial.println("TIME OUT");
            return TIME_OUT;
        }
    }

    serial_clear_read_buffer(); //lendo bytes ignorados
    
    int crcindex = 3 + rcv_buffer[2];
    uint crc = (rcv_buffer[crcindex] << 8) | rcv_buffer[crcindex + 1];
    unsigned int crc2 = get_crc(rcv_buffer, crcindex);
    
    if (crc - crc2 != 0)
    {
        return CRC_ERROR;
    }

    return NOT_ERROR;
}

char DTS2387::serial_read()
{
    return Serial2.read();
}

DTS2387::DTS2387(int uart_nr, char controlpin)
{
    DTS2387(uart_nr, controlpin, 1, 9600);
}

DTS2387::DTS2387(int uart_nr, char controlpin, char addr)
{
    DTS2387(uart_nr, controlpin, addr, 9600);
}

DTS2387::DTS2387(int uart_nr, char controlpin, char addr, unsigned long baudrate)
{
    _serial = uart_nr;
    _cntrl_pin = controlpin;
    _addr = addr;
    _baudrate = baudrate;
}

DTS2387::~DTS2387() {}

void DTS2387::begin()
{
    Serial2.begin(_baudrate);
    pinMode(_cntrl_pin, OUTPUT);
    digitalWrite(_cntrl_pin, RS485_RD);
}

unsigned int DTS2387::load() {
    return query(0x80, 25, _buffer);
}

void DTS2387::setRelayOn(bool value)
{
    char buf[11] = {0x00, 0x10, 0x00, 0x1A, 0x00, 0x01, 0x02, 0x00, 0x00, 0xA9, 0xFA};
    if (value) {
        buf[8] = 0x01;
        buf[9] = 0x68;
        buf[10] = 0x3A;
    }
    digitalWrite(_cntrl_pin, RS485_WR);
    Serial2.write(buf, 11);
    Serial2.flush();
    digitalWrite(_cntrl_pin, RS485_RD);
}

bool DTS2387::relayIsOn()
{
/*    
Meter ID	Function code	Register address	Data number	  Check code (CRC)
 1byte	         1byte	         2byte	            2byte	     2byte
*/
    char snd_buf[8] = {0x00, 0x03, 0x00, 0x1A, 0x00, 0x01, 0x00, 0x00};
    char rcv_buf[16];
    uint crc = get_crc(snd_buf, 6);
    snd_buf[6] = crc >> 8;
    snd_buf[7] = crc & 0xff;
    if (_query(snd_buf, 8, rcv_buf, 16) == NOT_ERROR)
    {
        return rcv_buf[5] == 0x01;
    }
    return true;
}

unsigned int DTS2387::query(char reg, char count, char buffer[])
{
    unsigned int crc = 0;
    int max_buf = 5 + count * 2;

    _sbuffer[0] = _addr;                 //ID do escravo
    _sbuffer[1] = MODBUS_FUNC_HOLD_REGS; //código da função
    _sbuffer[2] = 0x00;                  //registrador hi
    _sbuffer[3] = reg;                   //registrador lo
    _sbuffer[4] = 0x00;                  //data count hi
    _sbuffer[5] = count;                 //data count lo

    crc = get_crc(_sbuffer, 6);

    _sbuffer[6] = crc >> 8;   //crc hi
    _sbuffer[7] = crc & 0xff; //crc lo

    return _query(_sbuffer, 8, buffer, max_buf);

}

float DTS2387::queryDWord(char reg, float factor)
{
    char sbuf[] = {_addr, 0x03, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00};
    char rbuf[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    uint crc = get_crc(sbuf, 6);
    sbuf[6] = crc >> 8;
    sbuf[7] = crc & 0xff;
    uint result = _query(sbuf, 8, rbuf, 9);
    if (result == NOT_ERROR)
    {
        return ((rbuf[3] << 24) | (rbuf[4] << 16) | (rbuf[5] << 8) | rbuf[6]) * factor;
    }
    return 0;
}

float DTS2387::queryWord(char reg, float factor)
{
    char buf[8];
    if (query(reg, 1, buf) == NOT_ERROR)
    {
        return ((buf[3] << 8) | buf[4]) * factor;
    }
    return 0;
}

float DTS2387::getEnergy()
{
    return queryDWord(0x00, 0.01);
}

float DTS2387::getReverseEnergy()
{
    return queryDWord(0x08, 0.01);
}

float DTS2387::getForwordEnergy()
{
    return queryDWord(0x0A, 0.01);
}

float DTS2387::getFrequency()
{
    return queryWord(0x11, 0.01);
}

float DTS2387::getTotalRegister(char index, float factor)
{
    index += 3;
    return ((_buffer[index] << 24) | (_buffer[index+1] << 16) | (_buffer[index+2] << 8) | _buffer[index+3]) * factor;
}

float DTS2387::getPhaseRegister(char index, float factor)
{
    index += 3;
    return ((_buffer[index] << 8) | _buffer[index+1]) * factor;
}

float DTS2387::getVoltage(unsigned int phase)
{
    return getPhaseRegister(phase * 2, 0.1); //0 2 4
}

float DTS2387::getCurrent(unsigned int phase)
{
    return getPhaseRegister(6 + phase * 2, 0.01); //6 8 10
}

float DTS2387::getActivePower()
{
    return getTotalRegister(12, 0.001); //12 13 14 15
}

float DTS2387::getActivePower(unsigned int phase)
{
    return getPhaseRegister(16 + (phase * 2), 0.001); //16 18 20
}

float DTS2387::getReactivePower()
{
    return getTotalRegister(22, 0.001); //22 23 24 25
}

float DTS2387::getReactivePower(unsigned int phase)
{
    return getPhaseRegister(26 + (phase * 2), 0.001); //26 28 30
}

float DTS2387::getApparentPower()
{
    return getTotalRegister(32, 0.001); //32 33 34 35
}

float DTS2387::getApparentPower(unsigned int phase)
{
    return getPhaseRegister(36 + (phase * 2), 0.001); //36 38 40
}

float DTS2387::getPowerFactor()
{
    return getPhaseRegister(42, 0.001); //42 43
}

float DTS2387::getPowerFactor(unsigned int phase)
{
    return getPhaseRegister(44 + (phase * 2), 0.001); // 44 46 48
}
