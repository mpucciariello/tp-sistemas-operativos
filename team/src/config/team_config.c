#include "team_config.h"

void read_config(t_config* config_file);

int team_config_load() {
	return config_load(NULL, CONFIG_FILE_PATH, read_config, team_print_config);
}

void team_config_free() {
	utils_free_array(team_config->posiciones_entrenadores);
	utils_free_array(team_config->pokemon_entrenadores);
	utils_free_array(team_config->objetivos_entrenadores);
	free(team_config->ip_broker);
	free(team_config->log_file);
	free(team_config);
}

e_algoritmo_planificacion team_algoritmo_planificacion_from_string(char* algoritmo) {
	if (string_equals_ignore_case(algoritmo, "fifo")) {
		return FIFO;
	} else if (string_equals_ignore_case(algoritmo, "rr")) {
		return RR;
	} else if (string_equals_ignore_case(algoritmo, "sjf-cd")) {
		return SJF_CD;
	} else if (string_equals_ignore_case(algoritmo, "sjf-sd")) {
		return SJF_SD;
	} else {
		return -1;
	}
}

void read_config(t_config* config_file) {
	team_config = malloc(sizeof(t_team_config));
	team_config->posiciones_entrenadores = config_get_array_value(config_file, "POSICIONES_ENTRENADORES");
	team_config->pokemon_entrenadores = config_get_array_value(config_file, "POKEMON_ENTRENADORES");
	team_config->objetivos_entrenadores = config_get_array_value(config_file, "OBJETIVOS_ENTRENADORES");
	team_config->tiempo_reconexion = config_get_int_value(config_file, "TIEMPO_RECONEXION");
	team_config->retardo_ciclo_cpu = config_get_int_value(config_file, "RETARDO_CICLO_CPU");
	char* algoritmo = string_duplicate(config_get_string_value(config_file, "ALGORITMO_PLANIFICACION"));
	team_config->algoritmo_planificacion = team_algoritmo_planificacion_from_string(algoritmo);
	team_config->quantum = config_get_int_value(config_file, "QUANTUM");
	team_config->alpha = config_get_double_value(config_file, "ALPHA");
	team_config->estimacion_inicial = config_get_int_value(config_file, "ESTIMACION_INICIAL");
	team_config->ip_broker = string_duplicate(config_get_string_value(config_file, "IP_BROKER"));
	team_config->puerto_broker = config_get_int_value(config_file, "PUERTO_BROKER");
	team_config->ip_team = string_duplicate(config_get_string_value(config_file, "IP_TEAM"));
	team_config->puerto_team = config_get_int_value(config_file, "PUERTO_TEAM");
	team_config->log_file = malloc(sizeof(char*));
	team_config->log_file = string_duplicate(config_get_string_value(config_file, "LOG_FILE"));
}

char* team_algoritmo_planificacion_to_string(e_algoritmo_planificacion algoritmo) {
	switch (algoritmo) {
		case FIFO:
			return FIFO_STRING;
			break;
		case RR:
			return RR_STRING;
			break;
		case SJF_CD:
			return SJF_CD_STRING;
			break;
		case SJF_SD:
			return SJF_SD_STRING;
			break;
		default:
			return "";
			break;
	}
}

void team_print_config() {
	team_logger_info("POSICIONES_ENTRENADORES: %s", utils_array_to_string(team_config->posiciones_entrenadores));
	team_logger_info("POKEMON_ENTRENADORES: %s", utils_array_to_string(team_config->pokemon_entrenadores));
	team_logger_info("OBJETIVOS_ENTRENADORES: %s", utils_array_to_string(team_config->objetivos_entrenadores));
	team_logger_info("TIEMPO_RECONEXION: %d", team_config->tiempo_reconexion);
	team_logger_info("RETARDO_CICLO_CPU: %d", team_config->retardo_ciclo_cpu);
	team_logger_info("ALGORITMO_PLANIFICACION: %s", team_algoritmo_planificacion_to_string(team_config->algoritmo_planificacion));
	team_logger_info("QUANTUM: %d", team_config->quantum);
	team_logger_info("ALPHA: %f", team_config->alpha);
	team_logger_info("ESTIMACION_INICIAL: %d", team_config->estimacion_inicial);
	team_logger_info("IP_BROKER: %s", team_config->ip_broker);
	team_logger_info("PUERTO_BROKER: %d", team_config->puerto_broker);
	team_logger_info("LOG_FILE: %s", team_config->log_file);
	team_logger_info("IP_TEAM: %s", team_config->ip_team);
	team_logger_info("PUERTO_TEAM: %d", team_config->puerto_team);
}
