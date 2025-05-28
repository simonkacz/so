#include "execute.h"

void op_noop() {
    log_info(cpu_logger, "[EXECUTE] NOOP ejecutado correctamente");
}


void op_write(int direccion, char* datos, int socket_memoria) {
    log_info(cpu_logger, "PID: ,%d - Acción: ESCRIBIR - Dirección Física: %d - Valor: %s",pid,direccion,datos);//log obligatorio
    
    t_paquete* paquete = crear_paquete();
    
    agregar_a_paquete(paquete, &direccion, sizeof(int)); 
    agregar_a_paquete(paquete, datos, strlen(datos) + 1); 

    paquete->codigo_operacion = OP_WRITE;

    enviar_paquete(paquete, socket_memoria);
}

void op_read(int direccion, int tamanio, int socket_memoria) {
    log_info(cpu_logger, "PID: %d - Acción: LEER - Dirección Física: %d - Tamaño: %d", pid, direccion, tamanio);//log obligatorio

    t_paquete* paquete = crear_paquete();
    paquete->codigo_operacion = OP_READ;

    // pone la dir y tamaño de lo que quiere buscar CPU
    agregar_a_paquete(paquete, &direccion, sizeof(int));
    agregar_a_paquete(paquete, &tamanio, sizeof(int));

    enviar_paquete(paquete, socket_memoria);
    
    t_paquete* respuesta = recibir_paquete(socket_memoria);
    
    if (respuesta->codigo_operacion == RESPUESTA_READ) {
        char* datos = (char*)respuesta->buffer;
        log_info(cpu_logger, "[EXECUTE] Datos leídos: %s", datos);
    }
   

    eliminar_paquete(respuesta);
    eliminar_paquete(paquete);
}



void op_goto(int valor) {
    log_info(cpu_logger, "[EXECUTE] GOTO a PC: %d", valor);
    pc = valor;
}


//----------SYSCALLS----------

void syscall_io(int dispositivo, int tiempo) {
    log_info(cpu_logger, "PID: %d - Syscall IO - Dispositivo: %d - Tiempo: %d", pid, dispositivo, tiempo);

    t_paquete* paquete = crear_paquete();
    paquete->codigo_operacion = SYSCALL_IO;

    agregar_a_paquete(paquete, &pid, sizeof(int));
    agregar_a_paquete(paquete, &dispositivo, sizeof(int));
    agregar_a_paquete(paquete, &tiempo, sizeof(int));

    enviar_paquete(paquete, socket_dispatch.socket);

    log_info(cpu_logger, "Syscall IO enviada al Kernel (PID: %d, Dispositivo: %d, Tiempo: %d)", pid, dispositivo, tiempo);

    eliminar_paquete(paquete);
}

void syscall_init_proc(char* archivo_instrucciones, int tamanio) {
    log_info(cpu_logger, "Syscall INIT_PROC - Tamaño: %d", tamanio);

    t_paquete* paquete = crear_paquete();
    paquete->codigo_operacion = INIT_PROC;

    agregar_a_paquete(paquete, archivo_instrucciones, strlen(archivo_instrucciones) + 1);
    agregar_a_paquete(paquete, &tamanio, sizeof(int));

    enviar_paquete(paquete, socket_dispatch.socket);

    log_info(cpu_logger, "Syscall INIT_PROC enviada al Kernel (Tamaño: %d)", tamanio);

    eliminar_paquete(paquete);
}


void syscall_dump_memory() {
    log_info(cpu_logger, "PID: %d - Syscall DUMP_MEMORY - Solicitando volcado de memoria", pid);

    t_paquete* paquete = crear_paquete();
    paquete->codigo_operacion = SYSCALL_DUMP_MEMORY;

    agregar_a_paquete(paquete, &pid, sizeof(int));
    enviar_paquete(paquete, socket_dispatch.socket);

    log_info(cpu_logger, "Syscall DUMP_MEMORY enviada al Kernel (PID: %d)", pid);

    eliminar_paquete(paquete);
}

void syscall_exit() {
    log_info(cpu_logger, "PID: %d - Syscall EXIT - Terminando el proceso", pid);

    t_paquete* paquete = crear_paquete();
    paquete->codigo_operacion = SYSCALL_EXIT;

    agregar_a_paquete(paquete, &pid, sizeof(int));
    enviar_paquete(paquete, socket_dispatch.socket);

    log_info(cpu_logger, "Syscall EXIT enviada al Kernel (PID: %d)", pid);

    eliminar_paquete(paquete);
    
    exit(0);
}


