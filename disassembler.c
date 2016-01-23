/*
 *  Intel 8080 disassembler. Luke Zimmerer, 2016.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define OP(x)                  (buf[pc + x])
#define MNEM(x)                (mnemonics[buf[pc + x]])
#define SIZE_OF_OPCODE(x)      (opcode_bytes[buf[x]])
#define ONE_BYTE               1
#define TWO_BYTES              2
#define THREE_BYTES            3
#define CORRECT_NUMBER_OF_ARGS 2
#define ERROR                  0

static unsigned get_file_size(const char *filename);
static char*    read_file(const char *filename, const unsigned file_size);
static void     disassemble(const char *buf, const unsigned file_size);

int main(int argc, char* argv[])
{
    if (argc != CORRECT_NUMBER_OF_ARGS) {
        fprintf(stderr, "usage: %s filename\n", argv[0]);
        return EXIT_FAILURE;
    }

    const char *filename = argv[1];
    assert(filename);
    const int file_size = get_file_size(filename);
    if (!file_size) {
        fprintf(stderr, "Error: cannot open file or file is 0 bytes\n");
        return EXIT_FAILURE;
    }

    char *file_buf = read_file(filename, file_size);
    if (!file_buf) {
        fprintf(stderr, "Error: cannot read file into buffer\n");
        return EXIT_FAILURE;
    }

    disassemble(file_buf, file_size);

    free(file_buf);
    return EXIT_SUCCESS;
}

// Return size of file in bytes
static unsigned get_file_size(const char *filename)
{
    assert(filename);

    FILE *input_file = fopen(filename, "rb");
    if (!input_file) {
        return ERROR;
    }

    fseek(input_file, 0, SEEK_END);
    const unsigned file_size = ftell(input_file);
    fclose(input_file);

    return file_size;
}

// Read file into buffer and return buffer
static char* read_file(const char *filename, const unsigned file_size)
{
    assert(filename);
    assert(file_size > 0);

    FILE *input_file = fopen(filename, "rb");
    assert(input_file);
    rewind(input_file);

    char *file_buf = calloc(file_size, sizeof(*file_buf));
    if (!file_buf) {
        fclose(input_file);
        return ERROR;
    }

    const unsigned bytes_read = fread(file_buf, file_size, 1, input_file);
    fclose(input_file);

    /*
    if (bytes_read != file_size) {
        free(file_buf);
        return ERROR;
    }
    */

    return file_buf;
}

// Disassemble 8080 opcodes and print to stdout
static void disassemble(const char *buf, const unsigned file_size)
{
    assert(buf);
    assert(file_size > 0);

    // Mnemonics for all 8080 opcodes
    static const char *mnemonics[0x100] =
    {/*   0          1          2          3          4          5           6          7          8          9          A          b          c          d          e          f       */
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

    // Size (in bytes) of every 8080 opcode
    static const int opcode_bytes[0x100] =
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

    // Disassembly loop
    static unsigned pc;
    for (pc = 0; pc < file_size; pc += SIZE_OF_OPCODE(pc)) {
        switch (SIZE_OF_OPCODE(pc)) {
            case ONE_BYTE:
                printf("%02X\t\t%s\n", OP(0), MNEM(0));
                break;
            case TWO_BYTES:
                printf("%02X %02X\t\t%s %02X\n", OP(0), OP(1), MNEM(0), OP(2));
                break;
            case THREE_BYTES:
                printf("%02X %02X %02X\t%s %04X\n", OP(0), OP(1),
                    OP(2), MNEM(0), OP(1) | (OP(2) << 8));
                break;
            default:
                break;
        }
    }
}
