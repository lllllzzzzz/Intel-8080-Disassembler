/*
 *  Intel 8080 disassembler written in C.
 *  Author: Luke Zimmerer
 *
 *  Compile: gcc -std=gnu99
 */


#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>

#define OUTPUT_BUFFER_SIZE  64

int main(int argc, char* argv[])
{
    if (argc < 2 || argc > 4) {
        char tmp[260] = {0};
        for (int i = 0, j = strlen(argv[0]) - 1; argv[0][j] != '\\'; i++, j--) {
            tmp[i] = argv[0][j];
        }
        char exe_filename[260] = {0};
    //    char* exe_filename = (char*) malloc(strlen(tmp) * sizeof (char));
        for (int i = 0, j = strlen(tmp) - 1; j >= 0; i++, j--) {
            exe_filename[i] = tmp[j];
        }

        printf("Intel 8080 Disassembler\n2015 Luke Zimmerer\n\n\
Usage: %s [-ulhd] file...\n\n\
Arguments:\n\
  -u    print mnemonics in upper case\n\
  -l    print mnemonics in lower case\n\
  -h    print addresses in hexadecimal\n\
  -d    print addresses in decimal", exe_filename);

//        free(exe_filename);
        return EXIT_FAILURE;
    }

    typedef enum { HEXADECIMAL, DECIMAL, UPPER_CASE, LOWER_CASE, MNEMONICS_ONLY } arg;
    arg addr_base = HEXADECIMAL;
    arg letter_case = LOWER_CASE;
    int mnemonics_only = 0;

    /* Parse command-line arguments */
    for (int i = 0; i < argc; i++) {
        if(argv[i][0] == '-') {
            switch (toupper(argv[i][1])) {
                case 'U':
                    letter_case = UPPER_CASE;   /* upper case mnemonics */
                    break;
                case 'L':
                    letter_case = LOWER_CASE;   /* lower case mnemonics */
                    break;
                case 'H':
                    addr_base = HEXADECIMAL;    /* hexadecimal addresses */
                    break;
                case 'D':
                    addr_base = DECIMAL;        /* decimal addresses */
                    break;
                case 'M':
                    mnemonics_only = 1;         /* only print mnemonics */
                    break;
                default:
                    break;
            }
        }
    }

    FILE* input_file = fopen(argv[1], "rb");

    if (!input_file) {
        fprintf(stderr, "Error: cannot open input file");
        return EXIT_FAILURE;
    }

    fseek(input_file, 0, SEEK_END);
    long file_size = ftell(input_file);

    if (file_size == 0) {
        fclose(input_file);
        fprintf(stderr, "Error: input file is 0 bytes");
        return EXIT_FAILURE;
    }

    rewind(input_file);

    unsigned char *input_buf = (unsigned char*) malloc(file_size * sizeof (unsigned char));

    if (!input_buf) {
        fclose(input_file);
        fprintf(stderr, "Error: cannot allocate buffer");
        return EXIT_FAILURE;
    }

    /*size_t bytes_read = */fread(input_buf, file_size, sizeof (unsigned char), input_file);
//    printf("%d", bytes_read);
//    if (bytes_read != file_size) {
//        fclose(input_file);
//        free(buf);
//        fprintf(stderr, "Error: cannot read input file");
//        return EXIT_FAILURE;
//    }

    fclose(input_file);

    const char *mnemonics_upper_case[0x100] =
    {/*   0          1          2          3          4          5           6          7          8          9          A          B          C          D          E          F       */
     /*0*/"NOP",     "LXI B",   "STAX B",  "INX B",   "INR B",   "DCR B",    "MVI B",   "RLC",     "Illegal", "DAD B",   "LDAX B",  "DCX B",   "INR C",   "DCR C",   "MVI C",   "RRC",
     /*1*/"Illegal", "LXI D",   "STAX D",  "INX D",   "INR D",   "DCR D",    "MVI D",   "RAL",     "Illegal", "DAD D",   "LDAX D",  "DCX D",   "INR E",   "DCR E",   "MVI E",   "RAR",
     /*2*/"Illegal", "LXI H",   "SHLD",    "INX H",   "INR H",   "DCR H",    "MVI H",   "DAA",     "Illegal", "DAD H",   "LHLD",    "DCX H",   "INR L",   "DCR L",   "MVI L",   "CMA",
     /*3*/"Illegal", "LXI SP",  "STA",     "Illegal", "INR M",   "DCR M",    "MVI M",   "STC",     "Illegal", "DAD SP",  "LDA",     "DCX SP",  "INR A",   "DCR A",   "MVI A",   "CMC",
     /*4*/"MOV B,B", "MOV B,C", "MOV B,D", "MOV B,E", "MOV B,H", "MOV B,L",  "MOV B,M", "MOV B,A", "MOV C,B", "MOV C,C", "MOV C,D", "MOV C,E", "MOV C,H", "MOV C,L", "MOV C,M", "MOV C,A",
     /*5*/"MOV D,B", "MOV D,C", "MOV D,D", "MOV D,E", "MOV D,H", "MOV D,L",  "MOV D,M", "MOV D,A", "MOV E,B", "MOV E,C", "MOV E,D", "MOV E,E", "MOV E,H", "MOV E,L", "MOV E,M", "MOV E,A",
     /*6*/"MOV H,B", "MOV H,C", "MOV H,D", "MOV H,E", "MOV H,H", "MOV H,L",  "MOV H,M", "MOV H,A", "MOV L,B", "MOV L,C", "MOV L,D", "MOV L,E", "MOV L,H", "MOV L,L", "MOV L,M", "MOV L,A",
     /*7*/"MOV M,B", "MOV M,C", "MOV M,D", "MOV M,E", "MOV M,H", "MOV M,L",  "HLT",     "MOV M,A", "MOV A,B", "MOV A,C", "MOV A,D", "MOV A,E", "MOV A,H", "MOV A,L", "MOV A,M", "MOV A,A",
     /*8*/"ADD B",   "ADD C",   "ADD D",   "ADD E",   "ADD H",   "ADD L",    "ADD M",   "ADD A",   "ADC B",   "ADC C",   "ADC D",   "ADC E",   "ADC H",   "ADC L",   "ADC M",   "ADC A",
     /*9*/"SUB B",   "SUB C",   "SUB D",   "SUB E",   "SUB H",   "SUB L",    "SUB M",   "SBB A",   "SBB B",   "SBB C",   "SBB D",   "SBB E",   "SBB H",   "SBB L",   "SBB M",   "SBB A",
     /*A*/"ANA B",   "ANA C",   "ANA D",   "ANA E",   "ANA H",   "ANA L",    "ANA M",   "ANA A",   "XRA B",   "XRA C",   "XRA D",   "XRA E",   "XRA H",   "XRA L",   "XRA M",   "XRA A",
     /*B*/"ORA B",   "ORA C",   "ORA D",   "ORA E",   "ORA H",   "ORA L",    "ORA M",   "ORA A",   "CMP B",   "CMP C",   "CMP D",   "CMP E",   "CMP H",   "CMP L",   "CMP M",   "CMP A",
     /*C*/"RNZ",     "POP B",   "JNZ",     "JMP",     "CNZ",     "PUSH B",   "ADI",     "RST 0",   "RZ",      "RET",     "JZ",      "Illegal", "CZ",      "CALL",    "ACI",     "RST 1",
     /*D*/"RNC",     "POP D",   "JNC",     "OUT",     "CNC",     "PUSH D",   "SUI",     "RST 2",   "RC",      "Illegal", "JC",      "IN",      "CC",      "Illegal", "SBI",     "RST 3",
     /*E*/"RPO",     "POP H",   "JPO",     "XTHL",    "CPO",     "PUSH H",   "ANI",     "RST 4",   "RPE",     "PCHL",    "JPE",     "XCHG",    "CPE",     "Illegal", "XRI",     "RST 5",
     /*F*/"RP",      "POPPSW",  "JP",      "DI",      "CP",      "PUSH PSW", "ORI",     "RST 6",   "CM",      "SPHL",    "JM",      "EI",      "CM",      "Illegal", "CPI",     "RST 7"
    };

    const char *mnemonics_lower_case[0x100] =
    {/*   0          1          2          3          4          5           6          7          8          9          a          b          c          d          e          f       */
     /*0*/"nop",     "lxi b",   "stax b",  "inx b",   "inr b",   "dcr b",    "mvi b",   "rlc",     "illegal", "dad b",   "ldax b",  "dcx b",   "inr c",   "dcr c",   "mvi c",   "rrc",
     /*1*/"illegal", "lxi d",   "stax d",  "inx d",   "inr d",   "dcr d",    "mvi d",   "ral",     "illegal", "dad d",   "ldax d",  "dcx d",   "inr e",   "dcr e",   "mvi e",   "rar",
     /*2*/"illegal", "lxi h",   "shld",    "inx h",   "inr h",   "dcr h",    "mvi h",   "daa",     "illegal", "dad h",   "lhld",    "dcx h",   "inr l",   "dcr l",   "mvi l",   "cma",
     /*3*/"illegal", "lxi sp",  "sta",     "illegal", "inr m",   "dcr m",    "mvi m",   "stc",     "illegal", "dad sp",  "lda",     "dcx sp",  "inr a",   "dcr a",   "mvi a",   "cmc",
     /*4*/"mov b,b", "mov b,c", "mov b,d", "mov b,e", "mov b,h", "mov b,l",  "mov b,m", "mov b,a", "mov c,b", "mov c,c", "mov c,d", "mov c,e", "mov c,h", "mov c,l", "mov c,m", "mov c,a",
     /*5*/"mov d,b", "mov d,c", "mov d,d", "mov d,e", "mov d,h", "mov d,l",  "mov d,m", "mov d,a", "mov e,b", "mov e,c", "mov e,d", "mov e,e", "mov e,h", "mov e,l", "mov e,m", "mov e,a",
     /*6*/"mov h,b", "mov h,c", "mov h,d", "mov h,e", "mov h,h", "mov h,l",  "mov h,m", "mov h,a", "mov l,b", "mov l,c", "mov l,d", "mov l,e", "mov l,h", "mov l,l", "mov l,m", "mov l,a",
     /*7*/"mov m,b", "mov m,c", "mov m,d", "mov m,e", "mov m,h", "mov m,l",  "hlt",     "mov m,a", "mov a,b", "mov a,c", "mov a,d", "mov a,e", "mov a,h", "mov a,l", "mov a,m", "mov a,a",
     /*8*/"add b",   "add c",   "add d",   "add e",   "add h",   "add l",    "add m",   "add a",   "adc b",   "adc c",   "adc d",   "adc e",   "adc h",   "adc l",   "adc m",   "adc a",
     /*9*/"sub b",   "sub c",   "sub d",   "sub e",   "sub h",   "sub l",    "sub m",   "sbb a",   "sbb b",   "sbb c",   "sbb d",   "sbb e",   "sbb h",   "sbb l",   "sbb m",   "sbb a",
     /*a*/"ana b",   "ana c",   "ana d",   "ana e",   "ana h",   "ana l",    "ana m",   "ana a",   "xra b",   "xra c",   "xra d",   "xra e",   "xra h",   "xra l",   "xra m",   "xra a",
     /*b*/"ora b",   "ora c",   "ora d",   "ora e",   "ora h",   "ora l",    "ora m",   "ora a",   "cmp b",   "cmp c",   "cmp d",   "cmp e",   "cmp h",   "cmp l",   "cmp m",   "cmp a",
     /*c*/"rnz",     "pop b",   "jnz",     "jmp",     "cnz",     "push b",   "adi",     "rst 0",   "rz",      "ret",     "jz",      "illegal", "cz",      "call",    "aci",     "rst 1",
     /*d*/"rnc",     "pop d",   "jnc",     "out",     "cnc",     "push d",   "sui",     "rst 2",   "rc",      "illegal", "jc",      "in",      "cc",      "illegal", "sbi",     "rst 3",
     /*e*/"rpo",     "pop h",   "jpo",     "xthl",    "cpo",     "push h",   "ani",     "rst 4",   "rpe",     "pchl",    "jpe",     "xchg",    "cpe",     "illegal", "xri",     "rst 5",
     /*f*/"rp",      "poppsw",  "jp",      "di",      "cp",      "push psw", "ori",     "rst 6",   "cm",      "sphl",    "jm",      "ei",      "cm",      "illegal", "cpi",     "rst 7"
    };

    const char* mnemonics[100];

    if (letter_case == UPPER_CASE) {
        memcpy(mnemonics, mnemonics_upper_case, sizeof (mnemonics_upper_case));
    } else if (letter_case == LOWER_CASE) {
        memcpy(mnemonics, mnemonics_lower_case, sizeof (mnemonics_lower_case));
    }

    // const char** mnemonics = (const char**) malloc(sizeof (mnemonics_upper_case) * sizeof (const char));

    const int op_bytes[0x100] =
    {/*   0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F*/
     /*0*/1, 3, 1, 1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 2, 1,
     /*1*/1, 3, 1, 1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 2, 1,
     /*2*/1, 3, 3, 1, 1, 1, 2, 1, 1, 1, 3, 1, 1, 1, 2, 1,
     /*3*/1, 3, 3, 1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 2, 1,
     /*4*/1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
     /*5*/1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
     /*6*/1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
     /*7*/1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
     /*8*/1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
     /*9*/1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
     /*A*/1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
     /*B*/1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
     /*C*/1, 1, 3, 3, 3, 1, 1, 1, 1, 1, 3, 1, 3, 3, 2, 1,
     /*D*/1, 1, 3, 2, 3, 1, 1, 1, 1, 1, 3, 2, 3, 1, 2, 1,
     /*E*/1, 1, 3, 1, 3, 1, 2, 1, 1, 1, 3, 1, 3, 3, 2, 1,
     /*F*/1, 1, 3, 1, 3, 1, 2, 1, 1, 1, 3, 1, 3, 3, 2, 1
    };

//    FILE* output_file = fopen("output.txt", "w");
//
//    printf("Intel 8080 Disassembler by Luke Zimmerer\n");
//    printf("----------------------------------------\n");
//    printf("Filename: %s\n", argv[1]);
//    printf("Size: %lu bytes\n\n", file_size);

    int pc = 0, op;
//    char output_buf[OUTPUT_BUFFER_SIZE];

    while(pc < file_size)
    {
        op = input_buf[pc];

        if(mnemonics_only == 0) {
            if (addr_base == HEXADECIMAL) {
                printf("%04X: ", pc);
            } else if (addr_base == DECIMAL) {
                printf("%d: ", pc);
            }
        }

//        sprintf(output_buf, "%s", mnemonics[op]);

        if(mnemonics_only == 0) {
            if (op_bytes[op] == 1) {
                printf("%02X\t\t%s\n", op, mnemonics[op]);
            } else if (op_bytes[op] == 2) {
                printf("%02X %02X\t\t%s %02X\n", op, input_buf[pc + 1], 
                        mnemonics[op], input_buf[pc + 2]);
            } else if (op_bytes[op] == 3) {
                printf("%02X %02X %02X\t%s %04X\n", op, input_buf[pc + 1], 
                        input_buf[pc + 2], mnemonics[op], input_buf[pc + 1] | 
                        (input_buf[pc + 2] << 8));
            }
        } else {
            printf("%s\n", mnemonics[op]);
        }

        pc += op_bytes[op];
    }

    free(input_buf);
//    fclose(output_file);

    return EXIT_SUCCESS;
}
