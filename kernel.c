#include "kernel.h"
#include <pthread.h>

c_modulo socket_dispatch = {-1, NULL, NULL};
c_modulo socket_interrupt = {-1, NULL, NULL};


pthread_t hilo_interrupt;
pthread_t hilo_dispatch;
bool cpu_ocupada = false;

void k_load_socket(char* ip_kernel, char* dispatch_port, char* interrupt_port)
{
    socket_dispatch.ip = ip_kernel;
    socket_dispatch.puerto = dispatch_port;
    
    socket_interrupt.ip = ip_kernel;
    socket_interrupt.puerto = interrupt_port;
}

void k_send_dispatch(kernel_msg type, int id_cpu)
{
    switch (type)
    {  
        case HANDSHAKE_CPU: 
        {   
            char mensaje_id[10];
            snprintf(mensaje_id, sizeof(mensaje_id), "%d", id_cpu);
            enviar_mensaje(mensaje_id, socket_dispatch.socket);  
            break;
        }

        case SOLICITAR_INTERRUPCION:
        {
            log_info(cpu_logger, "[DISPATCH] Notificando interrupción para CPU ID: %d", id_cpu);

            t_paquete* paquete = crear_paquete();
            agregar_a_paquete(paquete, &id_cpu, sizeof(int));
            paquete->codigo_operacion = SOLICITAR_INTERRUPCION;

            enviar_paquete(paquete, socket_dispatch.socket);

            eliminar_paquete(paquete);

            break;
        }

        default:
            break;
    }
}

void k_send_interrupt(kernel_msg type, int id_cpu)
{
    switch (type)
    {
        case HANDSHAKE_CPU:
            enviar_mensaje("Conectado exitosamente a kernel (INTERRUPT)", socket_interrupt.socket);  
            break;
        
        default:
            break;
    }
}

int k_connect_dispatch() 
{
    socket_dispatch.socket = crear_conexion(socket_dispatch.ip, socket_dispatch.puerto);
    return socket_dispatch.socket;
}

int k_connect_interrupt() 
{
    socket_interrupt.socket = crear_conexion(socket_interrupt.ip, socket_interrupt.puerto);
    return socket_interrupt.socket;    
}

void k_close()
{
    if (socket_dispatch.socket != -1)
        close(socket_dispatch.socket);
    if (socket_interrupt.socket != -1)
        close(socket_interrupt.socket);
}

void iniciar_hilo_dispatch() {
    pthread_create(&hilo_dispatch, NULL, listen_dispatch, NULL);
    pthread_detach(hilo_dispatch);
}

void* listen_dispatch(void *arg) {
    while (1) {
        t_paquete* paquete = recibir_paquete(socket_dispatch.socket);

        if (paquete == NULL) {
            log_error(cpu_logger, "[DISPATCH] Se perdió la conexión o se cerró el socket.");
            break;
        }

        switch (paquete->codigo_operacion) {
            case ENVIAR_PROC_CPU:
                if (!cpu_ocupada) {
                    log_info(cpu_logger, "## Recibiendo proceso del Kernel...");

                    uint32_t pid_recibido;
                    uint32_t pc_recibido;

                    memcpy(&pid_recibido, paquete->buffer->stream + sizeof(int), sizeof(uint32_t));
                    memcpy(&pc_recibido, paquete->buffer->stream + sizeof(int), sizeof(uint32_t));

                    log_info(cpu_logger, "## PID: %d - PC: %d", pid_recibido, pc_recibido);

                    cpu_ocupada = true; 

                    load_program(pid_recibido, pc_recibido);

                } else {
                    log_warning(cpu_logger, "[DISPATCH] La CPU está ocupada, se ignora el paquete.");
                }
                break;

            default:
                log_warning(cpu_logger, "[DISPATCH] Código de operación desconocido: %d", paquete->codigo_operacion);
                break;
        }

        eliminar_paquete(paquete);
    }

    return NULL;
}



void iniciar_hilo_interrupt()
{
    pthread_create(&hilo_interrupt, NULL, listen_interrupt, NULL);
    pthread_detach(hilo_interrupt);
}

void* listen_interrupt(void *arg)
{
    while (1)
    {
        t_paquete* paquete = recibir_paquete(socket_interrupt.socket);
        
        if (paquete == NULL) {
            log_error(cpu_logger, "[INTERRUPT] Se perdió la conexión o se cerró el socket.");
            break;
        }

        if (paquete->codigo_operacion == INTERRUPCION_KERNEL) {
            log_info(cpu_logger, "## Llega interrupción al puerto Interrupt");//log obligatorio
            interrupcion_pendiente = true;
        } else {
            log_warning(cpu_logger, "[INTERRUPT] Código de operación desconocido: %d", paquete->codigo_operacion);
        }

        eliminar_paquete(paquete);
    }

    return NULL;
}

void solicitar_interrupt()
{
    if (interrupcion_pendiente) {
        log_info(cpu_logger, "## Interrupción detectada para PID: %d en PC: %p", pid, (void*)pc);

        t_paquete* paquete = crear_paquete();
        agregar_a_paquete(paquete, &pid, sizeof(int));
        agregar_a_paquete(paquete, &pc, sizeof(instruction*));

        paquete->codigo_operacion = SOLICITAR_INTERRUPCION;

        enviar_paquete(paquete, socket_dispatch.socket);

        log_info(cpu_logger, "## Notificación de interrupción enviada al Kernel (PID: %d, PC: %p)", pid, (void*)pc);

        eliminar_paquete(paquete);

        interrupcion_pendiente = false;
    }
}
