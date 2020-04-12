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

void emu_main(void);
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

void print_usage(char *progname);

/////////////// ENTRY ///////////////

int main(int argc, char *argv[]) {
  if(argc != 2){
    print_usage(argv[0]);
    exit(1);
  }
  
  emu_loadParseFile(argv[1]);
  emu_main();
}

void print_usage(char *progname){
  printf("Usage: %s FILENAME\n", progname);
}

/////////////// EMULATOR INTERNAL ///////////////

void emu_main(){
        printf("[EMU] Started\n");
        
        emu_printState();
        emu_printProgMemDump();
        
        while(dec!=7) {
                emu_printState();
                
                switch(dec) {
                case 0:
                        cpu_fetchEx();
                        break;
                case 1:
                        op_load();
                        break;
                case 2:
                        op_add();
                        break;
                case 3:
                        op_store();
                        break;
                case 4:
                        op_sub();
                        break;
                case 5:
                        op_input();
                        break;
                case 6:
                        op_output();
                        break;
                case 8:
                        op_jump();
                        break;
                case 9:
                        op_skipz();
                        break;
                }
        }
        
        printf("[EMU] Halted.\n[EMU] FINAL STATE");
        
        emu_printState();
        emu_printProgMemDump();
}

void emu_loadParseFile(char *filename){
        FILE *file;
        char cursor;

        printf("[INFO] Loading input file \'%s\'\n", filename);

        file = fopen(filename, "r");

        if(!file) {
                printf("[ERROR] Couldn't access \'%s\'\n", filename);
                exit(1);
        }

        int i = 0;

        while ((cursor = getc(file)) != EOF) {
                cursor-='0';

                if(cursor<0||cursor>9) {
                        printf("[ERROR] Couldn't parse invalid input: %c\n", cursor);
                        continue;
                }

                if(i%2==0)
                        progMem[pc+i/2].opCode=cursor;
                else
                        progMem[pc+i/2].deviceOrAddress=cursor;

                i++;
        }

        fclose(file);

        printf("[INFO] Loading completed\n");
}

void emu_printState() {
        int i;

        printf("[EMU] Memory contents = pc:%d acc:%d dec:%d mar:%d mdr:%d dm:[", pc, acc, dec, memAddReg, memDatReg);

        for (i = 0; i < MAXMEMORYSIZE; i++) {
                printf("%d", datMem[i]);

                if(i == MAXMEMORYSIZE - 1)
                        printf("]\n");
                else
                        printf(", ");
        }
}

void emu_printProgMemDump() {
        int i;

        printf("[EMU] --- Program Memory Dump ---\n[EMU]\tindex | opcode device/address\n");

        for (i = 0; i < MAXPROGRAMSIZE; i++) {

                if(progMem[i].opCode == 0)
                        break;

                printf("[EMU]\t%d | %d %d\n", i, progMem[i].opCode, progMem[i].deviceOrAddress);
        }

        printf("[EMU] --- Program Memory Dump ---\n");
}


/////////////// FETCH/EXECUTE ///////////////

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
        printf("[OUTPUT] Accumulator = %d\n",acc);
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
