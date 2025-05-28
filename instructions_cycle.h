#ifndef INSTRUCTION_CYCLE_H
#define INSTRUCTION_CYCLE_H

#include <stdint.h>
#include <utils/sockets.h>
#include <mmu.h>


extern t_log* cpu_logger;

typedef enum op_code
{
    NOOP = 0x0,
    WRITE = 0x1, 
    READ = 0x2,
    GOTO = 0x3,

    SYSCALL_IO = 0x10,
    SYSCALL_INIT_PROC = 0x11,
    SYSCALL_DUMP_MEMORY = 0x12,
    SYSCALL_EXIT = 0x13,
};


typedef struct {
    int op;
    int value;
    int dispositivo;
    int tiempo;
    char* archivo_instrucciones;
    int tamanio;
} instruction;


// funciones
void load_program(int ppid, const instruction* ppc);

t_list* fetch(int socket_memoria);
instruction decode (t_list* data, int socket_memoria);
void execute(instruction ins, int socket_memoria);

#endif
