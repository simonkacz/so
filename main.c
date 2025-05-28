#include "main.h"

// variables globales
t_config* config_file;
t_log* cpu_logger = NULL; // logger específico por cada CPU

bool interrupcion_pendiente = false;

// log rapido
void quick_log(char *msg)
{
    log_info(cpu_logger, "%s", msg);
}

// archivo de log para la CPU especifica 
char* get_logger_name(int id_cpu)
{
    char nombre_archivo[50];
    snprintf(nombre_archivo, sizeof(nombre_archivo), "cpu_%d.log", id_cpu );
    return nombre_archivo;
}

/*
t_log* crear_log_cpu(int id_cpu)
{
    char nombre_archivo[50];//preguntar el sabado 
    snprintf(nombre_archivo, sizeof(nombre_archivo), "cpu_%d.log", id_cpu);

    t_log* nuevo_logger = log_create(nombre_archivo, "CPU_INDIVIDUAL", 1, LOG_LEVEL_INFO);
    if (nuevo_logger == NULL)
    {
        log_error(logger, "error al crear el logger individual de CPU ID %d", id_cpu);
        return NULL;
    }

    return nuevo_logger;
}

*/



int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        printf("[ERROR] No se ha pasado el CPUID como parametro\nUso: ./cpu <CPUID>");
        return EXIT_FAILURE;
    }

    int res = 0;
    int id_cpu = atoi(argv[1]);

    c_modulo c_memoria = {-1, NULL, NULL};


    // creo el logger y el .config
    cpu_logger = log_create(get_logger_name(id_cpu), "CPU", 1, LOG_LEVEL_TRACE);
    if (cpu_logger == NULL)
    {
        printf("Ha ocurrido un error a la hora de inizializar el logger");
        
        return EXIT_FAILURE;
    }

    config_file = config_create("./cpu.config");
    if (config_file == NULL)
    {
        log_destroy(cpu_logger);
        log_error(cpu_logger, "algo salio mal a la hora de cargar");
        return EXIT_FAILURE;
    }

    // -------------------- conexion hacia Kernel --------------------
    k_load_socket(config_get_string_value(config_file, "IP_KERNEL"),
                  config_get_string_value(config_file, "PUERTO_KERNEL_DISPATCH"),
                  config_get_string_value(config_file, "PUERTO_KERNEL_INTERRUPT"));

    log_info(cpu_logger, "se ha iniciado la CPU ID:%d", id_cpu);

    // creo conexion a dispatch
    res = k_connect_dispatch();
    if (res == -1) {
    log_error(cpu_logger, "No se ha podido conectar a Dispatch del Kernel.");
    return EXIT_FAILURE;
    }

    // envio handshake con id_cpu a dispatch
    log_info(cpu_logger, "Enviando handshake a Kernel (DISPATCH) - ID: %d", id_cpu);
    k_send_dispatch(HANDSHAKE_CPU, id_cpu);
    log_info(cpu_logger, "Handshake DISPATCH de CPU %d enviado con éxito", id_cpu);


    // envio handshake con id_cpu a interrupt
    log_info(cpu_logger, "Enviando handshake a Kernel (INTERRUPT) - ID: %d", id_cpu);
    k_send_interrupt(HANDSHAKE_CPU, id_cpu);
    log_info(cpu_logger, "Handshake INTERRUPT de CPU %d enviado con exito", id_cpu);


    // creo conexion a interrupt
    res = k_connect_interrupt();
    if (res == -1)
    {
        log_error(cpu_logger, "no se pudo conectar al servidor INTERRUPT del Kernel");
        return EXIT_FAILURE;
    }

    
    iniciar_hilo_interrupt();

    // -------------------- conexion hacia memoria --------------------
    c_memoria.ip = config_get_string_value(config_file, "IP_MEMORIA");
    c_memoria.puerto = config_get_string_value(config_file, "PUERTO_MEMORIA");

    c_memoria.socket = crear_conexion(c_memoria.ip, c_memoria.puerto);
    enviar_mensaje("Pong", c_memoria.socket);

    log_info(cpu_logger, "mensaje enviado a memoria");

    // -------------------- manejo de interrupciones y ciclo de instruccion --------------------
    while (1)
    {
    // -------------------- SOLICITUD INTERRUPT --------------------
        solicitar_interrupt();  

    // -------------------- FETCH --------------------
        t_list* lista_inst = fetch(c_memoria.socket);

        if (!lista_inst) {
        log_error(cpu_logger, "Fallo en la fase FETCH.");
        break;
    }

    // -------------------- DECODE --------------------
        instruction inst = decode(lista_inst, c_memoria.socket);

    // -------------------- EXECUTE --------------------
        execute(inst, c_memoria.socket);
        
    //evito memory leak liberando la lista que me llegó
        list_destroy_and_destroy_elements(lista_inst, (void*)free);
    }
    // -------------------- finalizacion --------------------
    k_close();
    liberar_conexion(c_memoria.socket);

    log_destroy(cpu_logger);
    config_destroy(config_file);

    return 0;
}
