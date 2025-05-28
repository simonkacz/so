#include "instructions_cycle.h"

const instruction* pc = 0;
int pid = 0;

void load_program(int ppid, const instruction* ppc)
{
    pc = ppc;
    pid = ppid;

    log_info(cpu_logger, "## PID: %d - FETCH - Program Counter: %x", pid, (unsigned int) pc);//log obligatorio
}


t_list* fetch(int socket_memoria) {
    
    t_paquete* paquete = crear_paquete();

    agregar_a_paquete(paquete, &pid, sizeof(int));
    agregar_a_paquete(paquete, &pc, sizeof(instruction));
    paquete->codigo_operacion = SOLICITAR_INSTRUCCION;

    
    enviar_paquete(paquete, socket_memoria);
    eliminar_paquete(paquete);

    
    return recibir_paquete(socket_memoria);
}

instruction decode (t_list* data, int socket_memoria)
{
    
instruction ins = *((instruction*) list_get(data, 0));

    log_trace(cpu_logger, "## PID: %d - Decode - Instruction: %d", pid, ins.op);

    if (ins.op == WRITE || ins.op == READ || ins.op == GOTO) {
        // si necesitamos acceder a memoria, traducimos
        int direccion_logica = ins.value;
        int direccion_fisica = traducir_direccion(direccion_logica, socket_memoria);

        // reemplazo logica por fisica
        ins.value = direccion_fisica;

        log_trace(cpu_logger, "## PID: %d - Decode - Dirección lógica %d traducida a %d", pid, direccion_logica, direccion_fisica);
    } else {
        // si no requiere acceder a memoria, no traducimos
        log_trace(cpu_logger, "## PID: %d - Decode - Instrucción no requiere traducción de dirección", pid);
    }

    return ins;

}

void execute(instruction ins, int socket_memoria)
{   
    log_info(cpu_logger, "## PID: %d - Ejecutando: %d - Parametro: %d", pid, ins.op, ins.value);//log obligatorio

    switch (ins.op)
    {
    //----------OPERACIONES----------
        case NOOP:
            op_noop();
            pc++;
            break;
        case WRITE:
            op_write(ins.value,socket_memoria);
            pc++;
            break;
        case READ:
            op_read(ins.value,socket_memoria);
            pc++;
            break;
        case GOTO:
            op_goto(ins.value);
            pc = (instruction*) ins.value;//me quedo con el pc que viene del GOTO
            return; 
            break;

    //----------SYSCALLS----------
        case SYSCALL_IO:
        syscall_io(ins.dispositivo, ins.tiempo); 
            break;
        case SYSCALL_INIT_PROC:
        syscall_init_proc(ins.archivo_instrucciones, ins.tamanio);             
        break;
        case SYSCALL_DUMP_MEMORY:
            syscall_dump_memory();
            pc++;
            break;
        case SYSCALL_EXIT:
            syscall_exit();
            return;
            break;
        default:
            log_error(cpu_logger, "[ERROR] Instruction not found OPCODE:%x PID: %d PC:%x", (int)ins.op, pid, (unsigned int)pc);
            break;              
    }
}