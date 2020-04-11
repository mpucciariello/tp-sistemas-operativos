#ifndef CONFIG_TEAM_CONFIG_H_
#define CONFIG_TEAM_CONFIG_H_

#include <stdlib.h>
#include <commons/config.h>
#include <commons/string.h>

#include "../logger/team_logger.h"

#include "../../../shared-common/common/config.h"
#include "../../../shared-common/common/utils.h"

#define CONFIG_FILE_PATH "team.config"
#define FIFO_STRING "FIFO";
#define RR_STRING "ROUND ROBIN";
#define SJF_CD_STRING "SHORTEST JOB FIRST CON DESALOJO";
#define SJF_SD_STRING "SHORTEST JOB FIRST SIN DESALOJO";

typedef enum
{
	FIFO, RR, SJF_CD, SJF_SD
} e_algoritmo_planificacion;

typedef struct
{
	char** posiciones_entrenadores;
	char** pokemon_entrenadores;
	char** objetivos_entrenadores;
	int tiempo_reconexion;
	int retardo_ciclo_cpu;
	e_algoritmo_planificacion algoritmo_planificacion;
	int quantum;
	int estimacion_inicial;
	char* ip_broker;
	int puerto_broker;
	char* log_file;
} t_team_config;

t_team_config* team_config;

int team_config_load();
void team_config_free();
void team_print_config();

#endif /* CONFIG_TEAM_CONFIG_H_ */
