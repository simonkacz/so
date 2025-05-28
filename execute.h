#ifndef EXECUTE
#define EXECUTE

#include <instructions_cycle.h>
#include <stdint.h>
#include <utils/sockets.h>
#include <utils/general.h>


extern int pid;
extern int pc;

extern t_log* cpu_logger;
extern c_modulo socket_dispatch;


//-----OPERACIONES------
void op_noop();
void op_write(int direccion, char* datos, int socket_memoria);
void op_read(int direccion, int tamanio, int socket_memoria);
void op_goto(int valor);

//-----SYSCALLS------
void syscall_io(int dispositivo, int tiempo);
void syscall_init_proc(char* archivo_instrucciones, int tamanio);
void syscall_dump_memory();
void syscall_exit();



#endif /*EXECUTE*/