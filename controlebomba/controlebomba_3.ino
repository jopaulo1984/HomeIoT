
#include "agua.h"
#include "anlex.h"

#define TNCT1_VALUE 0xFFF0 //1ms
#define MAX_BUF 128
#define BUF_LEN 63

uint8_t tcount = 0;
uint8_t scount = 0;

AbastAgua abast;
AnLex anlex;

ISR(TIMER1_OVF_vect) {  //interrupção do TIMER1
  TCNT1 = TNCT1_VALUE;
  //
  if (++tcount==100) {
    //100ms
    abast.pump.doClock100ms();
    tcount = 0; 
    if (++scount==10) {
      //1s
      scount = 0;
      if (abast.pump.locked()) {
        setLed(!getLed());
      }
    }
  }
  abast.doUpdate();
}

void iniciaTimer(){  
  TCCR1A = 0;                        // confira timer para operação normal pinos OC1A e OC1B desconectados
  TCCR1B = 0;                        // limpa registrador
  TCCR1B |= (1<<CS10)|(1 << CS12);   // configura prescaler para 1024: CS12 = 1 e CS10 = 1 
  TCNT1 = TNCT1_VALUE;               // incia timer com valor para que estouro ocorra em 1 ms
                                     // TNCT1_VALUE = 65536-(16MHz/1024/1000Hz) = 65520 = 0xFFF0  
  TIMSK1 |= (1 << TOIE1);            // habilita a interrupção do TIMER1
}

String convertBoolToONOFFStr(bool value) {
  if (value) {
    return "ON";
  }
  return "OFF";
}

void printPump() {
  Serial.println("{""\"pump""\":""\"" + convertBoolToONOFFStr(abast.pump.isOn()) + """\"}");
}

void printMode() {
  Serial.println("{""\"manual""\":""\"" + convertBoolToONOFFStr(abast.isModeManual()) + """\"}");
}

void execute(String cmd) {
  uint8_t estado = 0;
  String temp = "";
  Token tk;
  anlex.setText(cmd);
  while (estado < 100) {
    tk = anlex.next();
    switch (estado) {
      case 0:
        if (tk.value=="SET") {
          estado = 1;
        } else if (tk.value=="GET") {
          estado = 50;
        } else if (tk.value=="HELP") {
          Serial.println("SET|GET INFO   - Retorna as informacoes do sistema.");
          Serial.println("SET|GET MANUAL - Obtem ou define o modo 'MANUAL' do sistema. Valores:  ON|OFF");
          Serial.println("SET|GET PUMP   - Obtem ou define o estado da bomba. Valores: ON|OFF");
          Serial.println("SET T_PRIMING  - Define o tempo de escorva em segundos.");
          Serial.println("SET T_LOCK     - Define o tempo de espera da bomba.");
          Serial.println("RESET STATE    - Redefine o estado da bomba.");
          Serial.println("RESET VOLUME   - Redefine o contador de volume da bomba.");
          Serial.println("HELP           - Obtem a ajuda.");
          estado = 100;
        } else if (tk.value=="RESET") {
          estado = 8;
        } else {
          estado = 101;
        }
        break;
      case 1:
        if (tk.value=="MANUAL") {
          estado = 4;
        } else if (tk.value=="PUMP") {
          estado = 5;
        } else if (tk.value=="T_PRIMING") {
          estado = 6;
        } else if (tk.value=="T_LOCK") {
          estado = 7;
        } else {
          estado = 101;
        }
        break;
      case 4:
        if (tk.value=="ON" or tk.value=="OFF") {
          abast.setModeManual(tk.value=="ON");
          printMode();
          estado = 100;
        } else {
          estado = 101;
        }
        break;
      case 5:
        if (tk.value=="ON" or tk.value=="OFF") {
          abast.pump.setOn(tk.value=="ON");
          printPump();
          estado = 100;
        } else {
          estado = 101;
        }
        break;
      case 50:
        if (tk.value=="INFO") {
          Serial.println(abast.getJSON());
          estado = 100;
        } else if (tk.value=="MANUAL") {
          printMode();
          estado = 100;
        } else if (tk.value=="PUMP") {
          printPump();
          estado = 100;
        } else {
          estado = 101;
        }
        break;
      case 6:
        if (tk.type==NUM) {
          abast.pump.setTPriming(tk.value.toInt());
          estado = 100;
          Serial.println(abast.getJSON());
        } else {
          estado = 101;
        }
        break;
      case 7:
        if (tk.type==NUM) {
          abast.pump.setTLock(tk.value.toInt());
          estado = 100;
          Serial.println(abast.getJSON());
        } else {
          estado = 101;
        }
        break;
      case 8:
        if (tk.value == "VOLUME") {
          abast.pump.resetVolumeCount();
        } else if (tk.value == "STATE") {          
          abast.reset();
        } else {
          estado = 101;
        }
        if (estado != 101) {
          Serial.println("{\"reset\":\"OK\"}");
          estado = 100;
        }
        break;
      default:
        estado = 100;
    }
  }
  if (estado==101) {
    Serial.println("Comando não reconhecido.");
  }
  anlex.setText("");
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  pinMode(LED, OUTPUT);
  abast.init();
  iniciaTimer();
}

String msg;
void loop() {
  if (Serial.available() > 0) {
    msg = "";
    while (Serial.available() > 0) { msg.concat(Serial.readString()); }
    execute(msg);
  }
}
