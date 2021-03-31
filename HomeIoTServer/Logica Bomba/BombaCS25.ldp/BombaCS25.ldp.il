; init

; Diagrama3

                LD    1
                ST    flbaixo


mainloop:

                CALL   Diagrama1

                JMP   mainloop

Diagrama1:
                LD    1
                SMZ   send("192.168.1.87", 30128, "GET INFO")

                LD    $("192.168.1.87:30128").status.manual
                ST    bmb

                LD    $("192.168.1.87:30128").measures.flowr
                ST    flowr

                LD    flowr
                GE    7.0
                JZ    CMP_1_0_FALSE
                LD    bmb
                JMP   CMP_1_0_EXIT
CMP_1_0_FALSE:
                LD    0
CMP_1_0_EXIT:
                ST    flowrok

                LD    bmb
                ST    NODE_11
                AND   flowrok
                SMZ   print("Flow:", flowr)

                LD    NODE_11
                ANDN  flowrok
                ST    NODE_14
                ANDN  flbaixo
                SMZ   print("Fluxo baixo.")

                LD    NODE_14
                ST    flbaixo

                RTN

; functions

Diagrama2:
                RTN
