from vm import VMInstruction, VMInstructions

def compile():
    instructions = []
    instr = None
    acc = 0
    estado = 0
    func = None
    arg = None
    args = []
    token = parser.read()
    while token is not None:
        if estado == 0:
            if token.type == TokenTypes.COMMENT:
                estado = 100
            elif token.type == TokenTypes.ID:
                if token.value in labels.keys():
                    token = parser.seek(parser.position() + 2)
                    continue
                elif token.value == "CALL":
                    instr = VMInstruction(VMInstructions.CALL, 0, {"line": token.line, "char": token.char, "index": token.index})
                    estado = 2
                elif token.value == "JMP":
                    estado = 3
                elif token.value == "JMZ":
                    if acc > 0:
                        estado = 3
                elif token.value == "JZ":
                    if acc == 0:
                        estado = 3
                elif token.value == "RTN":
                    return acc
                elif token.value == "LD":
                    parser.next()
                    acc = __get()
                    estado = 100
                elif token.value == "LDN":
                    parser.next()
                    acc = not __get()
                    estado = 100
                elif token.value == "AND":
                    parser.next()
                    acc = acc and __get()
                    estado = 100
                elif token.value == "ANDN":
                    parser.next()
                    acc = acc and not __get()
                    estado = 100
                elif token.value == "OR":
                    parser.next()
                    acc = acc or __get()
                    estado = 100
                elif token.value == "ORN":
                    parser.next()
                    acc = acc or not __get()
                    estado = 100
                elif token.value == "EQ":
                    parser.next()
                    acc = 1 if acc == __get() else 0
                    estado = 100
                elif token.value == "NE":
                    parser.next()
                    acc = 1 if acc != __get() else 0
                    estado = 100
                elif token.value == "GT":
                    parser.next()
                    acc = 1 if acc > __get() else 0
                    estado = 100
                elif token.value == "LT":
                    parser.next()
                    acc = 1 if acc < __get() else 0
                    estado = 100
                elif token.value == "GE":
                    parser.next()
                    acc = 1 if acc >= __get() else 0
                    estado = 100
                elif token.value == "LE":
                    parser.next()
                    acc = 1 if acc <= __get() else 0
                    estado = 100
                elif token.value == "ST":
                    parser.next()
                    __set(acc)
                    estado = 100
                elif token.value == "SMZ":
                    if acc is not None and acc > 0:
                        estado = 4
                    else:
                        parser.next()
        elif estado == 1: #sem função
            estado = 0
        elif estado == 2:
            position = parser.position()
            __jmp(token)
            execute()
            token = parser.seek(position)
            estado = 100
        elif estado == 3:
            token = __jmp(token)
            estado = 0
        elif estado == 4:
            if token.type == TokenTypes.ID:
                if token.value in functions.keys():
                    func = functions[token.value]
                    estado = 5
                else:
                    raise Exception("Erro de sintaxe. A função '%s' não existe. Linha: %i" % (token.value, token.line + 1))
            else:
                raise Exception("Erro de sintaxe. Esperado identificador para a função. Linha: %i" % (token.line + 1))
        elif estado == 5:
            if token.value == "(":
                arg = None
                args = []
                estado = 6
            else:
                raise Exception("Erro de sintaxe. Esperado '(' na função '%s'. Linha: %i" % (func, token.line + 1))
        elif estado == 6:
            if token.value == ")":
                if arg is not None: args.append(arg)
                func(*args)
                estado = 100
            elif token.value == ",":
                if arg is not None: args.append(arg)
                arg = None
            else:
                arg = __get()
        elif estado == 100:
            if token.value == ")":
                parser.next()
                return acc
            if not token.value == '\n':
                raise Exception("Erro de sintaxe. Esperado fim de linha. Linha: %i" % (token.line + 1))
            #token = parser.next()
            estado = 0
        
        token = parser.next()
    
    return acc



if __name__ == "__main__":
    
    import sys
    import os
    import shutil
    
    if len(sys.argv) < 2:        
        print ("Erro: Necessário nome do arquivo IL.")
        exit(1)
        
    try:
        shutil.copy(sys.argv[1], os.path.dirname(__file__) + "/logic/broker.il")
    except Exception as ex:
        print("Erro ao tentar copiar o arquivo.\n", str(ex))
    
    print("Arquivo copiado!")
    exit(0)