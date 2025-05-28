#ifndef KERNEL_H
#define KERNEL_H

#include <utils/sockets.h>
#include <utils/general.h>
#include <instructions_cycle.h>

extern int pid;
extern int pc;
extern t_log* cpu_logger;
extern bool interrupcion_pendiente;


typedef enum {
    HANDSHAKE_CPU = 1,
    INTERRUPCION_KERNEL = 2,
} kernel_msg;


void k_load_socket (char* ip_kernel, char* dispatch_port, char* interrupt_port);
void k_send_dispatch (kernel_msg type,int id_cpu);
void k_send_interrupt (kernel_msg type, int id_cpu);

int k_connect_dispatch();

int k_connect_interrupt();

void k_close();

void iniciar_hilo_dispatch();

void* listen_dispatch (void *arg);

void iniciar_hilo_interrupt();

void* listen_interrupt(void *arg);

void solicitar_interrupt();



#endif // KERNEL_H