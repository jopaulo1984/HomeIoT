#ifndef ANLEX_H
#define ANLEX_H

#include <Arduino.h>

#define ID 0
#define NUM 1
#define STR 2
#define PNT 3

struct Token{
  String value;
  char type;
};

class AnLex {
private:
  String intext;
  uint16_t pos;

  char nextChar();
  char readChar();
  bool isLetter(char);
  bool isNum(char);
  bool isSpace(char);
  
public:
  AnLex(String text);
  AnLex();

  void reset();

  Token next();
  String getTokensValuesFromIndex(uint8_t index, uint8_t len);

  void setText(String text);
  
};

#endif
