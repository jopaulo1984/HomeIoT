
from http import client, server
from threading import Timer
import _thread
import time
import json
import misc
from tokensparser import TokensParser, Token, TokenTypes

from datetime import datetime

devices = {"192.168.1.87:30128": {"ip": "192.168.1.87", "port": 30128, "vars": None, "svars": ""}}

headers = {"Content-type": "application/x-www-form-urlencoded", "Accept": "text/plain"}

stop_server = False

my_server = {"status": "stoped", "count": 0}

last_assign = ""

verbose = True

def send(ip, port, cmd):
    #requisitando dados do device...
    #t0 = datetime.now()
    httpclient = client.HTTPConnection(ip, port) #("192.168.1.87", 30128)
    httpclient.request("POST", "/", cmd, headers)
    response = httpclient.getresponse()
    #if verbose: print ("Elapsed:", datetime.now() - t0)
    #tempo até aqui, ~2.23s
    if response.status != 200:
        if verbose: print ("Erro ao tentar requisitar dados do device.")
        out = None
    else:
        out = response.read().decode().strip()
    httpclient.close()
    
    if verbose: print ("[ device ] cmd:", cmd, "; response:", out)
    
    addr = ip + ":" + str(port)
    
    device = devices[addr]
    
    device["svars"] = out
    
    try:
        device["vars"] = json.loads(out)
    except:
        device["vars"] = None
        
    #print ("Elapsed:", datetime.now() - t0)
    
    return out

def send_to_device(device, cmd):
    return send(device["ip"], device["port"], "cmd=" + cmd)

def send_to_webserver(addr, dados):
    #conectando com o servidor...
    #t0 = datetime.now()
    httpsclient = client.HTTPSConnection("condominioparaty.com.br")
    #enviando dados para o servidor...
    httpsclient.request("POST", addr, dados, headers)
    #objeto response <- resposta do servidor
    response = httpsclient.getresponse()
    #if verbose: print ("Elapsed:", datetime.now() - t0)
    #tempo até aqui, ~0.8s
    out = response.status, response.reason, response.read().decode().strip()    
    httpsclient.close()
    if verbose: print ("[ webserver ] addr:", addr, "; data:", dados, "; response:", out)
    return out

def assign_to_webserver():
    #return
    global last_assign    
    device = send_to_device(devices["192.168.1.87:30128"], "GET INFO")
    if device == last_assign:
        return    
    last_assign = device
    if device is None:
        return    
    #assinando no servidor web...
    #device_address <- groupid/devicename
    #code, reason, content = send_to_webserver("/iot/assign.php", "token=pEka15G61&device=t0R27g6/BombaCasa25&properties=" + str(device["svars"]))
    #if code != 201:        
    #    if verbose: print("Não foi possível assinar o equipamento no servidor web.")    

def timerassignloop():
    while True:
        assign_to_webserver()
        time.sleep(2)

def get_cmds_from_webserver():
    #requisitando dados do servidor...
    code, reason, content = send_to_webserver("/iot/cmds.php", "token=pEka15G61&device=t0R27g6/BombaCasa25")
    if code != 202:
        if verbose: print ("Erro ao tentar requisitar dados do servidor web.")
        return
    try:
        cmdstack = json.loads(content)
        for cmd in cmdstack:
            send_to_device(cmd)
            _thread.start_new_thread(assign_to_webserver, ())
            send_to_webserver("/iot/pop.php", "token=pEka15G61&device=t0R27g6/BombaCasa25")
    except:
        pass

def timergetcmdloop():
    while True:    
        get_cmds_from_webserver()
        time.sleep(1)
        
