
class VMStates:
    STOPPED = 0
    RUNNING = 1

class VMInstructions:
    HLT = 0
    LOAD = 1
    LOADN = 101
    LOADDEV = 2
    LOADDEVN = 201
    STR = 3
    STRN = 301
    STRDEV = 4
    STRDEVN = 401
    CALL = 5
    CALLMZ = 6
    RTN = 7
    PUSH = 8
    POP = 9
    TON = 10
    TOF = 11
    CTU = 12
    CTD = 13
    CTUD = 14
    LT = 15
    GT = 16
    LE = 17
    GE = 18
    EQ = 19
    NE = 20
    JMP = 21
    JMZ = 22
    JZ = 23
    AND = 24
    OR = 25
    ANDN = 241
    ORN = 251
    NOT = 26
    XOR = 27
    ADD = 28
    SUB = 29
    MUL = 30
    DIV = 31
    MOD = 32
    PWR = 33
    
class VMInstrunction:
    
    def __init__(self, opcode, oper, source):
        pass

class VirtualMachine:
    
    def __init__(self):
        self.__timers = []
        self.__conters = []
        self.__ci = 0
        self.__stack = []
        self.__state = VMStates.STOPPED
        self.__instrus = []
        
    def __execute(self, instr):
        pass
        
    def __run(self):
        self.__state = VMStates.RUNNING
    
    def __stop(self):
        pass
        
    def run(self):
        if self.running(): return
        self.__run()
        
    def stop(self):
        pass
    
    def stopped(self):
        return self.__state == VMStates.STOPPED
    
    def running(self):
        return self.__state == VMStates.RUNNING
    
    def state(self):
        return self.__state

if __name__ == "__main__":
    vm = VirtualMachine()
    vm.run()
    print(vm.state())