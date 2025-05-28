#ifndef MMU_H
#define MMU_H

#include "utils/general.h"
#include "utils/sockets.h"     
#include <math.h>
#include <stdlib.h>
#include <stdio.h>


extern t_log* cpu_logger;
extern c_modulo c_memoria;
extern int pid;


typedef struct {
    int nro_pagina;
    int marco;
} t_entrada_tlb;

void mmu_init(t_config* config);
int traducir_direccion(int direccion_logica, int tamanio_pagina);

int calcular_nro_pagina(int direccion_logica, int tamanio_pagina);
int calcular_desplazamiento(int direccion_logica, int tamanio_pagina);


#endif /*MMU_H*/