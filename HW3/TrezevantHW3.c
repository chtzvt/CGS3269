#include <stdio.h>
#include <stdlib.h>

#define MAXPROGRAMSIZE 100
#define MAXMEMORYSIZE 10
#define DUMP_PROG_RAW 0

typedef struct {
        int opCode, deviceOrAddress;
} Instruction;

Instruction progMem[MAXPROGRAMSIZE];
int datMem[MAXMEMORYSIZE];
Instruction insReg;

typedef struct {
        int numCycles, numFetchex, totalProgSz;
        int numLoad, numStore, numAdd, numSub, numIn, numOut, numJump, numSkipz;
} Stats;

Stats emu_Stats;

int acc;
int pc = 10;
int memAddReg;
int memDatReg;
int dec = 0;

void emu_main(void);
void emu_printState(void);
void emu_printProgMemDump(void);
void emu_loadParseFile(char *filename);
void emu_initStats();
void emu_printStats();

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
        if(argc != 2) {
                print_usage(argv[0]);
                exit(1);
        }

        printf("[META] Max program size: %d instructions\n", MAXPROGRAMSIZE);
        printf("[META] Data memory locations: %d\n", MAXMEMORYSIZE);

        emu_loadParseFile(argv[1]);

        emu_printProgMemDump();
        emu_printState();

        emu_initStats();

        emu_main();

        emu_printProgMemDump();
        emu_printState();

        emu_printStats();
}

void print_usage(char *progname){
  
        printf("Tiny Machine Emulator (c) 2020 Charlton Trezevant\n\n");
  
        printf("Usage: %s FILENAME\n\n", progname);

        printf("Max program size: %d instructions\n", MAXPROGRAMSIZE);
        printf("Data memory locations: %d\n\n", MAXMEMORYSIZE);

        printf("Tiny Machine ISA\n");
        printf("\topcode | name   arg\n");
        printf("\t-------+-----------------\n");
        printf("\t1      | LOAD   data addr\n");
        printf("\t2      | ADD    data addr\n");
        printf("\t3      | STORE  data addr\n");
        printf("\t4      | SUB    data addr\n");
        printf("\t5      | IN     data addr\n");
        printf("\t6      | OUT    data addr\n");
        printf("\t7      | END\n");
        printf("\t8      | JMP    inst addr\n");
        printf("\t9      | SKIPZ\n\n");

        printf("Example Program\n");
        printf("\t5 5\n");
        printf("\t6 7\n");
        printf("\t3 0\n");
        printf("\t5 5\n");
        printf("\t6 7\n");
        printf("\t3 1\n");
        printf("\t1 0\n");
        printf("\t4 1\n");
        printf("\t3 0\n");
        printf("\t6 7\n");
        printf("\t1 1\n");
        printf("\t6 7\n");
        printf("\t7 0\n");
}

/////////////// EMULATOR INTERNAL ///////////////

void emu_main(){
        printf("[EMU] RUNNING\n");

        while(dec != 7) {
                switch(dec) {
                case 0:
                        cpu_fetchEx();
                        emu_Stats.numFetchex++;
                        break;
                case 1:
                        op_load();
                        emu_Stats.numLoad++;
                        break;
                case 2:
                        op_add();
                        emu_Stats.numAdd++;
                        break;
                case 3:
                        op_store();
                        emu_Stats.numStore++;
                        break;
                case 4:
                        op_sub();
                        emu_Stats.numSub++;
                        break;
                case 5:
                        op_input();
                        emu_Stats.numIn++;
                        break;
                case 6:
                        op_output();
                        emu_Stats.numOut++;
                        break;
                case 8:
                        op_jump();
                        emu_Stats.numJump++;
                        break;
                case 9:
                        op_skipz();
                        emu_Stats.numSkipz++;
                        break;
                }

                emu_printState();
                emu_Stats.numCycles++;
        }

        printf("[EMU] HALTED\n");
}

void emu_loadParseFile(char *filename){
        FILE *file;
        int op, arg, i = 0;

        printf("[FILE] Loading input file \'%s\'\n", filename);

        file = fopen(filename, "r");

        if(!file) {
                printf("[FILE] ERROR: Couldn't access \'%s\'\n", filename);
                exit(1);
        }

        while (!feof(file)) {
                if(i >= MAXPROGRAMSIZE) {
                        printf("[FILE] ERROR: Input exceeds maximum program length of %d\n", MAXPROGRAMSIZE);
                        exit(1);
                }

                fscanf (file, "%d %d", &op, &arg);

                if(op < 0 || op > 9) {
                        printf("[FILE] ERROR: Couldn't parse invalid opcode: %d\n", op);
                        exit(1);
                }

                progMem[pc+i].opCode=op;
                progMem[pc+i].deviceOrAddress=arg;

                i++;
        }

        emu_Stats.totalProgSz = i;

        fclose(file);

        printf("[FILE] Assembled %d instruction(s)\n", emu_Stats.totalProgSz);
}

