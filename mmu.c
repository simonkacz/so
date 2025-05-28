#include "mmu.h"

t_list* tlb;
pthread_mutex_t mutex_tlb;
int entradas_tlb;
char* reemplazo_tlb;


void mmu_init(t_config* config) {
    entradas_tlb = config_get_int_value(config, "ENTRADAS_TLB");
    reemplazo_tlb = config_get_string_value(config, "REEMPLAZO_TLB");
    tlb = list_create();
    pthread_mutex_init(&mutex_tlb, NULL);
}

int calcular_nro_pagina(int direccion_logica, int tamanio_pagina) {
    return direccion_logica / tamanio_pagina;
}

int calcular_desplazamiento(int direccion_logica, int tamanio_pagina) {
    return direccion_logica % tamanio_pagina;
}

int buscar_en_tlb(int nro_pagina) {
    pthread_mutex_lock(&mutex_tlb);

    for (int i = 0; i < list_size(tlb); ++i) {
        t_entrada_tlb* entrada = list_get(tlb, i);
        if (entrada->nro_pagina == nro_pagina) {
        
            log_info(cpu_logger, "## PID: %d - TLB HIT - Pagina: %d", pid, nro_pagina);//log obligatorio
            pthread_mutex_unlock(&mutex_tlb);
            return entrada->marco;
        }
    }

    
    log_info(cpu_logger, "## PID: %d - TLB MISS - Pagina: %d", pid, nro_pagina);//log obligatorio

    pthread_mutex_unlock(&mutex_tlb);
    return -1; 
}

void actualizar_tlb(int nro_pagina, int marco) {
    pthread_mutex_lock(&mutex_tlb);

    if (list_size(tlb) >= entradas_tlb) {
        if (strcmp(reemplazo_tlb, "LRU") == 0) {
            list_remove_and_destroy_element(tlb, 0, free); // saca el más viejo
        }
    }

    t_entrada_tlb* nueva_entrada = malloc(sizeof(t_entrada_tlb));
    nueva_entrada->nro_pagina = nro_pagina;
    nueva_entrada->marco = marco;
    list_add(tlb, nueva_entrada);

    pthread_mutex_unlock(&mutex_tlb);
}

int traducir_direccion(int direccion_logica, int tamanio_pagina) {
    int nro_pagina = calcular_nro_pagina(direccion_logica, tamanio_pagina);
    int desplazamiento = calcular_desplazamiento(direccion_logica, tamanio_pagina);

    int marco = buscar_en_tlb(nro_pagina);
    if (marco == -1) {
        // si no está en la TLB, le pido a memoria (implementar!)
        // marco = solicitar_a_memoria(nro_pagina);
        actualizar_tlb(nro_pagina, marco);
    }

    return (marco * tamanio_pagina) + desplazamiento;
}