def run_virtual_machine():
    
    import os
    
    #verificando e existe o arquivo IL
    if not os.path.isfile("logic/broker.il"):
        print("Arquivo broker.il não encontrado")
        return
    
    #declarando variáveis de controle
    labels = {} #rótulos    
    ci = 0 #contador de instrução
    acc = 0 #acumulador
    stack = [] #pilha
    parser = TokensParser() #analisador lexico
    
    #variáveis e funções de uso geral
    variables = {}
    functions = {"httpsend": misc.httpsend, "print": print}
    
    def __get_dev():
        estado = 2
        result = None
        addr = ""
        obj = None
        properr = False
        token = parser.read()
        while token is not None:
            if estado == 2:
                if token.value == "(":
                    estado = 3
                else:
                    raise Exception("Erro de sintaxe. Esperado $('address'). Linha: %i" % token.line + 1)
            elif estado == 3:
                if token.type == TokenTypes.STRING:
                    addr = token.value
                    estado = 4
                else:
                    raise Exception("Erro de sintaxe. Esperado string após $(. Linha: %i" % token.line + 1)
            elif estado == 4:
                if token.value == ")":
                    result = devices[addr]["vars"]
                    estado = 5
                else:
                    raise Exception("Erro de sintaxe. Esperado $('address'). Linha: %i" % (token.line + 1))
            elif estado == 5:
                if token.value == ".":
                    if not properr and result is None:
                        properr = True
                        print("Erro de sintaxe. Objeto nulo não tem propriedade. Linha: %i" % (token.line + 1))
                    estado = 6
                else:
                    parser.previus()
                    return result
            elif estado == 6:
                if token.type == TokenTypes.ID:
                    if  result is not None: 
                        if token.value in result.keys():
                            result = result[token.value]
                        else:
                            raise Exception("Erro de sintaxe. A propriedade '%s' não foi encontrada no objeto. Linha: %i" % (token.value, token.line + 1))
                else:
                    raise Exception("Erro de sintaxe. Esperado identificador, mas encontrado '%s'. Linha: %i" % (token.value, token.line + 1))
                estado = 5
            token = parser.next()
        
        return result if result is not None else 0
        
    def __get():
        
        token = parser.read()
            
        if token.type == TokenTypes.INT:
            return int(token.value)

        if token.type == TokenTypes.HEX:
            return int(token.value, 16)

        if token.type == TokenTypes.FLOAT:
            return float(token.value)

        if token.value == "$":
            parser.next()
            return __get_dev()

        if token.type == TokenTypes.ID:
            if token.value in variables.keys():
                return variables[token.value]
            else:
                print("Variável '%s' não encontrada. Linha: %i" % (token.value, token.line))
        
        if token.type == TokenTypes.STRING:
            return token.value
        
        if token.value == "(":
            parser.next()
            return execute()
        
        return 0
    
    def __set_dev(acc, gentokens, index):
        pass   
    
    def __set(acc):        
        token = parser.read()
        if token.type == TokenTypes.ID:
            variables[token.value] = acc
        elif token.value == "$":
            token = __set_dev(acc)
    
    def __jmp(token):
        if not token.value in labels.keys():
            print("O rótulo '%s' não foi encontrado. Linha: %i" % (token.value, token.line))
            return
        return parser.seek(labels[token.value])
    
    def execute():        
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
    
    if verbose: print("Iniciando máquina virtual...")
    
    #carregando código IL e gerando tokens
    with open("logic/broker.il", "r") as f:
        parser.execute(f.read(), True)
    
    #obtendo rótulos
    estado = 0
    label = None
    token = parser.read()    
    while token is not None:
        if estado == 0:
            if token.type == TokenTypes.ID:
                label = token.value
                estado = 1
            token = parser.next()
        elif estado == 1:
            if token.value == ":":
                labels[label] = parser.position() + 1
                token = parser.next()
            else:
                estado = 0
    #
    
    if verbose: print("Máquina virtual rodando.")
    
    parser.reset()
    
    print("Executando máquina virtual...")
    
    execute()
    
    print("Execução encerrada.")
        
def run(v=True):
    global verbose
    verbose = v
    #_thread.start_new_thread(timerassignloop, ())
    #_thread.start_new_thread(timergetcmdloop, ())
    _thread.start_new_thread(run_virtual_machine, ())
    if verbose: print ("Servidor rodando!")
    my_server["status"] = "run"
    while True:
        if input() == 'q':
            break
#

if __name__ == "__main__":
    run(False)
    exit(0)