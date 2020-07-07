#ifndef CONFIG_BROKER_CONFIG_H_
#define CONFIG_BROKER_CONFIG_H_

#include <stdlib.h>
#include <commons/config.h>
#include <commons/string.h>

#include "../../../shared-common/common/config.h"

#include "../logger/broker_logger.h"

#define CONFIG_FILE_PATH "broker.config"

#define BUDDY_SYSTEM "BUDDY SYSTEM"
#define PARTICION_DINAMICA "PARTICION DINAMICA"
#define FIFO_STRING "FIFO"
#define LRU_STRING "LRU"
#define FIRST_FIT "FIRST FIT"
#define BEST_FIT "BEST FIT"

typedef enum
{
	BS, PD
} e_memory_struct;

typedef enum
{
	FIFO, LRU
} e_algoritmo_reemplazo;

typedef enum
{
	FF, BF
} e_algoritmo_particion_libre;

typedef struct
{
	int tamano_memoria;
	int tamano_minimo_particion;
	e_memory_struct estrategia_memoria;
	e_algoritmo_reemplazo algoritmo_reemplazo;
	e_algoritmo_particion_libre algoritmo_particion_libre;
	char* ip_broker;
	int puerto_broker;
	int frecuencia_compactacion;
	char* log_file;
} t_broker_config;

t_broker_config* broker_config;

int broker_config_load();
void broker_config_free();
void broker_print_config();

#endif /* CONFIG_BROKER_CONFIG_H_ */
