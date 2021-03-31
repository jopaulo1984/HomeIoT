r"""Parser para obter tokens

Sobre:
  - Autor: João Paulo
  - Versão: 1.0 - Testes
  - Homepage: http://jpcompweb.com.br
"""

import os
from os import path

class TokenTypes:
    KEY = 0
    ID = 1
    ADDRESS = 2
    FLOAT = 6
    CHAR = 7
    STRING = 8
    HEX = 9
    INT = 10
    ESPCAR = 11
    DATE = 12
    DATETIME = 13
    ARRAY = 14
    BOOL = 15
    COMMENT = 16
#
class Token:
    TYPES = {
        TokenTypes.KEY:"KEY",
        TokenTypes.ID:"ID",
        TokenTypes.ADDRESS:"ADDRESS",
        TokenTypes.FLOAT:"FLOAT",
        TokenTypes.CHAR:"CHAR",
        TokenTypes.STRING:"STRING",
        TokenTypes.HEX:"HEX",
        TokenTypes.INT:"INT",
        TokenTypes.ESPCAR:"ESPCAR",
        TokenTypes.BOOL:"BOOL",
        TokenTypes.ARRAY:"ARRAY",
        TokenTypes.COMMENT:"COMMENT"
    }
    
    def __init__(self, value, ttype, index, line, column):
        self.type = ttype
        self.value = value
        self.line = line
        self.column = column
        self.index = index

    def tuple(self):
        return self.value, self.type, self.line
    
    def __str__(self):
        return "<type=%s, value='%s'>" % (Token.TYPES[self.type], self.value)
