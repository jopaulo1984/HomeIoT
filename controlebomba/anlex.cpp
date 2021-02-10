#include "anlex.h"

char AnLex::nextChar() {
  char c = readChar();
  if (c!=NULL) {
    pos++;
  }
  return c;
}

char AnLex::readChar() {
  if (pos < intext.length()) {
    return intext[pos];
  }
  return NULL;
}

bool AnLex::isLetter(char c) {
  return ('a' <= c and c <= 'z') or ('A' <= c and c <= 'Z') or (c=='_');
}

bool AnLex::isNum(char c) {
  return '0' <= c and c <= '9';
}

bool AnLex::isSpace(char c) {
  return c == ' ' or c == '\n' or c == '\r';
}

AnLex::AnLex(String text) {
  setText(text);
}

AnLex::AnLex() {
  setText("");
}

void AnLex::reset() {
  pos = 0;
}

Token AnLex::next() {
  char estado = 0;
  char c;
  Token temp;
  temp.value = "";
  temp.type = -1;
  do {
    c = readChar();
    switch (estado) {
      case 0:
        if (isSpace(c)) {
          nextChar();
          continue;
        } else if (isLetter(c)) {
          temp.type = ID;
          estado = 1;
        } else if (isNum(c)) {
          temp.type = NUM;
          estado = 2;
        } else if (c=='"') {
          temp.type = STR;
          estado = 3;
        } else {
          temp.type = PNT;
          estado = 100;
        }
        if (estado!=3) {
          temp.value.concat(nextChar());
        } else {
          nextChar();          
        }
        break;
      case 1:
        if (isLetter(c) or isNum(c)) {
          temp.value.concat(nextChar());
        } else {
          estado = 100;
        }
        break;
      case 2:
        if (!isNum(c)) {
            estado = 100;
        } else {
          temp.value.concat(nextChar());
        }
        break;
      case 3:
        if (c=='"') {
          estado = 100;
        } else {
          temp.value.concat(nextChar());
        }
        break;
      default:
        estado = 100;
    }
  } while (estado < 100 and c != NULL);
  return temp;
}

String AnLex::getTokensValuesFromIndex(uint8_t index, uint8_t len) {
  reset();
  uint8_t i = 0;
  Token tk;
  String res = "";
  do {
    tk = next();
  } while (tk.type>-1 and i++ != index);
  res.concat(tk.value);
  if (res=="") {
    return res;
  }
  i = 1;
  while (tk.type>-1 and i++ < len) {
    tk = next();
    res.concat(tk.value);
  }
  return res;
}

void AnLex::setText(String text) {
  intext = text;
  reset();
}