void emu_printState() {
        int i;

        printf("[MEMORY] [mar:%3d] [dec:%2d] [mdr:%3d] [pc:%3d] [acc:%4d] [dm: ", memAddReg, dec, memDatReg, pc, acc);

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

        printf("[PRGMEM] pc | op      device/addr\n[PRGMEM] ---+-------------------\n");

        for (i = 0; i < MAXPROGRAMSIZE; i++) {

                if(progMem[i].opCode == 0)
                        continue;

                if(DUMP_PROG_RAW == 1)
                        printf("[PRGMEM] %d | %d      %d", i, progMem[i].opCode, progMem[i].deviceOrAddress);
                else
                        switch(progMem[i].opCode) {
                        case 1:
                                printf("[PRGMEM] %d | LOAD    %d", i, progMem[i].deviceOrAddress);
                                break;
                        case 2:
                                printf("[PRGMEM] %d | ADD     %d", i, progMem[i].deviceOrAddress);
                                break;
                        case 3:
                                printf("[PRGMEM] %d | STORE   %d", i, progMem[i].deviceOrAddress);
                                break;
                        case 4:
                                printf("[PRGMEM] %d | SUB     %d", i, progMem[i].deviceOrAddress);
                                break;
                        case 5:
                                printf("[PRGMEM] %d | INPUT   %d", i, progMem[i].deviceOrAddress);
                                break;
                        case 6:
                                printf("[PRGMEM] %d | OUTPUT  %d", i, progMem[i].deviceOrAddress);
                                break;
                        case 7:
                                printf("[PRGMEM] %d | END     %d", i, progMem[i].deviceOrAddress);
                                break;
                        case 8:
                                printf("[PRGMEM] %d | JUMP    %d", i, progMem[i].deviceOrAddress);
                                break;
                        case 9:
                                printf("[PRGMEM] %d | SKIPZ   %d", i, progMem[i].deviceOrAddress);
                                break;
                        default:
                                printf("[PRGMEM] %d | ??\?(%d) %d", i, progMem[i].opCode, progMem[i].deviceOrAddress);
                                break;
                        }

                if(i == pc)
                        printf("    <-pc-\n");
                else
                        printf("\n");
        }
}

void emu_initStats() {
        emu_Stats.numCycles = 0;
        emu_Stats.numFetchex = 0;
        emu_Stats.totalProgSz = 0;

        emu_Stats.numLoad = 0;
        emu_Stats.numStore = 0;
        emu_Stats.numAdd = 0;
        emu_Stats.numSub = 0;
        emu_Stats.numIn = 0;
        emu_Stats.numOut = 0;
        emu_Stats.numJump = 0;
        emu_Stats.numSkipz = 0;

        printf("[STATS] Init\n");
}

void emu_printStats(){
        printf("[STATS]\tinstructions loaded: %d\n", emu_Stats.totalProgSz);

        int numOpsExecuted = emu_Stats.numCycles - emu_Stats.numFetchex;

        printf("[STATS]\tinstructions executed: %d\n", numOpsExecuted);
        printf("[STATS]\t\tload:   %d (%.2f pct)\n", emu_Stats.numLoad, (float)(emu_Stats.numLoad * 100) / numOpsExecuted);
        printf("[STATS]\t\tstore:  %d (%.2f pct)\n", emu_Stats.numStore, (float)(emu_Stats.numStore * 100) / numOpsExecuted);
        printf("[STATS]\t\tadd:    %d (%.2f pct)\n", emu_Stats.numAdd, (float)(emu_Stats.numAdd * 100) / numOpsExecuted);
        printf("[STATS]\t\tsub:    %d (%.2f pct)\n", emu_Stats.numSub, (float)(emu_Stats.numSub * 100) / numOpsExecuted);
        printf("[STATS]\t\tinput:  %d (%.2f pct)\n", emu_Stats.numIn, (float)(emu_Stats.numIn * 100) / numOpsExecuted);
        printf("[STATS]\t\toutput: %d (%.2f pct)\n", emu_Stats.numOut, (float)(emu_Stats.numOut * 100) / numOpsExecuted);
        printf("[STATS]\t\tjump:   %d (%.2f pct)\n", emu_Stats.numJump, (float)(emu_Stats.numJump * 100) / numOpsExecuted);
        printf("[STATS]\t\tskipz:  %d (%.2f pct)\n", emu_Stats.numSkipz, (float)(emu_Stats.numSkipz * 100) / numOpsExecuted);
}

/////////////// FETCH/EXECUTE ///////////////

void cpu_fetchEx() {
        memAddReg = pc;
        ++pc;

        memDatReg = progMem[memAddReg].deviceOrAddress;
        insReg.deviceOrAddress = memDatReg;

        memDatReg = progMem[memAddReg].opCode;
        insReg.opCode = memDatReg;

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
        acc = acc - memDatReg;
        dec = 0;
}

void op_sub(){
        memAddReg = insReg.deviceOrAddress;
        memDatReg = datMem[memAddReg];
        acc = acc - memDatReg;
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
        if (acc == 0)
            ++pc;
        dec = 0;
}

void op_jump(){
        pc = insReg.deviceOrAddress;
        dec = 0;
}