#
class TokensParser:
    
    def __init__(self):
        self.__index = 0
        self.__tokens = []
        self.__erros = []

    def __getitem__(self, index):
        return self.__tokens[index]

    def __len__(self):
        return len(self.__tokens)

    def append(self, token):
        if token:
            self.__tokens.append(token)

    def read(self):        
        return self.seek(self.__index)

    def next(self):
        self.__index += 1
        return self.read()

    def previus(self):
        self.__index -= 1
        return self.read()

    def seek(self, index):
        if 0 <= index and index < len(self.__tokens):
            self.__index = index
            return self.__tokens[self.__index]
        elif index < 0:
            self.__index = -1
        else:
            self.__index = len(self.__tokens)
    
    def position(self):
        return self.__index
    
    def clear(self):
        self.__tokens.clear() # = []

    def items(self):
        return self.__tokens

    def reset(self):
        self.__index = 0

    def get_tokens_gen(self, code, retnewline=False, retspaces=False):
        
        texto = code
        
        def _next_(pos):
            if pos < len(texto):
                return texto[pos], pos + 1
            return None, pos + 1

        def _letra_(car):
            return (car != None) and (('a'<=car and car<='z') or ('A'<=car and car<='Z') or (car=='_'))

        def _num_(car):
            return (car != None) and (('0'<=car and car<='9'))

        def _hex_(car):
            return (car != None) and (_num_(car) or ('a'<=car and car<='f') or ('A'<=car and car<='F'))

        #self.__tokens.clear() # = TokenList()
        pos = 0
        row = 0
        col = 0
        column = 0
        estado = 0
        temp = ''
        msg = ''

        while (estado > -1) and (estado < 255):
            if estado == 0:
                car, pos = _next_(pos)
                if not car:
                    estado = 255
                elif (car == ' ' or car == '\t') and not retspaces:
                    pass
                elif car == ';':
                    estado = 20
                    column = col
                    temp = car                    
                elif _letra_(car):
                    estado = 1
                    column = col
                    temp = car
                elif car == '0':
                    estado = 2
                    column = col
                    temp = car
                elif _num_(car):
                    estado = 4
                    column = col
                    temp = car
                elif car in ('-','+'):
                    estado = 40
                    column = col
                    temp = car
                elif car == "'":
                    estado = 10
                elif car == '"':
                    estado = 12
                elif car in ('<','>'):
                    estado = 13
                    column = col
                    temp = car
                elif car == '[':
                    estado = 14
                    column = col
                    temp = car
                elif car == '#':
                    estado = 15
                    column = col
                    temp = car
                elif car == '\n':
                    if retnewline:
                        yield Token(car, TokenTypes.ESPCAR, pos - 1, row, col)
                    row += 1
                    col = -1
                    temp = ''
                else:
                    yield Token(car, TokenTypes.ESPCAR, pos - 1, row, col)
                    temp = ''

            elif estado == 1:
                car, pos = _next_(pos)
                if _letra_(car) or _num_(car):
                    temp += car
                else:
                    if temp.lower() in ('true','false'):
                        #self.__tokens.append(Token(temp , TokenTypes.BOOL, pos - len(temp) - 1, row, column))
                        yield Token(temp , TokenTypes.BOOL, pos - len(temp) - 1, row, column)
                    else:
                        #self.__tokens.append(Token(temp , TokenTypes.ID, pos - len(temp) - 1, row, column))
                        yield Token(temp , TokenTypes.ID, pos - len(temp) - 1, row, column)
                    temp = ''
                    estado = 0
                    pos -= 1
            elif estado == 2:
                car, pos = _next_(pos)
                if car == 'x' or car == 'X':
                    estado = 3
                    temp += car
                elif _num_(car):
                    estado = 4
                    temp += car
                elif _letra_(car):
                    msg = "Erro de sintaxe: identificador '{}' inválido".format(car)
                    estado = -1
                else:
                    estado = 0
                    #self.__tokens.append(Token(temp, TokenTypes.INT, pos - len(temp) - 1, row, col))
                    yield Token(temp, TokenTypes.INT, pos - len(temp) - 1, row, col)
                    temp = ''
                    pos -= 1
            elif estado == 3:
                car, pos = _next_(pos)
                if _hex_(car):
                    temp += car
                elif _letra_(car):
                    temp += car
                    msg = "Erro de sintaxe: identificador '{}' inválido".formatToken(temp)
                    estado = -1
                elif not temp  in  ['0x','0X']:
                    #self.__tokens.append(Token(temp, TokenTypes.HEX, pos - len(temp) - 1, row, col))
                    yield Token(temp, TokenTypes.HEX, pos - len(temp) - 1, row, col)
                    estado = 0
                    temp = ''
                    pos -= 1
                else:
                    msg = "Erro de sintaxe: identificador '{}' inválido".formatToken(temp)
                    estado = -1

            elif estado == 4:
                car, pos = _next_(pos)
                if _num_(car):
                    temp += car
                elif car == '.':
                    temp += car
                    estado = 5
                elif _letra_(car):
                    temp += car
                    msg = "Erro de sintaxe: identificador '{}' inválido".format(temp)
                    estado = -1
                else:
                    #self.__tokens.append(Token(temp, TokenTypes.INT, pos - len(temp) - 1, row, col))
                    yield Token(temp, TokenTypes.INT, pos - len(temp) - 1, row, col)
                    estado = 0
                    temp = ''
                    pos -= 1
            elif estado == 40:
                car, pos = _next_(pos)
                if car == '0':
                    estado = 2
                    temp += car
                    column = col
                elif _num_(car):
                    estado = 4
                    temp += car
                    column = col
                else:
                    #self.__tokens.append(Token(temp, TokenTypes.ESPCAR, pos - len(temp) - 1, row, col))
                    yield Token(temp, TokenTypes.ESPCAR, pos - len(temp) - 1, row, col)
                    estado = 0
                    temp = ''
                    pos -= 1                   
            elif estado == 5:
                car, pos = _next_(pos)
                if _num_(car):
                    temp += car
                elif _letra_(car):
                    temp += car
                    msg = "Erro de sintaxe: identificador '{}' inválido".format(temp)
                    estado = -1
                else:
                    #self.__tokens.append(Token(temp, TokenTypes.FLOAT, pos - len(temp) - 1, row, col))
                    yield Token(temp, TokenTypes.FLOAT, pos - len(temp) - 1, row, col)
                    estado = 0
                    temp = ''
                    pos -= 1
            elif estado == 10:
                car, pos = _next_(pos)
                if not car:
                    msg = "Erro de sintaxe: Esperado o caracter '."
                    estado = -1
                elif car == "'":
                    if len(temp) > 1:
                        #self.__tokens.append(Token(temp, TokenTypes.STRING, pos - len(temp) - 1, row, column))
                        yield Token(temp, TokenTypes.STRING, pos - len(temp) - 1, row, column)
                    else:
                        #self.__tokens.append(Token(temp, TokenTypes.CHAR, pos - len(temp) - 1, row, column))
                        yield Token(temp, TokenTypes.CHAR, pos - len(temp) - 1, row, column)
                    estado = 0
                    temp = ''
                else:
                    temp += car
            elif estado == 12:
                car, pos = _next_(pos)
                if not car:
                    msg = 'Erro de sintaxe: Esperado o caracter ".'
                    estado = -1
                elif car == '"':
                    #self.__tokens.append(Token(temp, TokenTypes.STRING, pos - len(temp) - 1, row, col))
                    yield Token(temp, TokenTypes.STRING, pos - len(temp) - 1, row, col)
                    estado = 0
                    temp = ''
                else:
                    temp += car
                    
            elif estado == 13:
                car, pos = _next_(pos)
                tk = temp
                if not car:
                    estado = 0
                elif car in ('>','=','<'):
                    tk = temp + car
                    estado = 0
                    temp = ''
                else:
                    pos -= 1
                    estado = 0
                #self.__tokens.append(Token(tk, TokenTypes.ESPCAR, pos - len(tk) - 1, row, column))
                yield Token(tk, TokenTypes.ESPCAR, pos - len(tk) - 1, row, column)
            elif estado == 14:
                car, pos = _next_(pos)
                if not car:
                    msg = 'Erro de sintaxe: Esperado o caracter ".'
                    estado = -1
                elif car == ']':
                    #self.__tokens.append(Token(temp + car, TokenTypes.ARRAY, pos - len(temp + car) - 1, row, col))
                    yield Token(temp + car, TokenTypes.ARRAY, pos - len(temp + car) - 1, row, col)
                    estado = 0
                    temp = ''
                else:
                    temp += car
            elif estado == 15:
                car, pos = _next_(pos)
                if not car:
                    msg = 'Erro de sintaxe: Esperado o caracter ".'
                    estado = -1
                elif car == '#':
                    #self.__tokens.append(Token(temp, TokenTypes.DATE, pos - len(temp) - 1, row, col))
                    yield Token(temp, TokenTypes.DATE, pos - len(temp) - 1, row, col)
                    estado = 0
                    temp = ''
                elif car == ' ':
                    estado = 16
                    temp += car
                else:
                    temp += car
                    
            elif estado == 16:
                car, pos = _next_(pos)
                if not car:
                    msg = 'Erro de sintaxe: Esperado o caracter ".'
                    estado = -1
                elif car == '#':
                    #self.__tokens.append(Token(temp, TokenTypes.DATETIME, pos - len(temp) - 1, row, col))
                    yield Token(temp, TokenTypes.DATETIME, pos - len(temp) - 1, row, col)
                    estado = 0
                    temp = ''
                else:
                    temp += car                
            
            elif estado == 20:
                car, pos = _next_(pos)
                if car is None or car == '\n':
                    #self.__tokens.append(Token(temp, TokenTypes.COMMENT, pos - len(temp) - 1, row, col))
                    yield Token(temp, TokenTypes.COMMENT, pos - len(temp) - 1, row, col)
                    if car is None:
                        estado = -1
                    else:
                        pos -= 1
                        estado = 0
                        temp = ''
                else:
                    temp += car
            
            elif estado == 30:
                
                car, pos = _next_(pos)
                
                if car in (' ','\t',None,'\n'):
                    
                    #self.__tokens.append(Token(temp, TokenTypes.ID, pos - len(temp) - 1, row, col))                    
                    yield Token(temp, TokenTypes.ID, pos - len(temp) - 1, row, col)
                    
                    if car is None:
                        estado = -1
                    else:
                        pos -= 1
                        estado = 0
                        temp = ''
                
                else:
                    
                    temp += car
            
            col+=1


            if estado < 0:
                self.__erros.append((row+1, col+1, msg))
        
        
        #self.__tokens.reset()
        
        #return self.__tokens
        
    def get_errors(self):
        return self.__erros
    
    def execute(self, code, retnewline=False, retspaces=False):
        self.__tokens = list(self.get_tokens_gen(code, retnewline, retspaces))
    
    def has_error(self):
        return len(self.__erros) > 0
#
