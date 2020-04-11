#include <stdio.h>
#include <stdlib.h>

#define MAXPROGRAMSIZE 100
#define MAXMEMORYSIZE 10

typedef struct {
        int opCode, deviceOrAddress;
} Instruction;

Instruction progMem[MAXPROGRAMSIZE];
int datMem[MAXMEMORYSIZE];
Instruction insReg;

int acc;
int pc = 10;
int memAddReg;
int memDatReg;
int dec = 0;

void emu_printState(void);
void emu_printProgMemDump(void);
void emu_loadParseFile(char *filename);
void emu_repl(void);

void cpu_fetchEx(void);
void op_load(void);
void op_store(void);
void op_add(void);
void op_sub(void);
void op_input(void);
void op_output(void);
void op_skipz(void);
void op_jump(void);

/////////////// EMULATOR INTERNAL ///////////////

void emu_loadParseFile(char *filename){
  
  emu_printProgMemDump();
}

void emu_printState() {
        int i;

        printf("pc:%d acc:%d dec:%d mar:%d mdr:%d dm:[", pc, acc, dec, memAddReg, memDatReg);

        for (i = 0; i < MAXMEMORYSIZE; i++) {
                printf("%d", datMem[i]);

                if(i == MAXMEMORYSIZE - 1)
                        printf("]");
                else
                        printf(", ");
        }
}

void emu_printProgMemDump() {
        int i;

        printf("--- program memory dump ---\nindex | opcode device/address");

        for (i = 0; i < MAXPROGRAMSIZE; i++) {

                if(progMem[i].opCode == 0)
                        break;

                printf("%d | %d %d\n", i, progMem[i].opCode, progMem[i].deviceOrAddress);
        }

        printf("--- program memory dump ---\n");
}


/////////////// FETCH/EXECUTE CYCLE ///////////////

void cpu_fetchEx() {
  memAddReg=pc;
  ++pc;

  memDatReg = progMem[memAddReg].deviceOrAddress;
  insReg.deviceOrAddress=memDatReg;

  memDatReg = progMem[memAddReg].opCode;
  insReg.opCode=memDatReg;

  dec = insReg.opCode;
}

/////////////// INSTRUCTIONS ///////////////

void op_load(){
        memAddReg = insReg.deviceOrAddress;
        memDatReg = datMem[memAddReg];
        acc = memDatReg;
        dec = 0;
}

void op_store(){
        memAddReg = insReg.deviceOrAddress;
        memDatReg = acc;
        datMem[memAddReg] = memDatReg;
        dec = 0;
}

void op_add(){
        memAddReg = insReg.deviceOrAddress;
        memDatReg = datMem[memAddReg];
        acc = acc-memDatReg;
        dec = 0;
}

void op_sub(){
        memAddReg = insReg.deviceOrAddress;
        memDatReg = datMem[memAddReg];
        acc = acc-memDatReg;
        dec = 0;
}

void op_input(){
        printf("[INPUT] Enter a value: ");
        scanf("%d",&acc);
        dec = 0;
}

void op_output(){
        printf("[OUTPUT] Acc: %d\n",acc);
        dec = 0;
}

void op_skipz(){
        if (acc==0)
                ++pc;
        dec = 0;
}

void op_jump(){
        pc = insReg.deviceOrAddress;
        dec = 0;
}
